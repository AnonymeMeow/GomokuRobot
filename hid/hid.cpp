#include "hid.hpp"

#include <hidapi.h>
#include <thread>
#include <condition_variable>
#include <cstring>

#include "../config.hpp"

namespace hid
{
    Logger logger("HID");

    hid_device *device;

    std::mutex act_lock;
    std::condition_variable act_cond;
    bool is_action_done = true;
    std::mutex key_lock;
    std::condition_variable key_cond;
    bool active = false;

    Report::Report(REPORT_TYPE type): type(type)
    {}

    Report::Report(const char str[]): type(STRING)
    {
        if (strlen(str) > 55)
        {
            logger.warn("String too long for report, truncated to 55 bytes.");
            memcpy(data.string, str, 55);
            data.string[55] = '\0';
            return;
        }
        strcpy(data.string, str);
    }

    Report::Report(float x, float y): type(XY_POS)
    {
        data.xy_pos.x = x;
        data.xy_pos.y = y;
    }

    Report::Report(float z): type(Z_POS)
    {
        data.z_pos = z;
    }

    Report::operator string() const
    {
        switch (type)
        {
        case STRING:
            return format("string: {}", data.string);
        case XY_POS:
            return format("XY position: {}, {}", data.xy_pos.x, data.xy_pos.y);
        case PUMP:
            return format("Toggle pump");
        case Z_POS:
            return format("Z position: {}", data.z_pos);
        case ACT_DONE:
            return format("Action done");
        case KEY_DOWN:
            return format("Key pressed");
        default:
            return format("Error report type: {}", (int)type);
        }
    }

    void send(const Report& report)
    {
        unsigned char buf[64]{};
        memcpy(buf + 1, &report, 60);
        int bytes = hid_write(device, buf, 64);
        if (bytes == -1)
        {
            logger.error("Failed to send hid report.");
            logger.error("Reason: {}", hid_error(device));
            return;
        }
        switch (report.type)
        {
        case XY_POS:
        case PUMP:
        case Z_POS:
            act_lock.lock();
            is_action_done = false;
            act_lock.unlock();
        default:
            break;
        }
        logger.trace("{} bytes sent: {}", bytes, (string)report);
    }

    void receiver_loop()
    {
        unsigned char buf[64];
        while (true)
        {
            memset(buf, 0, 64);
            int bytes = hid_read(device, buf, 64);
            if (!active)
            {
                logger.trace("Deamon receiver exits.");
                return;
            }
            if (bytes == -1)
            {
                logger.error("Error occured when reading from the hid device: {}", hid_error(device));
                exit();
                continue;
            }
            Report* report = (Report*)buf;
            logger.trace("{} bytes received: {}", bytes, (string)*report);

            if (report->type == ACT_DONE)
            {
                act_lock.lock();
                is_action_done = true;
                act_lock.unlock();
                act_cond.notify_all();
            }
            else if (report->type == KEY_DOWN)
            {
                key_cond.notify_all();
            }
        }
    }

    void wait_for_action_done()
    {
        auto lock = std::unique_lock(act_lock);
        if (!is_action_done)
        {
            act_cond.wait(lock);
        }
    }

    void wait_for_next_key()
    {
        auto lock = std::unique_lock(key_lock);
        key_cond.wait(lock);
    }

    bool init()
    {
        logger.info("Trying to open device {}...", config::hid_device_name.c_str());
        hid_init();
        hid_device_info* p_devices = hid_enumerate(0, 0);
        if (p_devices == NULL)
        {
            logger.error("Failed to get hid device list.");
            logger.error("Reason: {}", hid_error(NULL));
            return false;
        }
        hid_device_info* p_device = p_devices;
        while (p_device)
        {
            logger.trace("HID device: {}", p_device->product_string);
            if (config::hid_device_name.compare(p_device->product_string) == 0)
            {
                device = hid_open(p_device->vendor_id, p_device->product_id, p_device->serial_number);
                if (!device)
                {
                    logger.error("Failed to open device: {}.", p_device->product_string);
                    logger.error("Reason: {}", hid_error(device));
                    hid_free_enumeration(p_devices);
                    return false;
                }
                logger.info("Succeeded!");
                hid_free_enumeration(p_devices);
                active = true;
                std::thread(receiver_loop).detach();
                return true;
            }
            p_device = p_device->next;
        }
        logger.error("HID device {} not found.", config::hid_device_name.c_str());
        hid_free_enumeration(p_devices);
        return false;
    }

    void exit()
    {
        active = false;
        hid_close(device);
        hid_exit();
    }
}
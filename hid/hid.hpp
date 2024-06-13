#pragma once

#include "../logger.hpp"

namespace hid
{
    extern Logger logger;

    enum REPORT_TYPE
    {
        STRING,
        XY_POS,
        PUMP,
        Z_POS,
        ACT_DONE,
        KEY_DOWN,
    };

    struct Report
    {
        REPORT_TYPE type;
        union
        {
            char string[56];
            struct
            {
                float x, y;
            } xy_pos;
            float z_pos;
        } data;

        Report() = default;

        Report(REPORT_TYPE type);

        Report(const char str[]);

        Report(float x, float y);

        Report(float z);

        operator string() const;
    };

    bool init();

    void exit();

    void send(const Report&);

    void wait_for_action_done();

    void wait_for_next_key();

    void interrupt_key_wait();
}
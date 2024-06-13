#include "ai/gomokuai.hpp"
#include "opencv/opencv.hpp"
#include "hid/hid.hpp"

#include <thread>
#include <fstream>

static Logger logger("main");

float kx, ky, bx, by;
gomokuai::PIECE_TYPE ai_type = gomokuai::BLACK;
bool is_playing = false;

gomokuai::Coord_2D stone_fetch_point{7, 11};
gomokuai::Coord_2D idle_point{5, -4};

const char tmp_file_name[] = "coefs.tmp";

hid::Report convert(gomokuai::Coord_2D point)
{
    return {kx * point.row + bx, ky * point.col + by};
}

void func()
{
    int count = 0;
    if (ai_type == gomokuai::WHITE)
    {
        count = 1;
        hid::wait_for_next_key();
    }
    while (is_playing)
    {
        auto pos = opencv::get_ai_step(count);
        logger.trace("AI point: {}, {}.", pos.row, pos.col);
        hid::send(convert(stone_fetch_point));
        hid::wait_for_action_done();
        hid::send(hid::PUMP);
        hid::wait_for_action_done();
        hid::send(convert(pos));
        hid::wait_for_action_done();
        hid::send(hid::PUMP);
        hid::wait_for_action_done();
        hid::send(convert(idle_point));
        hid::wait_for_action_done();
        count += 2;
        hid::wait_for_next_key();
    }
}

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "aitest") == 0)
    {
        opencv::test();
        return 0;
    }
    if (!hid::init())
    {
        logger.error("Error occured, exiting.");
        hid::exit();
        return -1;
    }
    if (!opencv::init())
    {
        logger.error("Error occured, exiting.");
        opencv::exit();
        hid::exit();
        return -1;
    }
    gomokuai::init();
    std::vector<std::pair<std::pair<float, float>, std::pair<int, int>>> m;
    std::pair<float, float> tmp, p;
    std::thread player;
    while (true)
    {
        char command;
        float x, y;
        std::cin >> command;
        if (is_playing)
        {
            if (command == 's')
            {
                is_playing = false;
                hid::interrupt_key_wait();
                player.join();
            }
        }
        else if (command == 'g')
        {
            std::cin >> x >> y;
            hid::send({x, y});
            p = {x, y};
            hid::wait_for_action_done();
        }
        else if (command == 'x')
        {
            std::cin >> x;
            x = p.first + x;
            y = p.second;
            hid::send({x, y});
            p = {x, y};
            hid::wait_for_action_done();
        }
        else if (command == 'y')
        {
            std::cin >> y;
            x = p.first;
            y = p.second + y;
            hid::send({x, y});
            p = {x, y};
            hid::wait_for_action_done();
        }
        else if (command == 'p')
        {
            hid::send(hid::PUMP);
            hid::wait_for_action_done();
        }
        else if (command == 'z')
        {
            std::cin >> x;
            hid::send({x});
            hid::wait_for_action_done();
        }
        else if (command == 'o')
        {
            tmp = p;
            std::cin >> x >> y;
            m.push_back({tmp, {x, y}});
            logger.trace("stm point: {}, {}, grid: {}, {}", tmp.first, tmp.second, x, y);
        }
        else if (command == 'k')
        {
            kx = (m[1].first.first - m[0].first.first) / (m[1].second.first - m[0].second.first);
            ky = (m[1].first.second - m[0].first.second) / (m[1].second.second - m[0].second.second);
            bx = m[0].first.first - kx * m[0].second.first;
            by = m[0].first.second - kx * m[0].second.second;
            logger.trace("Coefs: {}, {}, {}, {}", kx, bx, ky, by);
            std::ofstream tmp_file(tmp_file_name);
            tmp_file << kx << bx << ky << by;
            tmp_file.close();
        }
        else if (command == 'r')
        {
            std::ifstream tmp_file(tmp_file_name);
            tmp_file >> kx >> bx >> ky >> by;
            logger.trace("Coefs: {}, {}, {}, {}", kx, bx, ky, by);
            tmp_file.close();
        }
        else if (command == 'h')
        {
            if (!hid::init())
            {
                logger.error("Reconnect failed.");
                hid::exit();
            }
        }
        else if (command == 'e')
        {
            is_playing = true;
            player = std::thread(func);
        }
        else if (command == 'q')
        {
            break;
        }
    }
    opencv::exit();
    hid::exit();
    return 0;
}
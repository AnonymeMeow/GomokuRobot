#include "ai/gomokuai.hpp"
#include "opencv/opencv.hpp"
#include "hid/hid.hpp"

static Logger logger("main");

int main()
{
    if (!hid::init())
    {
        logger.error("Error occured, exiting.");
        hid::exit();
        return -1;
    }
    opencv::init();
    std::vector<std::pair<std::pair<float, float>, std::pair<int, int>>> m;
    std::pair<float, float> tmp, p;
    while (true)
    {
        char command;
        float x, y;
        std::cin >> command;
        if (command == 'x')
        {
            std::cin >> x;
            std::cin >> command;
            if (command != 'y')
            {
                logger.error("");
                continue;
            }
            std::cin >> y;
            hid::send({x, y});
            p = {x, y};
        }
        else if (command == 'p')
        {
            hid::send(hid::PUMP);
        }
        else if (command == 'z')
        {
            std::cin >> x;
            hid::send({x});
        }
        else if (command == 'o')
        {
            tmp = p;
            int a, b;
            std::cin >> a;
            std::cin >> b;
            m.push_back({tmp, {a, b}});
            logger.trace("stm point: {}, {}, grid: {}, {}", tmp.first, tmp.second, a, b);
        }
        else if (command == 'q')
        {
            break;
        }
    }
    float kx = (m[1].first.first - m[0].first.first) / (m[1].second.first - m[0].second.first);
    float ky = (m[1].first.second - m[0].first.second) / (m[1].second.second - m[0].second.second);
    float bx = m[0].first.first - kx * m[0].second.first;
    float by = m[0].first.second - kx * m[0].second.second;
    logger.trace("{}, {}, {}, {}", kx, bx, ky, by);
    std::cin.get();
    gomokuai::init();
    int count = 0;
    while (true)
    {
        auto pos = opencv::get_ai_step(count);
        logger.info("coord: {}, {}", kx * pos.row + bx, ky * pos.col + by);
        std::cin.get();
        hid::send({kx * 7 + bx, ky * 11 + by});
        hid::wait_for_action_done();
        hid::send(hid::PUMP);
        hid::wait_for_action_done();
        hid::send({kx * pos.row + bx, ky * pos.col + by});
        hid::wait_for_action_done();
        hid::send(hid::PUMP);
        hid::wait_for_action_done();
        hid::send({kx * 5 + bx, ky * -3 + by});
        hid::wait_for_action_done();
        count += 2;
        std::cin.get();
    }
    opencv::exit();
    hid::exit();
    return 0;
}
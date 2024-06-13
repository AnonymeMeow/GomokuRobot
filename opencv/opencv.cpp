#include "opencv.hpp"

#include <thread>

#include "../config.hpp"

namespace opencv
{
    Logger logger("OpenCV");

    cv::VideoCapture cap;

    std::thread window_thread;
    bool is_active = false;

    cv::Mat img;

    void show_img(cv::Mat im, bool do_wait_key = true)
    {
        img = im;
        if (do_wait_key)
        {
            cv::waitKey();
        }
    }

    bool try_open_video(int index)
    {
        logger.info("Trying to open video{}...", index);
        cap.open(index);
        if (!cap.isOpened())
        {
            logger.error("Cannot open video{}.", index);
            cap.release();
            return false;
        }
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 3000);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 2000);
        cv::Mat img;
        if (!cap.read(img))
        {
            logger.error("Cannot read video{}.", index);
            cap.release();
            return false;
        }
        show_img(img, false);
        logger.info("Succeeded!");
        logger.trace("Video{} resolution: {}, {}.", index, img.cols, img.rows);
        return true;
    }

    void window_thread_run()
    {
        cv::namedWindow(window_title, cv::WINDOW_NORMAL);
        while (is_active)
        {
            cv::imshow(window_title, img);
            cv::waitKey(10);
        }
        cv::destroyAllWindows();
    }

    bool init()
    {
        is_active = true;
        window_thread = std::thread(window_thread_run);
        return try_open_video(config::video_device_id);
    }

    void exit()
    {
        is_active = false;
        window_thread.join();
        cap.release();
    }

    gomokuai::Coord_2D get_ai_step(int desired_count)
    {
        while (true)
        {
            cv::Mat img, grey, blue;
            cap.read(img);
            // show_img(img);

            // 定位点
            cv::inRange(img, cv::Scalar(159, 95, 0), cv::Scalar(255, 223, 127), blue);
            cv::GaussianBlur(blue, blue, cv::Size(5, 5), 0);
            // show_img(blue);
            std::vector<cv::Vec3f> anchor_circles;
            cv::HoughCircles(blue, anchor_circles, cv::HOUGH_GRADIENT, 1, 500, 300, 15, 60, 80);
            
            cv::inRange(img, cv::Scalar(159, 159, 0), cv::Scalar(255, 255, 127), blue);
            cv::GaussianBlur(blue, blue, cv::Size(5, 5), 0);
            // show_img(blue);
            std::vector<cv::Vec3f> anchor_circle;
            cv::HoughCircles(blue, anchor_circle, cv::HOUGH_GRADIENT, 1, 500, 300, 15, 60, 80);

            // 识别黑白棋子
            cv::cvtColor(img, grey, cv::COLOR_BGR2GRAY);
            cv::GaussianBlur(grey, grey, cv::Size(5, 5), 0);
            // show_img(grey);
            std::vector<cv::Vec3f> circles;
            cv::HoughCircles(grey, circles, cv::HOUGH_GRADIENT, 1, 100, 50, 20, 50, 70);

            if (anchor_circle.size() != 1 || anchor_circles.size() != 4)
            {
                continue;
            }
            std::vector<std::pair<float, cv::Vec2f>> dists;
            cv::Vec2f main_anchor(anchor_circle[0][0], anchor_circle[0][1]);
            for (int i = 0; i < 4; i++)
            {
                cv::Vec2f anchor(anchor_circles[i][0], anchor_circles[i][1]);
                dists.push_back({(main_anchor - anchor).dot(main_anchor - anchor), anchor});
            }
            std::sort(dists.begin(), dists.end(), [](const std::pair<float, cv::Vec2f>& x, const std::pair<float, cv::Vec2f>& y){ return x.first < y.first; });
            auto dx = (dists[1].second - dists[0].second)/8;
            auto dy = (dists[2].second - dists[0].second)/12;
            auto Dx = dx * 10;
            auto Dy = dy * 10;
            auto origin = dists[0].second - dx + dy;

            cv::cvtColor(img, blue, cv::COLOR_BGR2HSV);

            cv::Mat mask_black, mask_white;
            cv::inRange(blue, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 95), mask_black);
            cv::inRange(blue, cv::Scalar(0, 0, 191), cv::Scalar(255, 63, 255), mask_white);

            std::vector<cv::Vec3f> black, white;
            for (const auto& circle: circles)
            {
                int cx = circle[0], cy = circle[1], r = circle[2];
                if (cx - r <= 0 || cx + r >= img.cols || cy - r <= 0 || cy + r >= img.rows)
                {
                    continue;
                }
                cv::Mat black_roi = mask_black(cv::Rect(cx - r, cy - r, 2 * r, 2 * r));
                int black_count = cv::countNonZero(black_roi);
                if (((double)black_count) / r / r > 2.8)
                {
                    black.push_back(circle);
                    continue;
                }
                cv::Mat white_roi = mask_white(cv::Rect(cx - r, cy - r, 2 * r, 2 * r));
                int white_count = cv::countNonZero(white_roi);
                if (((double)white_count) / r / r > 2.8)
                {
                    white.push_back(circle);
                    continue;
                }
            }
            
            for (int i = 0; i < 11; i++)
            {
                cv::line(img, cv::Point(origin + i * dx), cv::Point(origin + i * dx + Dy), cv::Scalar(0, 0, 255), 5);
                cv::line(img, cv::Point(origin + i * dy), cv::Point(origin + i * dy + Dx), cv::Scalar(0, 0, 255), 5);
            }
            for (const auto& circle: anchor_circles)
            {
                cv::circle(img, cv::Point(circle[0], circle[1]), circle[2], cv::Scalar(255, 127, 0), 5, cv::LINE_AA, 0);
            }
            for (const auto& circle: anchor_circle)
            {
                cv::circle(img, cv::Point(circle[0], circle[1]), circle[2], cv::Scalar(255, 255, 0), 5, cv::LINE_AA, 0);
            }
            for (const auto& circle: circles)
            {
                cv::circle(img, cv::Point(circle[0], circle[1]), circle[2], cv::Scalar(0, 255, 255), 5, cv::LINE_AA, 0);
            }
            for (const auto& circle: black)
            {
                cv::circle(img, cv::Point(circle[0], circle[1]), circle[2], BLACK, 5, cv::LINE_AA, 0);
            }
            for (const auto& circle: white)
            {
                cv::circle(img, cv::Point(circle[0], circle[1]), circle[2], WHITE, 5, cv::LINE_AA, 0);
            }

            gomokuai::clear();
            int chess_count = 0;
            for (const auto& black_stone: black)
            {
                cv::Vec2f pos(black_stone[0], black_stone[1]);
                pos -= origin;
                float inner = pos.dot(dx);
                int x = inner / dx.dot(dx) + 0.5f;
                inner = pos.dot(dy);
                int y = inner / dy.dot(dy) + 0.5f;
                if (x >= 0 && x <= 10 && y >= 0 && y <= 10)
                {
                    gomokuai::put_chess({x, y}, gomokuai::BLACK);
                    chess_count++;
                }
            }
            if (chess_count != desired_count / 2 + desired_count % 2)
            {
                continue;
            }
            chess_count = 0;
            for (const auto& white_stone: white)
            {
                cv::Vec2f pos(white_stone[0], white_stone[1]);
                pos -= origin;
                float inner = pos.dot(dx);
                int x = inner / dx.dot(dx) + 0.5f;
                inner = pos.dot(dy);
                int y = inner / dy.dot(dy) + 0.5f;
                if (x >= 0 && x <= 10 && y >= 0 && y <= 10)
                {
                    gomokuai::put_chess({x, y}, gomokuai::WHITE);
                    chess_count++;
                }
            }
            if (chess_count != desired_count / 2)
            {
                continue;
            }

            show_img(img, false);

            return gomokuai::get_next_point(gomokuai::BLACK);
        }
    }
}
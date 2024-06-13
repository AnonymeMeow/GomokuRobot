#pragma once

#include <opencv2/opencv.hpp>

#include "../logger.hpp"
#include "../ai/gomokuai.hpp"

namespace opencv
{
    extern Logger logger;

    const cv::Scalar BLACK(0, 0, 0);
    const cv::Scalar WHITE(255, 255, 255);

    bool init();

    void exit();

    gomokuai::Coord_2D get_ai_step(int);

    inline const char window_title[] = "OpenCV Window";

    extern cv::Mat img;

    void test(gomokuai::PIECE_TYPE = gomokuai::BLACK);
}
#pragma once

#include <opencv2/opencv.hpp>

#include "../logger.hpp"
#include "../ai/gomokuai.hpp"

namespace opencv
{
    extern Logger logger;

    inline const char window_title[] = "OpenCV Window";

    const cv::Scalar BLACK(0, 0, 0);
    const cv::Scalar WHITE(255, 255, 255);

    void test(gomokuai::PIECE_TYPE);

    bool init();

    void exit();

    gomokuai::Cood_2D get_ai_step(int);
}
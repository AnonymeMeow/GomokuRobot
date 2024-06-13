#include "opencv.hpp"

namespace opencv
{
    const int A4_BOARD_WIDTH = 2100;
    const int A4_BOARD_HEIGHT = 2970;

    const int A4_BOARD_PADDING = 50;
    const int A4_BOARD_SPACING = (A4_BOARD_WIDTH - 2 * A4_BOARD_PADDING) / 10;

    void draw_circle(
        int x, int y,
        const cv::Scalar& color = BLACK,
        int radius = (int)(0.35 * A4_BOARD_SPACING)
    )
    {
        cv::circle(
            img,
            cv::Point(
                x * A4_BOARD_SPACING + A4_BOARD_PADDING,
                (y + 2) * A4_BOARD_SPACING + A4_BOARD_PADDING
            ),
            radius, color, cv::FILLED, cv::LINE_AA
        );
    }

    bool can_click = false;
    gomokuai::Coord_2D click_point;

    void onClick(int event, int x, int y, int flags, void* userdata)
    {
        if (event == cv::EVENT_LBUTTONDOWN && can_click)
        {
            int ix = (x + A4_BOARD_SPACING / 2 - A4_BOARD_PADDING) / A4_BOARD_SPACING;
            int iy = (y - 3 * A4_BOARD_SPACING / 2 - A4_BOARD_PADDING) / A4_BOARD_SPACING;
            if (ix >= 0 && ix < 11 && iy >= 0 && iy < 11)
            {
                click_point.col = ix;
                click_point.row = iy;
                can_click = false;
                return;
            }
        }
    }

    void draw_A4_board()
    {
        img = cv::Mat(A4_BOARD_HEIGHT, A4_BOARD_WIDTH, CV_8UC3, cv::Scalar(0, 95, 160));
        for (int i = 0; i < 11; i++)
        {
            cv::line(
                img,
                cv::Point(A4_BOARD_SPACING * i + A4_BOARD_PADDING, 2 * A4_BOARD_SPACING + A4_BOARD_PADDING),
                cv::Point(A4_BOARD_SPACING * i + A4_BOARD_PADDING, 12 * A4_BOARD_SPACING + A4_BOARD_PADDING),
                BLACK, 5
            );
            cv::line(
                img,
                cv::Point(A4_BOARD_PADDING, A4_BOARD_SPACING * (i + 2) + A4_BOARD_PADDING), 
                cv::Point(10 * A4_BOARD_SPACING + A4_BOARD_PADDING, A4_BOARD_SPACING * (i + 2) + A4_BOARD_PADDING),
                BLACK, 5
            );
        }
        draw_circle(5, 5, BLACK, 15);
        draw_circle(2, 2, BLACK, 15);
        draw_circle(2, 8, BLACK, 15);
        draw_circle(8, 2, BLACK, 15);
        draw_circle(8, 8, BLACK, 15);
        draw_circle(1, -1, cv::Scalar(255, 191, 0), 100);
        draw_circle(9, -1, cv::Scalar(255, 63, 0), 100);
        draw_circle(1, 11, cv::Scalar(255, 63, 0), 100);
        draw_circle(9, 11, cv::Scalar(255, 63, 0), 100);
    }

    void test(gomokuai::PIECE_TYPE ai_type)
    {
        cv::namedWindow(window_title, cv::WINDOW_NORMAL);
        gomokuai::init();
        draw_A4_board();
        cv::setMouseCallback(window_title, onClick);
        if (ai_type == gomokuai::BLACK)
        {
            auto p = gomokuai::get_next_point(ai_type);
            gomokuai::put_chess(p, ai_type);
            draw_circle(p.col, p.row);
        }
        else if (ai_type != gomokuai::WHITE)
        {
            return;
        }
        while (true)
        {
            can_click = true;
            cv::imshow(window_title, img);
            cv::waitKey(10);
            if (!can_click)
            {
                gomokuai::put_chess(click_point, (gomokuai::PIECE_TYPE)(3 - ai_type));
                draw_circle(click_point.col, click_point.row, ai_type == gomokuai::BLACK? WHITE: BLACK);
                auto p = gomokuai::get_next_point(ai_type);
                gomokuai::put_chess(p, ai_type);
                draw_circle(p.col, p.row, ai_type == gomokuai::BLACK? BLACK: WHITE);
            }
        }
    }
}
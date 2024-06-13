/*
 * https://github.com/anlingyi/xechat-idea
 *
 *        Apache License
 *   Version 2.0, January 2004
 * http://www.apache.org/licenses/
 */

#pragma once

#include "../logger.hpp"

namespace gomokuai
{
    extern Logger logger;

    // 棋子类型
    enum PIECE_TYPE
    {
        EMPTY,
        BLACK,
        WHITE,

        ERROR = -1,
    };

    struct Coord_2D
    {
        int row;
        int col;

        Coord_2D():
            row(-1),
            col(-1)
        {}

        Coord_2D(int x, int y):
            row(x),
            col(y)
        {}

        Coord_2D operator+ (const Coord_2D& other) const
        {
            return Coord_2D(row + other.row, col + other.col);
        }

        Coord_2D operator* (int num) const
        {
            return Coord_2D(row * num, col * num);
        }
    };
    

    // 初始化棋盘
    void init();

    // 清空棋盘
    void clear();

    // 获取棋盘上相应位置棋子类型
    PIECE_TYPE get_point(Coord_2D point);

    // 放置棋子
    void put_chess(Coord_2D point, PIECE_TYPE type);

    // 获取AI的下一步下棋点位
    Coord_2D get_next_point(PIECE_TYPE ai_piece_type);
}
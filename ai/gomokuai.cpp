/*
 * https://github.com/anlingyi/xechat-idea
 *
 *        Apache License
 *   Version 2.0, January 2004
 * http://www.apache.org/licenses/
 */

#include "gomokuai.hpp"

#include <vector>
#include <cstring>

#include "../config.hpp"

using std::vector;
using config::board_size;

namespace gomokuai
{
    Logger logger("GomokuAI");

    #define INFINITY 1000000000

    PIECE_TYPE* chessData;

    PIECE_TYPE ai_piece_type;
    float attack_coef;

    // 棋型
    struct ChessModel;

    vector<ChessModel*> chess_models;

    struct ChessModel
    {
        vector<string> values;
        int score;

        ChessModel(int set_score, vector<string>&& set_values):
            score(set_score),
            values(set_values)
        {
            chess_models.push_back(this);
        }
    }
        // 连五
        LIANWU(10000000, {"11111"}),
        // 活四
        HUOSI(1000000, {"011110"}),
        // 活三
        HUOSAN(10000, {"001110", "011100", "010110", "011010"}),
        // 冲四
        CHONGSI(9000, {"11110", "01111", "10111", "11011", "11101"}),
        // 活二
        HUOER(100, {"001100", "011000", "000110", "001010", "010100"}),
        // 活一
        HUOYI(80, {"010200", "002010", "020100", "001020", "201000", "000102", "000201"}),
        // 眠三
        MIANSAN(30, {"001112", "010112", "011012", "211100", "211010"}),
        // 眠二
        MIANER(10, {"011200", "001120", "002110", "021100", "110000", "000011", "000112", "211000"}),
        // 眠一
        MIANYI(1, {"001200", "002100", "000210", "000120", "210000", "000012"});

    enum RiskScore
    {
        HIGH_RISK = 800000,
        MEDIUM_RISK = 500000,
        LOW_RISK = 100000,
    };

    void init()
    {
        chessData = new PIECE_TYPE[board_size * board_size]();
    }

    void clear()
    {
        memset(chessData, EMPTY, board_size * board_size * sizeof(PIECE_TYPE));
    }

    PIECE_TYPE get_point(Coord_2D point)
    {
        if (
            point.row < 0 ||
            point.row >= board_size ||
            point.col < 0 ||
            point.col >= board_size
        )
        {
            return ERROR;
        }
        return chessData[point.row * board_size + point.col];
    }

    void put_chess(Coord_2D point, PIECE_TYPE type)
    {
        if (
            point.row < 0 ||
            point.row >= board_size ||
            point.col < 0 ||
            point.col >= board_size
        )
        {
            return;
        }
        chessData[point.row * board_size + point.col] = type;
    }

    vector<string> get_situation(Coord_2D point)
    {
        const static vector<Coord_2D> directions{
            {1, 0},
            {1, 1},
            {0, 1},
            {-1, 1}
        };
        vector<string> situations;
        PIECE_TYPE type = get_point(point);
        if (type == EMPTY)
        {
            return situations;
        }
        for (auto direction: directions)
        {
            string pieces;
            for (int i = -4; i <= 4; i++)
            {
                PIECE_TYPE piece_type = get_point(point + direction * i);
                if (piece_type == EMPTY)
                {
                    pieces.append("0");
                }
                else if (piece_type == type)
                {
                    pieces.append("1");
                }
                else if (piece_type == 3 - type)
                {
                    pieces.append("2");
                }
            }
            situations.push_back(pieces);
        }
        return situations;
    }

    ChessModel* get_chess_model(string& situation)
    {
        for (auto model: chess_models)
        {
            for (auto& str: model->values)
            {
                if (situation.find(str) != situation.npos)
                {
                    return model;
                }
            }
        }
        return nullptr;
    }

    bool check_chess_model(string& situation, ChessModel& model)
    {
        for (auto& str: model.values)
        {
            if (situation.find(str) != situation.npos)
            {
                return true;
            }
        }
        return false;
    }

    int evaluate(Coord_2D point)
    {
        // 分值
        int score = 0;
        // 活三数
        int huosan_count = 0;
        // 冲四数
        int chongsi_count = 0;
        // 同一方向既活三又冲四数
        int tf_count = 0;

        auto situations = get_situation(point);
        for (auto& situation: situations)
        {
            auto chess_model = get_chess_model(situation);
            if (chess_model != nullptr)
            {
                if (chess_model == &HUOSAN)
                {
                    huosan_count++;
                    if (check_chess_model(situation, CHONGSI))
                    {
                        tf_count++;
                    }
                }
                else if (chess_model == &CHONGSI)
                {
                    chongsi_count++;
                }
                score += chess_model->score;
            }
        }

        if (chongsi_count > 1 || tf_count > 1)
        {
            score += HIGH_RISK;
        }
        else if (chongsi_count > 0 && huosan_count > 0 || tf_count > 0 && huosan_count > 1)
        {
            score += MEDIUM_RISK;
        }
        else if (huosan_count > 1)
        {
            score += LOW_RISK;
        }

        return score;
    }

    Coord_2D get_best_point()
    {
        Coord_2D best;
        int score = -INFINITY;

        for (int i = 0; i < board_size; i++)
        {
            for (int j = 0; j < board_size; j++)
            {
                Coord_2D point(i, j);
                if (get_point(point) != EMPTY)
                {
                    continue;
                }

                put_chess(point, ai_piece_type);
                int ai_score = evaluate(point);
                put_chess(point, (PIECE_TYPE)(3 - ai_piece_type));
                int foe_score = evaluate(point);
                put_chess(point, EMPTY);
                int val = ai_score * attack_coef + foe_score;
                if (val > score)
                {
                    score = val;
                    best = point;
                }
            }
        }

        return best;
    }

    Coord_2D get_next_point(PIECE_TYPE ai_piece_type)
    {
        int piece_count = 0;
        int grid_count = board_size * board_size;
        for (int i = 0; i < grid_count; i++)
        {
            if (chessData[i] != EMPTY)
            {
                piece_count++;
            }
        }
        gomokuai::ai_piece_type = ai_piece_type;
        attack_coef = ai_piece_type == BLACK ? 1.8 : 0.5;

        if (piece_count == 0)
        {
            return Coord_2D(board_size / 2, board_size / 2);
        }
        return get_best_point();
    }
}
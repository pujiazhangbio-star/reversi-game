#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <stack>
#include <string>
#include <random>
#include <chrono>
using namespace std;

// 游戏常量定义
const int BOARD_SIZE = 8;
const char EMPTY = '.';
const char BLACK = 'B';
const char WHITE = 'W';

// 方向数组，用于检查八个方向
const int dx[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
const int dy[] = { -1, 0, 1, -1, 1, -1, 0, 1 };

// AI难度级别
enum class AIDifficulty { EASY, MEDIUM, HARD };

class OthelloGame {
private:
    char board[BOARD_SIZE][BOARD_SIZE];
    char currentPlayer;
    std::stack<std::pair<std::pair<int, int>, std::vector<std::pair<int, int>>>> moveHistory;
    bool vsComputer;
    AIDifficulty aiDifficulty;

    // 位置权重表（角落和边缘位置更有价值）
    const int positionWeights[BOARD_SIZE][BOARD_SIZE] = {
        {100, -20, 10, 5, 5, 10, -20, 100},
        {-20, -30, -5, -5, -5, -5, -30, -20},
        {10, -5, 1, 1, 1, 1, -5, 10},
        {5, -5, 1, 1, 1, 1, -5, 5},
        {5, -5, 1, 1, 1, 1, -5, 5},
        {10, -5, 1, 1, 1, 1, -5, 10},
        {-20, -30, -5, -5, -5, -5, -30, -20},
        {100, -20, 10, 5, 5, 10, -20, 100}
    };

public:
    OthelloGame(bool computerMode = false, AIDifficulty difficulty = AIDifficulty::MEDIUM)
        : vsComputer(computerMode), aiDifficulty(difficulty) {
        initializeBoard();
        currentPlayer = BLACK;
    }

    // 初始化棋盘
    void initializeBoard() {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }

        // 设置初始棋子
        board[3][3] = WHITE;
        board[3][4] = BLACK;
        board[4][3] = BLACK;
        board[4][4] = WHITE;
    }

    // 打印棋盘
    void printBoard() {
        std::cout << "  ";
        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << i << " ";
        }
        std::cout << std::endl;

        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << i << " ";
            for (int j = 0; j < BOARD_SIZE; j++) {
                std::cout << board[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "当前玩家: " << (currentPlayer == BLACK ? "黑棋(B)" : "白棋(W)") << std::endl;
    }

    // 检查位置是否在棋盘内
    bool isValidPosition(int x, int y) {
        return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
    }

    // 检查移动是否合法
    bool isValidMove(int x, int y, char player) {
        if (!isValidPosition(x, y) || board[x][y] != EMPTY) {
            return false;
        }

        char opponent = (player == BLACK) ? WHITE : BLACK;

        for (int dir = 0; dir < 8; dir++) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];

            if (isValidPosition(nx, ny) && board[nx][ny] == opponent) {
                // 沿着这个方向继续检查
                nx += dx[dir];
                ny += dy[dir];

                while (isValidPosition(nx, ny)) {
                    if (board[nx][ny] == EMPTY) {
                        break;
                    }
                    if (board[nx][ny] == player) {
                        return true;
                    }
                    nx += dx[dir];
                    ny += dy[dir];
                }
            }
        }

        return false;
    }

    // 获取所有合法移动位置
    std::vector<std::pair<int, int>> getValidMoves(char player) {
        std::vector<std::pair<int, int>> moves;

        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (isValidMove(i, j, player)) {
                    moves.push_back({ i, j });
                }
            }
        }

        return moves;
    }

    // 执行移动并翻转棋子
    std::vector<std::pair<int, int>> makeMove(int x, int y, char player) {
        std::vector<std::pair<int, int>> flippedPieces;

        if (!isValidMove(x, y, player)) {
            return flippedPieces;
        }

        board[x][y] = player;
        char opponent = (player == BLACK) ? WHITE : BLACK;

        // 记录被翻转的棋子位置
        std::vector<std::pair<int, int>> currentFlip;

        for (int dir = 0; dir < 8; dir++) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            std::vector<std::pair<int, int>> tempFlip;

            while (isValidPosition(nx, ny) && board[nx][ny] == opponent) {
                tempFlip.push_back({ nx, ny });
                nx += dx[dir];
                ny += dy[dir];
            }

            if (isValidPosition(nx, ny) && board[nx][ny] == player) {
                for (auto pos : tempFlip) {
                    board[pos.first][pos.second] = player;
                    currentFlip.push_back(pos);
                }
            }
        }

        // 保存移动历史用于悔棋
        moveHistory.push({ {x, y}, currentFlip });

        return currentFlip;
    }

    // 切换玩家
    void switchPlayer() {
        currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
    }

    // 评估函数 - 考虑位置权重、行动力、稳定性和翻子数
    int evaluatePosition(char player) {
        int opponent = (player == BLACK) ? WHITE : BLACK;
        int score = 0;

        // 1. 位置权重
        for (int r = 0; r < BOARD_SIZE; ++r) {
            for (int c = 0; c < BOARD_SIZE; ++c) {
                if (board[r][c] == player) {
                    score += positionWeights[r][c];
                }
                else if (board[r][c] == opponent) {
                    score -= positionWeights[r][c];
                }
            }
        }

        // 2. 行动力（可用步数）
        int playerMobility = getValidMoves(player).size();
        int opponentMobility = getValidMoves(opponent).size();
        score += (playerMobility - opponentMobility) * 5;

        return score;
    }

    // 极小极大算法 with alpha-beta pruning
    int minimax(int depth, int alpha, int beta, bool maximizingPlayer, char player) {
        if (depth == 0) {
            return evaluatePosition(player);
        }

        char opponent = (player == BLACK) ? WHITE : BLACK;
        char current = maximizingPlayer ? player : opponent;

        if (getValidMoves(current).empty()) {
            if (getValidMoves(opponent).empty()) {
                // 游戏结束
                int b = 0, w = 0;
                countPieces(b, w);
                if (player == BLACK) return (b - w) * 1000;
                else return (w - b) * 1000;
            }
            // 跳过回合
            return minimax(depth - 1, alpha, beta, !maximizingPlayer, player);
        }

        if (maximizingPlayer) {
            int maxEval = -1000000;
            for (int r = 0; r < BOARD_SIZE; ++r) {
                for (int c = 0; c < BOARD_SIZE; ++c) {
                    if (isValidMove(r, c, current)) {
                        // 保存当前状态
                        char boardCopy[BOARD_SIZE][BOARD_SIZE];
                        std::copy(&board[0][0], &board[0][0] + BOARD_SIZE * BOARD_SIZE, &boardCopy[0][0]);

                        makeMove(r, c, current);
                        int eval = minimax(depth - 1, alpha, beta, false, player);

                        // 恢复状态
                        std::copy(&boardCopy[0][0], &boardCopy[0][0] + BOARD_SIZE * BOARD_SIZE, &board[0][0]);

                        maxEval = std::max(maxEval, eval);
                        alpha = std::max(alpha, eval);
                        if (beta <= alpha) break;
                    }
                }
            }
            return maxEval;
        }
        else {
            int minEval = 1000000;
            for (int r = 0; r < BOARD_SIZE; ++r) {
                for (int c = 0; c < BOARD_SIZE; ++c) {
                    if (isValidMove(r, c, current)) {
                        // 保存当前状态
                        char boardCopy[BOARD_SIZE][BOARD_SIZE];
                        std::copy(&board[0][0], &board[0][0] + BOARD_SIZE * BOARD_SIZE, &boardCopy[0][0]);

                        makeMove(r, c, current);
                        int eval = minimax(depth - 1, alpha, beta, true, player);

                        // 恢复状态
                        std::copy(&boardCopy[0][0], &boardCopy[0][0] + BOARD_SIZE * BOARD_SIZE, &board[0][0]);

                        minEval = std::min(minEval, eval);
                        beta = std::min(beta, eval);
                        if (beta <= alpha) break;
                    }
                }
            }
            return minEval;
        }
    }

    // 电脑AI移动（根据难度选择不同的策略）
    std::pair<int, int> computerMove() {
        auto moves = getValidMoves(currentPlayer);

        if (moves.empty()) {
            return { -1, -1 };
        }

        switch (aiDifficulty) {
        case AIDifficulty::EASY: {
            // 简单：随机选择，但优先选择翻子数多的
            std::vector<std::pair<int, int>> goodMoves;
            for (auto& move : moves) {
                int flips = simulateMove(move.first, move.second, currentPlayer);
                if (flips > 2) {
                    goodMoves.push_back(move);
                }
            }
            if (!goodMoves.empty()) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, goodMoves.size() - 1);
                return goodMoves[dis(gen)];
            }
            else {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, moves.size() - 1);
                return moves[dis(gen)];
            }
        }

        case AIDifficulty::MEDIUM: {
            // 中等：贪心算法，选择翻子数最多的
            int bestFlip = -1;
            std::pair<int, int> bestMove = moves[0];
            for (auto& move : moves) {
                int flips = simulateMove(move.first, move.second, currentPlayer);
                if (flips > bestFlip) {
                    bestFlip = flips;
                    bestMove = move;
                }
            }
            return bestMove;
        }

        case AIDifficulty::HARD: {
            // 困难：使用极小极大算法，搜索深度为3
            int bestScore = -1000000;
            std::pair<int, int> bestMove = moves[0];

            for (auto& move : moves) {
                // 保存当前状态
                char boardCopy[BOARD_SIZE][BOARD_SIZE];
                std::copy(&board[0][0], &board[0][0] + BOARD_SIZE * BOARD_SIZE, &boardCopy[0][0]);

                makeMove(move.first, move.second, currentPlayer);
                int score = minimax(3, -1000000, 1000000, false, currentPlayer);

                // 恢复状态
                std::copy(&boardCopy[0][0], &boardCopy[0][0] + BOARD_SIZE * BOARD_SIZE, &board[0][0]);

                if (score > bestScore) {
                    bestScore = score;
                    bestMove = move;
                }
            }
            return bestMove;
        }
        }
        return moves[0];
    }

    // 模拟移动并返回翻转的棋子数量
    int simulateMove(int x, int y, char player) {
        if (!isValidMove(x, y, player)) return 0;

        char opponent = (player == BLACK) ? WHITE : BLACK;
        int total = 0;

        for (int d = 0; d < 8; ++d) {
            int nx = x + dx[d], ny = y + dy[d];
            int line = 0;
            while (isValidPosition(nx, ny) && board[nx][ny] == opponent) {
                ++line; nx += dx[d]; ny += dy[d];
            }
            if (line > 0 && isValidPosition(nx, ny) && board[nx][ny] == player) {
                total += line;
            }
        }
        return total;
    }

    // 悔棋功能
    bool undoMove() {
        if (moveHistory.empty()) {
            std::cout << "没有可以悔棋的步骤！" << std::endl;
            return false;
        }

        auto lastMove = moveHistory.top();
        moveHistory.pop();

        // 移除放置的棋子
        auto movePos = lastMove.first;
        board[movePos.first][movePos.second] = EMPTY;

        // 恢复被翻转的棋子
        char opponent = (currentPlayer == BLACK) ? WHITE : BLACK;
        for (auto pos : lastMove.second) {
            board[pos.first][pos.second] = opponent;
        }

        // 切换回上一个玩家
        switchPlayer();

        std::cout << "悔棋成功！" << std::endl;
        return true;
    }

    // 检查游戏是否结束
    bool isGameOver() {
        auto blackMoves = getValidMoves(BLACK);
        auto whiteMoves = getValidMoves(WHITE);

        return blackMoves.empty() && whiteMoves.empty();
    }

    // 统计棋子数量
    void countPieces(int& blackCount, int& whiteCount) {
        blackCount = 0;
        whiteCount = 0;

        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] == BLACK) {
                    blackCount++;
                }
                else if (board[i][j] == WHITE) {
                    whiteCount++;
                }
            }
        }
    }

    // 显示游戏结果
    void showResult() {
        int blackCount, whiteCount;
        countPieces(blackCount, whiteCount);

        std::cout << "\n游戏结束！" << std::endl;
        std::cout << "黑棋: " << blackCount << " 白棋: " << whiteCount << std::endl;

        if (blackCount > whiteCount) {
            std::cout << "黑棋获胜！" << std::endl;
        }
        else if (whiteCount > blackCount) {
            std::cout << "白棋获胜！" << std::endl;
        }
        else {
            std::cout << "平局！" << std::endl;
        }
    }

    // 主游戏循环
    void playGame() {
        std::cout << "=== 黑白棋游戏 ===" << std::endl;
        std::cout << "输入坐标格式: 行 列 (例如: 3 4)" << std::endl;
        std::cout << "输入 'undo' 可以悔棋" << std::endl;
        std::cout << "输入 'quit' 退出游戏" << std::endl;

        while (!isGameOver()) {
            printBoard();

            auto validMoves = getValidMoves(currentPlayer);
            if (validMoves.empty()) {
                std::cout << "当前玩家没有合法移动，跳过回合" << std::endl;
                switchPlayer();
                continue;
            }

            if (vsComputer && currentPlayer == WHITE) {
                // 电脑回合
                std::cout << "电脑思考中..." << std::endl;
                auto computerMovePos = computerMove();

                if (computerMovePos.first != -1) {
                    makeMove(computerMovePos.first, computerMovePos.second, currentPlayer);
                    std::cout << "电脑在位置 (" << computerMovePos.first << ", "
                        << computerMovePos.second << ") 落子" << std::endl;
                    switchPlayer();
                }
            }
            else {
                // 玩家回合
                std::string input;
                std::cout << "请输入您的移动: ";
                std::cin >> input;

                if (input == "quit") {
                    break;
                }
                else if (input == "undo") {
                    undoMove();
                    continue;
                }

                try {
                    int x = std::stoi(input);
                    int y;
                    std::cin >> y;

                    if (isValidMove(x, y, currentPlayer)) {
                        makeMove(x, y, currentPlayer);
                        switchPlayer();
                    }
                    else {
                        std::cout << "无效的移动！请重新输入。" << std::endl;
                    }
                }
                catch (std::exception& e) {
                    std::cout << "输入格式错误！请使用 '行 列' 格式。" << std::endl;
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                }
            }
        }

        if (isGameOver()) {
            showResult();
        }
    }
};

// 主菜单
void showMenu() {
    std::cout << "=== 黑白棋游戏菜单 ===" << std::endl;
    std::cout << "1. 双人对战" << std::endl;
    std::cout << "2. 人机对战(简单)" << std::endl;
    std::cout << "3. 人机对战(普通)" << std::endl;
    std::cout << "4. 人机对战(困难)" << std::endl;
    std::cout << "5. 退出游戏" << std::endl;
    std::cout << "请选择模式: ";
}

int main() {
    while (true) {
        showMenu();

        int choice;
        std::cin >> choice;

        if (choice == 5) {
            std::cout << "感谢游戏！再见！" << std::endl;
            break;
        }
        else if (choice >= 1 && choice <= 4) {
            bool vsComputer = (choice != 1);
            AIDifficulty difficulty;
            switch (choice) {
            case 2: difficulty = AIDifficulty::EASY; break;
            case 3: difficulty = AIDifficulty::MEDIUM; break;
            case 4: difficulty = AIDifficulty::HARD; break;
            default: difficulty = AIDifficulty::MEDIUM;
            }

            OthelloGame game(vsComputer, difficulty);
            game.playGame();
        }
        else {
            std::cout << "无效的选择！请重新输入。" << std::endl;
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }
    }

    return 0;
}

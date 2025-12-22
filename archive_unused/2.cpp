#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class SimpleOthello {
private:
    static const int BOARD_SIZE = 8;
    static const int EMPTY = 0;
    static const int BLACK = 1;
    static const int WHITE = 2;

    struct GameState {
        vector<vector<int>> board;
        int currentPlayer;
        int currentBlackCount;
        int currentWhiteCount;
        int emptyCount;
        int moveNumber;
        bool isGameActive;
        bool passOccurred;
    };

    struct MatchResult {
        double blackTotalPoints;
        double whiteTotalPoints;
        int blackTotalPieces;
        int whiteTotalPieces;
        int gamesPlayed;
        int blackWins;
        int whiteWins;
        int draws;
    };

    GameState currentGame;
    MatchResult matchResult;
    int totalGames;

public:
    SimpleOthello(int totalGamesCount = 2) : totalGames(totalGamesCount) {
        resetMatch();
        startNewGame();
    }

    // 重置整个比赛
    void resetMatch() {
        matchResult = { 0.0, 0.0, 0, 0, 0, 0, 0, 0 };
    }

    // 开始新游戏
    void startNewGame() {
        currentGame.board = vector<vector<int>>(BOARD_SIZE, vector<int>(BOARD_SIZE, EMPTY));
        // 设置初始4个棋子
        currentGame.board[3][3] = WHITE;
        currentGame.board[3][4] = BLACK;
        currentGame.board[4][3] = BLACK;
        currentGame.board[4][4] = WHITE;

        currentGame.currentPlayer = BLACK;
        currentGame.currentBlackCount = 2;
        currentGame.currentWhiteCount = 2;
        currentGame.emptyCount = 60;
        currentGame.moveNumber = 0;
        currentGame.isGameActive = true;
        currentGame.passOccurred = false;
    }

    // 检查是否为有效移动
    bool isValidMove(int row, int col, int player) {
        if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) return false;
        if (currentGame.board[row][col] != EMPTY) return false;

        // 检查是否能翻转对手棋子
        const int directions[8][2] = { {-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1} };

        for (int d = 0; d < 8; d++) {
            int dr = directions[d][0];
            int dc = directions[d][1];
            int r = row + dr;
            int c = col + dc;
            bool foundOpponent = false;

            while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE) {
                if (currentGame.board[r][c] == EMPTY) break;
                if (currentGame.board[r][c] == player) {
                    if (foundOpponent) return true;
                    break;
                }
                foundOpponent = true;
                r += dr;
                c += dc;
            }
        }
        return false;
    }

    // 获取所有有效移动
    vector<pair<int, int>> getValidMoves(int player) {
        vector<pair<int, int>> moves;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (isValidMove(i, j, player)) {
                    moves.push_back({ i, j });
                }
            }
        }
        return moves;
    }

    // 执行移动
    bool makeMove(int row, int col) {
        if (!currentGame.isGameActive) return false;
        if (!isValidMove(row, col, currentGame.currentPlayer)) {
            cout << "无效移动!" << endl;
            return false;
        }

        // 放置棋子
        currentGame.board[row][col] = currentGame.currentPlayer;
        currentGame.moveNumber++;

        // 翻转棋子
        flipPieces(row, col, currentGame.currentPlayer);

        // 重新统计棋子
        recountPieces();

        // 切换玩家
        currentGame.currentPlayer = (currentGame.currentPlayer == BLACK) ? WHITE : BLACK;
        currentGame.passOccurred = false;

        // 检查游戏是否结束
        checkGameEnd();

        return true;
    }

    // 翻转棋子
    void flipPieces(int row, int col, int player) {
        const int directions[8][2] = { {-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1} };

        for (int d = 0; d < 8; d++) {
            int dr = directions[d][0];
            int dc = directions[d][1];
            vector<pair<int, int>> toFlip;
            int r = row + dr;
            int c = col + dc;

            while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE) {
                if (currentGame.board[r][c] == EMPTY) break;
                if (currentGame.board[r][c] == player) {
                    for (auto pos : toFlip) {
                        currentGame.board[pos.first][pos.second] = player;
                    }
                    break;
                }
                toFlip.push_back({ r, c });
                r += dr;
                c += dc;
            }
        }
    }

    // 重新统计棋子数量
    void recountPieces() {
        currentGame.currentBlackCount = 0;
        currentGame.currentWhiteCount = 0;
        currentGame.emptyCount = 0;

        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (currentGame.board[i][j] == BLACK) currentGame.currentBlackCount++;
                else if (currentGame.board[i][j] == WHITE) currentGame.currentWhiteCount++;
                else currentGame.emptyCount++;
            }
        }
    }

    // 检查游戏是否结束
    void checkGameEnd() {
        auto blackMoves = getValidMoves(BLACK);
        auto whiteMoves = getValidMoves(WHITE);

        if (blackMoves.empty() && whiteMoves.empty()) {
            endGame();
        }
        else if (getValidMoves(currentGame.currentPlayer).empty()) {
            cout << "玩家" << (currentGame.currentPlayer == BLACK ? "黑方" : "白方") << "无法移动，跳过回合" << endl;
            currentGame.currentPlayer = (currentGame.currentPlayer == BLACK) ? WHITE : BLACK;

            if (currentGame.passOccurred) {
                endGame();
            }
            else {
                currentGame.passOccurred = true;
            }
        }
    }

    // 游戏结束
    void endGame() {
        currentGame.isGameActive = false;

        // 计算最终得分
        double blackPoints, whitePoints;
        int finalBlackScore, finalWhiteScore;

        if (currentGame.emptyCount == 0) {
            // 棋盘下满，直接比较棋子数
            finalBlackScore = currentGame.currentBlackCount;
            finalWhiteScore = currentGame.currentWhiteCount;
        }
        else {
            // 棋盘未下满，剩余空格双方各得一半
            double sharedPieces = currentGame.emptyCount / 2;
            finalBlackScore = currentGame.currentBlackCount + sharedPieces;
            finalWhiteScore = currentGame.currentWhiteCount + sharedPieces;


        }

        // 计算得分
        if (finalBlackScore > finalWhiteScore) {
            blackPoints = 1.0;
            whitePoints = 0.0;
        }
        else if (finalWhiteScore > finalBlackScore) {
            blackPoints = 0.0;
            whitePoints = 1.0;
        }
        else {
            blackPoints = 0.5;
            whitePoints = 0.5;
        }

        // 更新比赛结果
        matchResult.blackTotalPoints += blackPoints;
        matchResult.whiteTotalPoints += whitePoints;
        matchResult.blackTotalPieces += finalBlackScore;
        matchResult.whiteTotalPieces += finalWhiteScore;
        matchResult.gamesPlayed++;

        if (blackPoints > whitePoints) matchResult.blackWins++;
        else if (whitePoints > blackPoints) matchResult.whiteWins++;
        else matchResult.draws++;

        displayGameResult(finalBlackScore, finalWhiteScore);
    }

    // 显示游戏结果
    void displayGameResult(int blackScore, int whiteScore) {
        cout << "\n=== 本局游戏结束 ===" << endl;
        cout << "最终比分: 黑" << blackScore << " - 白" << whiteScore << endl;
        if (blackScore > whiteScore) {
            cout << "黑方获胜！得1分" << endl;
        }
        else if (whiteScore > blackScore) {
            cout << "白方获胜！得1分" << endl;
        }
        else {
            cout << "平局！双方各得0.5分" << endl;
        }
        displayMatchScore();
    }

    // 显示比赛总分
    void displayMatchScore() {
        cout << "\n=== 比赛总分 ===" << endl;
        cout << "已进行局数: " << matchResult.gamesPlayed << "/" << totalGames << endl;
        cout << "黑方: " << matchResult.blackTotalPoints << " 分 ("
            << matchResult.blackWins << "胜 " << matchResult.draws << "平)" << endl;
        cout << "白方: " << matchResult.whiteTotalPoints << " 分 ("
            << matchResult.whiteWins << "胜 " << matchResult.draws << "平)" << endl;
        cout << "总胜子数 - 黑方: " << matchResult.blackTotalPieces
            << ", 白方: " << matchResult.whiteTotalPieces << endl;

        if (matchResult.gamesPlayed >= totalGames) {
            displayFinalResult();
        }
    }

    // 显示最终结果
    void displayFinalResult() {
        cout << "\n=== 最终比赛结果 ===" << endl;
        if (matchResult.blackTotalPoints > matchResult.whiteTotalPoints) {
            cout << "🏆 黑方获得最终胜利！" << endl;
        }
        else if (matchResult.whiteTotalPoints > matchResult.blackTotalPoints) {
            cout << "🏆 白方获得最终胜利！" << endl;
        }
        else {
            if (matchResult.blackTotalPieces > matchResult.whiteTotalPieces) {
                cout << "🏆 大比分相同，黑方胜子数领先，黑方获胜！" << endl;
            }
            else if (matchResult.whiteTotalPieces > matchResult.blackTotalPieces) {
                cout << "🏆 大比分相同，白方胜子数领先，白方获胜！" << endl;
            }
            else {
                cout << "🤝 双方完全平局！" << endl;
            }
        }
    }

    // 显示棋盘
    void displayBoard() {
        cout << "\n  ";
        for (int i = 0; i < BOARD_SIZE; i++) {
            cout << i << " ";
        }
        cout << endl;

        for (int i = 0; i < BOARD_SIZE; i++) {
            cout << i << " ";
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (currentGame.board[i][j] == EMPTY) cout << ". ";
                else if (currentGame.board[i][j] == BLACK) cout << "X ";
                else cout << "O ";
            }
            cout << endl;
        }
    }

    // 显示实时比分
    void displayCurrentScore() {
        cout << "\n=== 当前比分 ===" << endl;
        cout << "第" << currentGame.moveNumber << "手" << endl;
        cout << "当前棋子: 黑" << currentGame.currentBlackCount
            << " - 白" << currentGame.currentWhiteCount << endl;
        cout << "剩余空格: " << currentGame.emptyCount << endl;
        cout << "当前玩家: " << (currentGame.currentPlayer == BLACK ? "黑方(X)" : "白方(O)") << endl;
        cout << "===============" << endl;
    }

    // 显示有效移动
    void displayValidMoves() {
        auto moves = getValidMoves(currentGame.currentPlayer);
        cout << "有效移动: ";
        for (auto move : moves) {
            cout << "(" << move.first << "," << move.second << ") ";
        }
        if (moves.empty()) {
            cout << "无";
        }
        cout << endl;
    }

    // 游戏主循环
    void play() {
        cout << "=== 奥赛罗棋游戏 ===" << endl;
        cout << "X = 黑方, O = 白方, . = 空格" << endl;
        cout << "输入格式: 行 列 (例如: 2 3)" << endl;
        cout << "输入 -1 -1 退出游戏" << endl;

        while (matchResult.gamesPlayed < totalGames) {
            cout << "\n=== 第" << (matchResult.gamesPlayed + 1) << "局 ===" << endl;
            startNewGame();

            while (currentGame.isGameActive) {
                displayBoard();
                displayCurrentScore();
                displayValidMoves();

                int row, col;
                cout << "请输入移动坐标: ";
                cin >> row >> col;

                if (row == -1 && col == -1) {
                    cout << "游戏退出" << endl;
                    return;
                }

                makeMove(row, col);
            }

            if (matchResult.gamesPlayed < totalGames) {
                cout << "\n按回车键开始下一局...";
                cin.ignore();
                cin.get();
            }
        }
    }
};

int main() {
    int totalGames;
    cout << "请输入比赛总局数（偶数）: ";
    cin >> totalGames;
    if (totalGames % 2 != 0) {
        cout << "总局数必须为偶数，自动调整为" << (totalGames + 1) << endl;
        totalGames++;
    }
    SimpleOthello game(totalGames);
    game.play();

    return 0;
}

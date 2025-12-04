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

    // ... existing implementation ...

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

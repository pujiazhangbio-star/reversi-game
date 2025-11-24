#include <iostream>
#include <vector>
#include <algorithm>
#include <stack>
#include <string>
#include <random>

// Console-only Othello implementation extracted from the integrated file.
// This file has no dependency on SFML and can be built with a standard C++17 toolchain.

const int BOARD_SIZE = 8;
const char EMPTY_C = '.';
const char BLACK_C = 'B';
const char WHITE_C = 'W';

const int dxs[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
const int dys[] = { -1, 0, 1, -1, 1, -1, 0, 1 };

enum class AIDifficulty { EASY, MEDIUM, HARD };

class OthelloGame {
private:
    char board[BOARD_SIZE][BOARD_SIZE];
    char currentPlayer;
    std::stack<std::pair<std::pair<int, int>, std::vector<std::pair<int, int>>>> moveHistory;
    bool vsComputer;
    AIDifficulty aiDifficulty;

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
        currentPlayer = BLACK_C;
    }

    void initializeBoard() {
        for (int i = 0; i < BOARD_SIZE; ++i)
            for (int j = 0; j < BOARD_SIZE; ++j) board[i][j] = EMPTY_C;
        board[3][3] = WHITE_C; board[3][4] = BLACK_C; board[4][3] = BLACK_C; board[4][4] = WHITE_C;
    }

    void printBoard() {
        std::cout << "  ";
        for (int i = 0; i < BOARD_SIZE; i++) std::cout << i << " ";
        std::cout << '\n';
        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << i << " ";
            for (int j = 0; j < BOARD_SIZE; j++) std::cout << board[i][j] << " ";
            std::cout << '\n';
        }
        std::cout << "当前玩家: " << (currentPlayer == BLACK_C ? "黑棋(B)" : "白棋(W)") << '\n';
    }

    bool isValidPosition(int x, int y) { return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE; }

    bool isValidMove(int x, int y, char player) {
        if (!isValidPosition(x,y) || board[x][y] != EMPTY_C) return false;
        char opponent = (player == BLACK_C) ? WHITE_C : BLACK_C;
        for (int dir=0; dir<8; ++dir) {
            int nx = x + dxs[dir], ny = y + dys[dir];
            if (isValidPosition(nx, ny) && board[nx][ny] == opponent) {
                nx += dxs[dir]; ny += dys[dir];
                while (isValidPosition(nx, ny)) {
                    if (board[nx][ny] == EMPTY_C) break;
                    if (board[nx][ny] == player) return true;
                    nx += dxs[dir]; ny += dys[dir];
                }
            }
        }
        return false;
    }

    std::vector<std::pair<int,int>> getValidMoves(char player) {
        std::vector<std::pair<int,int>> moves;
        for (int i=0;i<BOARD_SIZE;++i) for (int j=0;j<BOARD_SIZE;++j) if (isValidMove(i,j,player)) moves.push_back({i,j});
        return moves;
    }

    std::vector<std::pair<int,int>> makeMove(int x,int y,char player) {
        std::vector<std::pair<int,int>> flipped;
        if (!isValidMove(x,y,player)) return flipped;
        board[x][y] = player; char opponent = (player==BLACK_C?WHITE_C:BLACK_C);
        std::vector<std::pair<int,int>> currentFlip;
        for (int dir=0;dir<8;++dir) {
            int nx=x+dxs[dir], ny=y+dys[dir]; std::vector<std::pair<int,int>> temp;
            while (isValidPosition(nx,ny) && board[nx][ny]==opponent) { temp.push_back({nx,ny}); nx+=dxs[dir]; ny+=dys[dir]; }
            if (isValidPosition(nx,ny) && board[nx][ny]==player) {
                for (auto pos: temp) { board[pos.first][pos.second]=player; currentFlip.push_back(pos); }
            }
        }
        moveHistory.push({{x,y}, currentFlip});
        return currentFlip;
    }

    void switchPlayer() { currentPlayer = (currentPlayer==BLACK_C?WHITE_C:BLACK_C); }

    int evaluatePosition(char player) {
        char opponent = (player==BLACK_C?WHITE_C:BLACK_C);
        int score=0;
        for (int r=0;r<BOARD_SIZE;++r) for (int c=0;c<BOARD_SIZE;++c) {
            if (board[r][c]==player) score += positionWeights[r][c]; else if (board[r][c]==opponent) score -= positionWeights[r][c];
        }
        int playerMobility = (int)getValidMoves(player).size();
        int opponentMobility = (int)getValidMoves(opponent).size();
        score += (playerMobility - opponentMobility) * 5;
        return score;
    }

    void countPieces(int& b, int& w) {
        b=0; w=0; for (int i=0;i<BOARD_SIZE;++i) for (int j=0;j<BOARD_SIZE;++j) { if (board[i][j]==BLACK_C) ++b; else if (board[i][j]==WHITE_C) ++w; }
    }

    int minimax(int depth, int alpha, int beta, bool maximizingPlayer, char player) {
        if (depth==0) return evaluatePosition(player);
        char opponent = (player==BLACK_C?WHITE_C:BLACK_C);
        char current = maximizingPlayer?player:opponent;
        if (getValidMoves(current).empty()) {
            if (getValidMoves(opponent).empty()) { int b=0,w=0; countPieces(b,w); return (player==BLACK_C)?(b-w)*1000:(w-b)*1000; }
            return minimax(depth-1, alpha, beta, !maximizingPlayer, player);
        }
        if (maximizingPlayer) {
            int maxEval = -1000000;
            for (int r=0;r<BOARD_SIZE;++r) for (int c=0;c<BOARD_SIZE;++c) if (isValidMove(r,c,current)) {
                char boardCopy[BOARD_SIZE][BOARD_SIZE]; std::copy(&board[0][0], &board[0][0]+BOARD_SIZE*BOARD_SIZE, &boardCopy[0][0]);
                makeMove(r,c,current);
                int eval = minimax(depth-1, alpha, beta, false, player);
                std::copy(&boardCopy[0][0], &boardCopy[0][0]+BOARD_SIZE*BOARD_SIZE, &board[0][0]);
                maxEval = std::max(maxEval, eval); alpha = std::max(alpha, eval); if (beta <= alpha) break;
            }
            return maxEval;
        } else {
            int minEval = 1000000;
            for (int r=0;r<BOARD_SIZE;++r) for (int c=0;c<BOARD_SIZE;++c) if (isValidMove(r,c,current)) {
                char boardCopy[BOARD_SIZE][BOARD_SIZE]; std::copy(&board[0][0], &board[0][0]+BOARD_SIZE*BOARD_SIZE, &boardCopy[0][0]);
                makeMove(r,c,current);
                int eval = minimax(depth-1, alpha, beta, true, player);
                std::copy(&boardCopy[0][0], &boardCopy[0][0]+BOARD_SIZE*BOARD_SIZE, &board[0][0]);
                minEval = std::min(minEval, eval); beta = std::min(beta, eval); if (beta <= alpha) break;
            }
            return minEval;
        }
    }

    int simulateMove(int x,int y,char player) {
        if (!isValidMove(x,y,player)) return 0;
        char opponent = (player==BLACK_C?WHITE_C:BLACK_C);
        int total=0;
        for (int d=0;d<8;++d) {
            int nx=x+dxs[d], ny=y+dys[d], line=0;
            while (isValidPosition(nx,ny) && board[nx][ny]==opponent) { ++line; nx+=dxs[d]; ny+=dys[d]; }
            if (line>0 && isValidPosition(nx,ny) && board[nx][ny]==player) total+=line;
        }
        return total;
    }

    bool undoMove() {
        if (moveHistory.empty()) { std::cout << "没有可撤的记录\n"; return false; }
        auto last = moveHistory.top(); moveHistory.pop(); auto movePos = last.first; board[movePos.first][movePos.second]=EMPTY_C;
        char opponent = (currentPlayer==BLACK_C?WHITE_C:BLACK_C);
        for (auto pos: last.second) board[pos.first][pos.second]=opponent;
        switchPlayer(); std::cout<<"撤销成功\n"; return true;
    }

    bool isGameOver() { return getValidMoves(BLACK_C).empty() && getValidMoves(WHITE_C).empty(); }

    void showResult() { int b,w; countPieces(b,w); std::cout<<"\n游戏结束\n"; std::cout<<"黑: "<<b<<" 白: "<<w<<"\n"; if (b>w) std::cout<<"黑胜\n"; else if (w>b) std::cout<<"白胜\n"; else std::cout<<"平局\n"; }

    std::pair<int,int> computerMove() {
        auto moves = getValidMoves(currentPlayer);
        if (moves.empty()) return {-1,-1};
        switch (aiDifficulty) {
        case AIDifficulty::EASY: {
            std::vector<std::pair<int,int>> good;
            for (auto &m: moves) { int f = simulateMove(m.first,m.second,currentPlayer); if (f>2) good.push_back(m); }
            if (!good.empty()) { std::random_device rd; std::mt19937 gen(rd()); std::uniform_int_distribution<> dis(0, (int)good.size()-1); return good[dis(gen)]; }
            std::random_device rd; std::mt19937 gen(rd()); std::uniform_int_distribution<> dis(0, (int)moves.size()-1); return moves[dis(gen)];
        }
        case AIDifficulty::MEDIUM: {
            int best=-1; auto bestMove = moves[0]; for (auto &m: moves) { int f=simulateMove(m.first,m.second,currentPlayer); if (f>best) {best=f; bestMove=m;} } return bestMove; }
        case AIDifficulty::HARD: {
            int bestScore=-1000000; auto bestMove = moves[0]; for (auto &m: moves) {
                char boardCopy[BOARD_SIZE][BOARD_SIZE]; std::copy(&board[0][0], &board[0][0]+BOARD_SIZE*BOARD_SIZE, &boardCopy[0][0]);
                makeMove(m.first,m.second,currentPlayer); int score = minimax(3, -1000000, 1000000, false, currentPlayer);
                std::copy(&boardCopy[0][0], &boardCopy[0][0]+BOARD_SIZE*BOARD_SIZE, &board[0][0]); if (score>bestScore) { bestScore=score; bestMove=m; }
            } return bestMove; }
        }
        return moves[0];
    }

    void playGame() {
        std::cout<<"=== 翻转棋 (控制台) ===\n";
        std::cout<<"输入坐标格式: 行 列 (例如: 3 4)"<<std::endl;
        std::cout<<"输入 'undo' 撤销， 'quit' 退出"<<std::endl;
        while (!isGameOver()) {
            printBoard(); auto valid = getValidMoves(currentPlayer);
            if (valid.empty()) { std::cout<<"当前玩家无子可下，跳过...\n"; switchPlayer(); continue; }
            if (vsComputer && currentPlayer==WHITE_C) {
                std::cout<<"AI 思考中...\n"; auto mv = computerMove(); if (mv.first!=-1) { makeMove(mv.first,mv.second,currentPlayer); std::cout<<"AI 下子: ("<<mv.first<<","<<mv.second<<")\n"; switchPlayer(); }
            } else {
                std::string in; std::cout<<"请输入落子或命令: "; std::cin>>in; if (in=="quit") break; if (in=="undo") { undoMove(); continue; }
                try { int x = std::stoi(in); int y; std::cin>>y; if (isValidMove(x,y,currentPlayer)) { makeMove(x,y,currentPlayer); switchPlayer(); } else { std::cout<<"无效落子\n"; } }
                catch(...) { std::cout<<"格式错误, 请用: 行 列\n"; std::cin.clear(); std::cin.ignore(10000,'\n'); }
            }
        }
        if (isGameOver()) showResult();
    }
};

int main() {
    std::cout << "请选择模式: 1. 双人 2. 人机(简单) 3. 人机(中等) 4. 人机(困难)\n";
    int choice = 2; if (!(std::cin >> choice)) return 0;
    bool vsComputer = (choice != 1);
    AIDifficulty diff = AIDifficulty::MEDIUM;
    if (choice == 2) diff = AIDifficulty::EASY; else if (choice == 4) diff = AIDifficulty::HARD;
    OthelloGame game(vsComputer, diff);
    game.playGame();
    return 0;
}

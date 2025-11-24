#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <memory>
#include <stack>
#include <random>
#include <chrono>

#include <SFML/Graphics.hpp>
#include <SFML/Config.hpp>
#include <array>

#if defined(_WIN32)
#  define NOMINMAX
#  include <windows.h>
#  include "resource.h"
#endif

// 0 = empty, 1 = black, 2 = white
static const int BOARD_N = 8;

// --- Console Othello implementation (from attachment) ---
namespace ConsoleOthello {

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
        for (int i = 0; i < BOARD_SIZE; i++)
            for (int j = 0; j < BOARD_SIZE; j++) board[i][j] = EMPTY_C;
        board[3][3] = WHITE_C; board[3][4] = BLACK_C; board[4][3] = BLACK_C; board[4][4] = WHITE_C;
    }

    void printBoard() {
        std::cout << "  ";
        for (int i = 0; i < BOARD_SIZE; i++) std::cout << i << " ";
        std::cout << std::endl;
        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << i << " ";
            for (int j = 0; j < BOARD_SIZE; j++) std::cout << board[i][j] << " ";
            std::cout << std::endl;
        }
        std::cout << "当前玩家: " << (currentPlayer == BLACK_C ? "黑棋(B)" : "白棋(W)") << std::endl;
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
        int playerMobility = getValidMoves(player).size();
        int opponentMobility = getValidMoves(opponent).size();
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

int runConsoleGame() {
    std::cout << "请选择模式: 1. 双人 2. 人机(简单) 3. 人机(中等) 4. 人机(困难)\n";
    int choice = 2; std::cin >> choice;
    bool vsComputer = (choice != 1);
    AIDifficulty diff = AIDifficulty::MEDIUM;
    if (choice == 2) diff = AIDifficulty::EASY; else if (choice == 4) diff = AIDifficulty::HARD;
    OthelloGame game(vsComputer, diff);
    game.playGame();
    return 0;
}

int main() {
    std::cout << "请选择运行模式:\n1) GUI (SFML)\n2) 控制台模式\n输入数字并回车: ";
    int mode = 1;
    if (!(std::cin >> mode)) return 0;
    if (mode == 2) {
        return ConsoleOthello::runConsoleGame();
    } else {
        return runSFML();
    }
}

} // namespace ConsoleOthello

// --- End of console implementation ---


int runSFML() {
    enum class GameState { Start, Playing, End };
    GameState gameState = GameState::Start;
    enum class GameMode { None, PvP, PvC };
    GameMode gameMode = GameMode::None;


    // 棋盘与窗口参数
    const float cell = 80.f;           // 单元格边长
    const float margin = 40.f;         // 边缘留白
    const float boardSize = BOARD_N * cell;
    const unsigned winW = static_cast<unsigned>(boardSize + margin * 2);
    const unsigned winH = static_cast<unsigned>(boardSize + margin * 2);

    // UI font and start screen widgets (must be after winW/winH)
    sf::Font font;
    bool fontOk = false;
    std::string usedFont;
    // 1. 先尝试从文件系统加载（方便开发调试）
    const char* candidates[] = {"fonts/DejaVuSans.ttf", "./fonts/DejaVuSans.ttf"};
    for (auto p : candidates) {
        if (std::filesystem::exists(p) && font.openFromFile(p)) { usedFont = p; fontOk = true; break; }
    }
    // 2. 若文件不存在并且开启了资源嵌入（Windows），尝试从资源加载
#if defined(_WIN32) && defined(EMBED_FONT_RESOURCE)
    if (!fontOk) {
        HRSRC res = FindResource(nullptr, MAKEINTRESOURCE(IDR_FONT_DEJAVU), RT_RCDATA);
        if (res) {
            HGLOBAL hData = LoadResource(nullptr, res);
            if (hData) {
                void* pData = LockResource(hData);
                DWORD sz = SizeofResource(nullptr, res);
                if (pData && sz > 0) {
#if defined(SFML_VERSION_MAJOR) && (SFML_VERSION_MAJOR >= 3)
                    if (font.openFromMemory(pData, static_cast<std::size_t>(sz))) {
#else
                    if (font.loadFromMemory(pData, static_cast<std::size_t>(sz))) {
#endif
                        fontOk = true;
                        usedFont = "<embedded resource>";
                    }
                }
            }
        }
    }
#endif
    if (fontOk) {
        std::cout << "[Info] Using font: " << usedFont << std::endl;
    } else {
        std::cerr << "[Warn] Font not found. Provide fonts/DejaVuSans.ttf (development) or build with EMBED_FONT=ON on Windows." << std::endl;
    }
    auto makeText = [](const sf::Font& f, const std::string& s, unsigned int size){
#if defined(SFML_VERSION_MAJOR) && (SFML_VERSION_MAJOR >= 3)
    sf::Text t(f);
    t.setString(s);
    t.setCharacterSize(size);
    return t;
#else
    sf::Text t(s, f, size);
    return t;
#endif
    };
    sf::Text title = makeText(font, "Reversi", 48);
    title.setFillColor(sf::Color::White);
    title.setOutlineColor(sf::Color::Black);
    title.setOutlineThickness(2.f);
    title.setPosition(sf::Vector2f(winW/2.f-100.f, 80.f));
    sf::RectangleShape btn1(sf::Vector2f(300.f, 60.f)), btn2(sf::Vector2f(300.f, 60.f));
    btn1.setPosition(sf::Vector2f(winW/2.f-130.f, 200.f));
    btn2.setPosition(sf::Vector2f(winW/2.f-130.f, 300.f));
    btn1.setFillColor(sf::Color(60,180,60));
    btn2.setFillColor(sf::Color(60,60,180));
    sf::Text txt1 = makeText(font, "Player vs Computer", 30);
    sf::Text txt2 = makeText(font, "Player vs Player", 30);
    txt1.setFillColor(sf::Color::White);
    txt2.setFillColor(sf::Color::White);
    txt1.setOutlineColor(sf::Color::Black);
    txt2.setOutlineColor(sf::Color::Black);
    txt1.setOutlineThickness(2.f);
    txt2.setOutlineThickness(2.f);
    // center text inside buttons
    auto centerTextInRect = [](sf::Text& t, const sf::RectangleShape& r){
    sf::FloatRect b = t.getLocalBounds();
#if defined(SFML_VERSION_MAJOR) && (SFML_VERSION_MAJOR >= 3)
    t.setOrigin({b.position.x + b.size.x/2.f, b.position.y + b.size.y/2.f});
#else
    t.setOrigin({b.left + b.width/2.f, b.top + b.height/2.f});
#endif
    auto pos = r.getPosition();
    auto size = r.getSize();
    t.setPosition({pos.x + size.x/2.f, pos.y + size.y/2.f});
    };
    centerTextInRect(txt1, btn1);
    centerTextInRect(txt2, btn2);
    // 结束页文本
    sf::Text endTitle = makeText(font, "Game Over", 48);
    endTitle.setFillColor(sf::Color::White);
    endTitle.setOutlineColor(sf::Color::Black);
    endTitle.setOutlineThickness(2.f);
    endTitle.setPosition(sf::Vector2f(winW/2.f-140.f, 80.f));
    sf::Text endResult = makeText(font, "", 40);
    endResult.setFillColor(sf::Color::White);
    endResult.setOutlineColor(sf::Color::Black);
    endResult.setOutlineThickness(2.f);
    endResult.setPosition(sf::Vector2f(winW/2.f-120.f, 170.f));
    sf::Text endScore = makeText(font, "", 28);
    endScore.setFillColor(sf::Color::White);
    endScore.setOutlineColor(sf::Color::Black);
    endScore.setOutlineThickness(2.f);
    endScore.setPosition(sf::Vector2f(winW/2.f-120.f, 230.f));
    sf::Text endHint = makeText(font, "Enter: Play Again   Esc: Back to Start", 22);
    endHint.setFillColor(sf::Color::White);
    endHint.setOutlineColor(sf::Color::Black);
    endHint.setOutlineThickness(2.f);
    endHint.setPosition(sf::Vector2f(winW/2.f-180.f, 290.f));

    // 历史记录：每步保存棋盘和当前玩家
    std::vector<std::pair<int[BOARD_N][BOARD_N], int>> history;

    // VideoMode construction differs between SFML 2 and 3
#if defined(SFML_VERSION_MAJOR) && (SFML_VERSION_MAJOR >= 3)
    sf::VideoMode mode({winW, winH});
#else
    sf::VideoMode mode(winW, winH);
#endif
    sf::RenderWindow window(
        mode,
        "Reversi (SFML)",
        sf::Style::Default
    );
    window.setFramerateLimit(60);

    // 棋盘数据
    int board[BOARD_N][BOARD_N] = {0};
    int currentPlayer = 1; // 黑先
    // 初始四子
    board[3][3] = 2; // 白
    board[3][4] = 1; // 黑
    board[4][3] = 1; // 黑
    board[4][4] = 2; // 白

    // 方向数组
    const int dx[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
    const int dy[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };

    // 判断坐标是否合法
    auto isValidPos = [&](int r, int c) {
        return r >= 0 && r < BOARD_N && c >= 0 && c < BOARD_N;
    };

    // 判断落子是否合法
    auto isValidMove = [&](int r, int c, int player) {
        if (!isValidPos(r, c) || board[r][c] != 0) return false;
        int opponent = (player == 1 ? 2 : 1);
        for (int d = 0; d < 8; ++d) {
            int nr = r + dx[d], nc = c + dy[d];
            if (isValidPos(nr, nc) && board[nr][nc] == opponent) {
                nr += dx[d]; nc += dy[d];
                while (isValidPos(nr, nc)) {
                    if (board[nr][nc] == 0) break;
                    if (board[nr][nc] == player) return true;
                    nr += dx[d]; nc += dy[d];
                }
            }
        }
        return false;
    };

    auto hasValidAny = [&](int player) {
        for (int r = 0; r < BOARD_N; ++r)
            for (int c = 0; c < BOARD_N; ++c)
                if (isValidMove(r, c, player)) return true;
        return false;
    };

    auto countDiscs = [&]() {
        int b=0,w=0;
        for (int r=0;r<BOARD_N;++r)
            for (int c=0;c<BOARD_N;++c)
                if (board[r][c]==1) ++b; else if (board[r][c]==2) ++w;
        return std::pair<int,int>(b,w);
    };

    // 执行落子并翻子
    auto makeMove = [&](int r, int c, int player) {
        std::vector<std::pair<int,int>> flipped;
        if (!isValidMove(r, c, player)) return flipped;
        board[r][c] = player;
        int opponent = (player == 1 ? 2 : 1);
        for (int d = 0; d < 8; ++d) {
            int nr = r + dx[d], nc = c + dy[d];
            std::vector<std::pair<int,int>> temp;
            while (isValidPos(nr, nc) && board[nr][nc] == opponent) {
                temp.push_back({nr, nc});
                nr += dx[d]; nc += dy[d];
            }
            if (isValidPos(nr, nc) && board[nr][nc] == player) {
                for (auto& p : temp) {
                    board[p.first][p.second] = player;
                    flipped.push_back(p);
                }
            }
        }
        return flipped;
    };

    // 评估某一步可翻子的数量（不修改棋盘）
    auto evalFlipCount = [&](int r, int c, int player)->int {
        if (!isValidMove(r,c,player)) return 0;
        int opponent = (player==1?2:1);
        int total=0;
        for (int d=0; d<8; ++d) {
            int nr=r+dx[d], nc=c+dy[d];
            int line=0;
            while (isValidPos(nr,nc) && board[nr][nc]==opponent) {
                ++line; nr+=dx[d]; nc+=dy[d];
            }
            if (line>0 && isValidPos(nr,nc) && board[nr][nc]==player) total+=line;
        }
        return total;
    };

    auto saveHistory = [&]() {
        int boardCopy[BOARD_N][BOARD_N];
        std::copy(&board[0][0], &board[0][0] + BOARD_N*BOARD_N, &boardCopy[0][0]);
        history.push_back({});
        std::copy(&boardCopy[0][0], &boardCopy[0][0] + BOARD_N*BOARD_N, &history.back().first[0][0]);
        history.back().second = currentPlayer;
    };

    auto checkEndOrPass = [&]() {
        // 若当前玩家无棋可下，尝试跳过给对手；若双方都无棋可下则结束
        if (!hasValidAny(currentPlayer)) {
            int other = (currentPlayer==1?2:1);
            if (hasValidAny(other)) {
                currentPlayer = other; // 跳过
            } else {
                // 结束
                auto [b,w] = countDiscs();
                endScore.setString("Black: " + std::to_string(b) + "  White: " + std::to_string(w));
                if (gameMode == GameMode::PvC) {
                    // 人类=黑棋，AI=白棋
                    if (b>w) endResult.setString("You win!");
                    else if (b<w) endResult.setString("You lose!");
                    else endResult.setString("Draw");
                } else {
                    if (b>w) endResult.setString("Black wins");
                    else if (b<w) endResult.setString("White wins");
                    else endResult.setString("Draw");
                }
                gameState = GameState::End;
            }
        }
    };

    // 棋盘底色
    sf::RectangleShape tile({cell, cell});
    sf::Color green1(30, 120, 30);
    sf::Color green2(20, 100, 20);

    // 棋子圆形
    sf::CircleShape disc(cell * 0.40f); // 半径
    disc.setOrigin({disc.getRadius(), disc.getRadius()});

    auto toBoardRC = [&](sf::Vector2i mouse) -> std::pair<int,int> {
        float x = mouse.x - margin;
        float y = mouse.y - margin;
        if (x < 0 || y < 0) return {-1, -1};
        int c = static_cast<int>(x / cell);
        int r = static_cast<int>(y / cell);
        if (r < 0 || r >= BOARD_N || c < 0 || c >= BOARD_N) return {-1, -1};
        // 检查是否在棋盘内
        if (x >= boardSize || y >= boardSize) return {-1, -1};
        return {r, c};
    };

    while (window.isOpen()) {
    if (gameState == GameState::Start) {
            window.clear({20,40,60});
            window.draw(title);
            window.draw(btn1); window.draw(btn2);
            window.draw(txt1); window.draw(txt2);
            window.display();
            // Events for Start screen
#if defined(SFML_VERSION_MAJOR) && (SFML_VERSION_MAJOR >= 3)
            while (auto event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) window.close();
                else if (event->is<sf::Event::MouseButtonPressed>()) {
                    auto mouse = event->getIf<sf::Event::MouseButtonPressed>();
                    sf::Vector2f mp(static_cast<float>(mouse->position.x), static_cast<float>(mouse->position.y));
                    if (btn1.getGlobalBounds().contains(mp)) {
                        gameMode = GameMode::PvC; gameState = GameState::Playing;
                    } else if (btn2.getGlobalBounds().contains(mp)) {
                        gameMode = GameMode::PvP; gameState = GameState::Playing;
                    }
                }
            }
#else
            sf::Event ev;
            while (window.pollEvent(ev)) {
                if (ev.type == sf::Event::Closed) window.close();
                else if (ev.type == sf::Event::MouseButtonPressed) {
                    sf::Vector2f mp(static_cast<float>(ev.mouseButton.x), static_cast<float>(ev.mouseButton.y));
                    if (btn1.getGlobalBounds().contains(mp)) {
                        gameMode = GameMode::PvC; gameState = GameState::Playing;
                    } else if (btn2.getGlobalBounds().contains(mp)) {
                        gameMode = GameMode::PvP; gameState = GameState::Playing;
                    }
                }
            }
#endif
            continue;
        }
        if (gameState == GameState::End) {
#if defined(SFML_VERSION_MAJOR) && (SFML_VERSION_MAJOR >= 3)
            while (auto event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) window.close();
                else if (event->is<sf::Event::KeyPressed>()) {
                    auto key = event->getIf<sf::Event::KeyPressed>()->code;
                    if (key == sf::Keyboard::Key::Enter) {
                        std::fill(&board[0][0], &board[0][0]+BOARD_N*BOARD_N, 0);
                        board[3][3]=2; board[3][4]=1; board[4][3]=1; board[4][4]=2;
                        history.clear(); currentPlayer=1; gameState = GameState::Playing;
                    } else if (key == sf::Keyboard::Key::Escape) {
                        std::fill(&board[0][0], &board[0][0]+BOARD_N*BOARD_N, 0);
                        board[3][3]=2; board[3][4]=1; board[4][3]=1; board[4][4]=2;
                        history.clear(); currentPlayer=1; gameMode = GameMode::None; gameState = GameState::Start;
                    }
                }
            }
#else
            sf::Event ev;
            while (window.pollEvent(ev)) {
                if (ev.type == sf::Event::Closed) window.close();
                else if (ev.type == sf::Event::KeyPressed) {
                    auto key = ev.key.code;
                    if (key == sf::Keyboard::Enter) {
                        std::fill(&board[0][0], &board[0][0]+BOARD_N*BOARD_N, 0);
                        board[3][3]=2; board[3][4]=1; board[4][3]=1; board[4][4]=2;
                        history.clear(); currentPlayer=1; gameState = GameState::Playing;
                    } else if (key == sf::Keyboard::Escape) {
                        std::fill(&board[0][0], &board[0][0]+BOARD_N*BOARD_N, 0);
                        board[3][3]=2; board[3][4]=1; board[4][3]=1; board[4][4]=2;
                        history.clear(); currentPlayer=1; gameMode = GameMode::None; gameState = GameState::Start;
                    }
                }
            }
#endif
            window.clear({20,40,60});
            window.draw(endTitle);
            window.draw(endResult);
            window.draw(endScore);
            window.draw(endHint);
            window.display();
            continue;
        }
        // Events for Playing state
#if defined(SFML_VERSION_MAJOR) && (SFML_VERSION_MAJOR >= 3)
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::KeyPressed>()) {
                if (event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Z) {
                    if (!history.empty()) {
                        auto& last = history.back();
                        std::copy(&last.first[0][0], &last.first[0][0] + BOARD_N*BOARD_N, &board[0][0]);
                        currentPlayer = last.second; history.pop_back();
                    }
                }
            }
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (event->is<sf::Event::MouseButtonPressed>()) {
                if (event->getIf<sf::Event::MouseButtonPressed>()->button == sf::Mouse::Button::Left) {
                    auto rc = toBoardRC(sf::Mouse::getPosition(window));
                    int r = rc.first, c = rc.second;
                    if (r != -1 && isValidMove(r, c, currentPlayer)) {
                        saveHistory(); makeMove(r, c, currentPlayer);
                        currentPlayer = (currentPlayer == 1 ? 2 : 1); checkEndOrPass();
                    }
                }
            }
        }
#else
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Z) {
                    if (!history.empty()) {
                        auto& last = history.back();
                        std::copy(&last.first[0][0], &last.first[0][0] + BOARD_N*BOARD_N, &board[0][0]);
                        currentPlayer = last.second; history.pop_back();
                    }
                }
            }
            if (ev.type == sf::Event::Closed) {
                window.close();
            } else if (ev.type == sf::Event::MouseButtonPressed) {
                if (ev.mouseButton.button == sf::Mouse::Left) {
                    auto rc = toBoardRC(sf::Mouse::getPosition(window));
                    int r = rc.first, c = rc.second;
                    if (r != -1 && isValidMove(r, c, currentPlayer)) {
                        saveHistory(); makeMove(r, c, currentPlayer);
                        currentPlayer = (currentPlayer == 1 ? 2 : 1); checkEndOrPass();
                    }
                }
            }
        }
#endif

        // PvC：AI为白棋（2），轮到AI则下子（简单贪心：翻子数最多）
        if (gameState == GameState::Playing && gameMode == GameMode::PvC && currentPlayer == 2) {
            int bestR=-1,bestC=-1,bestFlip=0;
            for (int r=0;r<BOARD_N;++r) {
                for (int c=0;c<BOARD_N;++c) {
                    int f = evalFlipCount(r,c,2);
                    if (f>bestFlip) { bestFlip=f; bestR=r; bestC=c; }
                }
            }
            if (bestFlip>0) {
                saveHistory();
                makeMove(bestR,bestC,2);
                currentPlayer = 1;
                checkEndOrPass();
            } else {
                // AI无棋可下，尝试切回玩家；若玩家也无棋则结束
                checkEndOrPass();
            }
        }

        window.clear({12, 60, 12});

        // 绘制棋盘格
        for (int r = 0; r < BOARD_N; ++r) {
            for (int c = 0; c < BOARD_N; ++c) {
                bool alt = ((r + c) % 2 == 0);
                tile.setFillColor(alt ? green1 : green2);
                tile.setPosition({margin + c * cell, margin + r * cell});
                window.draw(tile);
            }
        }

        // 网格线（可选）
        sf::RectangleShape line;
        line.setFillColor({0, 0, 0, 80});
        // 竖线
        for (int c = 0; c <= BOARD_N; ++c) {
            line.setSize({2.f, boardSize});
            line.setPosition({margin + c * cell, margin});
            window.draw(line);
        }
        // 横线
        for (int r = 0; r <= BOARD_N; ++r) {
            line.setSize({boardSize, 2.f});
            line.setPosition({margin, margin + r * cell});
            window.draw(line);
        }

        // 绘制棋子
        for (int r = 0; r < BOARD_N; ++r) {
            for (int c = 0; c < BOARD_N; ++c) {
                if (board[r][c] == 0) continue;
                float cx = margin + c * cell + cell / 2.f;
                float cy = margin + r * cell + cell / 2.f;
                disc.setPosition({cx, cy});
                if (board[r][c] == 1) {
                    disc.setFillColor(sf::Color::Black);
                    disc.setOutlineColor({230,230,230});
                } else {
                    disc.setFillColor(sf::Color::White);
                    disc.setOutlineColor({30,30,30});
                }
                disc.setOutlineThickness(3.f);
                window.draw(disc);
            }
        }

        window.display();
    }

    return 0;
}

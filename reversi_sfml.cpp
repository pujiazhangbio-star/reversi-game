#include <string>
#include <vector>
#include <iostream>
#include <filesystem>

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


int main() {
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

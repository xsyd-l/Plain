#include "../include/Game.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    // 源文件为 UTF-8，程序输出 UTF-8 字节；将控制台输出代码页设为 UTF-8，
    // 避免中文被按 GBK(936) 解码成乱码
    SetConsoleOutputCP(CP_UTF8);
#endif
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Plain");
    Game game(window);
    game.run();
    return 0;
}
// Game 核心类头文件
// 未采用离散编译，不得引用此头文件！
// 
// Powerd By Yan

#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class Game {
public:
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    Game();
    void run();
private:
    void processEvents();
    void update();
    void render();
    sf::RenderWindow _window;
    sf::CircleShape  _player;
};

// END
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
#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Player.h"
#include "Enemy.h"
#include "Board.h"
#include "Piece.h"

class Game
{
public:
    Game();
    ~Game();
    void run();

private:
    void processEvents();
    void processKeys(const std::optional<sf::Event> t_event);
    void checkKeyboardState();
    void update(sf::Time t_deltaTime);
    void render();

    bool validateEntities(); // Check if all entities loaded successfully
    void initializePieces(); //create game pieces

    sf::RenderWindow window;
    sf::Font font;
    bool exitGame = false;
    bool m_gameValid = false;

    // Game Objects
    Player m_player;
    Enemy m_enemy;
    Board m_board;

    std::vector<Piece> m_playerPieces;
    std::vector<Piece> m_aiPieces;
};

#pragma warning( pop ) 
#endif
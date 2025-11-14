#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Board.h"
#include "GameState.h"
#include "Donkey.h"

#include <vector>


class Game
{
public:
    Game();
    ~Game();
    void run();

private:
    void render();

    // Init game
    void initializePieces(); //create game pieces
    bool validateGame();

    // Game loop
    void processEvents();
    void update(sf::Time t_deltaTime);

    // Input
    void processKeys(const std::optional<sf::Event> t_event);
    void checkKeyboardState();

    //mouse input
    void handleMouseClick(int mouseX, int mouseY);
    void selectPieceFromSelectionGrid(GridPos pos);
    void selectPieceFromGameBoard(GridPos pos);
    void placePiece(GridPos pos);
    void clearAllHighlights();

    void switchTurn();

    // Ai

    // Helper Functions

    sf::RenderWindow window;
    sf::Font font;
    bool exitGame = false;
    bool m_gameValid = false;

    // Game Objects
    Board m_board;

    // Pieces
    std::vector<std::unique_ptr<Piece>> m_allPieces;
    std::vector<Piece*> m_playerPieces;
    std::vector<Piece*> m_aiPieces;
    Piece* m_selectedPiece = nullptr;
    PieceOwner m_currentTurn = PieceOwner::PLAYER;
    int m_selectedPieceIndex;
    bool m_pieceSelected;

};

#pragma warning( pop ) 
#endif
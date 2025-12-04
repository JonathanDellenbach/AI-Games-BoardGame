#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Board.h"
#include "GameState.h"
#include "MiniMax.h"
#include "Donkey.h"
#include "Snake.h"
#include "Frog.h"

#include <vector>

enum class GameMode {
    PLAYER_VS_AI,
    AI_VS_AI
};

class Game
{
public:
    Game();
    ~Game();
    void run();

private:
    // Init game
    void initializePieces(); //create game pieces
    bool validateGame();

    // Game loop
    void processEvents();
    void processKeys(const std::optional<sf::Event> t_event);
    void checkKeyboardState();
    void update(sf::Time t_deltaTime);
    void render();
    void renderGameOver();

    // Input
    void handleMouseClick(int mouseX, int mouseY);
    void handlePlacementPhase(int mouseX, int mouseY);
    void handleMovementPhase(int mouseX, int mouseY);

    // Piece selection and movement
    void selectPieceFromSelectionGrid(GridPos pos);
    void selectPieceFromGameBoard(GridPos pos);
    void placePieceInPlacementPhase(GridPos pos);
    void movePiece(GridPos pos);

    // AI execution
    void executeAIPlacement();
    void executeAIMove();
    void executePlayerAIPlacement();
    void executePlayerAIMove();

    // Ui Helpters
    void highlightValidPlacements();
    void highlightValidMoves();
    void clearAllHighlights();
    void switchTurn();
    void renderModeSelection();

    // Game Comps
    Board m_board;
    GameState m_gameState;
    MiniMax m_ai{ PieceOwner::AI }; // ai
    MiniMax m_playerAI{ PieceOwner::PLAYER }; //ai as player

    // General SFML and locals
    sf::RenderWindow window;
    sf::Font font;
    bool exitGame = false;
    bool m_gameValid = false;

    GameMode m_gameMode;
    bool m_modeSelected = false;
    sf::Clock m_aiMoveTimer;
    float m_aiMoveDelay;
    bool m_waitingForNextMove;

    // Pieces
    std::vector<std::unique_ptr<Piece>> m_allPieces;
    std::vector<Piece*> m_playerPieces;
    std::vector<Piece*> m_aiPieces;

    // Game State
    Piece* m_selectedPiece;
    int m_selectedPieceIndex;
    bool m_pieceSelected;
    PieceOwner m_currentTurn{ PieceOwner::PLAYER };

    int m_playerPiecesPlaced;
    int m_aiPiecesPlaced;
};

#pragma warning( pop ) 
#endif
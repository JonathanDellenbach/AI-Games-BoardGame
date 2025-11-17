#include "Game.h"
#include <iostream>

Game::Game() :
    window{ sf::VideoMode{ sf::Vector2u{1920U, 1080U}, 32U }, "Board Game" },
    m_gameValid(false),
    m_selectedPiece(nullptr),
    m_selectedPieceIndex(-1),
    m_pieceSelected(false),
    m_playerPiecesPlaced(0),
    m_aiPiecesPlaced(0)
{
    initializePieces();
    m_gameValid = validateGame();
    std::cout << "Player Turn" << std::endl;
    // Note: Load a font when we can to track game state
}

Game::~Game()
{
}

void Game::run()
{
    if (!m_gameValid)
    {
        std::cout << "Game failed to initialize. Exiting..." << std::endl;
        return;
    }

    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    const float fps{ 60.0f };
    sf::Time timePerFrame = sf::seconds(1.0f / fps);

    while (window.isOpen())
    {
        processEvents();
        timeSinceLastUpdate += clock.restart();
        while (timeSinceLastUpdate > timePerFrame)
        {
            timeSinceLastUpdate -= timePerFrame;
            processEvents();
            update(timePerFrame);
        }
        render();
    }
}

void Game::processEvents()
{
    while (const std::optional newEvent = window.pollEvent())
    {
        if (newEvent->is<sf::Event::Closed>())
        {
            exitGame = true;
        }
        if (newEvent->is<sf::Event::KeyPressed>())
        {
            processKeys(newEvent);
        }
        if (newEvent->is<sf::Event::MouseButtonPressed>())
        {
            auto* mouseEvent = newEvent->getIf<sf::Event::MouseButtonPressed>();
            if (mouseEvent && mouseEvent->button == sf::Mouse::Button::Left)
            {
                handleMouseClick(mouseEvent->position.x, mouseEvent->position.y);
            }
        }
    }
}

void Game::processKeys(const std::optional<sf::Event> t_event)
{
    const sf::Event::KeyPressed* newKeypress = t_event->getIf<sf::Event::KeyPressed>();
    if (sf::Keyboard::Key::Escape == newKeypress->code)
    {
        exitGame = true;
    }
}

void Game::checkKeyboardState()
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
    {
        exitGame = true;
    }
}

void Game::update(sf::Time t_deltaTime)
{
    checkKeyboardState();
    if (exitGame)
    {
        window.close();
    }

    // Check for win condition
    PieceOwner winner = m_gameState.getWinner();
    if (winner != PieceOwner::NONE)
    {
        if (winner == PieceOwner::PLAYER) {
            std::cout << "\n=== PLAYER WINS! ===" << std::endl;
        }
        else {
            std::cout << "\n=== AI WINS! ===" << std::endl;
        }
        m_gameState.setPhase(GamePhase::GAME_OVER);
        return;
    }

    // AI turn
    if (m_currentTurn == PieceOwner::AI && m_gameState.getCurrentPhase() != GamePhase::GAME_OVER)
    {
        sf::sleep(sf::seconds(0.5f));  // Brief delay so human can see AI move

        if (m_gameState.getCurrentPhase() == GamePhase::PLACEMENT)
        {
            // Ai determines placement using minimax
        }
        else if (m_gameState.getCurrentPhase() == GamePhase::MOVEMENT)
        {
            // Ai Moves pieces
        }

        switchTurn();
    }


}

void Game::render()
{
    window.clear(sf::Color::Black);

    m_board.render(window);

    // Render all pieces
    for (auto& piece : m_allPieces) {
        piece->render(window);
    }

    window.display();
}

void Game::initializePieces()
{
    int numPieces = 5;
    int totalPieces = 10;

    m_playerPieces.reserve(numPieces);
    m_aiPieces.reserve(numPieces);

    // Player pieces
    m_allPieces.push_back(std::make_unique<Frog>(PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-frog.png"));
    m_allPieces.push_back(std::make_unique<Snake>(PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-snake.png"));
    m_allPieces.push_back(std::make_unique<Donkey>(PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-donkey.png"));
    m_allPieces.push_back(std::make_unique<Donkey>(PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-donkey.png"));
    m_allPieces.push_back(std::make_unique<Donkey>(PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-donkey.png"));

    // AI pieces
    m_allPieces.push_back(std::make_unique<Frog>(PieceOwner::AI, "ASSETS\\IMAGES\\red-frog.png"));
    m_allPieces.push_back(std::make_unique<Snake>(PieceOwner::AI, "ASSETS\\IMAGES\\red-snake.png"));
    m_allPieces.push_back(std::make_unique<Donkey>(PieceOwner::AI, "ASSETS\\IMAGES\\red-donkey.png"));
    m_allPieces.push_back(std::make_unique<Donkey>(PieceOwner::AI, "ASSETS\\IMAGES\\red-donkey.png"));
    m_allPieces.push_back(std::make_unique<Donkey>(PieceOwner::AI, "ASSETS\\IMAGES\\red-donkey.png"));

    // Give the player and ai their pieces from the total pool
    for (size_t i = 0; i < numPieces; i++) {
        m_playerPieces.push_back(m_allPieces[i].get());
    }
    for (size_t i = numPieces; i < totalPieces; i++) {
        m_aiPieces.push_back(m_allPieces[i].get());
    }

    //position player pieces in selection grid
    for (int i = 0; i < numPieces; i++)
    {
        //get cell position and set piece position
        m_playerPieces[i]->setGridPosition(0, i); //which grid cell piece is in
        sf::RectangleShape* cell = m_board.getPieceSelectionCell(0, i); //gt visual cell
        if (cell != nullptr)
        {
            sf::Vector2f cellPos = cell->getPosition();
            m_playerPieces[i]->setPosition(cellPos.x, cellPos.y);
        }
    }

    //position AI pieces in selection grid
    for (int i = 0; i < numPieces; i++)
    {
        //get cell position and set piece position
        m_aiPieces[i]->setGridPosition(1, i);
        sf::RectangleShape* cell = m_board.getPieceSelectionCell(1, i);
        if (cell != nullptr)
        {
            sf::Vector2f cellPos = cell->getPosition();
            m_aiPieces[i]->setPosition(cellPos.x, cellPos.y);
        }
    }

    std::cout << "pieces loaded" << std::endl;
}

bool Game::validateGame() {
    int numPieces = 5;
    if (m_playerPieces.size() != numPieces || m_aiPieces.size() != numPieces){
        std::cout << "Invalid piece count" << std::endl;
        return false;
    }

    std::cout << "Game initialized successfully" << std::endl;
    return true;
}

void Game::handleMouseClick(int mouseX, int mouseY)
{
    if (m_currentTurn != PieceOwner::PLAYER) //only allow clicks on player turn
    {
        return;
    }

    GamePhase phase = m_gameState.getCurrentPhase();

    if (phase == GamePhase::PLACEMENT)
    {
        handlePlacementPhase(mouseX, mouseY);
    }
    else if (phase == GamePhase::MOVEMENT)
    {
        handleMovementPhase(mouseX, mouseY);
    }
}

void Game::handlePlacementPhase(int mouseX, int mouseY)
{
    if (!m_selectedPiece) // No piece selected
    {
        if (m_board.isInSelectionGrid(mouseX, mouseY))
        {
            GridPos pos = m_board.screenToSelectionGrid(mouseX, mouseY);
            selectPieceFromSelectionGrid(pos);
        }
    }
    else // Piece selected
    {
        if (m_board.isInGameBoard(mouseX, mouseY))
        {
            GridPos pos = m_board.screenToGameBoard(mouseX, mouseY);

            // Validate placement using GameState
            if (m_gameState.isValidPlacement(pos.x, pos.y))
            {
                placePieceInPlacementPhase(pos);
            }
            else
            {
                std::cout << "Invalid placement - space occupied!" << std::endl;
            }
        }
        else
        {
            clearAllHighlights();
            m_selectedPiece = nullptr;
        }
    }
}

void Game::handleMovementPhase(int mouseX, int mouseY)
{
    if (!m_selectedPiece) // No piece selected
    {
        if (m_board.isInGameBoard(mouseX, mouseY))
        {
            GridPos pos = m_board.screenToGameBoard(mouseX, mouseY);
            selectPieceFromGameBoard(pos);
        }
    }
    else // Piece selected
    {
        if (m_board.isInGameBoard(mouseX, mouseY))
        {
            GridPos pos = m_board.screenToGameBoard(mouseX, mouseY);
            movePiece(pos);
        }
        else
        {
            clearAllHighlights();
            m_selectedPiece = nullptr;
        }
    }
}

void Game::selectPieceFromSelectionGrid(GridPos pos)
{
    clearAllHighlights();

    if (pos.x != 0) //only select pieces from player column
    {
        return;
    }

    for (Piece* piece : m_playerPieces)
    {
        if (piece->getGridCol() == pos.x && piece->getGridRow() == pos.y && piece->getGridCol() <= 1) //still in selection grid
        {
            m_selectedPiece = piece;

            sf::RectangleShape* cell = m_board.getPieceSelectionCell(pos.x, pos.y); //using hash to get cell to highlight
            if (cell)
            {
                cell->setFillColor(sf::Color::Yellow);
            }

            highlightValidPlacements();

            std::cout << "clicked selection grid piece at (" << pos.x << ", " << pos.y << ")" << std::endl;
            return;
        }
    }
}

void Game::selectPieceFromGameBoard(GridPos pos)
{
    clearAllHighlights();

    Piece* clickedPiece = m_gameState.getPieceAt(pos.x, pos.y);

    if (clickedPiece && clickedPiece->getOwner() == PieceOwner::PLAYER)
    {
        m_selectedPiece = clickedPiece;

        sf::RectangleShape* cell = m_board.getGameBoardCell(pos.x, pos.y);
        if (cell)
        {
            cell->setFillColor(sf::Color::Yellow);
        }

        highlightValidMoves();

        std::cout << "Selected piece from board at (" << pos.x << ", " << pos.y << ")" << std::endl;
    }
}

void Game::placePieceInPlacementPhase(GridPos pos)
{
    if (!m_selectedPiece)
    {
        return;
    }

    sf::RectangleShape* cell = m_board.getGameBoardCell(pos.x, pos.y);
    if (cell)
    {
        sf::Vector2f cellPos = cell->getPosition();
        m_selectedPiece->setPosition(cellPos.x, cellPos.y);

        // Update game state
        m_gameState.applyPlacement(pos.x, pos.y, m_selectedPiece);
        m_playerPiecesPlaced++;

        std::cout << "Placed piece at (" << pos.x << ", " << pos.y << ")" << std::endl;

        clearAllHighlights();
        m_selectedPiece = nullptr;


        // FOR TESTING: Switch to movement phase after player places all pieces
        // Remove the AI check for now
        // Check if placement phase is over
        if (m_playerPiecesPlaced >= 5)
        {
            m_gameState.setPhase(GamePhase::MOVEMENT);
            std::cout << "\n=== Movement Phase Started ===" << std::endl;
            std::cout << "You can now click and move your green pieces!" << std::endl;
        }

        /*if (m_playerPiecesPlaced >= 5 && m_aiPiecesPlaced >= 5)
        {
            m_gameState.setPhase(GamePhase::MOVEMENT);
            std::cout << "\n=== Movement Phase Started ===" << std::endl;
        }*/



        switchTurn();
    }
}

void Game::movePiece(GridPos pos)
{
    if (!m_selectedPiece)
    {
        return;
    }

    int fromCol = m_selectedPiece->getGridCol();
    int fromRow = m_selectedPiece->getGridRow();

    Move move(fromCol, fromRow, pos.x, pos.y, m_selectedPiece);

    if (m_gameState.isValidMove(move))
    {
        // Update visual position
        sf::RectangleShape* cell = m_board.getGameBoardCell(pos.x, pos.y);
        if (cell)
        {
            sf::Vector2f cellPos = cell->getPosition();
            m_selectedPiece->setPosition(cellPos.x, cellPos.y);
        }

        // Update game state
        m_gameState.applyMove(move);

        std::cout << "Moved piece from (" << fromCol << "," << fromRow
            << ") to (" << pos.x << "," << pos.y << ")" << std::endl;

        clearAllHighlights();
        m_selectedPiece = nullptr;
        switchTurn();
    }
    else
    {
        std::cout << "Invalid move!" << std::endl;
    }
}

void Game::highlightValidPlacements()
{
    // Highlight all empty spaces on the game board
    for (int col = 0; col < 5; col++)
    {
        for (int row = 0; row < 5; row++)
        {
            if (m_gameState.isValidPlacement(col, row))
            {
                sf::RectangleShape* cell = m_board.getGameBoardCell(col, row);
                if (cell)
                {
                    cell->setFillColor(sf::Color(100, 255, 100, 150)); // Light green
                }
            }
        }
    }
}

void Game::highlightValidMoves()
{
    if (!m_selectedPiece)
    {
        return;
    }

    int fromCol = m_selectedPiece->getGridCol();
    int fromRow = m_selectedPiece->getGridRow();

    // Check all possible destination squares
    for (int toCol = 0; toCol < 5; toCol++)
    {
        for (int toRow = 0; toRow < 5; toRow++)
        {
            Move move(fromCol, fromRow, toCol, toRow, m_selectedPiece);
            if (m_gameState.isValidMove(move))
            {
                sf::RectangleShape* cell = m_board.getGameBoardCell(toCol, toRow);
                if (cell)
                {
                    cell->setFillColor(sf::Color(100, 255, 100, 150)); // Light green
                }
            }
        }
    }
}

void Game::clearAllHighlights()
{
    for (int col = 0; col < 2; col++) //clear selection grid highlights
    {
        for (int row = 0; row < 5; row++)
        {
            sf::RectangleShape* cell = m_board.getPieceSelectionCell(col, row);
            if (cell)
            {
                cell->setFillColor(sf::Color(80, 80, 80));
            }
        }
    }

    for (int col = 0; col < 5; col++) //clear game board highlights
    {
        for (int row = 0; row < 5; row++)
        {
            sf::RectangleShape* cell = m_board.getGameBoardCell(col, row);
            if (cell)
            {
                cell->setFillColor(sf::Color(60, 60, 60));
            }
        }
    }
}

void Game::switchTurn()
{
    if (m_currentTurn == PieceOwner::PLAYER)
    {
        m_currentTurn = PieceOwner::AI;
        std::cout << "\nAI's Turn" << std::endl;
    }
    else
    {
        m_currentTurn = PieceOwner::PLAYER;
        std::cout << "\nPlayer Turn" << std::endl;
    }
}
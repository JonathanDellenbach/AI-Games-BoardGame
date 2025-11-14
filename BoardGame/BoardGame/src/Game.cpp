#include "Game.h"
#include <iostream>

Game::Game() :
    window{ sf::VideoMode{ sf::Vector2u{1920U, 1080U}, 32U }, "Board Game" },
    m_gameValid(false),
    m_selectedPiece(nullptr),
    m_selectedPieceIndex(-1),
    m_pieceSelected(false)
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

    // AI turn
    if (m_currentTurn == PieceOwner::AI)
    {
        sf::sleep(sf::seconds(0.3f));
        std::cout << "AI skips turn" << std::endl;
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
    m_playerPieces.reserve(3);
    m_aiPieces.reserve(3);

    // Player pieces
    //m_allPieces.push_back(std::make_unique<Frog>(PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-frog.png"));
   // m_allPieces.push_back(std::make_unique<Snake>(PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-snake.png"));
    m_allPieces.push_back(std::make_unique<Donkey>(PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-donkey.png"));
    m_allPieces.push_back(std::make_unique<Donkey>(PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-donkey.png"));
    m_allPieces.push_back(std::make_unique<Donkey>(PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-donkey.png"));

    // AI pieces
    //m_allPieces.push_back(std::make_unique<Frog>(PieceOwner::AI, "ASSETS\\IMAGES\\red-frog.png"));
    //m_allPieces.push_back(std::make_unique<Snake>(PieceOwner::AI, "ASSETS\\IMAGES\\red-snake.png"));
    m_allPieces.push_back(std::make_unique<Donkey>(PieceOwner::AI, "ASSETS\\IMAGES\\red-donkey.png"));
    m_allPieces.push_back(std::make_unique<Donkey>(PieceOwner::AI, "ASSETS\\IMAGES\\red-donkey.png"));
    m_allPieces.push_back(std::make_unique<Donkey>(PieceOwner::AI, "ASSETS\\IMAGES\\red-donkey.png"));

    // NOTE : REFACOTR BELOW TO BE MORE TYPE SAFE ON VECTOR SIZES IN THE FUTURE

    // Give the player and ai their pieces from the total pool
    for (size_t i = 0; i < 3; i++) {
        m_playerPieces.push_back(m_allPieces[i].get());
    }
    for (size_t i = 3; i < 6; i++) {
        m_aiPieces.push_back(m_allPieces[i].get());
    }

    //position player pieces in selection grid
    for (int i = 0; i < 3; i++)
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
    for (int i = 0; i < 3; i++)
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
    if (m_playerPieces.size() != 3 || m_aiPieces.size() != 3) {
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

    if (!m_selectedPiece) //no piece selected
    {
        if (m_board.isInSelectionGrid(mouseX, mouseY))
        {
            GridPos pos = m_board.screenToSelectionGrid(mouseX, mouseY);
            selectPieceFromSelectionGrid(pos);
        }
        else if (m_board.isInGameBoard(mouseX, mouseY))
        {
            GridPos pos = m_board.screenToGameBoard(mouseX, mouseY);
            selectPieceFromGameBoard(pos);
        }
    }
    
    else //piece selected
    {
        if (m_board.isInGameBoard(mouseX, mouseY)) // clicked in board
        {
            GridPos pos = m_board.screenToGameBoard(mouseX, mouseY);
            placePiece(pos);
        }
        else //clicked off board
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

            std::cout << "clicked selection grid piece at (" << pos.x << ", " << pos.y << ")" << std::endl;
            return;
        }
    }
}

void Game::selectPieceFromGameBoard(GridPos pos)
{
    clearAllHighlights();

    for (Piece* piece : m_playerPieces)
    {
        sf::RectangleShape* clickedCell = m_board.getGameBoardCell(pos.x, pos.y); //get clicked cell
        if (!clickedCell)
        {
            return;
        }

        sf::Vector2f clickedCellPos = clickedCell->getPosition(); //get top left of cell
        sf::Vector2f piecePos = piece->getPosition();

        //get center of clicked cell adding 50 on the x and y
        float distanceX = abs(piecePos.x - (clickedCellPos.x + 50.0f));
        float distanceY = abs(piecePos.y - (clickedCellPos.y + 50.0f));

        if (distanceX < 10.0f && distanceY < 10.0f)
        {
            m_selectedPiece = piece;
            clickedCell->setFillColor(sf::Color::Yellow);

            std::cout << "clciked game board grid piece at (" << pos.x << ", " << pos.y << ")" << std::endl;
            return;
        }
    }
}

void Game::placePiece(GridPos pos)
{
    if (!m_selectedPiece) //nothing selected cant place anything
    {
        return;
    }

    sf::RectangleShape* cell = m_board.getGameBoardCell(pos.x, pos.y); //get clicked cell
    if (cell)
    {
        sf::Vector2f cellPos = cell->getPosition();
        m_selectedPiece->setPosition(cellPos.x, cellPos.y); //place piece in cells position
        m_selectedPiece->setGridPosition(pos.x, pos.y); //sets grid position like (3,4)

        std::cout << "Placed piece at (" << pos.x << ", " << pos.y << ")" << std::endl;

        clearAllHighlights();

        m_selectedPiece = nullptr; //nothign is selected anymore
        switchTurn();
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
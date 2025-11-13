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
        sf::RectangleShape* cell = m_board.getPieceSelectionCell(0, i);
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

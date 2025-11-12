#include "Game.h"
#include <iostream>

Game::Game() :
    window{ sf::VideoMode{ sf::Vector2u{1920U, 1080U}, 32U }, "Board Game" },
    m_gameValid(false)
{
    m_gameValid = validateEntities();
    initializePieces();
}

Game::~Game()
{
}

bool Game::validateEntities()
{
    if (!m_player.isValid())
    {
        std::cout << "Failed to load player!" << std::endl;
        return false;
    }
    if (!m_enemy.isValid())
    {
        std::cout << "Failed to load enemy!" << std::endl;
        return false;
    }

    std::cout << "All entities loaded successfully!" << std::endl;
    return true;
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

    m_player.update(t_deltaTime);
    m_enemy.update(t_deltaTime);
}

void Game::render()
{
    window.clear(sf::Color::Black);

    m_board.render(window);

    //Player pieces
    for (auto& piece : m_playerPieces)
    {
        piece.render(window);
    }

    //AI pieces
    for (auto& piece : m_aiPieces)
    {
        piece.render(window);
    }

    //m_player.render(window);
    //m_enemy.render(window);

    window.display();
}

void Game::initializePieces()
{
    m_playerPieces.reserve(5);
    m_aiPieces.reserve(5);

    // Player pieces
    m_playerPieces.emplace_back(PieceType::FROG, PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-frog.png");
    m_playerPieces.emplace_back(PieceType::SNAKE, PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-snake.png");
    m_playerPieces.emplace_back(PieceType::DONKEY, PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-donkey.png");
    m_playerPieces.emplace_back(PieceType::DONKEY, PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-donkey.png");
    m_playerPieces.emplace_back(PieceType::DONKEY, PieceOwner::PLAYER, "ASSETS\\IMAGES\\green-donkey.png");

    // AI pieces
    m_aiPieces.emplace_back(PieceType::FROG, PieceOwner::AI, "ASSETS\\IMAGES\\red-frog.png");
    m_aiPieces.emplace_back(PieceType::SNAKE, PieceOwner::AI, "ASSETS\\IMAGES\\red-snake.png");
    m_aiPieces.emplace_back(PieceType::DONKEY, PieceOwner::AI, "ASSETS\\IMAGES\\red-donkey.png");
    m_aiPieces.emplace_back(PieceType::DONKEY, PieceOwner::AI, "ASSETS\\IMAGES\\red-donkey.png");
    m_aiPieces.emplace_back(PieceType::DONKEY, PieceOwner::AI, "ASSETS\\IMAGES\\red-donkey.png");

    //position player pieces in selection grid
    for (int i = 0; i < 5; i++)
    {
        m_playerPieces[i].setGridPosition(0, i);
        //get cell position and set piece position
        sf::RectangleShape* cell = m_board.getPieceSelectionCell(0, i);
        if (cell != nullptr)
        {
            sf::Vector2f cellPos = cell->getPosition();
            m_playerPieces[i].setPosition(cellPos.x, cellPos.y);
        }
    }

    //position AI pieces in selection grid
    for (int i = 0; i < 5; i++)
    {
        m_aiPieces[i].setGridPosition(1, i);
        //get cell position and set piece position
        sf::RectangleShape* cell = m_board.getPieceSelectionCell(1, i);
        if (cell != nullptr)
        {
            sf::Vector2f cellPos = cell->getPosition();
            m_aiPieces[i].setPosition(cellPos.x, cellPos.y);
        }
    }

    std::cout << "pieces loaded" << std::endl;
}
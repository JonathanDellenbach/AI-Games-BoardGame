#include "Game.h"
#include <iostream>

Game::Game() :
    window{ sf::VideoMode{ sf::Vector2u{1920U, 1080U}, 32U }, "Board Game" },
    m_gameValid(false),
    m_selectedPiece(nullptr),
    m_selectedPieceIndex(-1),
    m_pieceSelected(false),
    m_playerPiecesPlaced(0),
    m_aiPiecesPlaced(0),
    m_gameMode(GameMode::PLAYER_VS_AI),
    m_modeSelected(false),
    m_aiMoveDelay(0.5f),
    m_waitingForNextMove(false)
{
    if (!font.openFromFile("ASSETS/FONTS/Jersey20-Regular.ttf")) 
    {
        std::cout << "Error loading font!" << std::endl;
    }

    initializePieces();
    m_gameValid = validateGame();


    m_gameState.clearPositionHistory();

    std::cout << "Player Turn" << std::endl;
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
    while (const std::optional newEvent = window.pollEvent()) {
        if (newEvent->is<sf::Event::Closed>()) {
            exitGame = true;
        }
        if (newEvent->is<sf::Event::KeyPressed>()) {
            processKeys(newEvent);
        }
        if (m_modeSelected && m_gameMode == GameMode::PLAYER_VS_AI)
        {
            if (newEvent->is<sf::Event::MouseButtonPressed>()) {
                auto* mouseEvent = newEvent->getIf<sf::Event::MouseButtonPressed>();
                if (mouseEvent && mouseEvent->button == sf::Mouse::Button::Left) {
                    handleMouseClick(mouseEvent->position.x, mouseEvent->position.y);
                }
                else if (mouseEvent && mouseEvent->button == sf::Mouse::Button::Right) {
                    m_selectedPiece = nullptr;
                    clearAllHighlights();
                }
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

    if (!m_modeSelected)
    {
        if (sf::Keyboard::Key::Num1 == newKeypress->code)
        {
            m_gameMode = GameMode::PLAYER_VS_AI;
            m_modeSelected = true;
            std::cout << "Player vs AI mode selected!" << std::endl;
            std::cout << "Player Turn" << std::endl;
        }
        else if (sf::Keyboard::Key::Num2 == newKeypress->code)
        {
            m_gameMode = GameMode::AI_VS_AI;
            m_modeSelected = true;
            std::cout << "AI vs AI mode selected!" << std::endl;
            std::cout << "Player Turn" << std::endl;
        }
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

    if (!m_modeSelected)
    {
        return;
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

    if (m_gameState.getCurrentPhase() == GamePhase::GAME_OVER) //check turns depending on mode
    { 
        return;
    }

    if (m_gameMode == GameMode::AI_VS_AI) 
    {
        if (!m_waitingForNextMove) //Time based bool. Down below in the "else" the timer counts
        {
            //play current Player AIs move
            if (m_currentTurn == PieceOwner::PLAYER) 
            {
                if (m_gameState.getCurrentPhase() == GamePhase::PLACEMENT) 
                {
                    executePlayerAIPlacement();
                }
                else if (m_gameState.getCurrentPhase() == GamePhase::MOVEMENT) 
                {
                    executePlayerAIMove();
                }
            }
            else //AI turn
            { 
                if (m_gameState.getCurrentPhase() == GamePhase::PLACEMENT) 
                {
                    executeAIPlacement();
                }
                else if (m_gameState.getCurrentPhase() == GamePhase::MOVEMENT) 
                {
                    executeAIMove();
                }
            }

            switchTurn();
            m_waitingForNextMove = true;
            m_aiMoveTimer.restart(); //start timer 
        }
        else //wait for timer to finish then move
        { 
            if (m_aiMoveTimer.getElapsedTime().asSeconds() >= m_aiMoveDelay) 
            {
                m_waitingForNextMove = false;
            }
        }
    }
    else // AI turn
    {
        if (m_currentTurn == PieceOwner::AI && m_gameState.getCurrentPhase() != GamePhase::GAME_OVER) {
            sf::sleep(sf::seconds(0.25f));

            if (m_gameState.getCurrentPhase() == GamePhase::PLACEMENT) {
                executeAIPlacement();
            }
            else if (m_gameState.getCurrentPhase() == GamePhase::MOVEMENT) {
                executeAIMove();
            }

            switchTurn();
        }
    }
}

void Game::executeAIPlacement()
{
    if (m_aiPiecesPlaced >= 5) return;

    Piece* pieceToPlace = m_aiPieces[m_aiPiecesPlaced];
    auto placement = m_ai.findBestPlacement(m_gameState, pieceToPlace); // Just a basic heuristc, no need to use full search when board not full

    if (placement.first >= 0 && placement.second >= 0) {
        sf::RectangleShape* cell = m_board.getGameBoardCell(placement.first, placement.second);
        if (cell) {
            sf::Vector2f cellPos = cell->getPosition();
            pieceToPlace->setPosition(cellPos.x, cellPos.y);
            m_gameState.applyPlacement(placement.first, placement.second, pieceToPlace);
            m_aiPiecesPlaced++;

            std::cout << "AI placed piece at (" << placement.first << ", " << placement.second << ")" << std::endl;

            // Change the phase
            if (m_playerPiecesPlaced >= 5 && m_aiPiecesPlaced >= 5) {
                m_gameState.setPhase(GamePhase::MOVEMENT);
                std::cout << "\n=== Movement Phase Started ===" << std::endl;
            }
        }
    }
}

void Game::executeAIMove()
{
    Move aiMove = m_ai.findBestMove(m_gameState, 3);

    if (aiMove.piece) {
        sf::RectangleShape* cell = m_board.getGameBoardCell(aiMove.toCol, aiMove.toRow);
        if (cell) {
            sf::Vector2f cellPos = cell->getPosition();
            aiMove.piece->setPosition(cellPos.x, cellPos.y);
        }
        m_gameState.applyMove(aiMove);

        m_gameState.recordPosition();

        std::cout << "AI moved from (" << aiMove.fromCol << "," << aiMove.fromRow
            << ") to (" << aiMove.toCol << "," << aiMove.toRow << ")" << std::endl;
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

    if (!m_modeSelected)
    {
        renderModeSelection();
    }
    else if (m_gameState.getCurrentPhase() == GamePhase::GAME_OVER)
    {
        renderGameOver();
    }

    window.display();
}

void Game::renderGameOver()
{
    PieceOwner winner = m_gameState.getWinner();

    sf::Text winText(font);
    if (winner == PieceOwner::PLAYER) {
        winText.setString("PLAYER WINS!");
        winText.setFillColor(sf::Color::Green);
    }
    else if (winner == PieceOwner::AI) {
        winText.setString("AI WINS!");
        winText.setFillColor(sf::Color::Red);
    }

    winText.setCharacterSize(60);
    winText.setPosition({ 1100.0f, 450.0f });

    sf::Text resetText(font);
    resetText.setString("Press ESC to exit");
    resetText.setCharacterSize(25);
    resetText.setFillColor(sf::Color::White);
    resetText.setPosition({ 1100.0f, 550.0f });

    window.draw(winText);
    window.draw(resetText);
}

void Game::initializePieces()
{
    constexpr int NUM_PIECES = 5;

    m_playerPieces.reserve(NUM_PIECES);
    m_aiPieces.reserve(NUM_PIECES);


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
    for (size_t i = 0; i < NUM_PIECES; i++) {
        m_playerPieces.push_back(m_allPieces[i].get());
    }
    for (size_t i = NUM_PIECES; i < NUM_PIECES * 2; i++) {
        m_aiPieces.push_back(m_allPieces[i].get());
    }

    // Position pieces in selection grid - use NEGATIVE grid positions to indicate "not on board"
    for (int i = 0; i < NUM_PIECES; i++) {
        m_playerPieces[i]->setGridPosition(-1, i);  // -1 col means "in selection grid"
        sf::RectangleShape* cell = m_board.getPieceSelectionCell(0, i);
        if (cell) {
            sf::Vector2f cellPos = cell->getPosition();
            m_playerPieces[i]->setPosition(cellPos.x, cellPos.y);
        }
    }

    for (int i = 0; i < NUM_PIECES; i++) {
        m_aiPieces[i]->setGridPosition(-2, i);  // -2 col means "AI selection grid"
        sf::RectangleShape* cell = m_board.getPieceSelectionCell(1, i);
        if (cell) {
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
    if (m_currentTurn != PieceOwner::PLAYER) return;

    GamePhase phase = m_gameState.getCurrentPhase();

    if (phase == GamePhase::PLACEMENT) {
        handlePlacementPhase(mouseX, mouseY);
    }
    else if (phase == GamePhase::MOVEMENT) {
        handleMovementPhase(mouseX, mouseY);
    }
}

void Game::handlePlacementPhase(int mouseX, int mouseY)
{
    if (!m_selectedPiece) {
        if (m_board.isInSelectionGrid(mouseX, mouseY)) {
            GridPos pos = m_board.screenToSelectionGrid(mouseX, mouseY);
            selectPieceFromSelectionGrid(pos);
        }
    }
    else {
        if (m_board.isInGameBoard(mouseX, mouseY)) {
            GridPos pos = m_board.screenToGameBoard(mouseX, mouseY);
            if (m_gameState.isValidPlacement(pos.x, pos.y)) {
                placePieceInPlacementPhase(pos);
            }
            else {
                std::cout << "Invalid placement - space occupied!" << std::endl;
            }
        }
        else {
            clearAllHighlights();
            m_selectedPiece = nullptr;
        }
    }
}

void Game::handleMovementPhase(int mouseX, int mouseY)
{
    if (!m_selectedPiece) {
        if (m_board.isInGameBoard(mouseX, mouseY)) {
            GridPos pos = m_board.screenToGameBoard(mouseX, mouseY);
            selectPieceFromGameBoard(pos);
        }
    }
    else {
        if (m_board.isInGameBoard(mouseX, mouseY)) {
            GridPos pos = m_board.screenToGameBoard(mouseX, mouseY);
            movePiece(pos);
        }
        else {
            clearAllHighlights();
            m_selectedPiece = nullptr;
        }
    }
}

void Game::selectPieceFromSelectionGrid(GridPos pos)
{
    clearAllHighlights();
    if (pos.x != 0) return;

    // Find unplaced player piece at this row
    for (Piece* piece : m_playerPieces) {
        bool isPlaced = false;
        for (int col = 0; col < 5 && !isPlaced; col++)
        {
            for (int row = 0; row < 5 && !isPlaced; row++) 
            {
                if (m_gameState.getPieceAt(col, row) == piece) 
                {
                    isPlaced = true;
                }
            }
        }
        
        // Piece is in selection grid if its grid column is negative
        if (piece->getGridCol() < 0 && piece->getGridRow() == pos.y) {
            m_selectedPiece = piece;
            m_selectedPieceIndex = pos.y;

            sf::RectangleShape* cell = m_board.getPieceSelectionCell(pos.x, pos.y);
            if (cell) {
                cell->setFillColor(sf::Color::Yellow);
            }

            highlightValidPlacements();
            std::cout << "Selected piece from selection grid row " << pos.y << std::endl;
            return;
        }
    }
}

void Game::selectPieceFromGameBoard(GridPos pos)
{
    clearAllHighlights();

    Piece* clickedPiece = m_gameState.getPieceAt(pos.x, pos.y);

    if (clickedPiece && clickedPiece->getOwner() == PieceOwner::PLAYER) {
        m_selectedPiece = clickedPiece;

        sf::RectangleShape* cell = m_board.getGameBoardCell(pos.x, pos.y);
        if (cell) {
            cell->setFillColor(sf::Color::Yellow);
        }

        highlightValidMoves();
        std::cout << "Selected piece at (" << pos.x << ", " << pos.y << ")" << std::endl;
    }
}

void Game::placePieceInPlacementPhase(GridPos pos)
{
    if (!m_selectedPiece) return;

    sf::RectangleShape* cell = m_board.getGameBoardCell(pos.x, pos.y);
    if (cell) {
        sf::Vector2f cellPos = cell->getPosition();
        m_selectedPiece->setPosition(cellPos.x, cellPos.y);
        m_gameState.applyPlacement(pos.x, pos.y, m_selectedPiece);
        m_playerPiecesPlaced++;

        std::cout << "Placed piece at (" << pos.x << ", " << pos.y << ")" << std::endl;

        clearAllHighlights();
        m_selectedPiece = nullptr;
        m_selectedPieceIndex = -1;

        // Check if placement phase is over
        if (m_playerPiecesPlaced >= 5 && m_aiPiecesPlaced >= 5) {
            m_gameState.setPhase(GamePhase::MOVEMENT);
            std::cout << "\n=== Movement Phase Started ===" << std::endl;
        }

        switchTurn();
    }
}

void Game::movePiece(GridPos pos)
{
    if (!m_selectedPiece) return;

    int fromCol = m_selectedPiece->getGridCol();
    int fromRow = m_selectedPiece->getGridRow();

    Move move(fromCol, fromRow, pos.x, pos.y, m_selectedPiece);

    if (m_gameState.isValidMove(move)) {
        sf::RectangleShape* cell = m_board.getGameBoardCell(pos.x, pos.y);
        if (cell) {
            sf::Vector2f cellPos = cell->getPosition();
            m_selectedPiece->setPosition(cellPos.x, cellPos.y);
        }

        m_gameState.applyMove(move);

        m_gameState.recordPosition();

        std::cout << "Moved from (" << fromCol << "," << fromRow << ") to (" << pos.x << "," << pos.y << ")" << std::endl;

        clearAllHighlights();
        m_selectedPiece = nullptr;
        switchTurn();
    }
    else {
        std::cout << "Invalid move!" << std::endl;
    }
}

void Game::highlightValidPlacements()
{
    for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 5; row++) {
            if (m_gameState.isValidPlacement(col, row)) {
                sf::RectangleShape* cell = m_board.getGameBoardCell(col, row);
                if (cell) {
                    cell->setFillColor(sf::Color(100, 255, 100, 150));
                }
            }
        }
    }
}

void Game::highlightValidMoves()
{
    if (!m_selectedPiece) return;

    int fromCol = m_selectedPiece->getGridCol();
    int fromRow = m_selectedPiece->getGridRow();

    for (int toCol = 0; toCol < 5; toCol++) {
        for (int toRow = 0; toRow < 5; toRow++) {
            Move move(fromCol, fromRow, toCol, toRow, m_selectedPiece);
            if (m_gameState.isValidMove(move)) {
                sf::RectangleShape* cell = m_board.getGameBoardCell(toCol, toRow);
                if (cell) {
                    cell->setFillColor(sf::Color(100, 255, 100, 150));
                }
            }
        }
    }
}

void Game::clearAllHighlights()
{
    for (int col = 0; col < 2; col++) {
        for (int row = 0; row < 5; row++) {
            sf::RectangleShape* cell = m_board.getPieceSelectionCell(col, row);
            if (cell) {
                cell->setFillColor(sf::Color(80, 80, 80));
            }
        }
    }

    for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 5; row++) {
            sf::RectangleShape* cell = m_board.getGameBoardCell(col, row);
            if (cell) {
                cell->setFillColor(sf::Color(60, 60, 60));
            }
        }
    }
}

void Game::switchTurn()
{
    m_currentTurn = (m_currentTurn == PieceOwner::PLAYER) ? PieceOwner::AI : PieceOwner::PLAYER;
    std::cout << (m_currentTurn == PieceOwner::PLAYER ? "\nPlayer's Turn" : "\nAI's Turn") << std::endl;
}

void Game::renderModeSelection()
{
    sf::Text titleText(font);
    titleText.setString("SELECT GAME MODE");
    titleText.setCharacterSize(40);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition({ 1100.0f, 400.0f });

    sf::Text option1Text(font);
    option1Text.setString("Press 1 for Player VS AI");
    option1Text.setCharacterSize(30);
    option1Text.setFillColor(sf::Color::White);
    option1Text.setPosition({ 1100.0f, 500.0f });

    sf::Text option2Text(font);
    option2Text.setString("Press 2 for AI VS AI");
    option2Text.setCharacterSize(30);
    option2Text.setFillColor(sf::Color::White);
    option2Text.setPosition({ 1100.0f, 560.0f });

    window.draw(titleText);
    window.draw(option1Text);
    window.draw(option2Text);
}

void Game::executePlayerAIPlacement() // Handles Player AI Placement during AI vs AI
{
    if (m_playerPiecesPlaced >= 5) return;

    Piece* pieceToPlace = m_playerPieces[m_playerPiecesPlaced]; //get unplaced piece
    auto placement = m_playerAI.findBestPlacement(m_gameState, pieceToPlace); //find best place

    if (placement.first >= 0 && placement.second >= 0) //verify placement
    {
        sf::RectangleShape* cell = m_board.getGameBoardCell(placement.first, placement.second);
        if (cell) 
        {
            sf::Vector2f cellPos = cell->getPosition();
            pieceToPlace->setPosition(cellPos.x, cellPos.y);
            m_gameState.applyPlacement(placement.first, placement.second, pieceToPlace);
            m_playerPiecesPlaced++;

            std::cout << "Player AI placed piece at (" << placement.first << ", " << placement.second << ")" << std::endl;

            if (m_playerPiecesPlaced >= 5 && m_aiPiecesPlaced >= 5) //if all pieces are placed move to movement phase
            {
                m_gameState.setPhase(GamePhase::MOVEMENT);
                std::cout << "\n=== Movement Phase Started ===" << std::endl;
            }
        }
    }
}

void Game::executePlayerAIMove()
{
    Move playerAIMove = m_playerAI.findBestMove(m_gameState, 3);

    if (playerAIMove.piece)
    {
        sf::RectangleShape* cell = m_board.getGameBoardCell(playerAIMove.toCol, playerAIMove.toRow);
        if (cell)
        {
            sf::Vector2f cellPos = cell->getPosition();
            playerAIMove.piece->setPosition(cellPos.x, cellPos.y);
        }
        m_gameState.applyMove(playerAIMove);

        m_gameState.recordPosition();

        std::cout << "Player AI moved from (" << playerAIMove.fromCol << "," << playerAIMove.fromRow
            << ") to (" << playerAIMove.toCol << "," << playerAIMove.toRow << ")" << std::endl;
    }
}
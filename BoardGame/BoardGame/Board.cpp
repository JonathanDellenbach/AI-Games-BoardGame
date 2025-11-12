#include "Board.h"
#include <iostream>

Board::Board()
{
    initializePieceSelectionGrid();
    initializeGameBoard();
}

Board::~Board()
{
}

void Board::initializePieceSelectionGrid() //2x5 grid for pieces
{
    for (int col = 0; col < 2; col++)
    {
        for (int row = 0; row < 5; row++)
        {
            GridPos pos{ col, row };
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));

            cell.setPosition({ GRID_OFFSET_X + col * CELL_SIZE, GRID_OFFSET_Y + row * CELL_SIZE });

            cell.setFillColor(sf::Color(80, 80, 80));
            cell.setOutlineColor(sf::Color::White);
            cell.setOutlineThickness(2);

            pieceSelectionGrid[pos] = cell;
        }
    }

    //std::cout << "selection grid: " << pieceSelectionGrid.size() << " cells" << std::endl;
}

void Board::initializeGameBoard() //5x5 grid for game
{
    for (int col = 0; col < 5; col++)
    {
        for (int row = 0; row < 5; row++)
        {
            GridPos pos{ col, row };
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));

            cell.setPosition({ BOARD_OFFSET_X + col * CELL_SIZE, GRID_OFFSET_Y + row * CELL_SIZE });

            cell.setFillColor(sf::Color(60, 60, 60));
            cell.setOutlineColor(sf::Color::White);
            cell.setOutlineThickness(2);

            gameBoard[pos] = cell;
        }
    }

    //std::cout << "board: " << gameBoard.size() << " tiles" << std::endl;
}

void Board::render(sf::RenderWindow& window)
{
    //piece selection grid
    for (int col = 0; col < 2; col++)
    {
        for (int row = 0; row < 5; row++)
        {
            GridPos pos{ col, row };
            if (pieceSelectionGrid.find(pos) != pieceSelectionGrid.end())
            {
                window.draw(pieceSelectionGrid[pos]);
            }
        }
    }

    //game board grid
    for (int col = 0; col < 5; col++)
    {
        for (int row = 0; row < 5; row++)
        {
            GridPos pos{ col, row };
            if (gameBoard.find(pos) != gameBoard.end())
            {
                window.draw(gameBoard[pos]);
            }
        }
    }
}

sf::RectangleShape* Board::getPieceSelectionCell(int col, int row)
{
    GridPos pos{ col, row };
    auto it = pieceSelectionGrid.find(pos);
    if (it != pieceSelectionGrid.end())
    {
        return &(it->second);
    }
    return nullptr;
}

sf::RectangleShape* Board::getGameBoardCell(int col, int row)
{
    GridPos pos{ col, row };
    auto it = gameBoard.find(pos);
    if (it != gameBoard.end())
    {
        return &(it->second);
    }
    return nullptr;
}
#ifndef BOARD_HPP
#define BOARD_HPP

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

//Mental reminder Board.h/.cpp is for drawing board game cells and managing positions/grid logic
// 
//grid position
struct GridPos
{
    int x;
    int y;

    // check if two gridPosisitons are equal in case hashmap has same num
    bool operator==(const GridPos& other) const
    {
        return x == other.x && y == other.y;
    }
};


namespace std //covert GridPosition to unique num
{
    template <>
    struct hash<GridPos>
    {
        size_t operator()(const GridPos& pos) const //unique num
        {
            return hash<int>()(pos.x) ^ (hash<int>()(pos.y) << 1);
        }
    };
}

class Board
{
public:
    Board();
    ~Board();

    void render(sf::RenderWindow& window);

    void initializePieceSelectionGrid();
    void initializeGameBoard();

    sf::RectangleShape* getPieceSelectionCell(int col, int row);
    sf::RectangleShape* getGameBoardCell(int col, int row);

    //mouse
    GridPos screenToSelectionGrid(int mouseX, int mouseY) const; //convert pixel coord to selection grid pos
    GridPos screenToGameBoard(int mouseX, int mouseY) const; // convert pixel coord to board grid pos
    bool isInSelectionGrid(int mouseX, int mouseY) const; //ensures the click its inside of the selection grid
    bool isInGameBoard(int mouseX, int mouseY) const; //ensures the click its inside of the boaord grid

private:

    //grid setup
    const float CELL_SIZE = 100.0f;
    const float GRID_OFFSET_X = 50.0f;
    const float GRID_OFFSET_Y = 100.0f;
    const float BOARD_OFFSET_X = 400.0f;

    std::unordered_map<GridPos, sf::RectangleShape> pieceSelectionGrid;
    std::unordered_map<GridPos, sf::RectangleShape> gameBoard;
};

#endif
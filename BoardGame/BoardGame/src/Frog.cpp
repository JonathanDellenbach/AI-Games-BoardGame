#include "Frog.h"
#include "GameState.h"
#include <cmath>

Frog::Frog(PieceOwner owner, const std::string& texturePath)
    : Piece(PieceType::FROG, owner, texturePath) {
}

Frog::~Frog()
{
}

bool Frog::isValidMove(const GameState& state, int fromCol,
                        int fromRow, int toCol, int toRow) const
{
    int deltaCol = toCol - fromCol;
    int deltaRow = toRow - fromRow;

    // Can move one space in any direction
    if (std::abs(deltaCol) <= 1 && std::abs(deltaRow) <= 1) {
        if (deltaCol != 0 || deltaRow != 0) {
            return true;
        }
    }

    // If the above doesnt pass check if the frog can jump. 
    return canJumpTo(state, fromCol, fromRow, toCol, toRow);
}

bool Frog::canJumpTo(const GameState& state, int fromCol, int fromRow,
    int toCol, int toRow) const
{
    int deltaCol = toCol - fromCol;
    int deltaRow = toRow - fromRow;

    // Must move more than 1 square 
    if (std::abs(deltaCol) <= 1 && std::abs(deltaRow) <= 1) {
        return false;
    }

    // Must be in a straight line (horizontal, vertical, or diagonal)
    bool isHorizontal = (deltaRow == 0 && deltaCol != 0);
    bool isVertical = (deltaCol == 0 && deltaRow != 0);
    bool isDiagonal = (std::abs(deltaCol) == std::abs(deltaRow));

    if (!isHorizontal && !isVertical && !isDiagonal) {
        return false;
    }

    // Get direction of movement
    int dirCol = (deltaCol == 0) ? 0 : (deltaCol > 0 ? 1 : -1);
    int dirRow = (deltaRow == 0) ? 0 : (deltaRow > 0 ? 1 : -1);

    // The first square must have a piece to jump over
    int firstCol = fromCol + dirCol;
    int firstRow = fromRow + dirRow;

    if (firstCol < 0 || firstCol >= 5 || firstRow < 0 || firstRow >= 5) {
        return false;
    }

    if (state.isPositionEmpty(firstCol, firstRow)) {
        return false; // No adjacent piece to jump over
    }

    // Walk from the adjacent piece towards destination
    int currentCol = firstCol + dirCol;
    int currentRow = firstRow + dirRow;

    bool passedEmptySquare = false;

    while (currentCol != toCol || currentRow != toRow) {
        // Bounds check
        if (currentCol < 0 || currentCol >= 5 || currentRow < 0 || currentRow >= 5) {
            return false;
        }

        if (state.isPositionEmpty(currentCol, currentRow)) {
            passedEmptySquare = true;
        }
        else {
            // Found another piece
            if (passedEmptySquare) {
                // Cannot jump over a piece that's after an empty square
                return false;
            }
           
        }
        // Continue jumping
        currentCol += dirCol;
        currentRow += dirRow;
    }
    return !passedEmptySquare;
}

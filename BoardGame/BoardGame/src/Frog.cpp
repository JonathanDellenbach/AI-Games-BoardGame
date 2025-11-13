#include "Frog.h"
#include "GameState.h"
#include <cmath>

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

bool Frog::canJumpTo(const GameState& state, int fromCol, int fromRow, int toCol, int toRow) const
{
    int deltaCol = toCol - fromCol;
    int deltaRow = toRow - fromRow;

    // Must be in a straight line (horizontal, vertical, or diagonal)
    if (deltaCol != 0 && deltaRow != 0 && std::abs(deltaCol) != std::abs(deltaRow)) {
        return false; // Not a straight line
    }

    // Get direction
    int dirCol = 0;
    int dirRow = 0;

    if (deltaCol != 0) dirCol = deltaCol / std::abs(deltaCol); // Which direction is the frog going via Col
    if (deltaRow != 0) dirRow = deltaRow / std::abs(deltaRow); // Which direction is the frog going via Row

    // Check if there's at least one piece to jump over
    bool hasJumpedOverPiece = false;
    int currentCol = fromCol + dirCol; // The piece being jumped over coords
    int currentRow = fromRow + dirRow;

    while (currentCol != toCol || currentRow != toRow) { // Not at destination
        // Out of bounds check
        if (currentCol < 0 || currentCol >= 5 || currentRow < 0 || currentRow >= 5) {
            return false;
        }

        if (!state.isPositionEmpty(currentCol, currentRow)) {
            hasJumpedOverPiece = true;
        }

        // Move to next position in the path
        currentCol += dirCol;
        currentRow += dirRow;
    }

    return hasJumpedOverPiece;
}

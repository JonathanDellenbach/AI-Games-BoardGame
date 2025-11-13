#include "Snake.h"

Snake::Snake(PieceOwner owner, const std::string& texturePath)
    : Piece(PieceType::SNAKE, owner, texturePath) {
}

Snake::~Snake() {
}

bool Snake::isValidMove(const GameState& state, int fromCol, int fromRow,
    int toCol, int toRow) const {
    int deltaCol = toCol - fromCol;
    int deltaRow = toRow - fromRow;

    // Snake can move one space in any direction (cardinal + diagonal)
    // Valid moves: any combination where abs(delta) <= 1 for both axes
    // and at least one axis moves

    if (std::abs(deltaCol) <= 1 && std::abs(deltaRow) <= 1) {
        // Must actually move (not stay in same place)
        if (deltaCol != 0 || deltaRow != 0) {
            return true;
        }
    }

    return false;
}
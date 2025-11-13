#include "Donkey.h"
#include "GameState.h"

Donkey::Donkey(PieceOwner owner, const std::string& texturePath)
    : Piece(PieceType::DONKEY, owner, texturePath) {
}

Donkey::~Donkey() {
}

bool Donkey::isValidMove(const GameState& state, int fromCol, int fromRow,
    int toCol, int toRow) const {
    // Calculate movement delta
    int deltaCol = toCol - fromCol;
    int deltaRow = toRow - fromRow;

    // Donkey can only move one space in cardinal directions
    // Valid moves: (0,1), (0,-1), (1,0), (-1,0)

    if (deltaCol == 0 && deltaRow == 1) return true;  // Down
    if (deltaCol == 0 && deltaRow == -1) return true; // Up
    if (deltaCol == 1 && deltaRow == 0) return true;  // Right
    if (deltaCol == -1 && deltaRow == 0) return true; // Left

    return false;
}
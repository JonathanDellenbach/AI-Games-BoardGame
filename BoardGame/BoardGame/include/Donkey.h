#pragma once
#include "Piece.h"

class Donkey : public Piece {
public:
    Donkey(PieceOwner owner, const std::string& texturePath);
    virtual ~Donkey();

    // Override movement validation
    virtual bool isValidMove(const GameState& state, int fromCol, int fromRow,
        int toCol, int toRow) const override;
};


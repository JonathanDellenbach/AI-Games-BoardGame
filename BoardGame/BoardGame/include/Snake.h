#pragma once
#include "Piece.h"

class Snake : public Piece {
public:
    Snake(PieceOwner owner, const std::string& texturePath);
    virtual ~Snake();

    virtual bool isValidMove(const GameState& state, int fromCol, int fromRow,
        int toCol, int toRow) const override;
};



#pragma once
#include "Piece.h"

class Frog : public Piece {
public:
    Frog(PieceOwner owner, const std::string& texturePath);
    virtual ~Frog();

    virtual bool isValidMove(const GameState& state, int fromCol, int fromRow,
        int toCol, int toRow) const override;

private:
    bool canJumpTo(const GameState& state, int fromCol, int fromRow,
        int toCol, int toRow) const;
};


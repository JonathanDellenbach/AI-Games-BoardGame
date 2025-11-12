#ifndef PIECE_HPP
#define PIECE_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include "Board.h"

//mental reminder Piece.h/.cpp handles sprites which cell it's in owner and movement validation

enum class PieceType
{
    FROG,
    SNAKE,
    DONKEY,
    NONE
};

//which player owns the piece
enum class PieceOwner
{
    PLAYER,
    AI,
    NONE
};

class Piece
{
public:
    Piece(PieceType type, PieceOwner owner, const std::string& texturePath);
    ~Piece();

    void render(sf::RenderWindow& window);
    void setPosition(float x, float y);

    // Grid position
    void setGridPosition(int col, int row);
    GridPos getGridPosition() const { return GridPos{ m_gridCol, m_gridRow }; }

    PieceType getType() const { return m_type; }
    PieceOwner getOwner() const { return m_owner; }
    sf::Vector2f getPosition() const { return m_sprite.getPosition(); }

private:
    PieceType m_type;
    PieceOwner m_owner;
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    bool m_isValid;

    //which cell this piece is currently in
    int m_gridCol;
    int m_gridRow;

    void setupSprite();
};

#endif
#ifndef PIECE_HPP
#define PIECE_HPP

#include <SFML/Graphics.hpp>
#include <string>

//mental reminder Piece.h/.cpp handles sprites which cell it's in owner and movement validation
//Changed to base class for inheritance, the other pieces only have movement changes so they all have same architecture besides individual movement

// Just let compile know this exists without header reliance. 
// * To be added
class GameState;

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
    virtual ~Piece();

    // Pure virtual function for movement validation, since all pieces move different
    virtual bool isValidMove(const GameState& state, int fromCol, int fromRow,
        int toCol, int toRow) const = 0;

    void render(sf::RenderWindow& window);
    void setPosition(float x, float y);

    // Grid position
    void setGridPosition(int col, int row);
    int getGridCol() const { return m_gridCol; }
    int getGridRow() const { return m_gridRow; }

    PieceType getType() const { return m_type; }
    PieceOwner getOwner() const { return m_owner; }
    sf::Vector2f getPosition() const { return m_sprite.getPosition(); }
    bool isValid() const { return m_isValid; }

protected:
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
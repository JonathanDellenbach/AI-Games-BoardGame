#include "Piece.h"
#include <iostream>

Piece::Piece(PieceType type, PieceOwner owner, const std::string& texturePath) :
    m_type(type),
    m_owner(owner),
    m_sprite(m_texture),
    m_isValid(false),
    m_gridCol(-1),
    m_gridRow(-1)
{
    //load texture after sprite construction
    if (!m_texture.loadFromFile(texturePath))
    {
        std::cout << "Failed to load piece texture: " << texturePath << std::endl;
        m_isValid = false;
        return;
    }

    //update sprite with loaded texture
    m_sprite.setTexture(m_texture, true);
    setupSprite();
    m_isValid = true;

    std::cout << "Piece loaded successfully: " << texturePath << std::endl;
}

Piece::~Piece()
{
}

void Piece::setupSprite()
{
    sf::Vector2u textureSize = m_texture.getSize();

    //fit in cell
    float scaleX = 90.0f / textureSize.x;
    float scaleY = 90.0f / textureSize.y;
    float scale = std::min(scaleX, scaleY);

    m_sprite.setScale(sf::Vector2f(scale, scale));

    //center
    sf::FloatRect bounds = m_sprite.getLocalBounds();
    m_sprite.setOrigin(bounds.size / 2.0f);
}

void Piece::render(sf::RenderWindow& window)
{
    if (m_isValid)
    {
        window.draw(m_sprite);
    }
}

void Piece::setPosition(float x, float y)
{
    //center sprite in cell
    m_sprite.setPosition(sf::Vector2f(x + 50.0f, y + 50.0f));
}

void Piece::setGridPosition(int col, int row)
{
    m_gridCol = col;
    m_gridRow = row;
}
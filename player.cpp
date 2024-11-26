#include "player.h"

Player::Player(QColor color, int id) : color(color), id(id), hitCount(0)
{
    for (int i = 0; i < 4; i++)
    {
        tokens.push_back(new Token(color, i)); // Create tokens with the player's color
    }
}

Player::~Player()
{
    for (Token *token : tokens)
    {
        delete token;
    }
}

bool Player::hasWon() const
{
    for (Token *token : tokens)
    {
        if (!token->isHome)
        { // Check if all tokens are home
            return false;
        }
    }
    return true;
}

bool Player::canEnterHome() const
{
    return hitCount > 0;
}

void Player::incrementHitCount()
{
    hitCount++;
}

Token *Player::getToken(int tokenId) const
{
    if (tokenId >= 0 && tokenId < static_cast<int>(tokens.size()))
    {
        return tokens[tokenId];
    }
    return nullptr;
}

int Player::getTokenPosition(int tokenId) const
{
    if (tokenId >= 0 && tokenId < tokens.size())
    {
        return tokens[tokenId]->position; // Access token's position
    }
    return -1; // Invalid token ID
}

void Player::setTokenPosition(int tokenId, int newPosition)
{
    if (tokenId >= 0 && tokenId < tokens.size())
    {
        tokens[tokenId]->position = newPosition; // Set token's position
    }
}

void Player::initializeTokens(QGraphicsScene *scene, int startX, int startY)
{
    for (int i = 0; i < 4; i++)
    {
        // Create a graphical representation for each token
        QGraphicsEllipseItem *tokenGraphic = scene->addEllipse(
            startX + (i % 2) * 40, // Offset tokens in a 2x2 grid
            startY + (i / 2) * 40,
            30, // Token size
            30,
            QPen(Qt::black),
            QBrush(color));
        graphics.push_back(tokenGraphic); // Store the graphical token
        tokens[i]->position = -1;         // Initialize off the board
    }
}

void Player::updateTokenGraphics(int tokenId, int newPosition)
{
    if (tokenId >= 0 && tokenId < 4)
    {
        int x = (newPosition % 15) * 40; // Calculate grid position
        int y = (newPosition / 15) * 40;
        graphics[tokenId]->setRect(x, y, 30, 30); // Move graphical token
    }
}

void Player::updateTokenGraphics(int tokenId, int x, int y)
{
    if (tokenId >= 0 && tokenId < static_cast<int>(graphics.size()))
    {
        graphics[tokenId]->setRect(x, y, 30, 30);
    }
}

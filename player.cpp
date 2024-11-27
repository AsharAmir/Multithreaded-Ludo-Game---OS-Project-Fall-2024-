//player.cpp
#include "player.h"

Player::Player(QColor color, int id)
    : color(color), id(id), hitCount(0), isDisqualified_(false), noSixCount(0)
{
    for (int i = 0; i < 4; i++)
    {
        Token *token = new Token(color, i);
        token->setTokenNumber(i + 1); // Set token numbers 1-4
        tokens.push_back(token);
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
    return;
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
    if (tokenId >= 0 && tokenId < static_cast<int>(tokens.size()))
    {
        return tokens[tokenId]->position; // Access token's position
    }
    return -1; // Invalid token ID
}

void Player::setTokenPosition(int tokenId, int newPosition)
{
    if (tokenId >= 0 && tokenId < static_cast<int>(tokens.size()))
    {
        tokens[tokenId]->position = newPosition; // Set token's position
    }
}

void Player::initializeTokens(QGraphicsScene *scene, int startX, int startY, int tokenCount)
{
    for (int i = 0; i < tokenCount; i++)
    {
        // Create a graphical representation for each token
        QGraphicsEllipseItem *tokenGraphic = scene->addEllipse(
            startX + (i % 2) * 80, // Offset tokens in a 2x2 grid
            startY + (i / 2) * 80,
            30, // Token size
            30,
            QPen(Qt::black),
            QBrush(color.lighter()));
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

bool Player::hasActiveTokens() const
{
    for (Token *token : tokens)
    {
        if (!token->isInYard && !token->isHome)
        {
            return true;
        }
    }
    return false;
}

int Player::getActiveTokenCount() const
{
    int count = 0;
    for (Token *token : tokens)
    {
        if (!token->isInYard && !token->isHome)
        {
            count++;
        }
    }
    return count;
}

bool Player::hasBlockAt(int position) const
{
    int tokensAtPosition = 0;
    for (Token *token : tokens)
    {
        if (token->position == position)
        {
            tokensAtPosition++;
            if (tokensAtPosition >= 2)
            {
                return true;
            }
        }
    }
    return false;
}

int Player::getHitCount() const
{
    return hitCount;
}

const std::vector<Token *> &Player::getTokens() const
{
    return tokens;
}

void Player::disqualify()
{
    isDisqualified_ = true;
    for (Token *token : tokens)
    {
        token->isInYard = true; // Send all tokens back to yard
        token->position = -1;   // Reset position
    }
}

bool Player::isDisqualified() const
{
    return isDisqualified_;
}

int Player::getId() const
{
    return id;
}

QColor Player::getColor() const
{
    return color;
}

void Player::resetNoSixCount()
{
    noSixCount = 0;
}

void Player::incrementNoSixCount()
{
    noSixCount++;
}

int Player::getNoSixCount() const
{
    return noSixCount;
}
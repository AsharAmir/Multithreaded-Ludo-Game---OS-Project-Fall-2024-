#include "token.h"

Token::Token() : row(0), col(0), inPlay(false), position(0) {}

Token::Token(Token&& other) noexcept
    : row(other.row), col(other.col), inPlay(other.inPlay), position(other.position)
{
}

Token& Token::operator=(Token&& other) noexcept
{
    if (this != &other)
    {
        row = other.row;
        col = other.col;
        inPlay = other.inPlay;
        position = other.position;
    }
    return *this;
}


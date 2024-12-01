#include "token.h"

Token::Token() : row(0), col(0), inPlay(false), position(0),
                 homePosition(0), readyForHome(false), scored(false) {}

Token::Token(Token&& other) noexcept :
    row(other.row), col(other.col), inPlay(other.inPlay),
    position(other.position), homePosition(other.homePosition),
    readyForHome(other.readyForHome), scored(other.scored)
{
    other.row = 0;
    other.col = 0;
    other.inPlay = false;
    other.position = 0;
    other.homePosition = 0;
    other.readyForHome = false;
    other.scored = false;
}

Token& Token::operator=(Token&& other) noexcept
{
    if (this != &other) {
        row = other.row;
        col = other.col;
        inPlay = other.inPlay;
        position = other.position;
        homePosition = other.homePosition;
        readyForHome = other.readyForHome;
        scored = other.scored;

        other.row = 0;
        other.col = 0;
        other.inPlay = false;
        other.position = 0;
        other.homePosition = 0;
        other.readyForHome = false;
        other.scored = false;
    }
    return *this;
}

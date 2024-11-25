#include "game/token.h"

Token::Token(Color color)
    : m_isHome(true)
    , m_isFinished(false)
    , m_color(color)
{
}
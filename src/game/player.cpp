#include "game/player.h"

Player::Player(Token::Color color, int tokenCount)
    : m_color(color)
    , m_hitCount(0)
{
    for (int i = 0; i < tokenCount; ++i) {
        m_tokens.push_back(std::make_shared<Token>(color));
    }
}
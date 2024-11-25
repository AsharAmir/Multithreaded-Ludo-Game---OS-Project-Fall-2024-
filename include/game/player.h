#pragma once

#include "token.h"
#include <vector>
#include <memory>

class Player {
public:
    Player(Token::Color color, int tokenCount);

    const std::vector<std::shared_ptr<Token>>& tokens() const { return m_tokens; }
    Token::Color color() const { return m_color; }
    int hitCount() const { return m_hitCount; }
    void incrementHitCount() { ++m_hitCount; }

private:
    std::vector<std::shared_ptr<Token>> m_tokens;
    Token::Color m_color;
    int m_hitCount;
};
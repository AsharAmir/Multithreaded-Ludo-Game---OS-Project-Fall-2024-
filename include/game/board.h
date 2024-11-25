#pragma once

#include "player.h"
#include <array>
#include <vector>
#include <mutex>

class Board {
public:
    static constexpr int BOARD_SIZE = 15;
    
    Board();

    void initializeBoard();
    bool moveToken(std::shared_ptr<Token> token, int steps);
    bool isValidMove(const Token& token, int steps) const;
    bool isSafeSquare(const QPoint& pos) const;
    
    const std::vector<std::shared_ptr<Player>>& players() const { return m_players; }
    void addPlayer(Token::Color color, int tokenCount);

private:
    std::array<std::array<char, BOARD_SIZE>, BOARD_SIZE> m_grid;
    std::vector<std::shared_ptr<Player>> m_players;
    std::vector<QPoint> m_safeSquares;
    std::mutex m_mutex;
};
#include "game/board.h"
#include <algorithm>

Board::Board() {
    m_safeSquares = {
        {6, 2}, {2, 6}, {8, 6}, {6, 8},  // Common safe squares
        {6, 1}, {1, 6}, {8, 1}, {13, 6}, // Player-specific safe squares
        {6, 13}, {13, 8}, {8, 13}, {1, 8}
    };
}

void Board::initializeBoard() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Clear board
    for (auto& row : m_grid) {
        std::fill(row.begin(), row.end(), ' ');
    }

    // Mark safe squares
    for (const auto& pos : m_safeSquares) {
        m_grid[pos.x()][pos.y()] = '*';
    }

    // Initialize home positions
    m_grid[1][1] = 'R';   // Red home
    m_grid[1][13] = 'G';  // Green home
    m_grid[13][1] = 'Y';  // Yellow home
    m_grid[13][13] = 'B'; // Blue home
}

void Board::addPlayer(Token::Color color, int tokenCount) {
    m_players.push_back(std::make_shared<Player>(color, tokenCount));
}

bool Board::isValidMove(const Token& token, int steps) const {
    // Implementation of move validation logic
    return true; // Placeholder
}

bool Board::moveToken(std::shared_ptr<Token> token, int steps) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!isValidMove(*token, steps)) {
        return false;
    }

    // Implementation of token movement logic
    return true; // Placeholder
}

bool Board::isSafeSquare(const QPoint& pos) const {
    return std::find(m_safeSquares.begin(), m_safeSquares.end(), pos) != m_safeSquares.end();
}
#pragma once

#include <mutex>
#include <condition_variable>

struct Token
{
    int row, col;    // Current row and column on the board
    bool inPlay;     // Whether the token is currently in play
    int position;    // Position on the shared path
    int homePosition; // Position within the home path
    bool readyForHome; // Flag to indicate ready to move on the home path
    bool scored;     // Indicates whether the token has scored a point
    mutable std::mutex tokenMutex;
    std::condition_variable tokenCV;
    bool hasCompletedCycle;

    Token();
    Token(Token&& other) noexcept;
    Token& operator=(Token&& other) noexcept;

    // Prevent copying
    Token(const Token&) = delete;
    Token& operator=(const Token&) = delete;
};

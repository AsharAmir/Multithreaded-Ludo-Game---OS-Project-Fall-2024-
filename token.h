#pragma once

#include <mutex>
#include <condition_variable>

struct Token
{
    int row, col;
    bool inPlay;
    int position;
    std::mutex tokenMutex;
    std::condition_variable tokenCV;

    Token();
    Token(Token&& other) noexcept;
    Token& operator=(Token&& other) noexcept;

    // Prevent copying
    Token(const Token&) = delete;
    Token& operator=(const Token&) = delete;
};

#pragma once

#include <vector>
#include <atomic>
#include "token.h"

struct Player
{
    int id;
    std::vector<Token> tokens;
    std::atomic<int> hitRate;
    std::atomic<bool> isActive;
    int homePosition;
    std::vector<int> safeSquares;

    Player(int playerId = 0);
    Player& operator=(Player&& other) noexcept;
};

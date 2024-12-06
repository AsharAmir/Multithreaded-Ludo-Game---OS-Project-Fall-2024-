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
    bool hasHit;  // Track if the player has hit another token
    int unsuccessfulTurnsHits=0 ;
    int unsuccessfulTurnsSixes=0 ;

    Player(int playerId = 0);
    Player& operator=(Player&& other) noexcept;
    std::vector<std::pair<int, int>> previousPositions; // To store previous positions of tokens
    std::vector<bool> wasInPlay;
};

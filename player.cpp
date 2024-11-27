#include "player.h"
#include "constants.h"

Player::Player(int playerId) : id(playerId),
                               hitRate(100),
                               isActive(true),
                               homePosition(playerId * 13)
{
    safeSquares = {
        homePosition,
        (homePosition + 8) % BOARD_SQUARES,
        (homePosition + 13) % BOARD_SQUARES,
        (homePosition + 21) % BOARD_SQUARES,
        (homePosition + 34) % BOARD_SQUARES};
}

Player& Player::operator=(Player&& other) noexcept
{
    if (this != &other)
    {
        id = other.id;
        tokens = std::move(other.tokens);
        hitRate.store(other.hitRate.load());
        isActive.store(other.isActive.load());
        homePosition = other.homePosition;
        safeSquares = std::move(other.safeSquares);
    }
    return *this;
}


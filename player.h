#ifndef PLAYER_H
#define PLAYER_H

#include <QColor>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <vector>
#include "token.h"

class Player
{
public:
    Player(QColor color, int id); // Constructor
    ~Player();                    // Destructor

    // Token Management
    void initializeTokens(QGraphicsScene *scene, int startX, int startY); // Initialize all tokens on the board
    void updateTokenGraphics(int tokenId, int x, int y);
    void updateTokenGraphics(int tokenId, int newPosition);          // Update graphical position of a token
    Token *getToken(int tokenId) const;                              // Get a specific token by ID
    const std::vector<Token *> &getTokens() const { return tokens; } // Get all tokens

    // State Queries
    bool hasWon() const;                      // Check if the player has won
    bool canEnterHome() const;                // Check if the player can enter the home area
    int getId() const { return id; }          // Get the player's ID
    QColor getColor() const { return color; } // Get the player's color

    // Game Progress
    void incrementHitCount();                    // Increment the hit count
    int getHitCount() const { return hitCount; } // Get the hit count

    // Token Position Management
    int getTokenPosition(int tokenId) const;          // Get the current position of a token
    void setTokenPosition(int tokenId, int position); // Set the current position of a token

private:
    QColor color;                                 // Player's color
    int id;                                       // Player's ID
    int hitCount;                                 // Number of hits made by the player
    std::vector<Token *> tokens;                  // List of tokens owned by the player
    std::vector<QGraphicsEllipseItem *> graphics; // Graphical representation of tokens
};

#endif // PLAYER_H

//player.h

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
    Player(QColor color, int id);
    ~Player();

    void initializeTokens(QGraphicsScene *scene, int startX, int startY, int tokenCount);
    void updateTokenGraphics(int tokenId, int x, int y);
    void updateTokenGraphics(int tokenId, int newPosition);
    Token *getToken(int tokenId = 0) const; // Default parameter for backwards compatibility
    const std::vector<Token *> &getTokens() const;
    std::vector<Token *> &getTokens() { return tokens; }

    bool hasWon() const;
    bool canEnterHome() const;
    int getId() const;
    QColor getColor() const;

    void incrementHitCount();
    int getHitCount() const;
    void disqualify();
    bool isDisqualified() const;

    int getTokenPosition(int tokenId) const;
    void setTokenPosition(int tokenId, int position);

    bool hasActiveTokens() const;
    int getActiveTokenCount() const;
    bool hasBlockAt(int position) const;
    void resetNoSixCount();
    void incrementNoSixCount();
    int getNoSixCount() const;

private:
    QColor color;
    int id;
    int hitCount;
    std::vector<Token *> tokens;
    std::vector<QGraphicsEllipseItem *> graphics;
    bool isDisqualified_;
    int noSixCount;
};

#endif // PLAYER_H
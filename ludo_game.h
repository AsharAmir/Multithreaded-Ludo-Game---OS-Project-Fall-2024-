#pragma once

#include <QMainWindow>
#include <QTimer>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "player.h"
#include "dice.h"
#include "constants.h"

class LudoGame : public QMainWindow
{
    Q_OBJECT

public:
    // LudoGame(QWidget *parent = nullptr);
    LudoGame(QWidget *parent = nullptr, int numTokens = 4);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void drawLudoBoard(QPainter &painter);
    void drawPieces(QPainter &painter);
    void drawDice(QPainter &painter);
    void initializePlayers(int numTokens);
    void startGame();
    void playerThread(int playerId);
    void masterThread();
    void rollDice();
    void advanceTurn();
    void handleTokenSelection(const QPointF &mousePos);
    void moveToken(Token &token, int spaces);
    void checkAndProcessHits(Token &token, int newPos, int playerId);
    QPoint getStartingPosition(int playerId);
    QPointF calculateBoardPosition(int position);
    QRectF calculateTokenRect(const Token &token);
    void verifyTokenPosition(const Token &token);
    void updateTurnTimer();
    void initializePaths();
    void initializeNoGoPaths();
    void initializePlayerPaths();

    std::vector<std::vector<int>> ludoBoard;
    std::atomic<int> dice;
    std::mutex diceMutex, boardMutex;
    std::condition_variable diceCV, boardCV;
    Player players[MAX_PLAYERS];
    std::atomic<bool> gameRunning;
    std::atomic<int> currentPlayer;
    std::mutex tokenMutex[MAX_PLAYERS];
    Dice gameDice;
    int numTokens;
    QTimer *turnTimer;
    int remainingTime;
    bool waitingForMove;
    Token *selectedToken;
    int consecutiveSixes;
    // / Shared path for all players
    std::vector<PathCoordinate> sharedPath;
    std::vector<int> currentRoundPlayers; // Stores the order of players for the current round
    std::array<std::vector<QPoint>, 4> noGoPaths;

    // Safe zones for each player
    std::vector<PathCoordinate> playerSafePaths[MAX_PLAYERS];
    std::mutex turnMutex;
    std::condition_variable turnCV;
    std::atomic<int> lastPlayer{-1};
};

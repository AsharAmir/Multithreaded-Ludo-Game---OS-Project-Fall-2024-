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
    LudoGame(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void drawLudoBoard(QPainter &painter);
    void drawPieces(QPainter &painter);
    void drawDice(QPainter &painter);
    void initializePlayers();
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
    std::vector<PathCoordinate> bluePath;
    std::vector<PathCoordinate> redPath;
    std::vector<PathCoordinate> yellowPath;
    std::vector<PathCoordinate> greenPath;
    std::mutex turnMutex;
    std::condition_variable turnCV;
    std::atomic<int> lastPlayer{-1};
};

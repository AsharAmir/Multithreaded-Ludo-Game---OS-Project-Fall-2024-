#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPushButton>
#include <QLabel>
#include <QMutex>
#include <QVector>
#include <QThread>
#include <QTimer>
#include "player.h"
#include "dice.h"
#include "playerthread.h"
#include "masterthread.h"

// class PlayerThread;
// class MasterThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void rollDice();
    void updateGame();
    void startGame();
    void nextTurn();
    void handleConsecutiveSixes(int count);
    void handlePlayerTimeout();
    void handleTokenCapture(int playerId);
    void handlePlayerDisqualification(int playerId);
    void handleGameCompletion();

private:
    void initializeBoard();
    void drawBoard();
    void createPlayers();
    void moveToken(Player *player, int tokenId, int steps);
    bool checkCollision(int position);
    bool isValidMove(Player *player, int tokenId, int steps);
    void setupUI();
    void selectTokenDialog(const QVector<int> &validTokens, int diceValue);
    QPoint getboardCoordinates(int position, int playerId);
    void handleCaptures(Player *player, int newPosition);
    int calculateNewPosition(Player *player, int currentPos, int steps);
    void initializeThreads();
    void stopThreads();
    // void nextTurn();
    void checkPlayerStatus();
    bool canEnterHomeColumn(Player *player);
    void updatePlayerStats();
    void styleUIComponents();
    bool gameEnded();
    void updateBoard();

    QGraphicsScene *scene;
    QGraphicsView *view;
    QPushButton *rollButton;
    QLabel *statusLabel;
    QLabel *playerStatsLabel;
    Dice *dice;
    QVector<Player *> players;
    QVector<PlayerThread *> playerThreads;
    MasterThread *masterThread;
    int currentPlayer;
    QMutex gameMutex;
    bool gameStarted;
    QPushButton *startButton;
    int consecutiveSixesCount;
    QTimer *turnTimer;
    int turnCount;
    QVector<int> noSixCount;

    static const int BOARD_SIZE = 600;
    static const int GRID_SIZE = 15;
    static const int CELL_SIZE = BOARD_SIZE / GRID_SIZE;
    static const int MAX_NO_SIX_TURNS = 20;
    static const int TURN_TIMEOUT = 30000; // 30 seconds

    Token *createToken(Player *player, int tokenId);
};

// class PlayerThread : public QThread
// {
//     Q_OBJECT
// public:
//     PlayerThread(Player *player, QObject *parent = nullptr);
//     void run() override;
//     void stopThread();

// signals:
//     void tokenCaptured(int playerId);
//     void playerCompleted(int playerId);

// private:
//     Player *player;
//     bool running;
//     QMutex threadMutex;
// };

// class MasterThread : public QThread
// {
//     Q_OBJECT
// public:
//     MasterThread(QVector<Player *> &players, QObject *parent = nullptr);
//     void run() override;
//     void stopThread();

// signals:
//     void playerDisqualified(int playerId);
//     void gameCompleted();

// private:
//     QVector<Player *> &players;
//     bool running;
//     QMutex threadMutex;
// };

#endif // MAINWINDOW_H
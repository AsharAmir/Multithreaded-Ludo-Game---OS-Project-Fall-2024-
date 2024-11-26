#ifndef PLAYERTHREAD_H
#define PLAYERTHREAD_H

#include <QThread>
#include <QRandomGenerator>
#include <QWaitCondition>
#include <QMutex>
#include "player.h"

class PlayerThread : public QThread
{
    Q_OBJECT

public:
    explicit PlayerThread(Player *player, QObject *parent = nullptr);

    void startTurn(); // Start the player's turn
    void stopGame();  // Stop the game and terminate the thread

signals:
    void tokenCaptured(int playerId);
    void turnCompleted(int playerId);

protected:
    void run() override;

private:
    Player *player;
    bool isTurn;      // Indicates whether it's this player's turn
    bool gameRunning; // Indicates whether the game is running

    static QMutex mutex;             // Shared mutex for thread synchronization
    static QWaitCondition condition; // Shared condition variable
};

#endif // PLAYERTHREAD_H

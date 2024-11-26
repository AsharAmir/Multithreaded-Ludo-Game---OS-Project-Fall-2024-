#include "playerthread.h"
#include <QMutexLocker>

QMutex PlayerThread::mutex;             // Shared mutex
QWaitCondition PlayerThread::condition; // Shared condition variable

PlayerThread::PlayerThread(Player *player, QObject *parent)
    : QThread(parent), player(player), isTurn(false), gameRunning(true) {}

void PlayerThread::run()
{
    while (gameRunning)
    {
        QMutexLocker locker(&mutex); // Lock the shared mutex

        while (!isTurn && gameRunning)
        {
            condition.wait(&mutex); // Wait for the signal to take the turn
        }

        if (!gameRunning)
            break; // Exit if the game ends

        if (player->isDisqualified())
        {
            isTurn = false;
            emit turnCompleted(player->getId());
            continue;
        }

        // Simulate dice roll or action
        int action = QRandomGenerator::global()->bounded(1, 3);
        if (action == 1)
        {
            emit tokenCaptured(player->getId());
        }

        isTurn = false; // Mark turn as completed
        emit turnCompleted(player->getId());
    }
}

void PlayerThread::startTurn()
{
    QMutexLocker locker(&mutex);
    isTurn = true;       // Set turn flag
    condition.wakeAll(); // Notify waiting thread
}

void PlayerThread::stopGame()
{
    QMutexLocker locker(&mutex);
    gameRunning = false; // Stop the game
    condition.wakeAll(); // Notify all waiting threads to exit
}

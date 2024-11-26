#include "masterthread.h"
#include <QThread>

MasterThread::MasterThread(QVector<Player *> &players, QObject *parent)
    : QThread(parent), players(players) {}

void MasterThread::run()
{
    while (!isGameOver())
    {
        for (Player *player : players)
        {
            if (player->isDisqualified())
            {
                emit playerDisqualified(player->getId());
            }
        }
        QThread::msleep(1000); // Periodically check game state
    }
    emit gameCompleted();
}

bool MasterThread::isGameOver()
{
    int activePlayers = 0;
    for (Player *player : players)
    {
        if (!player->isDisqualified())
        {
            activePlayers++;
        }
    }
    return activePlayers <= 1; // Game over if one or no players remain
}

#ifndef MASTERTHREAD_H
#define MASTERTHREAD_H

#include <QThread>
#include "player.h"
#include <QVector>

class MasterThread : public QThread
{
    Q_OBJECT

public:
    explicit MasterThread(QVector<Player *> &players, QObject *parent = nullptr);

signals:
    void playerDisqualified(int playerId);
    void gameCompleted();

protected:
    void run() override;

private:
    QVector<Player *> &players;

    bool isGameOver();
};

#endif // MASTERTHREAD_H

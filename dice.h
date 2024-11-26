// dice.h
#ifndef DICE_H
#define DICE_H

#include <QObject>
#include <QRandomGenerator>

class Dice : public QObject
{
    Q_OBJECT

public:
    explicit Dice(QObject *parent = nullptr);
    int roll();
    int getCurrentValue() const { return currentValue; }

private:
    int currentValue;
    QRandomGenerator *generator;
};

#endif // DICE_H
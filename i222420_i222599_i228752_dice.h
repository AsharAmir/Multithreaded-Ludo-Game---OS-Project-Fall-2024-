#pragma once

#include <QRectF>
#include <QElapsedTimer>

struct Dice
{
    QRectF shape;
    int value;
    float rotation;
    bool isRolling;
    QElapsedTimer rollClock;

    Dice();
};


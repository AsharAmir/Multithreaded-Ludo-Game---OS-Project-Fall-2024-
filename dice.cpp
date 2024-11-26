#include "dice.h"

Dice::Dice(QObject *parent) : QObject(parent), generator(QRandomGenerator::global()) {}

int Dice::roll()
{
    return generator->bounded(1, 7); // Use `->` to call the method on the pointer
}

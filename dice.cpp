#include "dice.h"
#include "constants.h"

Dice::Dice() : value(1), rotation(0.0f), isRolling(false)
{
    shape = QRectF(0, 0, DICE_SIZE, DICE_SIZE);
}


#include "token.h"
#include <QBrush>
#include <QPen>

Token::Token(QColor color, int id) : QGraphicsEllipseItem(0, 0, 20, 20),
                                     position(0),
                                     isHome(false),
                                     isInYard(true),
                                     color(color),
                                     id(id) // Initialize the ID

{

    setBrush(QBrush(color));
    setPen(QPen(Qt::black));
}
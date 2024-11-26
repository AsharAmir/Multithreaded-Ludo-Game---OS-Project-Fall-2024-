#include "token.h"
#include <QGraphicsItem>
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QRectF>
#include <QFont>
#include <QRadialGradient>
#include <QGraphicsDropShadowEffect>

Token::Token(QColor color, int number, Player *owner)
    : tokenColor(color), tokenNumber(number), position(-1), isInYard(true), isHome(false), owner(owner)
{
    numberLabel = new QGraphicsTextItem(this);
    numberLabel->setPlainText(QString::number(number));
    numberLabel->setDefaultTextColor(Qt::black);

    QFont font;
    font.setBold(true);
    numberLabel->setFont(font);

    // Center the number
    QRectF textRect = numberLabel->boundingRect();
    numberLabel->setPos(-textRect.width() / 2, -textRect.height() / 2);
}

// QRectF Token::boundingRect() const
// {
//     return QRectF(-15, -15, 30, 30);
// }

void Token::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Draw token circle
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(tokenColor));
    painter->drawEllipse(boundingRect());

    // Add gradient effect
    QRadialGradient gradient(boundingRect().center(), 15);
    gradient.setColorAt(0, tokenColor.lighter(150));
    gradient.setColorAt(1, tokenColor);
    painter->setBrush(gradient);
    painter->drawEllipse(boundingRect());
}

void Token::setTokenNumber(int number)
{
    tokenNumber = number;
    numberLabel->setPlainText(QString::number(number));
    update();
}

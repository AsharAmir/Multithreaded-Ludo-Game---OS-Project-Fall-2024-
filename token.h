#ifndef TOKEN_H
#define TOKEN_H

#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QRectF>
#include <QFont>
#include <QRadialGradient>

class Player;

class Token : public QGraphicsItem
{
public:
    // Constructor
    Token(QColor color, int number, Player *owner = nullptr);

    // Getters
    int getPosition() const { return position; }
    bool getIsInYard() const { return isInYard; }
    bool getIsHome() const { return isHome; }
    Player *getOwner() const { return owner; }
    int getTokenNumber() const { return tokenNumber; }
    QColor getColor() const { return tokenColor; }
    int getId() const { return tokenNumber; }

    // Setters
    void setPosition(int pos)
    {
        position = pos;
        update();
    }
    void setIsInYard(bool yard)
    {
        isInYard = yard;
        update();
    }
    void setIsHome(bool home)
    {
        isHome = home;
        update();
    }
    void setOwner(Player *newOwner) { owner = newOwner; }
    void setTokenNumber(int number);

    // Graphics updates
    void updateNumberLabel();

    // Required QGraphicsItem overrides
    QRectF boundingRect() const override
    {
        return QRectF(-15, -15, 30, 30); // Adjust size as needed
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    int position;  // Position on the board (-1 if in yard)
    bool isInYard; // True if the token is in the yard
    bool isHome;   // True if the token has reached home
private:
    QColor tokenColor;
    int tokenNumber;
    Player *owner;
    QGraphicsTextItem *numberLabel;
};

#endif // TOKEN_H

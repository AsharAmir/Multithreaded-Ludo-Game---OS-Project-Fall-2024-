// token.h
#ifndef TOKEN_H
#define TOKEN_H

#include <QGraphicsEllipseItem>
#include <QColor>

class Token : public QGraphicsEllipseItem
{
public:
    Token(QColor color, int id);

    // Position
    int position;
    bool isHome;
    bool isInYard;

    // Properties
    QColor color;
    int getId() const { return id; }

    // Movement
    void moveToPosition(int x, int y);
    bool isInHomePath() const { return inHomePath; }
    void setInHomePath(bool value) { inHomePath = value; }

private:
    int id;
    bool inHomePath;
};

#endif // TOKEN_H
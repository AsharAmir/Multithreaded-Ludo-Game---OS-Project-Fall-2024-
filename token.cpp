#include "token.h"

Token::Token(QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent),
      row(0), col(0), inPlay(false), position(0),
      homePosition(0), readyForHome(false), scored(false),
      hasCompletedCycle(false), m_graphicsPos(0, 0) {}

Token::Token(Token &&other) noexcept
    : QGraphicsEllipseItem(other.parentItem()),
      row(other.row), col(other.col), inPlay(other.inPlay),
      position(other.position), homePosition(other.homePosition),
      readyForHome(other.readyForHome), scored(other.scored),
      hasCompletedCycle(other.hasCompletedCycle), m_graphicsPos(other.m_graphicsPos)
{
    other.row = 0;
    other.col = 0;
    other.inPlay = false;
    other.position = 0;
    other.homePosition = 0;
    other.readyForHome = false;
    other.scored = false;
    other.m_graphicsPos = QPointF(0, 0);
}

Token &Token::operator=(Token &&other) noexcept
{
    if (this != &other)
    {
        row = other.row;
        col = other.col;
        inPlay = other.inPlay;
        position = other.position;
        homePosition = other.homePosition;
        readyForHome = other.readyForHome;
        scored = other.scored;
        hasCompletedCycle = other.hasCompletedCycle;
        m_graphicsPos = other.m_graphicsPos;

        other.row = 0;
        other.col = 0;
        other.inPlay = false;
        other.position = 0;
        other.homePosition = 0;
        other.readyForHome = false;
        other.scored = false;
        other.m_graphicsPos = QPointF(0, 0);
    }
    return *this;
}

QPointF Token::graphicsPos() const
{
    return m_graphicsPos;
}

void Token::setGraphicsPos(const QPointF &pos)
{
    if (m_graphicsPos != pos)
    {
        m_graphicsPos = pos;
        setPos(pos); // Update the visual position
        emit graphicsPosChanged();
    }
}

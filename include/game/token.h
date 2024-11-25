#pragma once

#include <QPoint>

class Token {
public:
    enum class Color {
        RED,
        GREEN,
        YELLOW,
        BLUE
    };

    Token(Color color);

    QPoint position() const { return m_position; }
    void setPosition(const QPoint& pos) { m_position = pos; }
    bool isHome() const { return m_isHome; }
    void setHome(bool home) { m_isHome = home; }
    bool isFinished() const { return m_isFinished; }
    void setFinished(bool finished) { m_isFinished = finished; }
    Color color() const { return m_color; }

private:
    QPoint m_position;
    bool m_isHome;
    bool m_isFinished;
    Color m_color;
};
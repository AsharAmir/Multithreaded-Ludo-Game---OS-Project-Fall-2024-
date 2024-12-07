#pragma once

#include <QObject>
#include <QGraphicsEllipseItem>
#include <mutex>
#include <condition_variable>

class Token : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF graphicsPos READ graphicsPos WRITE setGraphicsPos NOTIFY graphicsPosChanged)

public:
    // Attributes
    int row;              // Current row on the board
    int col;              // Current column on the board
    bool inPlay;          // Whether the token is in play
    int position;         // Position on the shared path
    int homePosition;     // Position within the home path
    bool readyForHome;    // Flag to indicate ready to move on the home path
    bool scored;          // Indicates whether the token has scored a point
    bool hasCompletedCycle; // Indicates whether the token has completed a board cycle

    mutable std::mutex tokenMutex;
    std::condition_variable tokenCV;

    // Constructor and Destructor
    explicit Token(QGraphicsItem *parent = nullptr);
    Token(Token &&other) noexcept;
    Token &operator=(Token &&other) noexcept;

    // Prevent copying
    Token(const Token &) = delete;
    Token &operator=(const Token &) = delete;

    // Animation-related methods
    QPointF graphicsPos() const;
    void setGraphicsPos(const QPointF &pos);

signals:
    void graphicsPosChanged();

private:
    QPointF m_graphicsPos; // For animation
};

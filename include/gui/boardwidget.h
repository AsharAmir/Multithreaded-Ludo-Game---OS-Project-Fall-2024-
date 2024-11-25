#pragma once

#include <QWidget>
#include "game/board.h"

class BoardWidget : public QWidget {
    Q_OBJECT

public:
    BoardWidget(std::shared_ptr<Board> board, QWidget* parent = nullptr);

    void updateBoard();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void drawGrid(QPainter& painter);
    void drawTokens(QPainter& painter);
    void drawSafeSquares(QPainter& painter);
    QPoint boardToScreen(const QPoint& pos) const;
    QPoint screenToBoard(const QPoint& pos) const;

    std::shared_ptr<Board> m_board;
    int m_squareSize;
};
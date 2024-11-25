#include "gui/boardwidget.h"
#include <QPainter>
#include <QMouseEvent>

BoardWidget::BoardWidget(std::shared_ptr<Board> board, QWidget* parent)
    : QWidget(parent)
    , m_board(board)
    , m_squareSize(50)
{
    setMinimumSize(Board::BOARD_SIZE * m_squareSize, Board::BOARD_SIZE * m_squareSize);
}

void BoardWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawGrid(painter);
    drawSafeSquares(painter);
    drawTokens(painter);
}

void BoardWidget::drawGrid(QPainter& painter) {
    for (int i = 0; i < Board::BOARD_SIZE; ++i) {
        for (int j = 0; j < Board::BOARD_SIZE; ++j) {
            QRect rect(j * m_squareSize, i * m_squareSize, m_squareSize, m_squareSize);
            painter.drawRect(rect);
        }
    }
}

void BoardWidget::drawSafeSquares(QPainter& painter) {
    painter.setBrush(Qt::yellow);
    // Draw safe squares
}

void BoardWidget::drawTokens(QPainter& painter) {
    for (const auto& player : m_board->players()) {
        for (const auto& token : player->tokens()) {
            QPoint pos = boardToScreen(token->position());
            
            // Set color based on token's color
            QColor color;
            switch (token->color()) {
                case Token::Color::RED: color = Qt::red; break;
                case Token::Color::GREEN: color = Qt::green; break;
                case Token::Color::YELLOW: color = Qt::yellow; break;
                case Token::Color::BLUE: color = Qt::blue; break;
            }
            
            painter.setBrush(color);
            painter.drawEllipse(pos, m_squareSize/2, m_squareSize/2);
        }
    }
}

void BoardWidget::updateBoard() {
    update();
}

void BoardWidget::mousePressEvent(QMouseEvent* event) {
    QPoint boardPos = screenToBoard(event->pos());
    // Handle token selection and movement
}

QPoint BoardWidget::boardToScreen(const QPoint& pos) const {
    return QPoint(pos.x() * m_squareSize + m_squareSize/2,
                 pos.y() * m_squareSize + m_squareSize/2);
}

QPoint BoardWidget::screenToBoard(const QPoint& pos) const {
    return QPoint(pos.x() / m_squareSize,
                 pos.y() / m_squareSize);
}
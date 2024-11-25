#pragma once

#include <QMainWindow>
#include "game/board.h"
#include "boardwidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);

private slots:
    void startNewGame();
    void rollDice();

private:
    void setupUI();
    void createMenus();

    std::shared_ptr<Board> m_board;
    BoardWidget* m_boardWidget;
};
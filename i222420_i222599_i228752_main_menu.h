// main_menu.h
#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include "i222420_i222599_i228752_ludo_game.h"

class MainMenu : public QWidget {
    Q_OBJECT

public:
    MainMenu(QWidget *parent = nullptr);

private slots:
    void selectTokens(int tokens);
    void startGame();

private:
    void setupUI();
    void styleButtons();
    void setupHoverAnimation(QPushButton *button);
    
    int selectedTokens;
    QPushButton *tokenButtons[4];
    QPushButton *startButton;
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    LudoGame *gameWindow;
};
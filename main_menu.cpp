#include "main_menu.h"
#include "ludo_game.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QPropertyAnimation>

MainMenu::MainMenu(QWidget *parent) : QWidget(parent), selectedTokens(0), gameWindow(nullptr) {
    setupUI();
    styleButtons();
}

void MainMenu::setupUI() {
    // Updated window size
    setFixedSize(600, 700);
    setWindowTitle("Ludo Game");

    // Gradient background
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QColor(63, 81, 181))); // Deep blue
    setPalette(palette);
    setAutoFillBackground(true);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(30);
    mainLayout->setContentsMargins(50, 50, 50, 50);

    // Title Label
    titleLabel = new QLabel("Welcome to Ludo Game!", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Subtitle Label
    subtitleLabel = new QLabel("Select Number of Tokens to Play", this);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    // Token Selection Buttons
    auto *buttonGrid = new QHBoxLayout();
    for (int i = 0; i < 4; ++i) {
        tokenButtons[i] = new QPushButton(QString::number(i + 1), this);
        tokenButtons[i]->setFixedSize(100, 100); // Increased button size
        connect(tokenButtons[i], &QPushButton::clicked, this, [this, i]() { selectTokens(i + 1); });
        buttonGrid->addWidget(tokenButtons[i]);
        setupHoverAnimation(tokenButtons[i]);
    }
    mainLayout->addLayout(buttonGrid);

    // Start Button
    startButton = new QPushButton("Start Game", this);
    startButton->setFixedHeight(60);
    startButton->setEnabled(false); // Disabled until a token is selected
    connect(startButton, &QPushButton::clicked, this, &MainMenu::startGame);
    mainLayout->addWidget(startButton);

    // Center the window
    move(QGuiApplication::primaryScreen()->geometry().center() - frameGeometry().center());
}

void MainMenu::styleButtons() {
    QString buttonStyle =
        "QPushButton {"
        "   background-color: #FF5722;"
        "   border: 2px solid #FF5722;"
        "   color: white;"
        "   border-radius: 50px;"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #E64A19;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #BDBDBD;"
        "   border: 2px solid #9E9E9E;"
        "   color: #757575;"
        "}";

    QString startButtonStyle =
        "QPushButton {"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border: 3px solid #388E3C;"
        "   border-radius: 30px;"
        "   font-size: 26px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #388E3C;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #BDBDBD;"
        "   border: 3px solid #9E9E9E;"
        "   color: #757575;"
        "}";

    QString titleStyle =
        "QLabel {"
        "   font-size: 36px;"
        "   color: white;"
        "   font-weight: bold;"
        "}";

    QString subtitleStyle =
        "QLabel {"
        "   font-size: 20px;"
        "   color: #C5CAE9;"
        "   font-weight: normal;"
        "}";

    for (auto button : tokenButtons) {
        button->setStyleSheet(buttonStyle);
    }

    startButton->setStyleSheet(startButtonStyle);
    titleLabel->setStyleSheet(titleStyle);
    subtitleLabel->setStyleSheet(subtitleStyle);
}

void MainMenu::setupHoverAnimation(QPushButton *button) {
    // Animation for hover effect
    auto *hoverAnimation = new QPropertyAnimation(button, "geometry", this);
    hoverAnimation->setDuration(200);

    connect(button, &QPushButton::pressed, [button, hoverAnimation]() {
        QRect original = button->geometry();
        QRect larger = QRect(original.x() - 5, original.y() - 5, original.width() + 10, original.height() + 10);
        hoverAnimation->setStartValue(original);
        hoverAnimation->setEndValue(larger);
        hoverAnimation->start();
    });

    connect(button, &QPushButton::released, [button, hoverAnimation]() {
        QRect larger = button->geometry();
        QRect original = QRect(larger.x() + 5, larger.y() + 5, larger.width() - 10, larger.height() - 10);
        hoverAnimation->setStartValue(larger);
        hoverAnimation->setEndValue(original);
        hoverAnimation->start();
    });
}

void MainMenu::selectTokens(int tokens) {
    selectedTokens = tokens; // Set the selected tokens to the latest clicked button's value

    // Reset the style of all buttons
    for (int i = 0; i < 4; ++i) {
        tokenButtons[i]->setStyleSheet(
            "QPushButton {"
            "   background-color: #FF5722;" // Default color
            "   border: 2px solid #FF5722;"
            "   color: white;"
            "   border-radius: 50px;"
            "   font-size: 24px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "   background-color: #E64A19;" // Hover color
            "}"
            "QPushButton:disabled {"
            "   background-color: #BDBDBD;" // Disabled color
            "   border: 2px solid #9E9E9E;"
            "   color: #757575;"
            "}"
        );
    }

    // Highlight the newly clicked button
    tokenButtons[tokens - 1]->setStyleSheet(
        "QPushButton {"
        "   background-color: #2196F3;" // Highlight color
        "   border: 2px solid #1976D2;"
        "   color: white;"
        "   border-radius: 50px;"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1976D2;" // Highlight hover color
        "}"
    );

    // Enable the Start Game button since a selection was made
    startButton->setEnabled(true);
}


void MainMenu::startGame() {
    gameWindow = new LudoGame(nullptr, selectedTokens); // Pass selectedTokens to LudoGame
    gameWindow->show();
    this->hide();
}

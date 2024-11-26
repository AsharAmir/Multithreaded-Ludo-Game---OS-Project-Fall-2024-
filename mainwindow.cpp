#include "mainwindow.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <QMessageBox>

const int BOARD_POSITIONS = 52; // Total positions on the main track
const QPoint PLAYER_START_POSITIONS[4] = {
    QPoint(1, 6),  // Red start
    QPoint(8, 1),  // Green start
    QPoint(13, 8), // Yellow start
    QPoint(6, 13)  // Blue start
};

const QPoint PLAYER_YARD_POSITIONS[4][4] = {
    {{1, 1}, {1, 4}, {4, 1}, {4, 4}},        // Red yard
    {{1, 10}, {1, 13}, {4, 10}, {4, 13}},    // Green yard
    {{10, 1}, {10, 4}, {13, 1}, {13, 4}},    // Yellow yard
    {{10, 10}, {10, 13}, {13, 10}, {13, 13}} // Blue yard
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), currentPlayer(0), gameStarted(false)
{
    setFixedSize(800, 700);
    setupUI();
    initializeBoard();
    createPlayers();
    drawBoard();
}

void MainWindow::initializeBoard()
{
    scene->setSceneRect(0, 0, BOARD_SIZE, BOARD_SIZE);
    view->setRenderHint(QPainter::Antialiasing);

    // Draw grid lines
    for (int i = 0; i <= GRID_SIZE; i++)
    {
        scene->addLine(i * CELL_SIZE, 0, i * CELL_SIZE, BOARD_SIZE, QPen(Qt::black));
        scene->addLine(0, i * CELL_SIZE, BOARD_SIZE, i * CELL_SIZE, QPen(Qt::black));
    }
}

void MainWindow::drawBoard()
{
    QColor colors[4] = {Qt::red, Qt::green, Qt::yellow, Qt::blue};
    QPoint homePositions[4] = {
        QPoint(0, 0),                        // Red home zone
        QPoint(GRID_SIZE - 6, 0),            // Green home zone
        QPoint(0, GRID_SIZE - 6),            // Yellow home zone
        QPoint(GRID_SIZE - 6, GRID_SIZE - 6) // Blue home zone
    };

    // Draw home zones
    for (int i = 0; i < 4; i++)
    {
        for (int x = 0; x < 6; x++)
        {
            for (int y = 0; y < 6; y++)
            {
                scene->addRect(
                    (homePositions[i].x() + x) * CELL_SIZE,
                    (homePositions[i].y() + y) * CELL_SIZE,
                    CELL_SIZE,
                    CELL_SIZE,
                    QPen(Qt::black),
                    QBrush(colors[i]));
            }
        }
    }
}

void MainWindow::updateGame()
{
    scene->update(); // Refresh the graphics scene
    statusLabel->setText(QString("Player %1's turn").arg(currentPlayer + 1));
}

void MainWindow::setupUI()
{
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene);
    dice = new Dice();

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    layout->addWidget(view);

    startButton = new QPushButton("Start Game", this);
    layout->addWidget(startButton);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::startGame);

    rollButton = new QPushButton("Roll Dice", this);
    layout->addWidget(rollButton);
    connect(rollButton, &QPushButton::clicked, this, &MainWindow::rollDice);
    rollButton->setEnabled(false);

    statusLabel = new QLabel("Click Start to begin the game", this);
    layout->addWidget(statusLabel);

    setCentralWidget(centralWidget);
}

void MainWindow::createPlayers()
{
    QColor colors[4] = {Qt::red, Qt::green, Qt::yellow, Qt::blue};
    for (int i = 0; i < 4; i++)
    {
        Player *player = new Player(colors[i], i);
        players.push_back(player);

        // Initialize tokens in yard positions
        for (int j = 0; j < 4; j++)
        {
            QPoint yardPos = PLAYER_YARD_POSITIONS[i][j];
            player->initializeTokens(scene, yardPos.x() * CELL_SIZE, yardPos.y() * CELL_SIZE);
        }
    }
}

void MainWindow::startGame()
{
    gameStarted = true;
    currentPlayer = 0;
    statusLabel->setText("Game started! Player 1's turn.");
    rollButton->setEnabled(true);
    startButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete scene;
    delete view;
    delete rollButton;
    delete startButton;
    delete statusLabel;
    delete dice;

    for (Player *player : players)
    {
        delete player;
    }
}

void MainWindow::rollDice()
{
    if (!gameStarted)
    {
        statusLabel->setText("Game not started!");
        return;
    }

    gameMutex.lock();
    int value = dice->roll();
    statusLabel->setText(QString("Player %1 rolled: %2").arg(currentPlayer + 1).arg(value));

    // Check if player has any valid moves
    Player *currentPlayerObj = players[currentPlayer];
    bool hasValidMove = false;
    QVector<int> validTokens;

    for (int i = 0; i < 4; i++)
    {
        if (isValidMove(currentPlayerObj, i, value))
        {
            hasValidMove = true;
            validTokens.append(i);
        }
    }

    if (hasValidMove)
    {
        // If there are valid moves, let player choose which token to move
        if (validTokens.size() > 1)
        {
            selectTokenDialog(validTokens, value);
        }
        else
        {
            // If only one valid move, do it automatically
            moveToken(currentPlayerObj, validTokens[0], value);
        }
    }
    else
    {
        statusLabel->setText("No valid moves available. Next player's turn.");
        nextTurn();
    }

    gameMutex.unlock();
}

void MainWindow::selectTokenDialog(const QVector<int> &validTokens, int diceValue)
{
    QDialog dialog(this);
    dialog.setWindowTitle("Select Token to Move");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    for (int tokenId : validTokens)
    {
        QPushButton *btn = new QPushButton(QString("Move Token %1").arg(tokenId + 1), &dialog);
        connect(btn, &QPushButton::clicked, [this, tokenId, diceValue, &dialog]()
                {
            moveToken(players[currentPlayer], tokenId, diceValue);
            dialog.accept(); });
        layout->addWidget(btn);
    }

    dialog.exec();
}

bool MainWindow::isValidMove(Player *player, int tokenId, int steps)
{
    Token *token = player->getToken(tokenId);

    // If token is in yard, need a 6 to start
    if (token->isInYard)
    {
        return steps == 6;
    }

    // Check if move would exceed board limits
    int newPosition = calculateNewPosition(player, token->position, steps);
    if (newPosition >= BOARD_POSITIONS)
    {
        return false;
    }

    // Check if landing position is occupied by same player's token
    QPoint newPos = getboardCoordinates(newPosition, player->getId());
    for (Token *otherToken : player->getTokens())
    {
        if (otherToken != token &&
            getboardCoordinates(otherToken->position, player->getId()) == newPos)
        {
            return false;
        }
    }

    return true;
}

void MainWindow::moveToken(Player *player, int tokenId, int steps)
{
    Token *token = player->getToken(tokenId);

    // Handle moving out of yard
    if (token->isInYard && steps == 6)
    {
        QPoint startPos = PLAYER_START_POSITIONS[player->getId()];
        token->position = player->getId() * 13; // Starting position on main track
        token->isInYard = false;
        player->updateTokenGraphics(tokenId, startPos.x() * CELL_SIZE, startPos.y() * CELL_SIZE);
    }
    // Normal movement
    else if (!token->isInYard)
    {
        int newPosition = calculateNewPosition(player, token->position, steps);

        // Check for captures
        handleCaptures(player, newPosition);

        // Update token position
        token->position = newPosition;
        QPoint boardPos = getboardCoordinates(newPosition, player->getId());
        player->updateTokenGraphics(tokenId, boardPos.x() * CELL_SIZE, boardPos.y() * CELL_SIZE);
    }

    // Check for winning condition
    if (player->hasWon())
    {
        gameStarted = false;
        QMessageBox::information(this, "Game Over",
                                 QString("Player %1 has won!").arg(player->getId() + 1));
        rollButton->setEnabled(false);
        return;
    }

    // Give another turn if rolled a 6
    if (steps != 6)
    {
        nextTurn();
    }
}

void MainWindow::nextTurn()
{
    currentPlayer = (currentPlayer + 1) % 4;
    statusLabel->setText(QString("Player %1's turn").arg(currentPlayer + 1));
}

QPoint MainWindow::getboardCoordinates(int position, int playerId)
{
    // Logic to convert track positions to board coordinates based on Ludo rules.
    // This is placeholder logic and should be expanded for real board mapping.
    int x = (position % 15) * CELL_SIZE;
    int y = (position / 15) * CELL_SIZE;
    return QPoint(x, y);
}

void MainWindow::handleCaptures(Player *player, int newPosition)
{
    QPoint newPos = getboardCoordinates(newPosition, player->getId());

    for (Player *otherPlayer : players)
    {
        if (otherPlayer == player)
            continue;

        for (Token *otherToken : otherPlayer->getTokens())
        {
            QPoint otherPos = getboardCoordinates(otherToken->position, otherPlayer->getId());
            if (newPos == otherPos)
            {
                // Send captured token back to yard
                otherToken->isInYard = true;
                otherToken->position = -1;
                QPoint yardPos = PLAYER_YARD_POSITIONS[otherPlayer->getId()][0];
                otherPlayer->updateTokenGraphics(otherToken->getId(),
                                                 yardPos.x() * CELL_SIZE, yardPos.y() * CELL_SIZE);
            }
        }
    }
}

int MainWindow::calculateNewPosition(Player *player, int currentPos, int steps)
{
    // Calculate new position considering board wrapping and home stretch
    // This is a simplified version - you'll need to implement actual Ludo rules
    return (currentPos + steps) % BOARD_POSITIONS;
}
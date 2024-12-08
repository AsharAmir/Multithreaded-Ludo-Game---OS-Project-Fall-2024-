#include "i222420_i222599_i228752_ludo_game.h"
#include <QPainter>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <iostream>
#include <thread>
#include <QPainterPath>
#include <QThread>
#include "i222420_i222599_i228752_constants.h"
#include <QMessageBox>
#include <random>
#include <QPushButton>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QIcon>
#include <QFont>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>



#include <map>
using namespace std;


LudoGame::LudoGame(QWidget *parent, int numTokens) : QMainWindow(parent), remainingTime(10)// Updated constructor
{
    setFixedSize(BOARD_WIDTH, BOARD_HEIGHT);
    gameDice.shape.moveCenter(QPointF(GRID_SIZE * TILE_SIZE + 30 + DICE_SIZE / 2, 200 + DICE_SIZE / 2));

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&LudoGame::update));
    timer->start(16); // ~60 FPS

    initializePlayers(numTokens); // Pass numTokens to initializePlayers
    initializePaths();
    startGame();

    initializeNoGoPaths();

    pauseButton = new QPushButton("PAUSE", this);
    pauseButton->setGeometry(width() - 100, 10, 80, 40);  // Position at the top-right corner
    pauseButton->setStyleSheet("QPushButton { "
                            "background-color: white; "
                            "border: 2px solid black; "
                            "border-radius: 10px; "
                            "color: black; "
                            "font-weight: bold; "
                            "font-size: 14px; "
                            "}");

    connect(pauseButton, &QPushButton::clicked, this, &LudoGame::onPauseButtonClicked);

    createPauseDialog();

    turnTimer = new QTimer(this);
    turnTimer->setInterval(1000);
    connect(turnTimer, &QTimer::timeout, this, &LudoGame::updateTurnTimer);
}

void LudoGame::createPauseDialog()
{
    pauseDialog = new QDialog(this);
    pauseDialog->setWindowTitle("Pause Menu");
    pauseDialog->setFixedSize(220, 150);  // Set fixed size for the dialog

    // Create Resume and Quit buttons
    QPushButton *resumeButton = new QPushButton("Resume", pauseDialog);
    QPushButton *quitButton = new QPushButton("Quit Game", pauseDialog);

    // Set custom styles for the buttons
    resumeButton->setStyleSheet("QPushButton { font-size: 16px; background-color: lightgreen; border-radius: 10px; }");
    quitButton->setStyleSheet("QPushButton { font-size: 16px; background-color: lightcoral; border-radius: 10px; }");

    // Connect buttons to their respective slots
    connect(resumeButton, &QPushButton::clicked, this, &LudoGame::onResumeButtonClicked);
    connect(quitButton, &QPushButton::clicked, this, &LudoGame::onQuitButtonClicked);

    // Arrange buttons vertically
    QVBoxLayout *layout = new QVBoxLayout(pauseDialog);
    layout->addWidget(resumeButton);
    layout->addWidget(quitButton);
    pauseDialog->setLayout(layout);
}

void LudoGame::onPauseButtonClicked()
{
    // Pause any timers or game logic (for example, if there's a turn timer)
    turnTimer->stop();

    // Dim the background to indicate pause state
    QGraphicsOpacityEffect *dimEffect = new QGraphicsOpacityEffect(this);
    dimEffect->setOpacity(0.5);
    setGraphicsEffect(dimEffect);

    // Show the pause dialog, blocking the game window
    pauseDialog->exec();

    // Restore the background after the dialog is closed
    setGraphicsEffect(nullptr);
}

void LudoGame::onResumeButtonClicked()
{
    // Resume the game (e.g., restart timers)
    turnTimer->start();

    // Close the pause dialog
    pauseDialog->close();
}

void LudoGame::onQuitButtonClicked()
{
    // Ask the user to confirm quitting
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Quit Game", "Are you sure you want to quit?",
                                                                 QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QApplication::quit();
    }
}

void LudoGame::initializeNoGoPaths() {
    // Blue no go path
    noGoPaths[0] = {QPoint(0, 6), QPoint(1, 6), QPoint(2, 6), QPoint(3, 6), QPoint(4, 6), QPoint(5, 6), 
                    QPoint(6, 5), QPoint(6, 4), QPoint(6, 3), QPoint(6, 2), QPoint(6, 1), QPoint(6, 0)};

    // Yellow no go path
    noGoPaths[1] = {QPoint(6, 13), QPoint(6, 12), QPoint(6, 11), QPoint(6, 10), QPoint(6, 9), QPoint(5, 8),
                    QPoint(4, 8), QPoint(3, 8), QPoint(2, 8), QPoint(1, 8), QPoint(0, 8), QPoint(0, 7), QPoint(0, 6)};

    // Red no go path
    noGoPaths[2] = {QPoint(8, 0), QPoint(8, 1), QPoint(8, 2), QPoint(8, 3), QPoint(8, 4), QPoint(8, 5),
                    QPoint(9, 6), QPoint(10, 6), QPoint(11, 6), QPoint(12, 6), QPoint(13, 6), QPoint(14, 6)};

    // Green no go path
    noGoPaths[3] = {QPoint(14, 8), QPoint(13, 8), QPoint(12, 8), QPoint(11, 8), QPoint(10, 8), QPoint(9, 8),
                    QPoint(8, 9), QPoint(8, 10), QPoint(8, 11), QPoint(8, 12), QPoint(8, 13), QPoint(8, 14)};
}


void LudoGame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // Enable anti-aliasing for smooth rendering
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw the Ludo Board
    drawLudoBoard(painter);
    
    // Draw Game Pieces
    drawPieces(painter);

    // Draw Dice
    drawDice(painter);

    drawScorecard(painter);


    // **Draw Game Title**
    painter.setPen(Qt::darkGray);
    painter.setFont(QFont("Arial", 20, QFont::Bold));
    // Move the title to the right of the board
    painter.drawText(QRect(GRID_SIZE * TILE_SIZE + 30, 60, 200, 50),
                     Qt::AlignCenter,
                     "Ludo Game");


    // **Draw Timer**
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    painter.setBrush(QBrush(QColor(255, 255, 255, 200))); // Semi-transparent background
    painter.drawRoundedRect(QRect(GRID_SIZE * TILE_SIZE + 40, 10, 120, 40), 8, 8);
    painter.drawText(QRect(GRID_SIZE * TILE_SIZE + 40, 10, 120, 40),
                     Qt::AlignCenter,
                     QString("Time: %1s").arg(remainingTime));

    // **Draw Current Player Indicator**
    QColor playerColors[5] = {Qt::blue, Qt::yellow, Qt::red, Qt::green, Qt::white};

    QColor playerBorderColors[4] = {Qt::darkBlue, Qt::darkYellow, Qt::darkRed, Qt::darkGreen};

    // **Player Label**
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 14, QFont::Bold));

    QString playerLabels[5] = {"Blue", "Yellow", "Red", "Green", "Game Start"};
    QString playerText = QString("%1's turn now").arg(playerLabels[currentPlayer]);

    // Use QFontMetrics to calculate the bounding box for the text
    QFontMetrics fontMetrics(painter.font());
    QRect textRect = fontMetrics.boundingRect(playerText);

    // Add some padding to the calculated bounding rectangle
    int padding = 10; // Adjust padding as needed
    textRect.adjust(-padding, -padding, padding, padding);

    // Move the rectangle to the desired position
    textRect.moveTo(GRID_SIZE * TILE_SIZE + 10, 110);

    // Draw the rounded rectangle around the text with player color
    painter.setBrush(playerColors[currentPlayer]);
    painter.drawRoundedRect(textRect, 8, 8);

    // Draw the text inside the rectangle
    painter.drawText(textRect, Qt::AlignCenter, playerText);


}

QPointF LudoGame::calculateSidebarPosition(int playerId, int tokenIndex) {
    // Adjust these values as needed for spacing and alignment
    const int sidebarOffsetX = GRID_SIZE * TILE_SIZE + 50; // X offset for the sidebar
    const int sidebarOffsetY = 100; // Starting Y offset
    const int tokenSpacing = TILE_SIZE; // Spacing between tokens

    float x = sidebarOffsetX;
    float y = sidebarOffsetY + (playerId * 100) + (tokenIndex * tokenSpacing);

    return QPointF(x, y);
}

void LudoGame::drawScorecard(QPainter &painter) {
    // Define colors and labels for players
    QColor colors[4] = {QColor(0, 0, 139), QColor(184, 134, 11), QColor(139, 0, 0), QColor(50, 205, 50)};
    QString playerLabels[4] = {"Blue", "Yellow", "Red", "Green"};

    // Table dimensions (220 pixels wide in total)
    const int TABLE_WIDTH = 220; // Total table width
    const int TABLE_HEIGHT = 160; // Total height for 4 rows of players (40px each)
    const int TILE_SIZE = TABLE_WIDTH / 3; // Each column will be 220/3 = 73px wide

    const int MARGIN = 30; // Margin from the bottom-right corner

    // Calculate position for bottom-right corner
    int xPos = width() - TABLE_WIDTH - MARGIN; 
    int yPos = height() - TABLE_HEIGHT - MARGIN;

    // Draw grid (for player information)
    painter.setPen(QPen(Qt::black, 2)); // Grid lines
    painter.setBrush(Qt::white);

    // Draw grid cells
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 3; ++col) {
            painter.drawRect(xPos + col * TILE_SIZE, yPos + row * 40, TILE_SIZE, 40);
        }
    }

    // Draw the player data in the grid (Player Name, Score, Tokens)
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        int row = i; // Row for each player (0 to 3)
        
        // Player Name Column (first column)
        QRectF nameRect(xPos + 0 * TILE_SIZE + 5, yPos + row * 40 + 5, TILE_SIZE - 10, 40 - 10);
        painter.drawText(nameRect, Qt::AlignLeft, playerLabels[i]);

        // Score Column (second column)
        QRectF scoreRect(xPos + 1 * TILE_SIZE + 5, yPos + row * 40 + 5, TILE_SIZE - 10, 40 - 10);
        painter.drawText(scoreRect, Qt::AlignLeft, QString::number(players[i].score));

        // Tokens Column (third column)
        QRectF tokensRect(xPos + 2 * TILE_SIZE + 5, yPos + row * 40 + 5, TILE_SIZE - 10, 40 - 10);
        // painter.drawText(tokensRect, Qt::AlignCenter, QString("Tokens"));

        // Draw completed tokens as small colored circles side by side horizontally
        int tokenXPos = xPos + (2 * TILE_SIZE) + 5; // Right next to the "Tokens" column
        int tokenYPos = yPos + row * 40 + 40 / 2 - 5; // Centering vertically

        // Adjust horizontally (spacing between tokens)
        int tokenSpacing = 15; // Adjust this value for spacing between tokens

        for (size_t j = 0; j < players[i].completedTokens.size(); ++j) {
            painter.setBrush(colors[i]);
            painter.setPen(QPen(Qt::black, 2));
            // Draw each token side by side horizontally
            painter.drawEllipse(tokenXPos + j * tokenSpacing, tokenYPos, 12, 12); // Draw token circles
        }
    }

    // Draw headers (larger font for headers)
    painter.setFont(QFont("Arial", 12, QFont::Bold)); // Larger font for headers

    QRectF headerNameRect(xPos, yPos - 30, TILE_SIZE, 40);
    painter.drawText(headerNameRect, Qt::AlignCenter, "Player");

    QRectF headerScoreRect(xPos + TILE_SIZE, yPos - 30, TILE_SIZE, 40);
    painter.drawText(headerScoreRect, Qt::AlignCenter, "Score");

    QRectF headerTokensRect(xPos + TILE_SIZE * 2, yPos - 30, TILE_SIZE, 40);
    painter.drawText(headerTokensRect, Qt::AlignCenter, "Tokens");
}


void LudoGame::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QPointF mousePos = event->pos();

        // Handle dice roll
        if (gameDice.shape.contains(mousePos) && !gameDice.isRolling && !waitingForMove)
        {
            rollDice();

            // Check if the current player has no valid moves
            bool hasValidMoves = false;
            if (gameDice.value == 6)
            {
                // With a 6, player can either bring a new token into play or move existing ones
                for (const auto &token : players[currentPlayer].tokens)
                {
                    if (!token.inPlay || token.inPlay)
                    {
                        hasValidMoves = true;
                        break;
                    }
                }
            }
            else
            {
                // For other rolls, check if any token in play can be moved
                for (const auto &token : players[currentPlayer].tokens)
                {
                    if (token.inPlay)
                    {
                        hasValidMoves = true;
                        break;
                    }
                }
            }

            // If no valid moves, automatically advance to the next player
            if (!hasValidMoves)
            {
                waitingForMove = false;
                selectedToken = nullptr;
                turnTimer->stop();
                consecutiveSixes = 0;
                advanceTurn();
                update();
            }
            else
            {
                waitingForMove = true; // Set this flag to ensure player gets to move
            }
        }
        // Handle token selection when waiting for move
        else if (waitingForMove)
        {
            handleTokenSelection(mousePos);
        }
    }
}

void LudoGame::calculateHighlightedPositions() {
    highlightedCells.clear(); // Clear any previous highlights

    const auto& player = players[currentPlayer];
    const auto& path = playerPaths[currentPlayer];

    for (const auto& token : player.tokens) {
        if (!token.inPlay && gameDice.value == 6) {
            // Token can enter play, highlight the starting position
            highlightedCells.push_back(path[0]);
        } else if (token.inPlay) {
            int newPosition = token.position + gameDice.value;

            // Ensure the new position is within the path
            if (newPosition < path.size()) {
                highlightedCells.push_back(path[newPosition]);
            }
        }
    }

    update(); // Trigger a repaint to show the highlights
}

int globalNumTokens = 0;
void LudoGame::initializePlayers(int numTokens) // Updated method signature
{
    globalNumTokens = numTokens; // Store the number of tokens in a global variable
    if (numTokens < MIN_TOKENS)
        numTokens = MIN_TOKENS;
    if (numTokens > MAX_TOKENS)
        numTokens = MAX_TOKENS;

    // Initialize players in the correct order: Blue (0), Yellow (1), Green (3), Red (2)
    int playerOrder[MAX_PLAYERS] = {0, 1, 3, 2};
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        int playerId = playerOrder[i];
        players[playerId] = Player(playerId);
        std::vector<Token> newTokens(numTokens);
        players[playerId].tokens = std::move(newTokens);

        for (auto &token : players[playerId].tokens)
        {
            token.inPlay = false;
            token.position = players[playerId].homePosition;
        }
    }
}

void LudoGame::startGame()
{
    gameRunning = true;
    currentPlayer = 4;

    std::thread master(&LudoGame::masterThread, this);
    master.detach();

    for (int i = 0; i < MAX_PLAYERS; ++i)
    {
        std::thread player(&LudoGame::playerThread, this, i);
        player.detach();
    }
}

void LudoGame::initializePaths() {
    // Define full paths for each player, including normal and home paths.

    for (int i = 0; i < 4; ++i) {
        playerPaths[i].clear();
    }
    
    // Blue's path
    playerPaths[0] = {
        // Normal flow
        {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6},
        {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0},
        {7, 0}, {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5},
        {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6},
        {14, 7}, {14, 8}, {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8},
        {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14},
        {7, 14}, {6, 14}, {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9},
        {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7},
        // Home flow
        {1, 7}, {2, 7}, {3, 7}, {4, 7}, {5, 7}, {6, 7}
    };

    // Yellow's path
    playerPaths[1] = {
        // Normal flow
        {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9},
        {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7}, {0, 6},
        {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6},
        {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0},
        {7, 0}, {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5},
        {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6},
        {14, 7}, {14, 8}, {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8},
        {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14},
        {7, 14},
        // Home flow
        {7, 13}, {7, 12}, {7, 11}, {7, 10}, {7, 9}, {7, 8}
    };

    // Red's path
    playerPaths[2] = {
        // Normal flow
        {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5},
        {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6},
        {14, 7}, {14, 8}, {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8},
        {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14},
        {7, 14}, {6, 14}, {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9},
        {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7}, {0, 6},
        {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6},
        {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0},
        {7, 0},
        // Home flow
        {7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6}
    };

    // Green's path
    playerPaths[3] = {
        // Normal flow
        {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8},
        {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14},
        {7, 14}, {6, 14}, {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9},
        {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7}, {0, 6},
        {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6},
        {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0},
        {7, 0}, {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5},
        {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6},
        {14, 7},
        // Home flow
        {13, 7}, {12, 7}, {11, 7}, {10, 7}, {9, 7}, {8, 7}
    };

    initializePlayerPaths();
}

void LudoGame::initializePlayerPaths()
{
    // Safe zones for each player (leading to the center)
    playerSafePaths[0]  = {{1, 7}, {2, 7}, {3, 7}, {4, 7}, {5, 7}, {6, 7}};  // Blue safe zone
    playerSafePaths[2]  = {{7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6}};    // yellow safe zone
    playerSafePaths[1]  = {{7, 13}, {7, 12}, {7, 11}, {7, 10}, {7, 9}, {7, 8}};  // Red safe zone
    playerSafePaths[3]  = {{13, 7}, {12, 7}, {11, 7}, {10, 7}, {9, 7}, {8, 7}}; // Green safe zone
}

void LudoGame::drawLudoBoard(QPainter &painter)
{
    QColor greenColor(50, 205, 50);    // Bright Lime Green
    QColor redColor(220, 20, 60);      // Crimson Red
    QColor blueColor(30, 144, 255);    // Dodger Blue
    QColor yellowColor(255, 215, 0);   // Gold
    QColor greyColor(169, 169, 169);   // Light Gray (for neutral areas)
    QColor whiteColor(255, 255, 255);  // White (for background or highlight)



    // Create the grid
    for (int row = 0; row < GRID_SIZE; ++row)
    {
        for (int col = 0; col < GRID_SIZE; ++col)
        {
            QRectF tile(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE - 1, TILE_SIZE - 1);
            painter.fillRect(tile, whiteColor);

            // Define the home areas
            if (row < 6 && col < 6)
                painter.fillRect(tile, blueColor);
            else if (row < 6 && col > 8)
                painter.fillRect(tile, redColor);
            else if (row > 8 && col < 6)
                painter.fillRect(tile, yellowColor);
            else if (row > 8 && col > 8)
                painter.fillRect(tile, greenColor);

            // Define player paths
            if ((row == 7 && col < 6) || (row == 7 && col > 8))
                painter.fillRect(tile, whiteColor);
            if ((col == 7 && row < 6) || (col == 7 && row > 8))
                painter.fillRect(tile, whiteColor);

            // Safe zones
            if ((row == 6 && col == 1) || (row == 1 && col == 8) ||
                (row == 8 && col == 13) || (row == 13 && col == 6) || 
                (row == 2 && col == 6)  || (row == 6 && col == 12) ||
                (row == 8 && col == 2)  || (row == 12 && col == 8))
                painter.fillRect(tile, greyColor);
        }
    }

    // Draw colored paths
    for (int i = 1; i < 6; ++i)
    {
        painter.fillRect(QRectF(7 * TILE_SIZE, i * TILE_SIZE, TILE_SIZE - 1, TILE_SIZE - 1), redColor);
        painter.fillRect(QRectF((14 - i) * TILE_SIZE, 7 * TILE_SIZE, TILE_SIZE - 1, TILE_SIZE - 1), greenColor);
        painter.fillRect(QRectF(7 * TILE_SIZE, (14 - i) * TILE_SIZE, TILE_SIZE - 1, TILE_SIZE - 1), yellowColor);
        painter.fillRect(QRectF(i * TILE_SIZE, 7 * TILE_SIZE, TILE_SIZE - 1, TILE_SIZE - 1), blueColor);
    }

    // Draw the center square
    QPolygonF centerSquare;
    centerSquare << QPointF(6 * TILE_SIZE, 6 * TILE_SIZE)
                 << QPointF(9 * TILE_SIZE, 6 * TILE_SIZE)
                 << QPointF(9 * TILE_SIZE, 9 * TILE_SIZE)
                 << QPointF(6 * TILE_SIZE, 9 * TILE_SIZE);
    QPainterPath centerSquarePath;
    centerSquarePath.addPolygon(centerSquare);
    painter.fillPath(centerSquarePath, whiteColor);

    // Draw colored triangles in the center
    QPolygonF centerTriangle;

    // Blue triangle
    centerTriangle.clear();
    centerTriangle << QPointF(7.5f * TILE_SIZE, 7.5f * TILE_SIZE)
                   << QPointF(6 * TILE_SIZE, 6 * TILE_SIZE)
                   << QPointF(9 * TILE_SIZE, 6 * TILE_SIZE);
    QPainterPath blueTrianglePath;
    blueTrianglePath.addPolygon(centerTriangle);
    painter.fillPath(blueTrianglePath, redColor);

    // Yellow triangle
    centerTriangle.clear();
    centerTriangle << QPointF(7.5f * TILE_SIZE, 7.5f * TILE_SIZE)
                   << QPointF(9 * TILE_SIZE, 6 * TILE_SIZE)
                   << QPointF(9 * TILE_SIZE, 9 * TILE_SIZE);
    QPainterPath yellowTrianglePath;
    yellowTrianglePath.addPolygon(centerTriangle);
    painter.fillPath(yellowTrianglePath, greenColor);

    // Green triangle
    centerTriangle.clear();
    centerTriangle << QPointF(7.5f * TILE_SIZE, 7.5f * TILE_SIZE)
                   << QPointF(9 * TILE_SIZE, 9 * TILE_SIZE)
                   << QPointF(6 * TILE_SIZE, 9 * TILE_SIZE);
    QPainterPath greenTrianglePath;
    greenTrianglePath.addPolygon(centerTriangle);
    painter.fillPath(greenTrianglePath, yellowColor);

    // Red triangle
    centerTriangle.clear();
    centerTriangle << QPointF(7.5f * TILE_SIZE, 7.5f * TILE_SIZE)
                   << QPointF(6 * TILE_SIZE, 9 * TILE_SIZE)
                   << QPointF(6 * TILE_SIZE, 6 * TILE_SIZE);
    QPainterPath redTrianglePath;
    redTrianglePath.addPolygon(centerTriangle);
    painter.fillPath(redTrianglePath, blueColor);


    //highlight the possible moves
    painter.setBrush(QColor(128, 0, 128, 128)); // Semi-transparent purple
    painter.setPen(Qt::NoPen);

    for (const auto& cell : highlightedCells) {
        float x = cell.x() * TILE_SIZE;
        float y = cell.y() * TILE_SIZE;
        painter.drawRect(QRectF(x, y, TILE_SIZE, TILE_SIZE));
    }
}

void LudoGame::drawPieces(QPainter &painter)
{
    // Color definition for the players
    QColor colors[4] = {
        QColor(0, 0, 139),  // Dodger Blue: Bright and lively
        QColor(184, 134, 11),   // Golden Yellow: Warm and vibrant
        QColor(139, 0, 0),   // Crimson Red: Bold and striking
        QColor(50, 205, 50)    // Lime Green: Bright and fresh
    };

    // Draw tokens for each player
    for (int p = 0; p < MAX_PLAYERS; p++) {
        for (const auto &token : players[p].tokens) {
            std::lock_guard<std::mutex> lock(token.tokenMutex);

            QRectF tokenRect;
            float x = 0, y = 0; // Initialize for debugging

            if (!token.inPlay && !token.scored) {
                // Calculate home position for tokens not in play
                int tokenIndex = &token - &players[p].tokens[0];
                int row = tokenIndex / 2;
                int col = tokenIndex % 2;

                int startPositions[4][2] = {{2, 2}, {2, 11}, {11, 2}, {11, 11}};
                x = (startPositions[p][0] + col) * TILE_SIZE + (TILE_SIZE - PIECE_RADIUS * 2) / 2;
                y = (startPositions[p][1] + row) * TILE_SIZE + (TILE_SIZE - PIECE_RADIUS * 2) / 2;

                tokenRect = QRectF(x, y, PIECE_RADIUS * 2, PIECE_RADIUS * 2);
            } else {
                // Use playerPaths for all token positions
                const std::vector<QPoint>& path = playerPaths[p];

                if (token.position < 0 || token.position >= path.size()) {
                    std::cerr << "[ERROR] Invalid token position: " << token.position
                              << " for player " << p << ". Path size: " << path.size() << std::endl;
                    continue;
                }

                // Get position from the player's path
                const QPoint& pathPos = path[token.position];
                x = pathPos.x() * TILE_SIZE + (TILE_SIZE - PIECE_RADIUS * 2) / 2;
                y = pathPos.y() * TILE_SIZE + (TILE_SIZE - PIECE_RADIUS * 2) / 2;

                tokenRect = QRectF(x, y, PIECE_RADIUS * 2, PIECE_RADIUS * 2);
            }


            painter.setBrush(colors[p]);
            painter.setPen(QPen(Qt::black, 2));
            painter.drawEllipse(tokenRect);
        }
    }
}


void LudoGame::drawDice(QPainter &painter)
{
    painter.save();

    // Set dice position (30px to the right and down)
    painter.translate(gameDice.shape.center());

    // Smooth dice rolling animation
    if (gameDice.isRolling)
    {
        qint64 elapsedTime = gameDice.rollClock.elapsed();
        if (elapsedTime < 1000) // Rolling animation lasts 1 second
        {
            gameDice.rotation += 10.0f; // Faster rotation for a more dynamic feel
            painter.rotate(gameDice.rotation);
        }
        else
        {
            gameDice.isRolling = false;
        }
    }

    // Draw dice background (simulate 3D effect)
    QRectF diceRect(-DICE_SIZE / 2, -DICE_SIZE / 2, DICE_SIZE, DICE_SIZE);
    QLinearGradient gradient(diceRect.topLeft(), diceRect.bottomRight());
    gradient.setColorAt(0.0, QColor(255, 255, 255)); // Top-left highlight
    gradient.setColorAt(0.5, QColor(230, 230, 230)); // Mid-shadow
    gradient.setColorAt(1.0, QColor(180, 180, 180)); // Bottom-right shadow
    painter.setBrush(gradient);
    painter.setPen(QPen(Qt::black, 3));
    painter.drawRoundedRect(diceRect, 10, 10); // Rounded edges for realism

    // Add slight 3D edge effect
    QRectF smallerRect = diceRect.adjusted(2, 2, -2, -2);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(200, 200, 200, 150)); // Light shadow on edges
    painter.drawRoundedRect(smallerRect, 8, 8);

    // Draw dots on the dice
    painter.setBrush(Qt::black); // Standard black dots
    QVector<QPointF> dotPositions;
    switch (gameDice.value)
    {
    case 1:
        dotPositions = {{0, 0}};
        break;
    case 2:
        dotPositions = {{-10, -10}, {10, 10}};
        break;
    case 3:
        dotPositions = {{-10, -10}, {0, 0}, {10, 10}};
        break;
    case 4:
        dotPositions = {{-10, -10}, {10, -10}, {-10, 10}, {10, 10}};
        break;
    case 5:
        dotPositions = {{-10, -10}, {10, -10}, {0, 0}, {-10, 10}, {10, 10}};
        break;
    case 6:
        dotPositions = {{-10, -10}, {-10, 0}, {-10, 10}, {10, -10}, {10, 0}, {10, 10}};
        break;
    }

    // Draw dots with slight depth effect
    for (const auto &pos : dotPositions)
    {
        painter.setBrush(Qt::black);
        painter.drawEllipse(pos, 5, 5); // Larger dots for better visibility

        // Add highlight to the dots for realism
        painter.setBrush(QColor(70, 70, 70));
        painter.drawEllipse(pos.x() - 1, pos.y() - 1, 3, 3);
    }

    painter.restore();

    painter.setFont(QFont("Arial", 20, QFont::Bold));
    painter.setPen(Qt::black);
    painter.drawText(gameDice.shape.center().x(), gameDice.shape.center().y() + DICE_SIZE + 90,
                     "Roll the Dice"); // Text positioned near the dice
}



void LudoGame::rollDice()
{
    gameDice.value = QRandomGenerator::global()->bounded(1, 7);
    // testing
    // gameDice.value = 6;
    gameDice.isRolling = true;
    gameDice.rollClock.restart();
    gameDice.rotation = 0;
    players[currentPlayer].hasHit = false;
    remainingTime = 10;
    turnTimer->stop();

    if (gameDice.value == 6)
    {
        std::cout << "Player " << currentPlayer << " rolled a 6!" << std::endl;
        players[currentPlayer].unsuccessfulTurnsSixes = 0;
        cout << "Current Player " << currentPlayer << " Unsuccessful Turns : " << players[currentPlayer].unsuccessfulTurnsSixes << endl
             << endl;
        // QMessageBox::information(this, "Ludo Game", "You rolled a 6!");
        waitingForMove = true;
        calculateHighlightedPositions();
        selectedToken = nullptr;
        turnTimer->start();
    }
    else
    {
        players[currentPlayer].unsuccessfulTurnsSixes++;
        cout << "Current Player " << currentPlayer << " Unsuccessful Turns : " << players[currentPlayer].unsuccessfulTurnsSixes << endl
             << endl;
        consecutiveSixes = 0;
        waitingForMove = true;
        calculateHighlightedPositions();
        selectedToken = nullptr;
        turnTimer->start();
    }

    update();
}



void LudoGame::advanceTurn()
{
    if (currentRoundPlayers.empty()) // If round is over, prepare a new random order
    {
      // Reinitialize the players array
        currentRoundPlayers = {0, 1, 2, 3};
        std::shuffle(currentRoundPlayers.begin(), currentRoundPlayers.end(), *QRandomGenerator::global());
   }

    // Assign the next player and remove them from the round
    currentPlayer = currentRoundPlayers.back();

    std::cout << "Initial token coordinates for Player " << currentPlayer << ":\n";
    for (size_t i = 0; i < players[currentPlayer].tokens.size(); ++i) {
        std::cout << "Token " << i << ": (" << players[currentPlayer].tokens[i].col << ", " 
                  << players[currentPlayer].tokens[i].row << ")\n";
    }
    players[currentPlayer].previousPositions.clear();
    players[currentPlayer].wasInPlay.clear();

    for (auto& token : players[currentPlayer].tokens) {
        players[currentPlayer].previousPositions.push_back({token.col, token.row});
        players[currentPlayer].wasInPlay.push_back(token.inPlay);
    }

    currentRoundPlayers.pop_back();

    // Debugging output
    std::cout << "Turn advancing. Current player: " << currentPlayer
              << ". Remaining players: ";
    for (int p : currentRoundPlayers)
        std::cout << p << " ";
    std::cout << std::endl;

    // Reset state for the new turn
    waitingForMove = false;
    selectedToken = nullptr;
    remainingTime = 10;
    turnTimer->stop();
    turnTimer->start();

    update(); // Refresh UI or game state
}




void LudoGame::handleTokenSelection(const QPointF &mousePos)
{
    // Lock the board while handling selection
    std::lock_guard<std::mutex> lock(boardMutex);
    
    for (auto &token : players[currentPlayer].tokens)
    {
        QRectF tokenRect = calculateTokenRect(token);
        if (tokenRect.contains(mousePos))
        {
            bool validMove = false;

            if (gameDice.value == 6)
            {
                validMove = true;
                moveToken(token, gameDice.value);
                // moveTokenTest(token);
            }
            else if (token.inPlay)
            {
                validMove = true;
                moveToken(token, gameDice.value);
                // moveTokenTest(token);
            }

            if (validMove)
            {
                waitingForMove = false;
                selectedToken = nullptr;
                turnTimer->stop();

                if (gameDice.value == 6 && consecutiveSixes < 3)
                {
                    remainingTime = 10;
                }
                else
                {
                    advanceTurn();
                }
                update();
                return;
            }
        }
    }
}

void LudoGame::moveToken(Token &token, int spaces) {
    std::lock_guard<std::mutex> lock(token.tokenMutex);
    std::cout << "[DEBUG] Current Player: " << currentPlayer << " | Roll: " << spaces << std::endl;

    // Get the current player's path
    std::vector<QPoint>& path = playerPaths[currentPlayer];

    // Token enters play if it's not in play and the roll is 6
    if (!token.inPlay && spaces == 6) {
        token.inPlay = true;
        token.position = 0; // Start at the first position of the path
        token.row = path[token.position].y(); // Use y() method
        token.col = path[token.position].x(); // Use x() method
        token.hasCompletedCycle = false; // Reset cycle completion when token enters play
        std::cout << "[DEBUG] Token entered play at: (" << token.col << ", " << token.row << ") | Position: " << token.position << std::endl;
        update();
        return;
    }

    // If the token is in play
    if (token.inPlay) {
        int newPosition = token.position + spaces;

        // Check if the token is still within the player's path
        if (newPosition >= path.size()) {
            std::cout << "[DEBUG] Token cannot move, end of path reached." << std::endl;
            update();
            return;
        }

        // Check if the token completes its first cycle (if applicable)
        if (!token.hasCompletedCycle && newPosition < token.position) {
            token.hasCompletedCycle = true;
            std::cout << "[DEBUG] Token has completed its first cycle." << std::endl;
        }

        // Check if the token reaches the home path
        if (newPosition >= path.size() - 6 && !token.readyForHome) { // Last 6 tiles considered home path
            token.readyForHome = true;
            std::cout << "[DEBUG] Token is transitioning to home path." << std::endl;
        }

        // Handle movement along the path
        token.position = newPosition;
        token.row = path[newPosition].y(); // Use y() method
        token.col = path[newPosition].x(); // Use x() method
        std::cout << "[DEBUG] Token moved to: (" << token.col << ", " << token.row << ") | Position: " << newPosition << std::endl;

        // Check if the token has reached the end of its path
        if (newPosition == path.size() - 1) {
            // Mark the token as completed
            token.inPlay = false;
            token.scored = true;
            std::cout << "[DEBUG] Token has scored a point by reaching the end of the path." << std::endl;

            // Move the token to the sidebar and update the player's score
            QPointF sidebarPosition = calculateSidebarPosition(currentPlayer, players[currentPlayer].completedTokens.size());
            players[currentPlayer].completedTokens.push_back(sidebarPosition);
            players[currentPlayer].score++;  // Increase the player's score
            std::cout << "[DEBUG] Player " << currentPlayer << "'s score: " << players[currentPlayer].score << std::endl;

            // std::cout << "[[[DEBUG]]] Player " << currentPlayer << "'s score: " << players[currentPlayer].score << "NUMMTOKEN " << numTokens << std::endl;
            // token.inPlay = false;
            if (players[currentPlayer].score == globalNumTokens) {
                QString winnerColor;
                switch (currentPlayer) {
                    case 0: winnerColor = "Blue"; break;
                    case 1: winnerColor = "Yellow"; break;
                    case 2: winnerColor = "Red"; break;
                    case 3: winnerColor = "Green"; break;
                }

                QString message = QString("🎉 Congratulations, %1 Player! 🎉\n\n"
                                        "You have won the game!")
                                        .arg(winnerColor);

                QMessageBox msgBox;
                msgBox.setWindowTitle("Game Over");
                msgBox.setText(message);
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setStyleSheet(
                    "QMessageBox { background-color: #f0f8ff; font-family: Arial; font-size: 16px; } "
                    "QLabel { color: #333333; } "
                    "QPushButton { background-color: #87CEEB; border: none; padding: 5px; }");

                msgBox.exec(); // Show the message box

                QApplication::quit(); // End the application
            }


        // Check for hits
        checkAndProcessHits(token, newPosition, currentPlayer);
    }

    highlightedCells.clear();
    update();
}
}



void LudoGame::checkAndProcessHits(Token &attacker, int attackerPos, int playerId) {
    const auto& attackerPath = playerPaths[playerId];
    const std::vector<QPoint> safeZones = {
    {6, 2}, {8, 1}, {12, 6}, {13, 8}, {8, 12}, {6, 13}, {2, 8}, {1, 6}
};


    // Validate attacker's position
    if (attackerPos < 0 || attackerPos >= attackerPath.size()) {
        std::cerr << "[ERROR] Invalid attacker position: " << attackerPos << std::endl;
        return;
    }

    QPoint attackerCoord = attackerPath[attackerPos]; // Get attacker's board coordinates

    // Check against other players' tokens
    for (int p = 0; p < MAX_PLAYERS; ++p) {
        if (p == playerId) continue; // Skip self

        const auto& victimPath = playerPaths[p];
        for (auto& victim : players[p].tokens) {
            if (!victim.inPlay) continue;

            // Get victim's board coordinates
            if (victim.position < 0 || victim.position >= victimPath.size()) continue;
            QPoint victimCoord = victimPath[victim.position];

            // Compare positions for a hit
            if (attackerCoord == victimCoord) {
                // Ensure position is not in a safe zone
                if (std::find(safeZones.begin(), safeZones.end(), victimCoord) == safeZones.end()) {
                    victim.inPlay = false; // Reset victim's status
                    victim.position = -1;

                    // Update hit rates
                    players[p].hitRate--;
                    players[playerId].hitRate++;
                    players[playerId].unsuccessfulTurnsHits = 0;
                    players[playerId].hasHit = true;

                    std::cout << "[DEBUG] Player " << playerId << " hit Player " << p
                              << "'s token at (" << victimCoord.x() << ", " << victimCoord.y() << ")." << std::endl;
                } else {
                    std::cout << "[DEBUG] Token in safe zone at (" << victimCoord.x() << ", " << victimCoord.y() << ")." << std::endl;
                }
            }
        }
    }
}


QPoint LudoGame::getStartingPosition(int playerId)
{
    switch (playerId)
    {
    case 0: // Blue
        return QPoint(1, 6);
    case 1: // Yellow
        return QPoint(6, 13);
    case 2: // Red
        return QPoint(8, 1);
    case 3: // Green
        return QPoint(13, 8);
    default:
        return QPoint(0, 0);
    }
}


QPointF LudoGame::calculateBoardPosition(int playerId, int position) {
    const std::vector<QPoint>& path = playerPaths[playerId];

    // Validate position
    if (position < 0 || position >= path.size()) {
        return QPointF(-1, -1); // Return invalid point for out-of-bounds
    }

    // Get the grid coordinates
    int gridX = path[position].x();
    int gridY = path[position].y();

    // Convert grid coordinates to pixel coordinates
    float pixelX = gridX * TILE_SIZE;
    float pixelY = gridY * TILE_SIZE;

    return QPointF(pixelX, pixelY);
}



QRectF LudoGame::calculateTokenRect(const Token &token) {
    QPointF position;

    if (!token.inPlay && !token.scored) {
        // Calculate home position for tokens not in play
        int tokenIndex = &token - &players[currentPlayer].tokens[0];
        int row = tokenIndex / 2;
        int col = tokenIndex % 2;

        int startPositions[4][2] = {{2, 2}, {2, 11}, {11, 2}, {11, 11}};
        position = QPointF((startPositions[currentPlayer][0] + col) * TILE_SIZE,
                           (startPositions[currentPlayer][1] + row) * TILE_SIZE);
    } else {
        // Use board position for tokens in play
        position = calculateBoardPosition(currentPlayer, token.position);
    }

    // Center token in the grid square
    return QRectF(position.x() + (TILE_SIZE - PIECE_RADIUS * 2) / 2,
                  position.y() + (TILE_SIZE - PIECE_RADIUS * 2) / 2,
                  PIECE_RADIUS * 2, PIECE_RADIUS * 2);
}



void LudoGame::verifyTokenPosition(const Token &token)
{
    std::cout << "Token position verification:"
              << "\nPlayer: " << currentPlayer
              << "\nIn play: " << token.inPlay
              << "\nRow: " << token.row
              << "\nCol: " << token.col
              << "\nPosition: " << token.position << std::endl;
}

void LudoGame::updateTurnTimer()
{
    remainingTime--;
    if (remainingTime <= 0)
    {
        turnTimer->stop();
        if (waitingForMove)
        {
            waitingForMove = false;
            selectedToken = nullptr;
            consecutiveSixes = 0;
        }
        advanceTurn();
    }
    update();
}

void LudoGame::playerThread(int playerId)
{
    while (players[playerId].isActive && gameRunning)
    {
        int diceValue = 0;
        
        {
            std::unique_lock<std::mutex> diceLock(diceMutex);
            diceCV.wait(diceLock, [this, playerId] { return currentPlayer == playerId; });

            diceValue = QRandomGenerator::global()->bounded(1, 7);
            std::cout << "Player " << playerId << " rolled " << diceValue << std::endl;

            diceCV.notify_all();
        }

        {
            std::unique_lock<std::mutex> boardLock(boardMutex);
            boardCV.wait(boardLock, [this, playerId] { return currentPlayer == playerId; });

            bool moved = false;
            for (auto &token : players[playerId].tokens)
            {
                if (token.inPlay)
                {
                    int newPos = (token.position + diceValue) % BOARD_SQUARES;
                    checkAndProcessHits(token, newPos, playerId);
                    token.position = newPos;
                    moved = true;
                    break;
                }
            }

            lastPlayer = playerId;
            currentPlayer = (currentPlayer + 1) % MAX_PLAYERS;

            boardCV.notify_all();

            if (!moved)
            {
                std::cout << "Player " << playerId << " has no valid moves" << std::endl;
            }
        }
    }
}

void LudoGame::masterThread()
{
    while (gameRunning)
    {
        int activePlayers = 0;
        for (int i = 0; i < MAX_PLAYERS; ++i)
        {
            if (players[i].isActive)
                activePlayers++;
        }

        if (activePlayers <= 1)
        {
            gameRunning = false;
            std::cout << "Game Over!" << std::endl;
            break;
        }

        for (int i = 0; i < MAX_PLAYERS; ++i)
        {
            if (players[i].isActive && players[i].hitRate == 0)
            {
                std::cout << "Player " << i << " is eliminated due to low hit rate." << std::endl;
                players[i].isActive = false;
            }
        }

        QThread::currentThread()->msleep(1000);
    }
}


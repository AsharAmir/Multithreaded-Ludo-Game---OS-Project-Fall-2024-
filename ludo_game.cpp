#include "ludo_game.h"
#include <QPainter>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <iostream>
#include <thread>
#include <QPainterPath>
#include <QThread>
#include "constants.h"
#include <QMessageBox>
#include <random>

using namespace std;


LudoGame::LudoGame(QWidget *parent, int numTokens) : QMainWindow(parent), remainingTime(10) // Updated constructor
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

    turnTimer = new QTimer(this);
    turnTimer->setInterval(1000);
    connect(turnTimer, &QTimer::timeout, this, &LudoGame::updateTurnTimer);
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

    // **Draw Game Title**
    painter.setPen(Qt::darkGray);
    painter.setFont(QFont("Arial", 20, QFont::Bold));
    // Move the title to the right of the board
    painter.drawText(QRect(GRID_SIZE * TILE_SIZE - 10, 150, 200, 50),
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

    // painter.setBrush(playerColors[currentPlayer]);
    // painter.setPen(playerBorderColors[currentPlayer]);
    // painter.drawEllipse(GRID_SIZE * TILE_SIZE, 70, 40, 40);

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
    textRect.moveTo(GRID_SIZE * TILE_SIZE, 70);

    // Draw the rounded rectangle around the text with player color
    painter.setBrush(playerColors[currentPlayer]);
    painter.drawRoundedRect(textRect, 8, 8);

    // Draw the text inside the rectangle
    painter.drawText(textRect, Qt::AlignCenter, playerText);

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


void LudoGame::initializePlayers(int numTokens) // Updated method signature
{
    // std::cout << "Enter number of tokens per player (1-4): ";
    // std::cin >> numTokens;

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


// void LudoGame::initializePaths()
// {
//     // Shared main path (common to all players)
//     sharedPath = {
//         // Blue's path (left vertical)
//         {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6},
//         {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0},
        
//         // Transition to Red's path (top horizontal)
//         {7, 0}, {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5},
        
//         // Red's path (right vertical)
//         {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6},
//         {14, 7}, {14, 8}, {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8},
        
//         // Transition to Yellow's path (bottom horizontal)
//         {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14},
//         {7, 14}, {6, 14},
        
//         // Yellow's path (left vertical)
//         {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9}, {5, 8},
        
//         // Transition to Green's path (left horizontal)
//         {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7}, {0, 6}
//     };

//     // Initialize individual paths
//     initializePlayerPaths();
// }

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

            if (!token.inPlay) {
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

            // Debugging output for token rendering
            // std::cout << "[DEBUG] Player: " << p
            //           << " | Token: (" << x << ", " << y << ")"
            //           << " | In Play: " << token.inPlay
            //           << " | Ready for Home: " << token.readyForHome
            //           << " | Scored: " << token.scored
            //           << std::endl;

            painter.setBrush(colors[p]);
            painter.setPen(QPen(Qt::black, 2));
            painter.drawEllipse(tokenRect);
        }
    }
}



void LudoGame::drawDice(QPainter &painter)
{
    painter.save();

    // Position and transform for dice
    painter.translate(gameDice.shape.center());

    // Smooth dice rolling animation
    if (gameDice.isRolling)
    {
        qint64 elapsedTime = gameDice.rollClock.elapsed();
        if (elapsedTime < 700) // Rolling time extended to 700ms
        {
            gameDice.rotation += 5.0f; // Smooth rotation increment
            painter.rotate(gameDice.rotation);
        }
        else
        {
            gameDice.isRolling = false;
        }
    }

    // Draw dice background (3D effect)
    QRectF diceRect(-DICE_SIZE / 2, -DICE_SIZE / 2, DICE_SIZE, DICE_SIZE);
    QLinearGradient gradient(diceRect.topLeft(), diceRect.bottomRight());
    gradient.setColorAt(0.0, QColor(255, 255, 255)); // Highlight
    gradient.setColorAt(1.0, QColor(200, 200, 200)); // Shadow
    painter.setBrush(gradient);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRoundedRect(diceRect, 5, 5); // Rounded edges for a modern look

    // Draw dots on the dice
    if (!gameDice.isRolling)
    {
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

        // Draw dots
        for (const auto &pos : dotPositions)
        {
            painter.drawEllipse(pos, 4, 4); // Larger dots for better visibility
        }
    }

    painter.restore();

    // Add "Roll the Dice" text
    painter.setFont(QFont("Arial", 14, QFont::Bold));
    painter.setPen(Qt::black);
    painter.drawText(gameDice.shape.center().x() - 50, gameDice.shape.center().y() + DICE_SIZE + 20,
                     "Roll the Dice"); // Text positioned near the dice
}


void LudoGame::rollDice()
{
    gameDice.value = QRandomGenerator::global()->bounded(5, 7);
    //testing
    // gameDice.value = 6;
    gameDice.isRolling = true;
    gameDice.rollClock.restart();
    gameDice.rotation = 0;
    players[currentPlayer].hasHit=false;
    remainingTime = 10;
    turnTimer->stop();

    if (gameDice.value == 6)
    {
        players[currentPlayer].unsuccessfulTurnsSixes = 0;
        cout << "Current Player " << currentPlayer << " Unsuccessful Turns : " << players[currentPlayer].unsuccessfulTurnsSixes << endl << endl;
        // QMessageBox::information(this, "Ludo Game", "You rolled a 6!");
        waitingForMove = true;
        selectedToken = nullptr;
        turnTimer->start();
    }
    else
    {
        players[currentPlayer].unsuccessfulTurnsSixes++;
        cout << "Current Player " << currentPlayer << " Unsuccessful Turns : " << players[currentPlayer].unsuccessfulTurnsSixes << endl << endl;
        consecutiveSixes = 0;
        waitingForMove = true;
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

        // Declare the random number generator
        std::random_device rd;              // Seed generator
        std::mt19937 rng(rd());             // Mersenne Twister engine

        // Shuffle the players until we get a different player than the current one
        do {
            std::shuffle(currentRoundPlayers.begin(), currentRoundPlayers.end(), rng);
        } while (currentRoundPlayers.front() == currentPlayer);
    }

    // Assign the next player and remove them from the round
    currentPlayer = currentRoundPlayers.back();
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

// void LudoGame::moveTokenTest(Token &token) {
//     int spaces;
//     std::cout << "Enter the number of spaces to move the token: ";
//     std::cin >> spaces;

//     // Validate input
//     if (std::cin.fail() || spaces < 0) {
//         std::cin.clear(); // Clear the error flag
//         std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
//         std::cout << "Invalid input. Please enter a valid non-negative number." << std::endl;
//         return;
//     }

//     std::lock_guard<std::mutex> lock(token.tokenMutex);
//     std::cout << "[DEBUG] Current Player: " << currentPlayer << " | Roll: " << spaces << std::endl;

//     if (!token.inPlay && spaces == 6) {
//         token.inPlay = true;
//         int startingPositions[4] = {0, 39, 13, 26};
//         token.position = startingPositions[currentPlayer];
//         token.row = sharedPath[token.position].y;
//         token.col = sharedPath[token.position].x;
//         token.hasCompletedCycle = false; // Reset cycle completion when token enters play
//         std::cout << "[DEBUG] Token entered play at: (" << token.col << ", " << token.row << ") | Position: " << token.position << std::endl;
//         update();
//         return;
//     }

//     if (token.inPlay) {
//         int newPosition = token.position + spaces;
//         newPosition %= sharedPath.size(); // Wrap around the shared path

//         // Check if the token completes its first cycle
//         if (!token.hasCompletedCycle && newPosition < token.position) {
//             token.hasCompletedCycle = true;
//             std::cout << "[DEBUG] Token has completed its first cycle." << std::endl;
//         }

//         // Transition to the home path after completing a cycle
//         QPoint currentPos(sharedPath[newPosition].x, sharedPath[newPosition].y);
//         if (token.hasCompletedCycle &&
//             std::find(noGoPaths[currentPlayer].begin(), noGoPaths[currentPlayer].end(), currentPos) != noGoPaths[currentPlayer].end() &&
//             !token.readyForHome) {
//             token.readyForHome = true;
//             token.homePosition = (spaces - (newPosition - token.position) % playerSafePaths[currentPlayer].size());
//             if (token.homePosition < 0) token.homePosition += playerSafePaths[currentPlayer].size(); // Ensure non-negative
//             token.row = playerSafePaths[currentPlayer][token.homePosition].y;
//             token.col = playerSafePaths[currentPlayer][token.homePosition].x;
//             std::cout << "[DEBUG] Token is transitioning to home path at position: (" << token.col << ", " << token.row << ") | Home Position: " << token.homePosition << std::endl;
//             update();
//             return;
//         }

//         if (token.readyForHome) {
//             // Handle movement along the home path
//             int newHomePosition = token.homePosition + spaces;
//             if (newHomePosition < playerSafePaths[currentPlayer].size()) {
//                 token.homePosition = newHomePosition;
//                 token.row = playerSafePaths[currentPlayer][newHomePosition].y;
//                 token.col = playerSafePaths[currentPlayer][newHomePosition].x;
//                 std::cout << "[DEBUG] Token moved along home path to: (" << token.col << ", " << token.row << ") | Home Position: " << newHomePosition << std::endl;
//             } else {
//                 std::cout << "[DEBUG] Token has scored a point by reaching the end of the home path." << std::endl;
//                 token.inPlay = false;
//                 token.scored = true;
//             }
//             update();
//             return;
//         }

//         // Regular movement along the shared path
//         token.position = newPosition;
//         token.row = sharedPath[newPosition].y;
//         token.col = sharedPath[newPosition].x;
//         std::cout << "[DEBUG] Token continued along shared path to: (" << token.col << ", " << token.row << ") | Position: " << newPosition << std::endl;

//         checkAndProcessHits(token, newPosition, currentPlayer);
//     }

//     update();
// }


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
            token.inPlay = false;
            token.scored = true;
            std::cout << "[DEBUG] Token has scored a point by reaching the end of the path." << std::endl;
        }

        // Check for hits
        checkAndProcessHits(token, newPosition, currentPlayer);
    }

    update();
}




// void LudoGame::checkAndProcessHits(Token &token, int newPos, int playerId)
// {
//     // Define the safe zone coordinates
//     const std::vector<QPoint> safeZones = {
//         {6, 2}, {8, 1}, {12, 6}, {13, 8}, {8, 12}, {6, 13}, {2, 8}, {1, 6}
//     };

//     for (int p = 0; p < MAX_PLAYERS; p++)
//     {
//         if (p == playerId) continue; // Skip checking against the current player's tokens

//         for (auto &otherToken : players[p].tokens)
//         {
//             // Check if the other token is in play and occupies the same position as the current token
//             if (otherToken.inPlay && otherToken.position == newPos)
//             {
//                 // Check if the position is in a safe zone
//                 bool isSafe = false;

//                 for (const QPoint &safePos : safeZones)
//                 {
//                     if (playerPaths[p][newPos] == safePos) // Compare coordinates
//                     {
//                         isSafe = true;
//                         break;
//                     }
//                 }

//                 // If not in a safe zone, process the hit
//                 if (!isSafe)
//                 {
//                     // Deactivate the hit token and reset its position
//                     otherToken.inPlay = false;
//                     otherToken.position = -1; // Reset position to indicate it's no longer in play

//                     // Update hit rates
//                     players[p].hitRate--;
//                     players[playerId].hitRate++;
//                     players[playerId].unsuccessfulTurnsHits = 0;

//                     // Mark the hitting player
//                     players[playerId].hasHit = true;

//                     std::cout << "[DEBUG] Player " << playerId << " hit Player " << p
//                               << "'s token. Token is now inactive." << std::endl;
//                     std::cout << "[DEBUG] Attacker coordinates: (" << token.col << ", " << token.row << ")"
//                               << " | Victim coordinates: (" << otherToken.col << ", " << otherToken.row << ")" << std::endl;
//                 }
//                 else
//                 {
//                     std::cout << "[DEBUG] Player " << playerId << " attempted to hit Player " << p
//                               << "'s token, but it is in a safe zone." << std::endl;
//                 }
//             }
//         }
//     }
// }

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


// QPointF LudoGame::calculateBoardPosition(int playerId, int position)
// {
//     // Validate playerId
//     if (playerId < 0 || playerId >= MAX_PLAYERS) {
//         std::cerr << "[ERROR] Invalid player ID: " << playerId << std::endl;
//         return QPointF(0, 0);
//     }

//     // Get the player's specific path
//     const std::vector<QPoint>& path = playerPaths[playerId];

//     // Validate position
//     if (position < 0 || position >= path.size()) {
//         std::cerr << "[ERROR] Invalid position: " << position 
//                   << " for player " << playerId << ". Path size: " << path.size() << std::endl;
//         return QPointF(0, 0);
//     }

//     // Get the grid coordinates from the player's path
//     int gridX = path[position].x();
//     int gridY = path[position].y();

//     // Convert grid coordinates to pixel coordinates
//     float pixelX = gridX * TILE_SIZE + TILE_SIZE / 2;
//     float pixelY = gridY * TILE_SIZE + TILE_SIZE / 2;

//     return QPointF(pixelX, pixelY);
// }

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



// QRectF LudoGame::calculateTokenRect(const Token &token)
// {
//     if (!token.inPlay)
//     {
//         // Calculate home position for tokens not in play
//         int tokenIndex = 0;
//         for (size_t i = 0; i < players[currentPlayer].tokens.size(); ++i)
//         {
//             if (&players[currentPlayer].tokens[i] == &token)
//             {
//                 tokenIndex = i;
//                 break;
//             }
//         }

//         int row = tokenIndex / 2;
//         int col = tokenIndex % 2;

//         // Starting positions for each player's home area
//         int startPositions[4][2] = {
//             {2, 2},   // Blue
//             {2, 11},  // Yellow
//             {11, 2},  // Red
//             {11, 11}  // Green
//         };

//         float x = (startPositions[currentPlayer][0] + col) * TILE_SIZE + (TILE_SIZE - PIECE_RADIUS * 2) / 2;
//         float y = (startPositions[currentPlayer][1] + row) * TILE_SIZE + (TILE_SIZE - PIECE_RADIUS * 2) / 2;

//         return QRectF(x, y, PIECE_RADIUS * 2, PIECE_RADIUS * 2);
//     }
//     else
//     {
//         // Calculate position for tokens in play using calculateBoardPosition
//         QPointF pos = calculateBoardPosition(currentPlayer, token.position);

//         // If the token is ready for the home path, validate its position
//         if (token.readyForHome) {
//             verifyTokenPosition(token);
//         }

//         return QRectF(pos.x() - PIECE_RADIUS,
//                       pos.y() - PIECE_RADIUS,
//                       PIECE_RADIUS * 2,
//                       PIECE_RADIUS * 2);
//     }
// }

QRectF LudoGame::calculateTokenRect(const Token &token) {
    QPointF position;

    if (!token.inPlay) {
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


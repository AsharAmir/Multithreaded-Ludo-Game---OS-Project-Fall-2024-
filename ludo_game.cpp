#include "ludo_game.h"
#include <QPainter>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <iostream>
#include <thread>
#include <QPainterPath>
#include <QThread>
#include "constants.h"


LudoGame::LudoGame(QWidget *parent) : QMainWindow(parent)
{
    setFixedSize(GRID_SIZE * TILE_SIZE + DICE_SIZE + 60, GRID_SIZE * TILE_SIZE);
    gameDice.shape.moveCenter(QPointF(GRID_SIZE * TILE_SIZE + 30 + DICE_SIZE / 2, 200 + DICE_SIZE / 2));

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&LudoGame::update));
    timer->start(16); // ~60 FPS

    initializePlayers();
    initializePaths();
    startGame();

    turnTimer = new QTimer(this);
    turnTimer->setInterval(1000);
    connect(turnTimer, &QTimer::timeout, this, &LudoGame::updateTurnTimer);
}

void LudoGame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    drawLudoBoard(painter);
    drawPieces(painter);
    drawDice(painter);

    // Draw timer
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(QRect(GRID_SIZE * TILE_SIZE + 10, 10, 50, 30),
                     Qt::AlignCenter,
                     QString::number(remainingTime));

    // Draw current player indicator
    QColor playerColors[4] = {Qt::blue, Qt::yellow, Qt::red, Qt::green};
    painter.setBrush(playerColors[currentPlayer]);
    painter.drawEllipse(GRID_SIZE * TILE_SIZE + 10, 50, 20, 20);
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

            // If no valid moves, automatically advance to next player
            if (!hasValidMoves)
            {
                waitingForMove = false;
                selectedToken = nullptr;
                turnTimer->stop();
                consecutiveSixes = 0;
                advanceTurn();
                update();
            }
        }
        // Handle token selection when waiting for move
        else if (waitingForMove)
        {
            handleTokenSelection(mousePos);
        }
    }
}

void LudoGame::initializePlayers()
{
    std::cout << "Enter number of tokens per player (1-4): ";
    std::cin >> numTokens;

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
    currentPlayer = 0;

    std::thread master(&LudoGame::masterThread, this);
    master.detach();

    for (int i = 0; i < MAX_PLAYERS; ++i)
    {
        std::thread player(&LudoGame::playerThread, this, i);
        player.detach();
    }
}

void LudoGame::initializePaths()
{
    // Blue path
    bluePath = {
        {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0}, {7, 0}, {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6}, {14, 7}, {14, 8}, {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8}, {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14}, {7, 14}, {6, 14}, {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9}, {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7}, {0, 6},
        // Blue's colored path
        {1, 7},
        {2, 7},
        {3, 7},
        {4, 7},
        {5, 7},
        {6, 7}};

    // Red path
    redPath = {
        {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6}, {14, 7}, {14, 8}, {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8}, {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14}, {7, 14}, {6, 14}, {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9}, {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7}, {0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0}, {7, 0},
        // Red's colored path
        {7, 1},
        {7, 2},
        {7, 3},
        {7, 4},
        {7, 5},
        {7, 6}};

    // Yellow path
    yellowPath = {
        {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8}, {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14}, {7, 14}, {6, 14}, {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9}, {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7}, {0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0}, {7, 0}, {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6}, {14, 7},
        // Yellow's colored path
        {13, 7},
        {12, 7},
        {11, 7},
        {10, 7},
        {9, 7},
        {8, 7}};

    // Green path
    greenPath = {
        {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9}, {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7}, {0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0}, {7, 0}, {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6}, {14, 7}, {14, 8}, {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8}, {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14}, {7, 14},
        // Green's colored path
        {7, 13},
        {7, 12},
        {7, 11},
        {7, 10},
        {7, 9},
        {7, 8}};
}

void LudoGame::drawLudoBoard(QPainter &painter)
{
    // Colors
    QColor redColor(255, 0, 0);
    QColor greenColor(0, 255, 0);
    QColor yellowColor(255, 255, 0);
    QColor blueColor(0, 0, 255);
    QColor whiteColor(255, 255, 255);
    QColor greyColor(192, 192, 192);

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
                (row == 8 && col == 13) || (row == 13 && col == 6))
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
    QColor colors[4] = {
        QColor(0, 0, 255),   // Blue
        QColor(255, 255, 0), // Yellow
        QColor(255, 0, 0),   // Red
        QColor(0, 255, 0)    // Green
    };

    std::vector<PathCoordinate> *paths[4] = {&bluePath, &yellowPath, &redPath, &greenPath};

    // Draw tokens for each player
    for (int p = 0; p < MAX_PLAYERS; p++)
    {
        for (const auto &token : players[p].tokens)
        {
            std::lock_guard<std::mutex> lock(token.tokenMutex);
            
            QRectF tokenRect;
            if (!token.inPlay)
            {
                // Calculate home position
                int tokenIndex = &token - &players[p].tokens[0];
                int row = tokenIndex / 2;
                int col = tokenIndex % 2;
                
                int startPositions[4][2] = {{2, 2}, {2, 11}, {11, 2}, {11, 11}};
                float x = (startPositions[p][0] + col) * TILE_SIZE + (TILE_SIZE - PIECE_RADIUS * 2) / 2;
                float y = (startPositions[p][1] + row) * TILE_SIZE + (TILE_SIZE - PIECE_RADIUS * 2) / 2;
                
                tokenRect = QRectF(x, y, PIECE_RADIUS * 2, PIECE_RADIUS * 2);
            }
            else
            {
                // Use token's current position on its path
                const auto &pathPos = (*paths[p])[token.position];
                float x = pathPos.x * TILE_SIZE + (TILE_SIZE - PIECE_RADIUS * 2) / 2;
                float y = pathPos.y * TILE_SIZE + (TILE_SIZE - PIECE_RADIUS * 2) / 2;
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
    painter.translate(gameDice.shape.center());

    if (gameDice.isRolling)
    {
        qint64 elapsedTime = gameDice.rollClock.elapsed();
        if (elapsedTime < 500)
        {
            gameDice.rotation += 15.0f;
            painter.rotate(gameDice.rotation);
        }
        else
        {
            gameDice.isRolling = false;
        }
    }

    QColor playerColors[4] = {
        QColor(0, 0, 255),   // Blue
        QColor(255, 255, 0), // Yellow
        QColor(255, 0, 0),   // Red
        QColor(0, 255, 0)    // Green
    };

    painter.fillRect(QRectF(-DICE_SIZE / 2, -DICE_SIZE / 2, DICE_SIZE, DICE_SIZE), playerColors[currentPlayer]);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(QRectF(-DICE_SIZE / 2, -DICE_SIZE / 2, DICE_SIZE, DICE_SIZE));

    if (!gameDice.isRolling)
    {
        painter.setBrush(Qt::white);
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

        for (const auto &pos : dotPositions)
        {
            painter.drawEllipse(pos, 3, 3);
        }
    }

    painter.restore();
}

void LudoGame::rollDice()
{
    gameDice.value = QRandomGenerator::global()->bounded(1, 7);
    gameDice.isRolling = true;
    gameDice.rollClock.restart();
    gameDice.rotation = 0;

    remainingTime = 10;
    turnTimer->stop();

    if (gameDice.value == 6)
    {
        consecutiveSixes++;
        if (consecutiveSixes == 3)
        {
            consecutiveSixes = 0;
            advanceTurn();
        }
        else
        {
            waitingForMove = true;
            selectedToken = nullptr;
            turnTimer->start();
        }
    }
    else
    {
        consecutiveSixes = 0;
        waitingForMove = true;
        selectedToken = nullptr;
        turnTimer->start();
    }

    update();
}

void LudoGame::advanceTurn()
{
    switch (currentPlayer)
    {
    case 0:
        currentPlayer = 1;
        break;
    case 1:
        currentPlayer = 3;
        break;
    case 2:
        currentPlayer = 0;
        break;
    case 3:
        currentPlayer = 2;
        break;
    }
    waitingForMove = false;
    selectedToken = nullptr;
    remainingTime = 10;
    turnTimer->stop();
    turnTimer->start();
    update();
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
            }
            else if (token.inPlay)
            {
                validMove = true;
                moveToken(token, gameDice.value);
            }

            if (validMove)
            {
                waitingForMove = false;
                selectedToken = nullptr;
                turnTimer->stop();

                if (gameDice.value == 6 && consecutiveSixes < 3)
                {
                    consecutiveSixes++;
                    remainingTime = 10;
                }
                else
                {
                    consecutiveSixes = 0;
                    advanceTurn();
                }
                update();
                return;
            }
        }
    }
}

void LudoGame::moveToken(Token &token, int spaces)
{
    std::lock_guard<std::mutex> lock(token.tokenMutex);

    if (!token.inPlay && spaces == 6)
    {
        // Place token at starting position
        token.inPlay = true;
        QPoint startPos = getStartingPosition(currentPlayer);
        token.row = startPos.y();
        token.col = startPos.x();
        
        // Set the correct initial position based on player color
        switch (currentPlayer)
        {
        case 0: // Blue
            token.position = 0;
            break;
        case 1: // Yellow
            token.position = 13;
            break;
        case 2: // Red
            token.position = 26;
            break;
        case 3: // Green
            token.position = 39;
            break;
        }
        
        update();
        return;
    }

    if (token.inPlay)
    {
        std::vector<PathCoordinate> *currentPath = nullptr;
        int pathOffset = 0;
        
        // Select the correct path and offset based on player color
        switch (currentPlayer)
        {
        case 0: // Blue
            currentPath = &bluePath;
            pathOffset = 0;
            break;
        case 1: // Yellow
            currentPath = &yellowPath;
            pathOffset = 13;
            break;
        case 2: // Red
            currentPath = &redPath;
            pathOffset = 26;
            break;
        case 3: // Green
            currentPath = &greenPath;
            pathOffset = 39;
            break;
        }

        if (!currentPath) return;

        // Calculate new position
        int currentPathIndex = token.position - pathOffset;
        int newPathIndex = currentPathIndex + spaces;
        
        // Check if move is valid
        if (newPathIndex >= currentPath->size())
        {
            // Token can't move beyond the end of the path
            return;
        }

        // Update token's position
        token.position = newPathIndex + pathOffset;
        token.row = (*currentPath)[newPathIndex].y;
        token.col = (*currentPath)[newPathIndex].x;

        // Check for hits with other players' tokens
        checkAndProcessHits(token, token.position, currentPlayer);
    }
    update();
}


void LudoGame::checkAndProcessHits(Token &token, int newPos, int playerId)
{
    for (int p = 0; p < MAX_PLAYERS; p++)
    {
        if (p != playerId)
        {
            for (auto &otherToken : players[p].tokens)
            {
                if (otherToken.inPlay && otherToken.position == newPos)
                {
                    bool isSafe = false;
                    for (int safe : players[p].safeSquares)
                    {
                        if (newPos == safe)
                        {
                            isSafe = true;
                            break;
                        }
                    }

                    if (!isSafe)
                    {
                        otherToken.inPlay = false;
                        players[p].hitRate--;
                        players[playerId].hitRate++;
                    }
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

QPointF LudoGame::calculateBoardPosition(int position)
{
    // Convert board position to grid coordinates
    std::vector<PathCoordinate> *currentPath;
    switch (currentPlayer)
    {
    case 0: // Blue
        currentPath = &bluePath;
        break;
    case 1: // Yellow
        currentPath = &yellowPath;
        break;
    case 2: // Red
        currentPath = &redPath;
        break;
    case 3: // Green
        currentPath = &greenPath;
        break;
    default:
        return QPointF(0, 0);
    }

    if (position >= 0 && position < currentPath->size())
    {
        // Get the grid coordinates from the path
        int gridX = (*currentPath)[position].x;
        int gridY = (*currentPath)[position].y;

        // Convert grid coordinates to pixel coordinates
        float pixelX = gridX * TILE_SIZE + TILE_SIZE / 2;
        float pixelY = gridY * TILE_SIZE + TILE_SIZE / 2;

        return QPointF(pixelX, pixelY);
    }

    return QPointF(0, 0);
}


QRectF LudoGame::calculateTokenRect(const Token &token)
{
    if (!token.inPlay)
    {
        // Calculate home position for tokens not in play
        int tokenIndex = 0;
        for (size_t i = 0; i < players[currentPlayer].tokens.size(); ++i)
        {
            if (&players[currentPlayer].tokens[i] == &token)
            {
                tokenIndex = i;
                break;
            }
        }

        int row = tokenIndex / 2;
        int col = tokenIndex % 2;

        // Starting positions for each player's home area
        int startPositions[4][2] = {
            {2, 2},   // Blue
            {2, 11},  // Yellow
            {11, 2},  // Red
            {11, 11}  // Green
        };

        float x = (startPositions[currentPlayer][0] + col) * TILE_SIZE + (TILE_SIZE - PIECE_RADIUS * 2) / 2;
        float y = (startPositions[currentPlayer][1] + row) * TILE_SIZE + (TILE_SIZE - PIECE_RADIUS * 2) / 2;

        return QRectF(x, y, PIECE_RADIUS * 2, PIECE_RADIUS * 2);
    }
    else
    {
        // Calculate position for tokens in play
        QPointF pos = calculateBoardPosition(token.position);
        return QRectF(pos.x() - PIECE_RADIUS, 
                     pos.y() - PIECE_RADIUS,
                     PIECE_RADIUS * 2, 
                     PIECE_RADIUS * 2);
    }
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
            advanceTurn();
        }
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


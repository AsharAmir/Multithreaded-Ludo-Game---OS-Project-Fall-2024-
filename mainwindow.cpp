#include "mainwindow.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QMutex>

QMutex gameMutex;
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

// MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), currentPlayer(0), gameStarted(false)
// {
//     setFixedSize(800, 700);
//     setupUI();
//     initializeBoard();
//     createPlayers();
//     drawBoard();
// }

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
    if (!playerTokensInitialized) {
        // Initialize tokens for all players
        for (int i = 0; i < 4; i++) {
            initializeTokensForPlayer(i);
        }
        playerTokensInitialized = true;
    }
    
    gameStarted = true;
    currentPlayerIndex = 0;
    statusLabel->setText("Game started! Player 1's turn.");
    rollButton->setEnabled(true);
    startButton->setEnabled(false);
}

// destructor for mainwindow
MainWindow::~MainWindow()
{
    delete scene;
    delete view;
    delete dice;
    delete startButton;
    delete rollButton;
    delete statusLabel;
    delete playerStatsLabel;
    delete turnTimer;

    for (PlayerThread *thread : playerThreads)
    {
        thread->quit();
        thread->wait();
        delete thread;
    }

    delete masterThread;

    qDeleteAll(players);
    stopThreads();
    players.clear();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , currentPlayerIndex(0)
    , playerTokensInitialized(false)
{
    setFixedSize(1024, 768);
    setStyleSheet("background-color: #2C3E50;");
    setupUI();
    initializeBoard();
    createPlayers();
    drawBoard();

    // Initialize turn timer
    turnTimer = new QTimer(this);
    turnTimer->setInterval(TURN_TIMEOUT);
    connect(turnTimer, &QTimer::timeout, this, &MainWindow::handlePlayerTimeout);

    // Initialize no six counter for each player
    noSixCount.resize(4);
    noSixCount.fill(0);

    initializeThreads();

    // Initialize player tokens
    for (int i = 0; i < players.size(); ++i) {
        createPlayerToken(i);
    }
}

void MainWindow::styleUIComponents()
{
    // Style the buttons
    QString buttonStyle =
        "QPushButton {"
        "   background-color: #3498DB;"
        "   border: none;"
        "   color: white;"
        "   padding: 15px 32px;"
        "   text-align: center;"
        "   font-size: 16px;"
        "   margin: 4px 2px;"
        "   border-radius: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980B9;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #BDC3C7;"
        "}";

    rollButton->setStyleSheet(buttonStyle);
    startButton->setStyleSheet(buttonStyle);

    // Style the labels
    QString labelStyle =
        "QLabel {"
        "   color: white;"
        "   font-size: 14px;"
        "   padding: 10px;"
        "}";

    statusLabel->setStyleSheet(labelStyle);
    playerStatsLabel->setStyleSheet(labelStyle);

    // Add shadow effects
    auto shadowEffect = new QGraphicsDropShadowEffect;
    shadowEffect->setBlurRadius(15);
    shadowEffect->setColor(QColor(0, 0, 0, 80));
    shadowEffect->setOffset(5, 5);
    view->setGraphicsEffect(shadowEffect);
}

void MainWindow::setupUI()
{
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    dice = new Dice(this);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // Add game title
    QLabel *titleLabel = new QLabel("Ludo Game", this);
    titleLabel->setStyleSheet(
        "font-size: 24px; color: white; font-weight: bold; padding: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    layout->addWidget(view);

    // Create control panel
    QHBoxLayout *controlLayout = new QHBoxLayout();

    startButton = new QPushButton("Start Game", this);
    controlLayout->addWidget(startButton);

    rollButton = new QPushButton("Roll Dice", this);
    controlLayout->addWidget(rollButton);
    rollButton->setEnabled(false);

    // Add dice label to control panel
    diceLabel = new QLabel("Dice: 0", this);
    controlLayout->addWidget(diceLabel);

    layout->addLayout(controlLayout);

    // Status and stats panel
    QHBoxLayout *infoLayout = new QHBoxLayout();

    statusLabel = new QLabel("Click Start to begin the game", this);
    playerStatsLabel = new QLabel("Player Stats", this);

    infoLayout->addWidget(statusLabel);
    infoLayout->addWidget(playerStatsLabel);

    layout->addLayout(infoLayout);

    setCentralWidget(centralWidget);

    // Connect signals
    connect(startButton, &QPushButton::clicked, this, &MainWindow::startGame);
    connect(rollButton, &QPushButton::clicked, this, &MainWindow::rollDice);

    styleUIComponents();
}

// void MainWindow::handleTokenCapture(int playerId)
// {
//     players[playerId]->incrementHitCount();
//     updatePlayerStats();
// }

void MainWindow::handlePlayerDisqualification(int playerId)
{
    players[playerId]->disqualify();
    updatePlayerStats();
}

void MainWindow::handleGameCompletion()
{
    QMessageBox::information(this, "Game Over", "The game has ended!");
    rollButton->setEnabled(false);
}

void MainWindow::initializeThreads()
{
    for (Player *player : players)
    {
        PlayerThread *thread = new PlayerThread(player, this);
        playerThreads.append(thread);

        connect(thread, &PlayerThread::turnCompleted, this, &MainWindow::nextTurn);
        connect(thread, &PlayerThread::tokenCaptured, this, &MainWindow::handleTokenCapture, Qt::UniqueConnection);

        thread->start();
    }
}

// Stop threads when the game ends
void MainWindow::stopThreads()
{
    for (PlayerThread *thread : playerThreads)
    {
        thread->stopGame(); // Signal threads to stop
        thread->wait();     // Wait for threads to finish
        delete thread;
    }
    playerThreads.clear();
}

bool MainWindow::gameEnded()
{
    int activePlayers = 0;

    for (Player *player : players)
    {
        if (!player->isDisqualified() && !player->hasWon())
        {
            activePlayers++;
        }
    }

    // Game ends if there's only one active player or someone has won
    return activePlayers <= 1 || std::any_of(players.begin(), players.end(), [](Player *p)
                                             { return p->hasWon(); });
}

void MainWindow::nextTurn()
{
    currentPlayer = (currentPlayer + 1) % players.size();

    if (gameEnded()) // Check if the game has ended
    {
        stopThreads();
        QMessageBox::information(this, "Game Over", "The game has ended!");
        return;
    }

    playerThreads[currentPlayer]->startTurn(); // Notify the next player
}

void MainWindow::handleTokenCapture(int playerId)
{
    players[playerId]->incrementHitCount();
    updatePlayerStats();
}

void MainWindow::handlePlayerTimeout()
{
    statusLabel->setText(QString("Player %1's turn timed out!").arg(currentPlayer + 1));
    nextTurn();
}

void MainWindow::handleConsecutiveSixes(int count)
{
    if (count == 3)
    {
        statusLabel->setText("Three consecutive sixes! Turn lost.");
        consecutiveSixesCount = 0;
        nextTurn();
    }
}

bool MainWindow::canEnterHomeColumn(Player *player)
{
    return player->getHitCount() > 0;
}

void MainWindow::updatePlayerStats()
{
    QString stats;
    // qDebug() << "Number of players:" << players.size();
    for (int i = 0; i < players.size(); ++i)
    {
        Player *player = players[i];
        stats += QString("Player %1: Hits=%2 %3\n")
                     .arg(i + 1)
                     .arg(player->getHitCount())
                     .arg(player->isDisqualified() ? "(DISQUALIFIED)" : "");
    }
    playerStatsLabel->setText(stats);
}

bool isRolling = false;

void MainWindow::rollDice()
{
    qDebug() << "Rolling dice for player" << currentPlayerIndex;
    
    if (!playerTokensInitialized || currentPlayerIndex < 0 || currentPlayerIndex >= 4) {
        qDebug() << "Invalid state detected";
        return;
    }
    
    if (isRolling) {
        qDebug() << "Already rolling";
        return;
    }
    isRolling = true;
    
    int diceValue = QRandomGenerator::global()->bounded(1, 7);
    diceLabel->setText(QString("Dice: %1").arg(diceValue));
    qDebug() << "Rolled:" << diceValue;
    
    QVector<int> validTokens;
    for (int i = 0; i < TOKENS_PER_PLAYER; i++) {
        if (canMoveToken(currentPlayerIndex, i, diceValue)) {
            validTokens.append(i);
        }
    }
    
    qDebug() << "Valid tokens:" << validTokens;
    
    if (!validTokens.isEmpty()) {
        QDialog dialog(this);
        dialog.setWindowTitle("Select Token to Move");
        QVBoxLayout* layout = new QVBoxLayout(&dialog);
        
        for (int tokenIndex : validTokens) {
            QPushButton* btn = new QPushButton(QString("Move Token %1").arg(tokenIndex + 1), &dialog);
            connect(btn, &QPushButton::clicked, [this, tokenIndex, diceValue, &dialog]() {
                moveSelectedToken(currentPlayerIndex, tokenIndex, diceValue);
                dialog.accept();
            });
            layout->addWidget(btn);
        }
        
        dialog.exec();
    }
    
    currentPlayerIndex = (currentPlayerIndex + 1) % 4;
    updatePlayerStats();
    statusLabel->setText(QString("Player %1's turn").arg(currentPlayerIndex + 1));
    
    isRolling = false;
}


// void MainWindow::moveSelectedToken(int playerIndex, int tokenIndex, int diceValue)
// {
//     QLabel* token = playerTokens[playerIndex][tokenIndex];
//     QPoint currentPos = token->pos() / CELL_SIZE;
    
//     if (isTokenInYard(currentPos, playerIndex) && diceValue == 6) {
//         // Move to start position
//         QPoint startPos = PLAYER_START_POSITIONS[playerIndex];
//         token->move(startPos.x() * CELL_SIZE, startPos.y() * CELL_SIZE);
//     } else {
//         // Move on board
//         QPoint newPos = calculateNewPosition(currentPos, diceValue);
//         token->move(newPos.x() * CELL_SIZE, newPos.y() * CELL_SIZE);
        
//         // Check for captures
//         checkForCaptures(newPos, playerIndex);
//     }
// }

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

void MainWindow::moveToken(Player *player, int tokenId, int steps)
{
    Token *token = player->getToken(tokenId);
    if (!token)
        return;

    if (token->getIsInYard() && steps == 6)
    {
        // Move out of yard
        token->setPosition(player->getId() * 13); // Starting position
        token->setIsInYard(false);

        QPoint boardPos = getboardCoordinates(token->getPosition(), player->getId());
        player->updateTokenGraphics(tokenId, boardPos.x() * CELL_SIZE, boardPos.y() * CELL_SIZE);
    }
    else if (!token->getIsInYard())
    {
        // Normal movement
        int newPosition = calculateNewPosition(player, token->getPosition(), steps);

        // Check for captures
        handleCaptures(player, newPosition);

        // Update token position
        token->setPosition(newPosition);
        QPoint boardPos = getboardCoordinates(newPosition, player->getId());
        player->updateTokenGraphics(tokenId, boardPos.x() * CELL_SIZE, boardPos.y() * CELL_SIZE);
    }
}

// void MainWindow::handleCaptures(Player *currentPlayer, int position)
// {
//     for (Player *otherPlayer : players)
//     {
//         if (otherPlayer == currentPlayer)
//             continue;

//         for (Token *otherToken : otherPlayer->getTokens())
//         {
//             QPoint otherPos = getboardCoordinates(otherToken->getPosition(), otherPlayer->getId());
//             QPoint currentPos = getboardCoordinates(position, currentPlayer->getId());

//             if (otherPos == currentPos)
//             {
//                 // Capture token
//                 otherToken->setIsInYard(true);
//                 otherToken->setPosition(-1);
//                 otherPlayer->updateTokenGraphics(otherToken->getId());
//             }
//         }
//     }
// }

QPoint MainWindow::getboardCoordinates(int position, int /*playerId*/)
{
    // Calculate board coordinates based on position
    int x = (position % 8) * CELL_SIZE;
    int y = (position / 8) * CELL_SIZE;
    return QPoint(x, y);
}

int MainWindow::calculateNewPosition(Player * /*player*/, int currentPos, int steps)
{
    // Calculate new position considering board boundaries
    int newPos = currentPos + steps;
    if (newPos >= BOARD_POSITIONS)
    {
        newPos = BOARD_POSITIONS - 1;
    }
    return newPos;
}

void MainWindow::updateBoard()
{
    // Update token positions on board
    for (Player *player : players)
    {
        for (int i = 0; i < 4; i++)
        {
            Token *token = player->getToken(i);
            if (token)
            {
                int pos = token->getPosition();
                QPoint coords = getboardCoordinates(pos, player->getId());
                token->setPos(coords.x(), coords.y());
            }
        }
    }
    scene->update();
}

Token *MainWindow::createToken(Player *player, int tokenId)
{
    QColor playerColor = player->getColor();
    return new Token(playerColor, tokenId, player);
}

bool MainWindow::isValidMove(Player *player, int tokenIndex, int steps)
{
    Token *token = player->getToken(tokenIndex);
    if (!token)
        return false;

    if (token->getIsInYard())
    {
        return steps == 6; // Can only leave yard with a 6
    }

    int newPosition = calculateNewPosition(player, token->getPosition(), steps);
    QPoint newPos = getboardCoordinates(newPosition, player->getId());

    // Check for collisions with other tokens
    for (Player *otherPlayer : players)
    {
        for (Token *otherToken : otherPlayer->getTokens())
        {
            if (getboardCoordinates(otherToken->getPosition(), player->getId()) == newPos)
            {
                return false;
            }
        }
    }
    return true;
}

// void MainWindow::moveToken(Player *player, int tokenId, int steps)
// {
//     Token *token = player->getToken(tokenId);
//     if (!token)
//         return;

//     if (token->getIsInYard() && steps == 6)
//     {
//         // Move out of yard
//         token->setPosition(player->getId() * 13); // Starting position
//         token->setIsInYard(false);
//     }
//     else if (!token->getIsInYard())
//     {
//         // Move on main track
//         int newPosition = calculateNewPosition(player, token->getPosition(), steps);
//         token->setPosition(newPosition);
//     }
// }

void MainWindow::handleCaptures(Player *currentPlayer, int position)
{
    for (Player *otherPlayer : players)
    {
        if (otherPlayer == currentPlayer)
            continue;

        for (Token *otherToken : otherPlayer->getTokens())
        {
            QPoint otherPos = getboardCoordinates(otherToken->getPosition(), otherPlayer->getId());
            QPoint currentPos = getboardCoordinates(position, currentPlayer->getId());

            if (otherPos == currentPos)
            {
                // Capture token
                otherToken->setIsInYard(true);
                otherToken->setPosition(-1);

                // Use the yard position to update the graphics
                int tokenId = otherToken->getId();
                QPoint yardPos = PLAYER_YARD_POSITIONS[otherPlayer->getId()][tokenId];
                otherPlayer->updateTokenGraphics(tokenId, yardPos.x() * CELL_SIZE, yardPos.y() * CELL_SIZE);
            }
        }
    }
}

void MainWindow::createPlayerToken(int playerIndex)
{
    if (playerColors.isEmpty()) {
        playerColors << "#FF0000" << "#00FF00" << "#0000FF" << "#FFFF00";
    }
    
    QLabel* token = new QLabel(this);
    token->setFixedSize(30, 30);
    token->setStyleSheet(QString("background-color: %1; border-radius: 15px;")
                        .arg(playerColors[playerIndex % playerColors.size()]));
    token->show();
    token->raise();
    
    // Fix: Add token to vector instead of assignment
    playerTokens[playerIndex].append(token);
    
    // Set initial position at yard
    QPoint yardPos = PLAYER_YARD_POSITIONS[playerIndex][0];
    token->move(yardPos.x() * CELL_SIZE, yardPos.y() * CELL_SIZE);
}

void MainWindow::nextPlayer()
{
    currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
}

QPoint MainWindow::calculateNewPosition(Player* player, int diceValue)
{
    int currentPos = player->getTokenPosition(currentTokenId);
    
    // Assuming board positions are numbered sequentially
    int newPos = currentPos + diceValue;
    
    // Convert position number to x,y coordinates
    int x = (newPos % (boardWidth/CELL_SIZE)) * CELL_SIZE;
    int y = (newPos / (boardWidth/CELL_SIZE)) * CELL_SIZE;
    
    return QPoint(x + START_POSITION_X, y + START_POSITION_Y);
}

bool MainWindow::isValidMove(const QPoint& position)
{
    // Add your move validation logic here
    return position.x() >= 0 && position.x() < boardWidth &&
           position.y() >= 0 && position.y() < boardHeight;
}

bool MainWindow::checkForHit(const QPoint& position)
{
    // Add your hit detection logic here
    return false;
}

void MainWindow::initializeTokenPositions()
{
    for (int playerIndex = 0; playerIndex < 4; playerIndex++) {
        for (int tokenIndex = 0; tokenIndex < TOKENS_PER_PLAYER; tokenIndex++) {
            QLabel* token = new QLabel(this);
            token->setFixedSize(30, 30);
            token->setStyleSheet(QString("background-color: %1; border-radius: 15px;")
                               .arg(playerColors[playerIndex]));
            
            // Set initial position in home yard
            QPoint yardPos = PLAYER_YARD_POSITIONS[playerIndex][tokenIndex];
            token->move(yardPos.x() * CELL_SIZE, yardPos.y() * CELL_SIZE);
            token->show();
            token->raise();
            
            playerTokens[playerIndex].append(token);
        }
    }
}

void MainWindow::moveSelectedToken(int playerIndex, int tokenIndex, int diceValue)
{
    QLabel* token = playerTokens[playerIndex][tokenIndex];
    QPoint currentPos = token->pos() / CELL_SIZE;
    
    if (isTokenInYard(currentPos, playerIndex) && diceValue == 6) {
        // Move to start position
        QPoint startPos = PLAYER_START_POSITIONS[playerIndex];
        token->move(startPos.x() * CELL_SIZE, startPos.y() * CELL_SIZE);
    } else {
        // Move on board
        QPoint newPos = calculateNewPositionFromPoint(currentPos, diceValue);
        token->move(newPos.x() * CELL_SIZE, newPos.y() * CELL_SIZE);
        
        // Check for captures
        checkForCaptures(newPos, playerIndex);
    }
}

bool MainWindow::canMoveToken(int playerIndex, int tokenIndex, int diceValue)
{
    QLabel* token = playerTokens[playerIndex][tokenIndex];
    QPoint pos = token->pos() / CELL_SIZE; // Convert to grid coordinates
    
    // Check if token is in yard and dice shows 6
    if (isTokenInYard(pos, playerIndex) && diceValue == 6) {
        return true;
    }
    
    // Check if token is on board and can move
    if (!isTokenInYard(pos, playerIndex)) {
        QPoint newPos = calculateNewPositionFromPoint(pos, diceValue);
        return isValidMove(newPos);
    }
    
    return false;
}

bool MainWindow::isTokenInYard(const QPoint& pos, int playerIndex)
{
    // Check if position matches any yard position for this player
    for (int i = 0; i < 4; i++) {
        if (pos == PLAYER_YARD_POSITIONS[playerIndex][i]) {
            return true;
        }
    }
    return false;
}

void MainWindow::checkForCaptures(const QPoint& newPos, int currentPlayerIndex)
{
    // Check all other players' tokens
    for (int playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (playerIndex == currentPlayerIndex) continue;
        
        for (int tokenIndex = 0; tokenIndex < TOKENS_PER_PLAYER; tokenIndex++) {
            QLabel* otherToken = playerTokens[playerIndex][tokenIndex];
            QPoint otherPos = otherToken->pos() / CELL_SIZE;
            
            if (otherPos == newPos) {
                // Send token back to yard
                QPoint yardPos = PLAYER_YARD_POSITIONS[playerIndex][tokenIndex];
                otherToken->move(yardPos.x() * CELL_SIZE, yardPos.y() * CELL_SIZE);
            }
        }
    }
}

QPoint MainWindow::calculateNewPositionFromPoint(const QPoint& currentPos, int steps)
{
    // Simple board movement logic - customize based on your board layout
    int x = currentPos.x();
    int y = currentPos.y();
    
    // Example movement pattern
    x = (x + steps) % (BOARD_SIZE / CELL_SIZE);
    
    return QPoint(x, y);
}

void MainWindow::initializeTokensForPlayer(int playerIndex)
{
    // Initialize vector for this player if needed
    if (playerTokens[playerIndex].isEmpty()) {
        for (int i = 0; i < TOKENS_PER_PLAYER; i++) {
            QLabel* token = new QLabel(this);
            token->setFixedSize(30, 30);
            token->setStyleSheet(QString("background-color: %1; border-radius: 15px;")
                               .arg(playerColors[playerIndex]));
            
            // Place in yard position
            QPoint yardPos = PLAYER_YARD_POSITIONS[playerIndex][i];
            token->move(yardPos.x() * CELL_SIZE, yardPos.y() * CELL_SIZE);
            token->show();
            token->raise();
            
            playerTokens[playerIndex].append(token);
        }
    }
}

//ashar was here
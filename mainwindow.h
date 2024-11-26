#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPushButton>
#include <QLabel>
#include <QMutex>
#include <QVector>
#include "player.h"
#include "dice.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr); // Constructor
    ~MainWindow();                                  // Destructor

private slots:
    void rollDice();   // Slot for rolling the dice
    void updateGame(); // Slot to update the game state
    void startGame();  // Slot to start the game
    void nextTurn();   // Slot to move to the next player's turn

private:
    void initializeBoard();                                                 // Initialize the game board
    void drawBoard();                                                       // Draw the board visuals
    void createPlayers();                                                   // Create and initialize players
    void moveToken(Player *player, int tokenId, int steps);                 // Move a player's token
    bool checkCollision(int position);                                      // Check for collisions at a given position
    bool isValidMove(Player *player, int tokenId, int steps);               // Validate a token's move
    void setupUI();                                                         // Set up the UI components
    void selectTokenDialog(const QVector<int> &validTokens, int diceValue); // Dialog to select a token
    QPoint getboardCoordinates(int position, int playerId);                 // Convert position to board coordinates
    void handleCaptures(Player *player, int newPosition);                   // Handle captures when a token lands
    int calculateNewPosition(Player *player, int currentPos, int steps);    // Calculate new position

    QGraphicsScene *scene;     // Graphics scene for the game
    QGraphicsView *view;       // Graphics view for the scene
    QPushButton *rollButton;   // Button for rolling the dice
    QLabel *statusLabel;       // Label for displaying game status
    Dice *dice;                // Dice object for the game
    QVector<Player *> players; // List of players
    int currentPlayer;         // Index of the current player
    QMutex gameMutex;          // Mutex for game synchronization
    bool gameStarted;          // Flag to check if the game has started
    QPushButton *startButton;  // Button to start the game

    // Constants for board dimensions
    static const int BOARD_SIZE = 600;                   // Size of the board in pixels
    static const int GRID_SIZE = 15;                     // Number of grid cells per side
    static const int CELL_SIZE = BOARD_SIZE / GRID_SIZE; // Size of each cell
};

#endif // MAINWINDOW_H

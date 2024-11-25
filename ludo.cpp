#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <random>
#include <vector>
#include <mutex>
#include <condition_variable>

using namespace std;

// Constants for board size and players
const int BOARD_SIZE = 15;
const int MAX_PLAYERS = 4;
const int MAX_TOKENS = 4;

// Colors for different players
enum class PlayerColor
{
    RED,
    GREEN,
    YELLOW,
    BLUE
};

// Structure to represent a token
struct Token
{
    int x;
    int y;
    bool isHome;
    bool isFinished;

    Token() : x(-1), y(-1), isHome(true), isFinished(false) {}
};

// Structure for thread parameters
struct ThreadParams
{
    int playerId;
    int tokenCount;
    vector<Token> tokens;

    ThreadParams(int id, int count) : playerId(id), tokenCount(count)
    {
        tokens.resize(count);
    }
};

// Global variables
char board[BOARD_SIZE][BOARD_SIZE];
mutex boardMutex;
mutex diceMutex;
condition_variable diceCV;
bool diceInUse = false;

// Safe squares for each player
vector<pair<int, int>> safeSquares = {
    {6, 2}, {2, 6}, {8, 6}, {6, 8}, // Common safe squares
    {6, 1},
    {1, 6},
    {8, 1},
    {13, 6}, // Player-specific safe squares
    {6, 13},
    {13, 8},
    {8, 13},
    {1, 8}};

// Initialize the board
void initializeBoard()
{
    // Fill board with empty spaces
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            board[i][j] = ' ';
        }
    }

    // Mark safe squares
    for (const auto &square : safeSquares)
    {
        board[square.first][square.second] = '*';
    }

    // Mark home yards
    board[1][1] = 'R';   // Red home
    board[1][13] = 'G';  // Green home
    board[13][1] = 'Y';  // Yellow home
    board[13][13] = 'B'; // Blue home
}

// Display the board
void displayBoard()
{
    cout << "\033[2J\033[1;1H"; // Clear screen
    cout << "Ludo Board:\n\n";

    for (int i = 0; i < BOARD_SIZE; i++)
    {
        cout << "   ";
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            cout << "+---";
        }
        cout << "+\n   ";

        for (int j = 0; j < BOARD_SIZE; j++)
        {
            cout << "| " << board[i][j] << " ";
        }
        cout << "|\n";
    }

    cout << "   ";
    for (int j = 0; j < BOARD_SIZE; j++)
    {
        cout << "+---";
    }
    cout << "+\n";
}

// Roll dice function
int rollDice()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 6);
    return dis(gen);
}

// Player thread function
void *playerThread(void *arg)
{
    ThreadParams *params = (ThreadParams *)arg;

    // Initialize player tokens in home yard
    for (int i = 0; i < params->tokenCount; i++)
    {
        params->tokens[i].isHome = true;
        params->tokens[i].isFinished = false;
    }

    return nullptr;
}

int main()
{
    cout << "Welcome to Ludo Game!\n\n";

    // Initialize the board
    initializeBoard();

    // Get number of tokens from user
    int tokenCount;
    do
    {
        cout << "Enter number of tokens per player (1-4): ";
        cin >> tokenCount;
    } while (tokenCount < 1 || tokenCount > 4);

    // Create thread parameters for each player
    vector<ThreadParams> threadParams;
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        threadParams.emplace_back(i, tokenCount);
    }

    // Create threads for each player
    vector<pthread_t> threads(MAX_PLAYERS);
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        pthread_create(&threads[i], nullptr, playerThread, &threadParams[i]);
    }

    // Display initial board
    displayBoard();

    // Wait for threads to finish
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        pthread_join(threads[i], nullptr);
    }

    return 0;
}
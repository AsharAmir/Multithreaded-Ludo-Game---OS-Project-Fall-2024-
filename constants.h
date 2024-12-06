#pragma once

#include <QPoint>

const int GRID_SIZE = 15;
const int TILE_SIZE = 40;
const int MAX_PLAYERS = 4;
const int MAX_TOKENS = 4;
const int PIECE_RADIUS = 15;
const int DICE_SIZE = 40;
const float ROTATION_SPEED = 720.0f; // degrees per second
const int MIN_TOKENS = 1;
const int BOARD_SQUARES = 52;
const int SAFE_SQUARES_PER_PLAYER = 5;
const int BOARD_WIDTH = GRID_SIZE * TILE_SIZE + DICE_SIZE + 240;
const int BOARD_HEIGHT = GRID_SIZE * TILE_SIZE;

extern const QPoint START_SQUARES[4];

struct PathCoordinate
{
    int x, y;
    bool isColoredPath;
    int playerColor; // -1 for white path, 0-3 for colored paths
};

#include "gui/mainwindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_board(std::make_shared<Board>())
{
    setupUI();
    createMenus();
}

void MainWindow::setupUI() {
    setWindowTitle("Ludo Game");
    
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    
    m_boardWidget = new BoardWidget(m_board, this);
    layout->addWidget(m_boardWidget);
    
    setCentralWidget(centralWidget);
    resize(800, 800);
}

void MainWindow::createMenus() {
    QMenu* gameMenu = menuBar()->addMenu(tr("&Game"));
    
    QAction* newGameAction = new QAction(tr("&New Game"), this);
    connect(newGameAction, &QAction::triggered, this, &MainWindow::startNewGame);
    gameMenu->addAction(newGameAction);
}

void MainWindow::startNewGame() {
    m_board->initializeBoard();
    
    // Add players with 4 tokens each
    m_board->addPlayer(Token::Color::RED, 4);
    m_board->addPlayer(Token::Color::GREEN, 4);
    m_board->addPlayer(Token::Color::YELLOW, 4);
    m_board->addPlayer(Token::Color::BLUE, 4);
    
    m_boardWidget->updateBoard();
}

void MainWindow::rollDice() {
    // Dice rolling implementation
}
#include <QApplication>
#include "ludo_game.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    LudoGame game;
    game.show();
    return app.exec();
}


// main.cpp
#include <QApplication>
#include "main_menu.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    MainMenu menu;
    menu.show();
    
    return app.exec();
}
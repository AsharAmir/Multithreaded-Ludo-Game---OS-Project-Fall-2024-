// main.cpp

//22i-2420 ASHHAR AMIR
//22I-2599 MAAZUDDIN
//22I-8752 HAFSA SULEMAN


#include <QApplication>
#include "i222420_i222599_i228752_main_menu.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    MainMenu menu;
    menu.show();
    
    return app.exec();
}
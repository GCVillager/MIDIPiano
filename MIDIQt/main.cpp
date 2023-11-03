#include "MIDIQt.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MIDIQt w;
    w.show();
    return a.exec();
}

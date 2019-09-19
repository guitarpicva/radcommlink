#include "paterm.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PATerm w;
    w.show();
    return a.exec();
}

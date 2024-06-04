#include "BaseConversion.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BaseConversion w;
    w.show();
    return a.exec();
}

#include "BaseConversion.h"
#include <QtWidgets/QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Ìí¼Ó·­ÒëÎÄ¼þ
    QTranslator* trans = new QTranslator;
    trans->load("BaseConversion_zh.qm");
    a.installTranslator(trans);

    BaseConversion w;
    w.show();
    return a.exec();
}

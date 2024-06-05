#include "BaseConversion.h"
#include <QtWidgets/QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // ��ӷ����ļ�
    QTranslator* trans = new QTranslator;
    trans->load("BaseConversion_zh.qm");
    a.installTranslator(trans);

    BaseConversion w;
    w.show();
    return a.exec();
}

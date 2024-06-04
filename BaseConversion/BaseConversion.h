#pragma once

#include <QtWidgets/QWidget>
#include "ui_BaseConversion.h"

class BaseConversionServer;
class BaseConversionClient;

class BaseConversion : public QWidget
{
    Q_OBJECT

public:
    BaseConversion(QWidget *parent = nullptr);
    ~BaseConversion();

private:
    Ui::BaseConversionClass ui;
    BaseConversionServer* server;
    BaseConversionClient* client;


    QString message;
};

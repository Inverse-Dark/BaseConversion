#pragma once

#include <QTcpServer>

class BaseConversionServer :
    public QTcpServer
{
    Q_OBJECT
public:
    BaseConversionServer(QObject* parent = nullptr);

    QList<QTcpSocket*> getAllClients();
protected:
    void incomingConnection(qintptr socketDescriptor) override;

signals:
    void ReceivedData(QTcpSocket* socket);
    void newConnect(QTcpSocket* socket);
    void removeClient();

private slots:
    void handleReadyRead();

private:
    QList<QTcpSocket*> clients;
};


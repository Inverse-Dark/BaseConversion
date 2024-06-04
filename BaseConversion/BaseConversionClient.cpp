#include "BaseConversionClient.h"
#include <QTcpSocket>

BaseConversionClient::BaseConversionClient(QObject* parent)
{
    connected = false;
}

void BaseConversionClient::connectToServer(const QString& address, quint16 port)
{
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &BaseConversionClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &BaseConversionClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &BaseConversionClient::onReadyRead);
    socket->connectToHost(address, port);
    connected = socket->waitForConnected();
}

void BaseConversionClient::disconnectToServer()
{
    socket->disconnectFromHost();
    connected = socket->waitForDisconnected();
}

void BaseConversionClient::sendData(const QByteArray& data)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(data);
    }
}

bool BaseConversionClient::isConnected()
{
    return connected;
}

void BaseConversionClient::onConnected()
{
    qDebug() << "connect success!";
}

void BaseConversionClient::onDisconnected()
{
    qDebug() << "disconnect!";
}

void BaseConversionClient::onReadyRead()
{
    emit ReceivedData(socket);
}

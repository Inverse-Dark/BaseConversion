#include "BaseConversionServer.h"
#include <QTcpSocket>

BaseConversionServer::BaseConversionServer(QObject* parent)
{
}

QList<QTcpSocket*> BaseConversionServer::getAllClients()
{
	return clients;
}

void BaseConversionServer::incomingConnection(qintptr socketDescriptor)
{
	QTcpSocket* socket = new QTcpSocket(this);
	if (!socket->setSocketDescriptor(socketDescriptor)) {
		delete socket;
		return;
	}

	// ��ӿͻ��˵��б���
	clients.append(socket);

	// �����ź����
	connect(socket, &QTcpSocket::readyRead, this, &BaseConversionServer::handleReadyRead);
	connect(socket, &QTcpSocket::disconnected, socket, [this, socket]() {
		clients.removeAll(socket);
		socket->deleteLater();
		emit removeClient();
		});

	emit newConnect(socket);
}

void BaseConversionServer::handleReadyRead()
{
	QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
	if (socket) {
		emit ReceivedData(socket);
	}
}

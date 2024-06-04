#pragma once
#include <QObject>

class QTcpSocket;

class BaseConversionClient :
	public QObject
{
	Q_OBJECT
public:
	BaseConversionClient(QObject* parent = nullptr);
	void connectToServer(const QString& address, quint16 port);
	void disconnectToServer();
	void sendData(const QByteArray& data);

	bool isConnected();

private slots:
	void onConnected();
	void onDisconnected();
	void onReadyRead();

signals:
	void ReceivedData(QTcpSocket* socket);

private:
	QTcpSocket* socket;
	bool connected;

};


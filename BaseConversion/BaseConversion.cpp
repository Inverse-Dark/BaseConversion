#include "BaseConversion.h"
#include "BaseConversionServer.h"
#include "BaseConversionClient.h"
#include <QPushButton>
#include <QDateTime>
#include <QTcpSocket>

QString byteArray2Bin(QByteArray data);
QString byteArray2Dec(QByteArray data);
QString byteArray2Hex(QByteArray data);
QString hex2QString(QByteArray data);

BaseConversion::BaseConversion(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    //ui.splitter->setSizes(QList<int>({ (int)(width() * 0.2) ,(int)(width() * 80) }));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setLayout(ui.horizontalLayout);
    ui.ServerWidget->setLayout(ui.gridLayoutServer);
    ui.ClientWidget->setLayout(ui.gridLayoutClient);

    // ����������ʽ��ƥ��IP��ַ
    QRegExp ipAddressRegExp("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    // ����������֤��
    QRegExpValidator* validator = new QRegExpValidator(ipAddressRegExp, this);
    // ����һ��������֤�������Ʒ�Χ�� 0 �� 65535 ֮��
    QIntValidator* portValidator = new QIntValidator(0, 65535, this);
    // ����֤�����õ�QLineEdit��
    ui.lineEditServerPort->setValidator(portValidator);
    ui.lineEditClientIP->setValidator(validator);
    ui.lineEditClientPort->setValidator(portValidator);

    ui.lineEditServerPort->setText("8000");
    ui.lineEditClientIP->setText("127.0.0.1");
    ui.lineEditClientPort->setText("8000");

    server = new BaseConversionServer();
    client = new BaseConversionClient();
    
    // server->listen(QHostAddress::Any, 8000);
    // client->connectToServer("127.0.0.1", 8080);

    // ��������ť
    connect(ui.pushButtonOpen, &QPushButton::clicked, this, [&]() {
        static int serverCnt = 1;
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        if (serverCnt % 2) {
            // ��ʼ����
            server->listen(QHostAddress::Any, ui.lineEditServerPort->text().toInt());
            if (server->isListening()) {
                ui.pushButtonOpen->setText(tr("Close"));
                message = QString(tr("[%1]# Open server surrces. port: %2\n")).arg(date).arg(ui.lineEditServerPort->text().toInt());
                serverCnt++;
            }else{
                message = QString(tr("[%1]# Open server failed. port:%2\n")).arg(date).arg(ui.lineEditServerPort->text().toInt());
            }
            ui.plainTextEditDataLogServer->appendPlainText(message);
        }
        else {
            server->close();
            if (!server->isListening()) {
                ui.pushButtonOpen->setText(tr("Open"));
                message = QString(tr("[%1]# Close server surrces\n")).arg(date);
                serverCnt++;
            }
            else {
                message = QString(tr("[%1]# Close server failed\n")).arg(date);
            }
            ui.plainTextEditDataLogServer->appendPlainText(message);
        }
        });
    // ���Ӱ�ť
    connect(ui.pushButtonConnect, &QPushButton::clicked, this, [&]() {
        static int clientCnt = 1;
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        if (clientCnt % 2) {
            // ���ӷ�����
            client->connectToServer(ui.lineEditClientIP->text(), ui.lineEditClientPort->text().toInt());
            if (client->isConnected()) {
                ui.pushButtonConnect->setText(tr("Disconnect"));
                message = QString(tr("[%1]# Connect %2:%3 surrces\n")).arg(date)
					.arg(ui.lineEditClientIP->text()).arg(ui.lineEditClientPort->text().toInt());
                clientCnt++;
            }
            else {
                message = QString(tr("[%1]# Connect %2:%3 failed\n")).arg(date)
                    .arg(ui.lineEditClientIP->text()).arg(ui.lineEditClientPort->text().toInt());
            }
            ui.plainTextEditDataLogClient->appendPlainText(message);
        }
        else {
			client->disconnectToServer();
            if (!client->isConnected()) {
                ui.pushButtonConnect->setText(tr("Connect"));
                message = QString(tr("[%1]# Disconnect %2:%3 surrces\n")).arg(date)
                    .arg(ui.lineEditClientIP->text()).arg(ui.lineEditClientPort->text().toInt());
                clientCnt++;
            }
            else {
                message = QString(tr("[%1]# Disconnect %2:%3 failed\n")).arg(date)
                    .arg(ui.lineEditClientIP->text()).arg(ui.lineEditClientPort->text().toInt());
            }
            ui.plainTextEditDataLogClient->appendPlainText(message);
        }
        });
    
    // �µ�����
    connect(server, &BaseConversionServer::newConnect, this, [&](QTcpSocket* socket) {
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        if (socket) {
            // ��ȡ�ͻ��˵� IP ��ַ�Ͷ˿�
            QHostAddress clientAddress = socket->peerAddress();
            quint16 clientPort = socket->peerPort();
            message = QString(tr("[%1] client %2:%3 join.\n")).arg(date)
                .arg(clientAddress.toString()).arg(clientPort);
            ui.plainTextEditDataLogServer->appendPlainText(message);

            QStringList clients;
            for (int i = 0; i < server->getAllClients().size(); i++) {
				clients.append(server->getAllClients()[i]->peerAddress().toString());
            }
            ui.comboBoxAllClient->clear();
            ui.comboBoxAllClient->addItems(clients);
        }
        });
    // �Ͽ�����
    connect(server, &BaseConversionServer::removeClient, this, [&]() {
        QStringList clients;
        for (int i = 0; i < server->getAllClients().size(); i++) {
            clients.append(server->getAllClients()[i]->peerAddress().toString());
        }
        ui.comboBoxAllClient->clear();
        ui.comboBoxAllClient->addItems(clients);
        });

    // ���������ܵ�����
    connect(server, &BaseConversionServer::ReceivedData, this, [&](QTcpSocket* socket) {
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        QByteArray data = socket->readAll();

		message = QString(tr("[%1]# RESV>\n%2\n")).arg(date).arg(QString(data));
		ui.plainTextEditDataLogServer->appendPlainText(message);
        });
    // �ͻ��˽��ܵ�����
    connect(client, &BaseConversionClient::ReceivedData, this, [&](QTcpSocket* socket) {
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        QByteArray data = socket->readAll();

        message = QString(tr("[%1]# RESV>\n%2\n")).arg(date).arg(QString(data));
        ui.plainTextEditDataLogClient->appendPlainText(message);
        });
    // ��������������
    connect(ui.pushButtonSendServer, &QPushButton::clicked, this, [&]() {
        // ��ȡʱ��
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        // ��ȡ��ǰѡ�еĿͻ���
        QTcpSocket* socket = server->getAllClients().at(ui.comboBoxAllClient->currentIndex());
        if (!socket) {
            message = QString(tr("[%1]# SEND error, no client selected.\n")).arg(date);
            ui.plainTextEditDataLogServer->appendPlainText(message);
            return;
        }
        // ��ȡ�ı����е�����
		QByteArray data = ui.plainTextEditSendDataServer->toPlainText().toLocal8Bit();
        // �������������16��������
        if (ui.checkBoxInputHEXServer->isChecked()) {
            data = hex2QString(data).toLocal8Bit();
        }
        else {
            data += '\r';
        }
        // ��������˷���16��������
        if (ui.checkBoxSendHEXServer->isChecked()) {
            // data = byteArray2Hex(data).toLocal8Bit();
            data = static_cast<QByteArray>(byteArray2Hex(data).toLocal8Bit());
        }
        // �������ͻ���
        socket->write(data);

        // ��ʽ����־��Ϣ
        message = QString(tr("[%1]# SEND>\n%2\n")).arg(date).arg(QString(data));
        ui.plainTextEditDataLogServer->appendPlainText(message);
        });
    // �ͻ��˷�������
    connect(ui.pushButtonSendClient, &QPushButton::clicked, this, [&]() {
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        if (!client->isConnected()) {
            message = QString(tr("[%1]# SEND error, no connection to server.\n")).arg(date);
            ui.plainTextEditDataLogClient->appendPlainText(message);
            return;
        }
        // ��ȡ�ı����е�����
        QByteArray data = ui.plainTextEditSendDataClient->toPlainText().toLocal8Bit();
        // �������������16��������
        if (ui.checkBoxInputHEXClient->isChecked()) {
            data = hex2QString(data).toLocal8Bit();
        }
        else {
            data += '\r';
        }

        // ��������˷���16��������
        if (ui.checkBoxSendHEXClient->isChecked()) {
            data = byteArray2Hex(data).toLocal8Bit();
        }
        // ������������
        client->sendData(data);

        message = QString(tr("[%1]# SEND>\n%2\n")).arg(date).arg(QString(data));
        ui.plainTextEditDataLogClient->appendPlainText(message);
        });

    // ��հ�ť
    connect(ui.pushButtonCleanClientLog, &QPushButton::clicked, this, [&]() {
        ui.plainTextEditDataLogClient->clear();
        });
    connect(ui.pushButtonCleanServerLog, &QPushButton::clicked, this, [&]() {
        ui.plainTextEditDataLogServer->clear();
        });
}

BaseConversion::~BaseConversion()
{}

QString hex2QString(QByteArray data)
{
    QString hexString(data);
    QString asciiString;

    // ȷ���ַ���������ż��
    if ((hexString.length()) % 2 != 0) {
        return QString("Invalid hex string. Length should be even.");
    }
	for (int i = 0; i < hexString.length(); i += 2) {
        QString hexPair = hexString.mid(i, 2);
        bool ok;
        int value = hexPair.toInt(&ok, 16);
        if (!ok) {
			return QString("Invalid hex pair : %1").arg(hexPair);
        }
        asciiString.append(QChar(value));
    }
    return asciiString;
}

QString byteArray2Hex(QByteArray data)
{
    QString hexString;
    for (char c : data) {
        hexString.append(QString::number(static_cast<unsigned char>(c), 16).toUpper().rightJustified(2, '0'));
    }
    return hexString;
}

QString byteArray2Dec(QByteArray data)
{
    QString decString;
    for (char c : data) {
        decString.append(QString::number(static_cast<unsigned char>(c), 10).toUpper());
    }
    return decString;
}

QString byteArray2Bin(QByteArray data)
{
    QString binString;
    for (char c : data) {
        binString.append(QString::number(static_cast<unsigned char>(c), 2).toUpper());
    }
    return binString;
}

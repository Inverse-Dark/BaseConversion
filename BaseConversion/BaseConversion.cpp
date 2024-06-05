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

    // 创建正则表达式来匹配IP地址
    QRegExp ipAddressRegExp("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    // 创建输入验证器
    QRegExpValidator* validator = new QRegExpValidator(ipAddressRegExp, this);
    // 创建一个整数验证器，限制范围在 0 到 65535 之间
    QIntValidator* portValidator = new QIntValidator(0, 65535, this);
    // 将验证器设置到QLineEdit上
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

    // 开启服务按钮
    connect(ui.pushButtonOpen, &QPushButton::clicked, this, [&]() {
        static int serverCnt = 1;
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        if (serverCnt % 2) {
            // 开始监听
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
    // 连接按钮
    connect(ui.pushButtonConnect, &QPushButton::clicked, this, [&]() {
        static int clientCnt = 1;
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        if (clientCnt % 2) {
            // 连接服务器
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
    
    // 新的连接
    connect(server, &BaseConversionServer::newConnect, this, [&](QTcpSocket* socket) {
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        if (socket) {
            // 获取客户端的 IP 地址和端口
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
    // 断开连接
    connect(server, &BaseConversionServer::removeClient, this, [&]() {
        QStringList clients;
        for (int i = 0; i < server->getAllClients().size(); i++) {
            clients.append(server->getAllClients()[i]->peerAddress().toString());
        }
        ui.comboBoxAllClient->clear();
        ui.comboBoxAllClient->addItems(clients);
        });

    // 服务器接受到数据
    connect(server, &BaseConversionServer::ReceivedData, this, [&](QTcpSocket* socket) {
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        QByteArray data = socket->readAll();

		message = QString(tr("[%1]# RESV>\n%2\n")).arg(date).arg(QString(data));
		ui.plainTextEditDataLogServer->appendPlainText(message);
        });
    // 客户端接受到数据
    connect(client, &BaseConversionClient::ReceivedData, this, [&](QTcpSocket* socket) {
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        QByteArray data = socket->readAll();

        message = QString(tr("[%1]# RESV>\n%2\n")).arg(date).arg(QString(data));
        ui.plainTextEditDataLogClient->appendPlainText(message);
        });
    // 服务器发送数据
    connect(ui.pushButtonSendServer, &QPushButton::clicked, this, [&]() {
        // 获取时间
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        // 获取当前选中的客户端
        QTcpSocket* socket = server->getAllClients().at(ui.comboBoxAllClient->currentIndex());
        if (!socket) {
            message = QString(tr("[%1]# SEND error, no client selected.\n")).arg(date);
            ui.plainTextEditDataLogServer->appendPlainText(message);
            return;
        }
        // 获取文本域中的数据
		QByteArray data = ui.plainTextEditSendDataServer->toPlainText().toLocal8Bit();
        // 如果设置了输入16进制数据
        if (ui.checkBoxInputHEXServer->isChecked()) {
            data = hex2QString(data).toLocal8Bit();
        }
        else {
            data += '\r';
        }
        // 如果设置了发送16进制数据
        if (ui.checkBoxSendHEXServer->isChecked()) {
            // data = byteArray2Hex(data).toLocal8Bit();
            data = static_cast<QByteArray>(byteArray2Hex(data).toLocal8Bit());
        }
        // 发送至客户端
        socket->write(data);

        // 格式化日志信息
        message = QString(tr("[%1]# SEND>\n%2\n")).arg(date).arg(QString(data));
        ui.plainTextEditDataLogServer->appendPlainText(message);
        });
    // 客户端发送数据
    connect(ui.pushButtonSendClient, &QPushButton::clicked, this, [&]() {
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        if (!client->isConnected()) {
            message = QString(tr("[%1]# SEND error, no connection to server.\n")).arg(date);
            ui.plainTextEditDataLogClient->appendPlainText(message);
            return;
        }
        // 获取文本域中的数据
        QByteArray data = ui.plainTextEditSendDataClient->toPlainText().toLocal8Bit();
        // 如果设置了输入16进制数据
        if (ui.checkBoxInputHEXClient->isChecked()) {
            data = hex2QString(data).toLocal8Bit();
        }
        else {
            data += '\r';
        }

        // 如果设置了发送16进制数据
        if (ui.checkBoxSendHEXClient->isChecked()) {
            data = byteArray2Hex(data).toLocal8Bit();
        }
        // 发送至服务器
        client->sendData(data);

        message = QString(tr("[%1]# SEND>\n%2\n")).arg(date).arg(QString(data));
        ui.plainTextEditDataLogClient->appendPlainText(message);
        });

    // 清空按钮
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

    // 确保字符串长度是偶数
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

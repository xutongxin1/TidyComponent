//
// Created by xtx on 25-6-5.
//

#include "EDAChromeHttpServer.h"
#include "EDAChromeHttpServer.h"

#include <QCoreApplication>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDebug>
#include <QFile>

#include "ElaPushButton.h"
#include "mainwindow.h"

EDAChromeHttpServer::EDAChromeHttpServer(QObject *parent)
    : QObject(parent)
      , m_server(std::make_unique<QTcpServer>(this))
      , m_chromeProcess(std::make_unique<QProcess>(this)) {
    connect(m_server.get(), &QTcpServer::newConnection,
            this, &EDAChromeHttpServer::handleNewConnection);

    // connect(m_chromeProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
    //         this, &EDAChromeHttpServer::handleChromeFinished);
    // Start HTTP server
    if (!m_server->listen(QHostAddress::LocalHost, m_port)) {
        qDebug() << "Failed to start HTTP server on port" << m_port
            << ":" << m_server->errorString();
        return;
    }

    qDebug() << "HTTP server listening on port" << m_port;
}

EDAChromeHttpServer::~EDAChromeHttpServer() {
    stop();
}

void EDAChromeHttpServer::start() {


    // Check Chrome path
    QString chromePath = "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe";
    if (!QFile::exists(chromePath)) {
        qDebug() << "Chrome not found at:" << chromePath;
        m_server->close();
        return;
    }
    // Start Chrome in incognito mode with blank page
    QStringList arguments;
    arguments << "--user-data-dir=" + ChromePATH << "--disable-web-security" << "--allow-file-access-from-files" <<
        "--new-window" << "--no-first-run" << "--no-default-browser-check" <<"about:blank";

    m_chromeProcess->start(chromePath, arguments);

    if (!m_chromeProcess->waitForStarted(5000)) {
        qDebug() << "Failed to start Chrome:" << m_chromeProcess->errorString();
        m_server->close();
        return;
    }

    qDebug() << "Chrome started in incognito mode";
}

void EDAChromeHttpServer::stop() {
    if (m_chromeProcess->state() != QProcess::NotRunning) {
        m_chromeProcess->terminate();
        if (!m_chromeProcess->waitForFinished(5000)) {
            m_chromeProcess->kill();
        }
    }

    if (m_server->isListening()) {
        m_server->close();
    }
}

void EDAChromeHttpServer::handleNewConnection() {
    while (m_server->hasPendingConnections()) {
        QTcpSocket *socket = m_server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead,
                this, &EDAChromeHttpServer::handleClientData);
        connect(socket, &QTcpSocket::disconnected,
                socket, &QTcpSocket::deleteLater);
    }
}

void EDAChromeHttpServer::handleClientData() {
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) return;

    QByteArray data = socket->readAll();

    // Simple HTTP response
    QByteArray response = "HTTP/1.1 200 OK\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n";
    socket->write(response);
    socket->close();

    // Parse request to find JSON body
    QString request = QString::fromUtf8(data);

    // Check if it's a POST request
    if (!request.startsWith("POST")) {
        return;
    }

    // Find body (after empty line)
    int bodyIndex = request.indexOf("\r\n\r\n");
    if (bodyIndex == -1) {
        return;
    }

    QString body = request.mid(bodyIndex + 4);

    // Parse JSON
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(body.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << parseError.errorString();
        return;
    }

    if (!jsonDoc.isObject()) {
        qDebug() << "Received JSON is not an object";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    qDebug() << "Received JSON:" << jsonObj;

    emit jsonReceived(jsonObj);
}

void EDAChromeHttpServer::handleChromeFinished() {
    qDebug() << "Chrome closed, stopping HTTP server";

    // if (m_server->isListening()) {
    //     m_server->close();
    // }
}

void MainWindow::InitEDAChromeHttpServer() {
    EDA_Server = new EDAChromeHttpServer(this);
    // Connect signals
    connect(_openEDAChromeButton, &QPushButton::clicked, [&]() {
        EDA_Server->start();
    });

    connect(EDA_Server, &EDAChromeHttpServer::jsonReceived, [&](const QJsonObject &json) {
        qDebug() << json;

        // 从JSON的arguments数组中获取完整字符串
        QJsonArray arguments = json["arguments"].toArray();
        if (arguments.size() > 0) {
            QString fullString = arguments[0].toString();

            // 解析器件编号
            QString deviceId;
            if (fullString.contains("器件编号:")) {
                int startIndex = fullString.indexOf("器件编号:") + QString("器件编号:").length();
                int endIndex = fullString.indexOf(",", startIndex);
                if (endIndex != -1) {
                    deviceId = fullString.mid(startIndex, endIndex - startIndex).trimmed();
                }
            }

            // 解析器件型号
            QString deviceName;
            if (fullString.contains("器件型号:")) {
                int nameStart = fullString.indexOf("器件型号:") + QString("器件型号:").length();
                int nameEnd = fullString.indexOf(",", nameStart);
                if (nameEnd == -1) {
                    nameEnd = fullString.indexOf("封装:");
                    if (nameEnd == -1) {
                        nameEnd = fullString.length();
                    }
                }
                deviceName = fullString.mid(nameStart, nameEnd - nameStart).trimmed();
            }

            // 解析封装
            QString package;
            if (fullString.contains("封装:")) {
                int packageStart = fullString.indexOf("封装:") + QString("封装:").length();
                package = fullString.mid(packageStart).trimmed();
            }

            QString value;
            if (fullString.contains("值:")) {
                int valueStart = fullString.indexOf("值:") + QString("值:").length();
                value = fullString.mid(valueStart).trimmed();
            }

            // 根据器件编号是否为undefined决定搜索内容
            QString searchText;
            if (deviceId != "undefined" && !deviceId.isEmpty()) {
                searchText = deviceId;
            } else if (!deviceName.isEmpty() && !package.isEmpty()) {
                searchText = deviceName + " " + package;
            } else if (!value.isEmpty() ) {
                searchText = value;
            }

            // 设置搜索框文本并触发搜索
            if (!searchText.isEmpty()) {
                _searchBox->setText(searchText);
                search(); // 直接调用search函数
            }
        }
    });
}

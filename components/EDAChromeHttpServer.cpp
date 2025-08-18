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
        "--new-window" << "--no-first-run" << "--no-default-browser-check" << "about:blank";

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
    if (!socket)
        return;

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

    // Find headers and body separator
    int bodyIndex = request.indexOf("\r\n\r\n");
    if (bodyIndex == -1) {
        return;
    }

    // Extract headers to check for chunked encoding
    QString headers = request.left(bodyIndex);
    QString bodyPart = request.mid(bodyIndex + 4);

    // Check if Transfer-Encoding is chunked
    if (headers.contains("Transfer-Encoding: chunked", Qt::CaseInsensitive)) {
        // Parse chunked body
        QString decodedBody;
        int pos = 0;

        while (pos < bodyPart.length()) {
            // Find the end of chunk size line
            int endOfSize = bodyPart.indexOf("\r\n", pos);
            if (endOfSize == -1)
                break;

            // Extract chunk size (in hexadecimal)
            QString chunkSizeStr = bodyPart.mid(pos, endOfSize - pos).trimmed();

            // Convert hex to int
            bool ok;
            int chunkSize = chunkSizeStr.toInt(&ok, 16);
            if (!ok) {
                qDebug() << "Failed to parse chunk size:" << chunkSizeStr;
                break;
            }

            // If chunk size is 0, we've reached the end
            if (chunkSize == 0) {
                break;
            }

            // Move past the size line and \r\n
            pos = endOfSize + 2;

            // Extract chunk data
            if (pos + chunkSize <= bodyPart.length()) {
                decodedBody += bodyPart.mid(pos, chunkSize);
                pos += chunkSize + 2; // Skip chunk data and trailing \r\n
            } else {
                qDebug() << "Incomplete chunk data";
                break;
            }
        }

        bodyPart = decodedBody;
    }
    // If not chunked, check for Content-Length
    else if (headers.contains("Content-Length:", Qt::CaseInsensitive)) {
        // Body is already in bodyPart, no additional processing needed
    }

    // Parse JSON
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(bodyPart.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << parseError.errorString();
        qDebug() << "Body content:" << bodyPart; // 调试用，显示实际内容
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
    // connect(_openEDAChromeButton, &QPushButton::clicked, [&]() {
    //     EDA_Server->start();
    // });

    connect(EDA_Server, &EDAChromeHttpServer::jsonReceived, [&](const QJsonObject &json) {
        qDebug() << json;

        QString component_code_raw = json["component_code"].toString();
        QString comment = json["comment"].toString();
        QString footprint = json["footprint"].toString();
        QString unique_id = json["unique_id"].toString();

        // 处理可能为null的数值字段
        double mid_x = json["mid_x"].isNull() ? 0.0 : json["mid_x"].toDouble();
        double mid_y = json["mid_y"].isNull() ? 0.0 : json["mid_y"].toDouble();

        // 处理component_code格式 "component_code,undefined"
        QString component_code;
        if (!component_code_raw.isEmpty()) {
            int commaIndex = component_code_raw.indexOf(',');
            if (commaIndex != -1) {
                component_code = component_code_raw.left(commaIndex);
            } else {
                component_code = component_code_raw;
            }
        }

        // 根据器件编号是否为undefined决定搜索内容
        QString searchText;
        if (component_code != "undefined" && !component_code.isEmpty()) {
            searchText = component_code;
        } else if (!comment.isEmpty() && !footprint.isEmpty()) {
            searchText = comment + " " + footprint;
        } else if (!comment.isEmpty()) {
            searchText = comment;
        } else if (!footprint.isEmpty()) {
            searchText = footprint;
        }

        // 设置搜索框文本并触发搜索
        if (!searchText.isEmpty()) {
            _searchBox->setText(searchText);
            search(); // 直接调用search函数
        }
    });
}

//
// Created by xtx on 25-6-5.
//

#ifndef EDACHROMEHTTPSERVER_H
#define EDACHROMEHTTPSERVER_H
#include <QObject>
#include <QTcpServer>
#include <QProcess>
#include <QJsonObject>
#include <memory>

class EDAChromeHttpServer : public QObject
{
    Q_OBJECT

public:
        explicit EDAChromeHttpServer(QObject *parent = nullptr);
        ~EDAChromeHttpServer();

        void start();
        void stop();

        signals:
            void jsonReceived(const QJsonObject &json);

    private slots:
        void handleNewConnection();
        void handleClientData();
        void handleChromeFinished();

    private:
        std::unique_ptr<QTcpServer> m_server;
        std::unique_ptr<QProcess> m_chromeProcess;
        const int m_port = 18888;
};


#endif //EDACHROMEHTTPSERVER_H

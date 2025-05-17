// SerialPortManager.h
#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QString>
#include <QList>
#include <QSerialPortInfo>
#include <functional>

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
        explicit SerialPortManager(QObject *parent = nullptr);
        ~SerialPortManager();

        // 连接状态枚举
        enum ConnectionStatus {
            Disconnected,  // 已断开连接
            Connecting,    // 正在连接
            Connected      // 已连接
        };

        // 启动连接，自动重连
        void startConnection();

        // 关闭连接
        void closeConnection();

        // 获取当前连接状态
        ConnectionStatus getConnectionStatus() const;

        // 设置数据接收回调函数
        void setDataReceivedCallback(std::function<void(const QString&)> callback);

        // 设置连接成功回调函数
        void setConnectedCallback(std::function<void()> callback);

        // 设置断开连接回调函数
        void setDisconnectedCallback(std::function<void()> callback);

        // 向串口写入数据
        bool writeData(const QString& data) const;

    private slots:
        void attemptConnection();
        void onSerialPortReadyRead();
        void onSerialPortError(QSerialPort::SerialPortError error);

    private:
        QSerialPort* m_serialPort;
        QTimer* m_reconnectTimer;
        ConnectionStatus m_connectionStatus;

        std::function<void(const QString&)> m_dataReceivedCallback;
        std::function<void()> m_connectedCallback;
        std::function<void()> m_disconnectedCallback;

        bool m_autoReconnectEnabled;

        void handleDisconnection();
};

#endif // SERIALPORTMANAGER_H
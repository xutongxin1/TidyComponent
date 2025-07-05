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

class SerialPortManager : public QObject {
        Q_OBJECT

    public:
        explicit SerialPortManager(QObject *parent = nullptr);
        ~SerialPortManager();

        using MessageCallback = std::function<void(const QString&)>;
        // 连接状态枚举
        enum ConnectionStatus {
            Disconnected, // 已断开连接
            Connecting, // 正在连接
            Connected // 已连接
        };

        // 启动连接，自动重连
        void startConnection();

        // 关闭连接
        void closeConnection();

        // 获取当前连接状态
        ConnectionStatus getConnectionStatus() const;

        // 设置数据接收回调函数
        void setDataReceivedCallback(std::function<void(const QString &)> callback);

        // 设置连接成功回调函数
        void setConnectedCallback(std::function<void()> callback);

        // 设置断开连接回调函数
        void setDisconnectedCallback(std::function<void()> callback);

        // 向串口写入数据
        bool writeData(const QString &data) const;

        // 新的基于模式匹配的回调注册函数
        void connectPattern(const QString &pattern, MessageCallback callback);

        // 断开特定模式的连接
        void disconnectPattern(const QString &pattern);

        QString m_buffer;
    private slots:
        void attemptConnection();
        void onSerialPortReadyRead();
        void onSerialPortError(QSerialPort::SerialPortError error);

    private:
        QSerialPort *m_serialPort;
        QTimer *m_reconnectTimer;
        ConnectionStatus m_connectionStatus;

        std::function<void()> m_connectedCallback;
        std::function<void()> m_disconnectedCallback;

        bool m_autoReconnectEnabled;

        // 处理完整消息
        void processCompleteMessages(const QString &data);

        // 查找匹配模式并执行回调
        bool findAndExecuteCallback(const QString &message);

        // 存储模式-回调对
        std::map<QString, MessageCallback> m_patternCallbacks;

        // 未匹配消息的默认回调
        MessageCallback m_unmatchedCallback;


        void handleDisconnection();
};

#endif // SERIALPORTMANAGER_H

// SerialPortManager.cpp
#include "SerialPortManager.h"
#include <QDebug>

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
    , m_reconnectTimer(new QTimer(this))
    , m_connectionStatus(Disconnected)
    , m_autoReconnectEnabled(false)
{
    // 连接定时器到尝试连接的槽函数
    connect(m_reconnectTimer, &QTimer::timeout, this, &SerialPortManager::attemptConnection);

    // 连接串口信号
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPortManager::onSerialPortReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialPortManager::onSerialPortError);
}

SerialPortManager::~SerialPortManager()
{
    closeConnection();
}

void SerialPortManager::startConnection()
{
    if (m_connectionStatus == Disconnected) {
        m_autoReconnectEnabled = true;
        attemptConnection();
    }
}

void SerialPortManager::closeConnection()
{
    m_autoReconnectEnabled = false;
    m_reconnectTimer->stop();

    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }

    m_connectionStatus = Disconnected;
}

SerialPortManager::ConnectionStatus SerialPortManager::getConnectionStatus() const
{
    return m_connectionStatus;
}

void SerialPortManager::setDataReceivedCallback(std::function<void(const QString&)> callback)
{
    m_dataReceivedCallback = callback;
}

void SerialPortManager::setConnectedCallback(std::function<void()> callback)
{
    m_connectedCallback = callback;
}

void SerialPortManager::setDisconnectedCallback(std::function<void()> callback)
{
    m_disconnectedCallback = callback;
}

bool SerialPortManager::writeData(const QString& data) const {
    if (m_connectionStatus != Connected || !m_serialPort->isOpen()) {
        return false;
    }

    QByteArray byteData = data.toUtf8();
    qint64 bytesWritten = m_serialPort->write(byteData);

    return (bytesWritten == byteData.size());
}

void SerialPortManager::attemptConnection()
{
    if (m_connectionStatus == Connected) {
        return;
    }

    m_connectionStatus = Connecting;

    // 关闭任何现有连接
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }

    // 查找所有可用的串口
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    // 查找特定序列号的端口
    bool found = false;
    for (const QSerialPortInfo& info : ports) {
        if (info.serialNumber() == "TIDYCOMPONENT") {
            m_serialPort->setPort(info);
            found = true;
            break;
        }
    }

    if (!found) {
        // 未找到串口，如果启用了自动重连则稍后重试
        if (m_autoReconnectEnabled && !m_reconnectTimer->isActive()) {
            m_reconnectTimer->start(3000); // 每3秒重试一次
        }
        return;
    }

    // 配置串口
    m_serialPort->setBaudRate(QSerialPort::Baud115200);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    // 尝试打开端口
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_connectionStatus = Connected;
        m_reconnectTimer->stop();

        // 如果设置了连接成功回调，则调用它
        if (m_connectedCallback) {
            m_connectedCallback();
        }
    } else {
        // 打开端口失败，如果启用了自动重连则稍后重试
        m_connectionStatus = Disconnected;
        if (m_autoReconnectEnabled && !m_reconnectTimer->isActive()) {
            m_reconnectTimer->start(3000); // 每3秒重试一次
        }
    }
}

void SerialPortManager::onSerialPortReadyRead()
{
    if (m_dataReceivedCallback) {
        QByteArray data = m_serialPort->readAll();
        QString message = QString::fromUtf8(data);
        m_dataReceivedCallback(message);
    }
}

void SerialPortManager::onSerialPortError(QSerialPort::SerialPortError error)
{
    // 忽略NoError
    if (error == QSerialPort::NoError) {
        return;
    }

    // 检查断开连接错误
    if (error == QSerialPort::ResourceError ||
        error == QSerialPort::PermissionError ||
        error == QSerialPort::DeviceNotFoundError) {
        handleDisconnection();
    }
}

void SerialPortManager::handleDisconnection()
{
    if (m_connectionStatus != Disconnected) {
        // 如果端口仍然打开，则关闭它
        if (m_serialPort->isOpen()) {
            m_serialPort->close();
        }

        m_connectionStatus = Disconnected;

        // 如果设置了断开连接回调，则调用它
        if (m_disconnectedCallback) {
            m_disconnectedCallback();
        }

        // 如果启用了自动重连，则启动重连定时器
        if (m_autoReconnectEnabled && !m_reconnectTimer->isActive()) {
            m_reconnectTimer->start(3000); // 每3秒重试一次
        }
    }
}
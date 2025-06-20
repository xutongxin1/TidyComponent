#include "mainwindow.h"
//
// Created by xtx on 25-5-20.
//

void MainWindow::initSerialPort() {
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &portInfo : serialPortInfos) {
        qDebug() << "\n"
            << "Port:" << portInfo.portName() << "\n"
            << "Location:" << portInfo.systemLocation() << "\n"
            << "Description:" << portInfo.description() << "\n"
            << "Manufacturer:" << portInfo.manufacturer() << "\n"
            << "Serial number:" << portInfo.serialNumber() << "\n"
            << "Vendor Identifier:"
            << (portInfo.hasVendorIdentifier()
                    ? QByteArray::number(portInfo.vendorIdentifier(), 16)
                    : QByteArray()) << "\n"
            << "Product Identifier:"
            << (portInfo.hasProductIdentifier()
                    ? QByteArray::number(portInfo.productIdentifier(), 16)
                    : QByteArray());
    }

    serialManager = new SerialPortManager(this);

    // 设置回调函数
    serialManager->setConnectedCallback([&]() {
        qDebug() << "成功连接到串口!";
        connectUserStateAction->setProperty("ElaIconType", QChar((unsigned short) ElaIconType::PlugCircleCheck));
        connectUserStateAction->setText("已连接");

        if (serialManager->writeData("AT\r\n")) {
        }
        ShowSuccessInfo("串口连接成功!", "已连接到用户侧设备");
    });

    serialManager->setDisconnectedCallback([&]() {
        qDebug() << "串口连接已断开!";
        connectUserStateAction->setProperty("ElaIconType", QChar((unsigned short) ElaIconType::Plug));
        connectUserStateAction->setText("未连接");
        ShowWarningInfo("串口连接已断开!", "请检查设备连接状态");
        isConnectedToMesh = false;
    });

    // 启动连接，开始自动重连
    qDebug() << "开始尝试连接到用户侧...";
    serialManager->startConnection();

    serialManager->connectPattern("OK!", [&](const QString &message) {
        qDebug() << "收到OK响应:" << message;
        _serial_isOK = true;
        if (serialManager->writeData("MESH_WRITE\r\n")) {
        }
        ShowSuccessInfo("用户侧正常应答");
    });

    serialManager->connectPattern("MESH_OK", [&](const QString &message) {
        qDebug() << "收到MESH_OK响应:" << message;
        _serial_isMESH_OK = true;
    });

    serialManager->connectPattern("RESET_OK!", [&](const QString &message) {
        qDebug() << "收到RESET_OK响应:" << message;
        _serial_isOK = false;
        _serial_isMESH_OK = false;
    });
    serialManager->connectPattern("MESH_WRITE_OK!", [&](const QString &message) {
        qDebug() << "收到MESH_WRITE_OK响应:" << message;
        _serial_is_inWriteMOde = true;
        ShowSuccessInfo("MESH应答正常", "已进入MESH写入模式");
    });
    serialManager->connectPattern("MESH_CLOSE_OK!", [&](const QString &message) {
        qDebug() << "收到MESH_CLOSE_OK响应:" << message;
        _serial_is_inWriteMOde = false;
    });
    serialManager->connectPattern("MESH_CONNECT!", [&](const QString &message) {
        qDebug() << "收到MESH_CONNECT响应:" << message;
        connectUserStateAction->setText("已连接到MESH网络");
        ShowSuccessInfo("MESH连接正常");
        isConnectedToMesh = true;
    });
}

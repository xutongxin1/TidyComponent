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
        UpdateApplyLogic(); //刷新，使申请可用
        serialManager->m_buffer.clear();
    });
    serialManager->connectPattern("0xC001 10", [&](const QString &message) {
        qDebug() << "元器件放回查找响应:" << message;
        if (QStringList parts = message.split(' ', Qt::SkipEmptyParts); parts.size() >= 3) {
            QString cid;
            if (parts[2] == "10") {
                cid = parts[3];
            } else {
                cid = parts[2];
            }

#ifdef DEBUG
            if (_addComponentStep == 3) {
                if (cid == _addingComponentObj->jlcid) {
                    _addComponent_isNFC_Write_success = true;
                }
                //TODO:写入错误处理
            }
#endif

            qDebug() << "提取的CID:" << cid;
            if (model->component_record_Hash_cid.contains(cid)) {
                component_record_struct *record = model->component_record_Hash_cid.value(cid);
                if (record->isApply == ComponentState_OUT) {
                    ApplyComponentIN(record, apply_type_normal, LED_MODE_FLASH_FAST_3);
                    ShowInfoInfo("ID:" + record->jlcid, "元器件NFC查找放回");
                    if (_searchBox->text().isEmpty()) {
                        _searchBox->setText(record->jlcid);
                    }
                    // search(); //刷新搜索结果
                } else if (record->isApply == ComponentState_Ready) {
                    ApplyComponentIN(record, apply_type_normal, LED_MODE_FLASH_FAST_3);
                    ShowWarningInfo("ID:" + record->jlcid + "正在申请放回", "这，这对吗？");
                    if (_searchBox->text().isEmpty()) {
                        _searchBox->setText(record->jlcid);
                    }
                    // search(); //刷新搜索结果
                }
            } else {
                qDebug() << "尝试找一个不存在的cid" << cid;
            }
        }
    });
    serialManager->connectPattern("0xC001 20 ", [&](const QString &message) {
        QString temp = message;
        qDebug() << "NFC写入返回:" << message;
        QTextStream stream(&temp);
        QString CID, data;
        stream >> data >> data >> CID;
        if (_addComponentStep == 3) {
            if (CID == _addingComponentObj->jlcid) {
                _addComponent_isNFC_Write_success = true;
            }
            //TODO:写入错误处理
        }
    });
    serialManager->connectPattern("0xC001 30 ", [&](const QString &message) {
        QString temp = message;
        qDebug() << "条码读取:" << message;
        QTextStream stream(&temp);
        QString CID, data;
        stream >> data >> data >> CID;
        if (model->searchType == ComponentTableModel::SEARCH_BOM) {
            ShowWarningInfo(CID, "二维码读取成功，但请先退出BOM表搜索模式");
        } else if (_addComponentStep == 1) {
            _addComponent_EditBox->setText(CID);
            ShowInfoInfo(CID, "二维码读取成功");
        } else {
            _searchBox->setText(CID);
            ShowInfoInfo(CID, "二维码读取成功");
        }
    });

    serialManager->connectPattern("0xC102 10", [&](const QString &message) {
        CX02_SerialRecive(message, DeviceType_A42);
    });
    serialManager->connectPattern("0xC202 10", [&](const QString &message) {
        CX02_SerialRecive(message, DeviceType_A21);
    });
    serialManager->connectPattern("0xC302 10", [&](const QString &message) {
        CX02_SerialRecive(message, DeviceType_B53);
    });

    serialManager->connectPattern("0xC103 10", [&](const QString &message) {
        CX03_SerialRecive(message, DeviceType_A42);
    });
    serialManager->connectPattern("0xC203 10", [&](const QString &message) {
        CX03_SerialRecive(message, DeviceType_A21);
    });
    serialManager->connectPattern("0xC303 10", [&](const QString &message) {
        CX03_SerialRecive(message, DeviceType_B53);
    });

    serialManager->connectPattern("0xC104 10", [&](const QString &message) {
        CX04_SerialRecive(message, DeviceType_A42);
    });
    serialManager->connectPattern("0xC204 10", [&](const QString &message) {
        CX04_SerialRecive(message, DeviceType_A21);
    });
    serialManager->connectPattern("0xC304 10", [&](const QString &message) {
        CX04_SerialRecive(message, DeviceType_B53);
    });

    serialManager->connectPattern("C101SendError ", [&](const QString &message) {
        CX01Error_SerialRecive(message, DeviceType_A42);
    });
    serialManager->connectPattern("C201SendError ", [&](const QString &message) {
        CX01Error_SerialRecive(message, DeviceType_A21);
    });
    serialManager->connectPattern("C301SendError ", [&](const QString &message) {
        CX01Error_SerialRecive(message, DeviceType_B53);
    });
}
void MainWindow::CX02_SerialRecive(const QString &message, DeviceType device_type) {
    qDebug() << "元器件取出响应:" << message;
    QString temp = message;

    QTextStream stream(&temp);
    QString MAC, coordinate, data;
    stream >> data >> data >> MAC >> coordinate >> data;
    qDebug() << "提取的MAC地址:" << MAC;
    qDebug() << "提取的位置:" << coordinate;
    if (_addComponentStep == 4 && (_addComponent_Type == DeviceType_A42 || _addComponent_Type == DeviceType_A21)) {
        if (MAC == _addingComponentObj->MAC &&
            (_addComponent_Type == DeviceType_A42 && coordinate.toInt() == _addingComponentObj->coordinate / 10) ||
            _addComponent_Type == DeviceType_A21 && coordinate.toInt() == _addingComponentObj->coordinate) {
            _addComponent_isPutInComponent = true;
        } else {
            ShowWarningInfo("检测到取出但似乎放错了");
        }
    } else {
        if (device_type == DeviceType_B53) {
            if (model->component_record_Hash_MACD.contains(QString(MAC + coordinate))) {
                component_record_struct *record = model->component_record_Hash_MACD.value(MAC + coordinate);
                if (record->isApply == ComponentState_APPLYOUT) {
                    ShowSuccessInfo("ID:" + record->jlcid, "元器件取出成功");
                    colorAllocator->deallocateColor(LED_MODE_STATIC, record->color);
                    record->color = "已取出";
                    record->isApply = ComponentState_OUT;
                    model->updateColumnWithRoles(0);
                    UpdateApplyLogic();
                } else {
                    ShowErrorInfo("MAC:" + MAC + " 坐标:" + coordinate, "正在尝试未申请取出");
                    //客观上认为已经取出了
                    record->color = "已取出";
                    record->isApply = ComponentState_OUT;
                    model->updateColumnWithRoles(0);
                    UpdateApplyLogic();
                }
            }
        } else if (device_type == DeviceType_A42) {
            for (int i = coordinate.toInt() * 10 + 1; i <= coordinate.toInt() * 10 + 2; i++) {
                if (model->component_record_Hash_MACD.contains(QString(MAC + QString::number(i)))) {
                    component_record_struct *record = model->component_record_Hash_MACD.value(MAC + QString::number(i));
                    if (record->isApply == ComponentState_APPLYOUT) {
                        //A42不会对放回成功发出说明
                        // ShowSuccessInfo("ID:" + record->jlcid, "元器件取出成功");
                        colorAllocator->deallocateColor(LED_MODE_STATIC, record->color);
                    }
                    record->color = "已取出";
                    record->isApply = ComponentState_OUT;
                    model->updateColumnWithRoles(0);
                    UpdateApplyLogic();
                    //A42不会存在异常行为
                }
            }
        } else if (device_type == DeviceType_A21) {
            if (model->component_record_Hash_MACD.contains(QString(MAC + coordinate))) {
                component_record_struct *record = model->component_record_Hash_MACD.value(MAC + coordinate);
                if (record->isApply == ComponentState_APPLYOUT) {
                    //A21不会对放回成功发出说明
                    // ShowSuccessInfo("ID:" + record->jlcid, "元器件取出成功");
                    colorAllocator->deallocateColor(LED_MODE_STATIC, record->color);
                }
                record->color = "已取出";
                record->isApply = ComponentState_OUT;
                model->updateColumnWithRoles(0);
                UpdateApplyLogic();
                //21不会存在异常行为
            }
        }
    }
}

void MainWindow::CX03_SerialRecive(const QString &message, DeviceType device_type) {
    qDebug() << "元器件放回响应:" << message;
    QString temp = message;

    QTextStream stream(&temp);
    QString MAC, coordinate, data;
    stream >> data >> data >> MAC >> coordinate >> data;

    qDebug() << "提取的MAC地址:" << MAC;
    qDebug() << "提取的位置:" << coordinate;
    if (_addComponentStep == 4 && _addComponent_Type == DeviceType_B53) {
        if (MAC == _addingComponentObj->MAC && coordinate.toInt() == _addingComponentObj->coordinate) {
            _addComponent_isPutInComponent = true;
        } else {
            ShowWarningInfo("检测到放入但似乎放错了");
        }
    } else {
        if (device_type == DeviceType_B53) {
            if (model->component_record_Hash_MACD.contains(QString(MAC + coordinate))) {
                component_record_struct *record = model->component_record_Hash_MACD.
                    value(MAC + coordinate);
                if (record->isApply == ComponentState_APPLYIN) {
                    ShowSuccessInfo("ID:" + record->jlcid, "元器件放回成功");
                    colorAllocator->deallocateColor(LED_MODE_FLASH_FAST_3, record->color);
                    record->color = "就绪";
                    record->isApply = ComponentState_Ready;
                    model->updateColumnWithRoles(0);
                    if (_searchBox->text() == record->jlcid) {
                        //清空搜索框
                        searchBoxClear();
                    }
                    UpdateApplyLogic();
                } else {
                    ShowErrorInfo("MAC:" + MAC + " 坐标:" + coordinate, "正在尝试未申请放回");
                    //客观上认为已经放回了
                    record->color = "就绪";
                    record->isApply = ComponentState_Ready;
                    model->updateColumnWithRoles(0);
                    UpdateApplyLogic();
                }
            }
        } else if (device_type == DeviceType_A42) {
            for (int i = coordinate.toInt() * 10 + 1; i <= coordinate.toInt() * 10 + 2; i++) {
                if (model->component_record_Hash_MACD.contains(QString(MAC + QString::number(i)))) {
                    component_record_struct *record = model->component_record_Hash_MACD.value(MAC + QString::number(i));
                    if (record->isApply == ComponentState_APPLYIN) {
                        //A42不会对放回成功发出说明
                        // ShowSuccessInfo("ID:" + record->jlcid, "元器件放回成功");
                        colorAllocator->deallocateColor(LED_MODE_FLASH_FAST_3, record->color);
                        if (_searchBox->text() == record->jlcid) {
                            _searchBox->setText(""); //清空搜索框
                        }
                    }
                    record->color = "就绪";
                    record->isApply = ComponentState_Ready;
                    model->updateColumnWithRoles(0);
                    UpdateApplyLogic();
                    //A42不会存在异常行为
                }
            }
        } else if (device_type == DeviceType_A21) {
            if (model->component_record_Hash_MACD.contains(QString(MAC + coordinate))) {
                component_record_struct *record = model->component_record_Hash_MACD.value(MAC + coordinate);
                if (record->isApply == ComponentState_APPLYIN) {
                    //A21不会对放回成功发出说明
                    // ShowSuccessInfo("ID:" + record->jlcid, "元器件放回成功");
                    colorAllocator->deallocateColor(LED_MODE_FLASH_FAST_3, record->color);
                    if (_searchBox->text() == record->jlcid) {
                        _searchBox->setText(""); //清空搜索框
                    }
                }
                record->color = "就绪";
                record->isApply = ComponentState_Ready;
                model->updateColumnWithRoles(0);
                UpdateApplyLogic();
                //A21不会存在异常行为
            }
        }
    }
}
void MainWindow::CX04_SerialRecive(const QString &message, DeviceType device_type) {
    qDebug() << "元器件超时响应:" << message;

    QString temp = message;

    QTextStream stream(&temp);
    QString MAC, coordinate, data;
    stream >> data >> data >> MAC >> coordinate >> data;

    qDebug() << "提取的MAC地址:" << MAC;
    qDebug() << "提取的位置:" << coordinate;
    if (device_type == DeviceType_B53) {
        if (model->component_record_Hash_MACD.contains(QString(MAC + coordinate))) {
            component_record_struct *record = model->component_record_Hash_MACD.value(MAC + coordinate);
            if (record->isApply == ComponentState_APPLYIN) {
                ShowWarningInfo("ID:" + record->jlcid, "元器件超时未放回");
                colorAllocator->deallocateColor(LED_MODE_FLASH_FAST_3, record->color);
                record->color = "已取出";
                record->isApply = ComponentState_OUT;
                model->updateColumnWithRoles(0);
                UpdateApplyLogic();
            } else if (record->isApply == ComponentState_APPLYOUT) {
                ShowWarningInfo("ID:" + record->jlcid, "元器件超时未取出");
                colorAllocator->deallocateColor(LED_MODE_STATIC, record->color);
                record->color = "就绪";
                record->isApply = ComponentState_Ready;
                model->updateColumnWithRoles(0);
                UpdateApplyLogic();
            } else {
                ShowErrorInfo("MAC:" + MAC + " 坐标:" + coordinate, "系统错误，刚刚是否有未经授权的操作?");
            }
        }
    } else if (device_type == DeviceType_A42) {
        for (int i = coordinate.toInt() * 10 + 1; i <= coordinate.toInt() * 10 + 2; i++) {
            if (model->component_record_Hash_MACD.contains(QString(MAC + QString::number(i)))) {
                component_record_struct *record = model->component_record_Hash_MACD.value(MAC + QString::number(i));
                if (record->isApply == ComponentState_APPLYIN) {
                    //A42不会对超时发出警告
                    // ShowWarningInfo("ID:" + record->jlcid, "元器件超时未放回");
                    colorAllocator->deallocateColor(LED_MODE_FLASH_FAST_3, record->color);
                    record->color = "已取出";
                    record->isApply = ComponentState_OUT;
                    model->updateColumnWithRoles(0);
                    UpdateApplyLogic();
                } else if (record->isApply == ComponentState_APPLYOUT) {
                    //A42不会对超时发出警告
                    // ShowWarningInfo("ID:" + record->jlcid, "元器件超时未取出");
                    colorAllocator->deallocateColor(LED_MODE_STATIC, record->color);
                    record->color = "就绪";
                    record->isApply = ComponentState_Ready;
                    model->updateColumnWithRoles(0);
                    UpdateApplyLogic();
                } else {
                    // ShowErrorInfo("MAC:" + MAC + " 坐标:" + QString::number(i), "系统错误，刚刚是否有未经授权的操作?");
                }
            }
        }
    } else if (device_type == DeviceType_A21) {
        if (model->component_record_Hash_MACD.contains(QString(MAC + coordinate))) {
            component_record_struct *record = model->component_record_Hash_MACD.value(MAC + coordinate);
            if (record->isApply == ComponentState_APPLYIN) {
                //A21不会对超时发出警告
                // ShowWarningInfo("ID:" + record->jlcid, "元器件超时未放回");
                colorAllocator->deallocateColor(LED_MODE_FLASH_FAST_3, record->color);
                record->color = "已取出";
                record->isApply = ComponentState_OUT;
                model->updateColumnWithRoles(0);
                UpdateApplyLogic();
            } else if (record->isApply == ComponentState_APPLYOUT) {
                //A21不会对超时发出警告
                // ShowWarningInfo("ID:" + record->jlcid, "元器件超时未取出");
                colorAllocator->deallocateColor(LED_MODE_STATIC, record->color);
                record->color = "就绪";
                record->isApply = ComponentState_Ready;
                model->updateColumnWithRoles(0);
                UpdateApplyLogic();
            } else {
                // ShowErrorInfo("MAC:" + MAC + " 坐标:" + QString::number(i), "系统错误，刚刚是否有未经授权的操作?");
            }
        }
    }
}
void MainWindow::CX01Error_SerialRecive(const QString &message, DeviceType device_type) {
    QString temp = message;
    qDebug() << "元器件请求错误:" << message;
    QTextStream stream(&temp);
    QString MAC, coordinate, data;
    stream >> data >> MAC >> coordinate >> data;
    if (model->component_record_Hash_MACD.contains(QString(MAC + coordinate))) {
        component_record_struct *record = model->component_record_Hash_MACD.value(MAC + coordinate);
        if (record->isApply == ComponentState_APPLYIN) {
            record->isApply = ComponentState_OUT;
            colorAllocator->deallocateColor(LED_MODE_FLASH_FAST_3, record->color);
            record->color = "已取出";
            model->updateColumnWithRoles(0);
            ShowErrorInfo("MAC:" + MAC + " 坐标:" + coordinate, "没有B53设备响应这个操作");
            UpdateApplyLogic();
        } else if (record->isApply == ComponentState_APPLYOUT) {
            record->isApply = ComponentState_Ready;
            colorAllocator->deallocateColor(LED_MODE_STATIC, record->color);
            record->color = "就绪";
            model->updateColumnWithRoles(0);
            ShowErrorInfo("MAC:" + MAC + " 坐标:" + coordinate, "没有设备响应这个操作");
            UpdateApplyLogic();
        }
    }
}

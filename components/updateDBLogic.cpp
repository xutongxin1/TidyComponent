//
// Created by xtx on 24-9-13.
//
#include <QDir>
#include <qnetworkreply.h>

#include "GetRequestHandler.h"
#include "mainwindow.h"

void MainWindow::loadDataFromFolder() {
    QDir dbDir(DBPATH);

    if (!dbDir.exists()) {
        qWarning() << "db文件夹不存在：" << DBPATH;
        dbDir.mkpath("."); // 创建文件夹
        return;
    }

    // 获取所有json文件
    QStringList jsonFiles = dbDir.entryList(QStringList() << "*.json", QDir::Files);

    for (const QString &fileName : jsonFiles) {
        // 跳过配置文件
        if (fileName == "device_config.json") {
            continue;
        }
        QString filePath = DBPATH + fileName;
        QFile file(filePath);

        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "无法打开文件：" << filePath;
            continue;
        }

        QByteArray jsonData = file.readAll();
        file.close();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "JSON 解析错误：" << parseError.errorString() << " 文件：" << filePath;
            continue;
        }

        if (!doc.isObject()) {
            qWarning() << "JSON 格式错误，不是对象：" << filePath;
            continue;
        }

        QJsonObject obj = doc.object();
        component_record_struct record;

        // 解析数据
        record.name = obj.value("name").toString();
        record.jlcid = obj.value("jlcid").toString();
        record.weight = static_cast<float>(obj.value("weight").toDouble());
        record.discription = obj.value("discription").toString();
        record.more_data = obj.value("more_data").toString();
        record.package = obj.value("package").toString();
        record.pdf_url = obj.value("pdf_url").toString();
        record.pdf_name = obj.value("pdf_name").toString();
        record.price = obj.value("price").toString();
        record.inventory = obj.value("inventory").toString();
        record.PID = obj.value("PID").toString();
        record.MAC = obj.value("MAC").toString();
        record.device_type = obj.value("device_type").toString();
        record.coordinate = obj.value("coordinate").toString();
        record.pdf_FileUrl = obj.value("pdf_FileUrl").toString();

        // 解析数组
        QJsonArray aliasesArray = obj.value("aliases").toArray();
        record.aliases.clear();
        for (const QJsonValue &value : aliasesArray) {
            record.aliases.append(value.toString());
        }
        record.aliases.resize(5); // 限制别名数量为 5

        QJsonArray pngArray = obj.value("png_FileUrl").toArray();
        for (const QJsonValue &value : pngArray) {
            record.png_FileUrl.append(value.toString());
        }

        QJsonArray schArray = obj.value("sch_svg_FileUrl").toArray();
        for (const QJsonValue &value : schArray) {
            record.sch_svg_FileUrl.append(value.toString());
        }

        QJsonArray pcbArray = obj.value("pcb_svg_FileUrl").toArray();
        for (const QJsonValue &value : pcbArray) {
            record.pcb_svg_FileUrl.append(value.toString());
        }

        addComponentToLib(record,true);
    }
    loadDeviceConfig();
    reactComponentHash();// 手动更新组件哈希表
}

void MainWindow::SaveDataToFolder() {
    QDir dbDir(DBPATH);

    if (!dbDir.exists()) {
        dbDir.mkpath(".");
    }

    // 清理旧的json文件（可选）
    // QStringList oldFiles = dbDir.entryList(QStringList() << "*.json", QDir::Files);
    // for (const QString &file : oldFiles) {
    //     if (file != "device_config.json") {
    //         dbDir.remove(file);
    //     }
    // }

    // 保存每个元件
    for (component_record_struct record : model->component_record) {
        SaveSingleComponent(record);
    }
}

// 保存单个元件
void MainWindow::SaveSingleComponent(component_record_struct record) {
    QDir dbDir(DBPATH);

    if (!dbDir.exists()) {
        dbDir.mkpath(".");
    }

    QString filePath = DBPATH + record.jlcid + ".json";
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开文件进行写入：" << filePath;
        return;
    }

    QJsonObject obj;
    obj["name"] = record.name;
    obj["jlcid"] = record.jlcid;
    obj["weight"] = record.weight;
    obj["discription"] = record.discription;
    obj["more_data"] = record.more_data;
    obj["package"] = record.package;
    obj["pdf_url"] = record.pdf_url;
    obj["pdf_name"] = record.pdf_name;
    obj["price"] = record.price;
    obj["inventory"] = record.inventory;
    obj["PID"] = record.PID;
    obj["MAC"] = record.MAC;
    obj["device_type"] = record.device_type;
    obj["coordinate"] = record.coordinate;
    obj["pdf_FileUrl"] = record.pdf_FileUrl;

    // 转换数组
    QJsonArray aliasesArray;
    for (const QString &alias : record.aliases) {
        if (!alias.isEmpty()) {
            aliasesArray.append(alias);
        }
    }
    obj["aliases"] = aliasesArray;

    QJsonArray pngArray;
    for (const QString &url : record.png_FileUrl) {
        pngArray.append(url);
    }
    obj["png_FileUrl"] = pngArray;

    QJsonArray schArray;
    for (const QString &url : record.sch_svg_FileUrl) {
        schArray.append(url);
    }
    obj["sch_svg_FileUrl"] = schArray;

    QJsonArray pcbArray;
    for (const QString &url : record.pcb_svg_FileUrl) {
        pcbArray.append(url);
    }
    obj["pcb_svg_FileUrl"] = pcbArray;

    QJsonDocument doc(obj);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    // 同步更新设备配置文件
    if (!record.MAC.isEmpty() && !record.coordinate.isEmpty() && !record.device_type.isEmpty()) {
        updateDeviceConfig(record.MAC, record.coordinate, record.device_type);
    }
}
void MainWindow::SaveSingleComponent(const QString &jlcid) {
    if (!model->component_record_Hash_cid.contains(jlcid)) {
        qWarning() << "找不到jlcid：" << jlcid;
        return;
    }

    component_record_struct *record = model->component_record_Hash_cid.value(jlcid);
    SaveSingleComponent(*record);
}

// 3. 更新设备配置文件
void MainWindow::updateDeviceConfig(const QString &MAC, const QString &coordinate, const QString &type) {
    // 查找是否已存在该MAC
    bool found = false;
    for (DeviceInfo &device : _config.devices) {
        if (device.MAC == MAC) {
            // 检查coordinate是否已存在
            if (!device.coordinates.contains(coordinate)) {
                device.coordinates.append(coordinate);
            }
            found = true;
            break;
        }
    }

    // 如果不存在，新增设备
    if (!found) {
        DeviceInfo newDevice;
        newDevice.MAC = MAC;
        newDevice.coordinates.append(coordinate);
        newDevice.type = type;
        _config.devices.append(newDevice);
    }

    // 保存配置文件
    saveDeviceConfig();
}

// 4. 保存设备配置文件
bool MainWindow::saveDeviceConfig() {
    QString configPath = CONFIGPATH + "device_config.json";
    QFile file(configPath);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开设备配置文件进行写入:" << configPath;
        return false;
    }

    QJsonObject rootObj;
    QJsonArray devicesArray;

    for (const DeviceInfo &device : _config.devices) {
        QJsonObject deviceObj;
        deviceObj.insert("MAC", device.MAC);
        deviceObj.insert("type", device.type);

        QJsonArray coordArray;
        for (const QString &coord : device.coordinates) {
            coordArray.append(coord);
        }
        deviceObj.insert("coordinates", coordArray);

        devicesArray.append(deviceObj);
    }

    rootObj.insert("devices", devicesArray);

    QJsonDocument doc(rootObj);
    file.write(doc.toJson());
    file.close();

    qDebug() << "设备配置已保存到:" << configPath;
    return true;
}

// 5. 读取设备配置文件
void MainWindow::loadDeviceConfig() {
    QString configPath = CONFIGPATH + "device_config.json";
    QFile file(configPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "设备配置文件不存在，创建新配置";
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "设备配置文件JSON解析错误：" << parseError.errorString();
        return;
    }

    QJsonObject rootObj = doc.object();
    QJsonArray devicesArray = rootObj.value("devices").toArray();

    for (const QJsonValue &value : devicesArray) {
        QJsonObject deviceObj = value.toObject();
        DeviceInfo device;
        device.MAC = deviceObj.value("MAC").toString();
        device.type = deviceObj.value("type").toString();

        QJsonArray coordArray = deviceObj.value("coordinates").toArray();
        for (const QJsonValue &coordValue : coordArray) {
            device.coordinates.append(coordValue.toString());
        }

        _config.devices.append(device);
        _config.deviceMap.insert(device.MAC, &_config.devices.last());
    }
}

// 辅助函数：获取特定MAC的设备信息
MainWindow::DeviceInfo *MainWindow::getDeviceByMAC(const QString &MAC) const {
    return _config.deviceMap.value(MAC, nullptr);
}

// 辅助函数：删除单个元件文件
void MainWindow::deleteSingleComponent(const QString &jlcid) const {
    QString filePath = DBPATH + jlcid + ".json";
    QFile file(filePath);

    if (file.exists()) {
        if (!file.remove()) {
            qWarning() << "无法删除文件：" << filePath;
        }
    }
}

bool MainWindow::isExistingComponent(const QString &CID) const {
    return model->component_record_Hash_cid.contains(CID);
}
void MainWindow::reactComponentHash() const {
    model->component_record_Hash_cid.clear();
    model->component_record_Hash_MACD.clear();
    for (int i = 0; i < model->component_record.size(); ++i) {
        component_record_struct &record = model->component_record[i];
        updateSearchKey(record);
        model->component_record_Hash_cid.insert(record.jlcid, &model->component_record[i]);
        model->component_record_Hash_MACD.insert(record.MAC + record.coordinate, &model->component_record[i]);
    }
}
void MainWindow::addComponentToLib(const component_record_struct &_addingComponentObj, const bool isReacting) const {
    model->component_record.append(_addingComponentObj);
    if (!isReacting) {
        reactComponentHash();
    }
}
void MainWindow::replaceComponentToLib(const component_record_struct &_replacingComponentObj) const {
    // 检查该 jlcid 是否已存在于哈希表中
    if (model->component_record_Hash_cid.contains(_replacingComponentObj.jlcid)) {
        // 如果存在，先在列表中找到并替换
        for (int i = 0; i < model->component_record.size(); ++i) {
            if (model->component_record[i].jlcid == _replacingComponentObj.jlcid) {
                // 替换列表中的元素
                model->component_record[i] = _replacingComponentObj;
                // 更新哈希表中的元素
                reactComponentHash();
                break;
            }
        }
    } else {
        // 如果不存在，直接添加
        addComponentToLib(_replacingComponentObj);
    }
}

void MainWindow::updateSearchKey(component_record_struct &record) {
    record.searchKey = record.name + record.jlcid + record.
        discription + record.package + record.more_data;
    bool isAlias = false;
    for (const auto &alias : record.aliases) {
        if (!alias.isEmpty()) {
            isAlias = true;
            record.searchKey += alias;

        }
    }
    record.isAlias = isAlias;
}
void MainWindow::updateOneComponent(const QString &CID) {
    qDebug() << "https://api.h49591b27.nyat.app:10268/item/" + CID;
    getRequest("https://api.h49591b27.nyat.app:10268/item/" + CID, [&,CID](const QJsonObject &jsonObj) {
                   qDebug() << jsonObj;
                   component_record_struct _tmpComponentObj;
                   AnalyzeComponentData(CID, jsonObj, _tmpComponentObj);
                   replaceComponentToLib(_tmpComponentObj);
                   updateComponentInfo(tableView->selectionModel()->selection(), QItemSelection());
                   model->updateDataWithNoRowChange();
                   SaveSingleComponent(_tmpComponentObj);
                   _showInfo_updateInfoButton->setEnabled(true);
                   ShowSuccessInfo(CID + "信息更新成功");
               }, [&](const QNetworkReply::NetworkError error, const QString &ErrorInfo) {
                   if (error == QNetworkReply::NetworkError::InternalServerError) {
                       ShowErrorInfo("服务端错误，请联系服务器管理员");
                   }
                   ShowErrorInfo("？");
                   _showInfo_updateInfoButton->setEnabled(true);
               }, [&] {
                   ShowErrorInfo("请求超时，请检查网络连接");
                   _showInfo_updateInfoButton->setEnabled(true);
               });
}

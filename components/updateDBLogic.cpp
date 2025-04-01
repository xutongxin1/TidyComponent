//
// Created by xtx on 24-9-13.
//
#include "mainwindow.h"
void MainWindow::loadData() const {
    QFile file(CONFIGPATH + "db.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("无法打开 db.json 文件。");
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON 解析错误：" << parseError.errorString();
        return;
    }

    QJsonArray jsonArray = doc.array();

    for (const auto &value : jsonArray) {
        if (!value.isObject())
            continue;

        QJsonObject obj = value.toObject();
        component_record_struct record;

        record.name = obj.value("name").toString();
        // record.color = obj.value("color").toString();
        record.jlcid = obj.value("jlcid").toString();
        record.weight = static_cast<float>(obj.value("weight").toDouble());
        record.discription = obj.value("discription").toString();
        record.more_data = obj.value("more_data").toString();
        record.package = obj.value("package").toString();
        record.pdf_url = obj.value("pdf_url").toString();
        record.pdf_name = obj.value("pdf_name").toString();
        record.aliases = obj.value("aliases").toVariant().toStringList().toVector();
        record.aliases.resize(5); // 限制别名数量为 5
        record.png_FileUrl = obj.value("png_FileUrl").toVariant().toStringList().toVector();
        record.sch_svg_FileUrl = obj.value("sch_svg_FileUrl").toVariant().toStringList().toVector();
        record.pcb_svg_FileUrl = obj.value("pcb_svg_FileUrl").toVariant().toStringList().toVector();
        record.pdf_FileUrl = obj.value("pdf_FileUrl").toString();
        record.price= obj.value("price").toString();
        record.inventory = obj.value("inventory").toString();

        // 调用添加函数
        addComponentToLib(record);
    }
}
// WriteData() 函数实现
void MainWindow::SaveData() const {
    QFile file(CONFIGPATH + "db.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("无法打开 db.json 文件进行写入。");
        return;
    }

    QJsonArray jsonArray;

    for (const component_record_struct &record : model->component_record) {
        QJsonObject obj;

        obj["name"] = record.name;
        // obj["color"] = record.color;
        obj["jlcid"] = record.jlcid;
        obj["weight"] = record.weight;
        obj["discription"] = record.discription;
        obj["more_data"] = record.more_data;
        obj["package"] = record.package;
        obj["pdf_url"] = record.pdf_url;
        obj["pdf_name"] = record.pdf_name;
        obj["price"] = record.price;
        obj["inventory"] = record.inventory;

        // 将 QVector<QString> 转换为 QJsonArray
        QJsonArray aliasesArray;
        for (const QString &alias : record.aliases) {
            aliasesArray.append(alias);
        }
        obj["aliases"] = aliasesArray;

        QJsonArray png_FileUrlArray;
        for (const QString &url : record.png_FileUrl) {
            png_FileUrlArray.append(url);
        }
        obj["png_FileUrl"] = png_FileUrlArray;

        QJsonArray sch_svg_FileUrlArray;
        for (const QString &url : record.sch_svg_FileUrl) {
            sch_svg_FileUrlArray.append(url);
        }
        obj["sch_svg_FileUrl"] = sch_svg_FileUrlArray;

        QJsonArray pcb_svg_FileUrlArray;
        for (const QString &url : record.pcb_svg_FileUrl) {
            pcb_svg_FileUrlArray.append(url);
        }
        obj["pcb_svg_FileUrl"] = pcb_svg_FileUrlArray;

        obj["pdf_FileUrl"] = record.pdf_FileUrl;

        jsonArray.append(obj);
    }

    QJsonDocument doc(jsonArray);
    file.write(doc.toJson(QJsonDocument::Indented)); // 使用缩进格式，便于阅读
    file.close();
}

bool MainWindow::isExistingComponent(const QString &CID) const {
    return model->component_record_Hash.contains(CID);
}
void MainWindow::addComponentToLib(const component_record_struct &_addingComponentObj) const {
    model->component_record.append(_addingComponentObj);
    model->component_record_Hash.insert(_addingComponentObj.jlcid, _addingComponentObj);
    // if (model->component_record_Hash.contains("C569043")) {
    //     const std::shared_ptr<component_record_struct> tmp = model->component_record_Hash.value("C569043");
    //     qDebug() << tmp->name;
    // }
}
void MainWindow::updateSearchKey(component_record_struct &_addingComponentObj) {
    _addingComponentObj.searchKey = _addingComponentObj.name + _addingComponentObj.jlcid + _addingComponentObj.
        discription + _addingComponentObj.package + _addingComponentObj.more_data;
    for (const auto &alias : _addingComponentObj.aliases) {
        _addingComponentObj.searchKey += alias;
    }
}

// void MainWindow::importExcelToJson() {
//     // 打开 Excel 文件
//     QXlsx::Document xlsx("output.xlsx");
//     if (!xlsx.load()) {
//         qDebug() << "无法加载 Excel 文件。";
//         return;
//     }
//
//     QJsonObject data;
//     QJsonObject defaultObj;
//
//     int rowCount = xlsx.dimension().rowCount(); // 获取行数
//     int colCount = xlsx.dimension().columnCount(); // 获取列数
//
//     // 遍历 Excel 数据并填充 JSON 结构
//     for (int row = 2; row <= rowCount; ++row) {
//         // 假设第一行是标题，从第二行开始
//         QString itemId = QString::number(row - 1); // 从 1 开始编号
//         QJsonObject infoObj;
//
//         infoObj["Name"] = xlsx.read(row, 1).toString(); // 假设第一列是 "名称"
//         QJsonObject info;
//
//         info["JLCID"] = xlsx.read(row, 2).toString(); // "嘉立创ID"
//         info["tbLink"] = xlsx.read(row, 3).toString(); // "淘宝链接"
//         info["value"] = xlsx.read(row, 4).toString(); // "值"
//         info["package"] = xlsx.read(row, 5).toString(); // "封装"
//         info["Alias1"] = xlsx.read(row, 6).toString(); // "别名1"
//         info["Alias2"] = xlsx.read(row, 7).toString(); // "别名2"
//         info["Alias3"] = xlsx.read(row, 8).toString(); // "别名3"
//         info["Alias4"] = xlsx.read(row, 9).toString(); // "别名4"
//         info["Alias5"] = xlsx.read(row, 10).toString(); // "别名5"
//         info["Alias6"] = xlsx.read(row, 11).toString(); // "别名6"
//         info["Alias7"] = xlsx.read(row, 12).toString(); // "别名7"
//         info["Alias8"] = xlsx.read(row, 13).toString(); // "别名8"
//         info["Alias9"] = xlsx.read(row, 14).toString(); // "别名9"
//         info["Alias10"] = xlsx.read(row, 15).toString(); // "别名10"
//         info["MAC"] = xlsx.read(row, 16).toString(); // "对应mesh地址"
//         info["INAddr"] = xlsx.read(row, 17).toString(); // "对应内部地址"
//         info["Color"] = xlsx.read(row, 18).toString(); // "颜色"
//
//         infoObj["Info"] = info;
//         defaultObj[itemId] = infoObj;
//     }
//
//     data["_default"] = defaultObj;
//
//     // 保存为 JSON 文件
//     QFile file("db.json");
//     if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//         QTextStream stream(&file);
//         // stream.setCodec("GBK"); // 设置编码
//         QJsonDocument doc(data);
//         stream << doc.toJson(QJsonDocument::Indented);
//         file.close();
//         qDebug() << "db.json 文件已成功创建。";
//     } else {
//         qDebug() << "无法创建 JSON 文件。";
//     }
//
//     // 重新加载数据
//     loadData();
// }
//
// void MainWindow::exportJsonToExcel() {
//     // 读取 JSON 文件
//     QFile file("db.json");
//     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//         QMessageBox::warning(this, "Warning", "Cannot open db.json file");
//         return;
//     }
//
//     QByteArray jsonData = file.readAll();
//     file.close();
//
//     QJsonDocument document = QJsonDocument::fromJson(jsonData);
//     QJsonObject jsonObject = document.object();
//     QJsonObject defaultObject = jsonObject["_default"].toObject();
//
//     QVector<QVector<QVariant> > rows;
//
//     // 提取信息并组织成 QVector
//     for (const QString &key : defaultObject.keys()) {
//         QJsonObject itemData = defaultObject[key].toObject();
//         QJsonObject info = itemData["Info"].toObject();
//         QVector<QVariant> row = {
//             itemData["Name"].toString(),
//             info["JLCID"].toString(),
//             info["tbLink"].toString(),
//             info["value"].toString(),
//             info["package"].toString(),
//             info["Alias1"].toString(),
//             info["Alias2"].toString(),
//             info["Alias3"].toString(),
//             info["Alias4"].toString(),
//             info["Alias5"].toString(),
//             info["Alias6"].toString(),
//             info["Alias7"].toString(),
//             info["Alias8"].toString(),
//             info["Alias9"].toString(),
//             info["Alias10"].toString(),
//             info["MAC"].toString(),
//             info["INAddr"].toString(),
//             info["Color"].toString()
//         };
//         rows.append(row);
//     }
//
//     // 创建 Excel 文件并写入数据
//     QXlsx::Document xlsx;
//     QXlsx::Worksheet *worksheet = xlsx.currentWorksheet();
//
//     // 设置表头
//     QStringList headers = {
//         "名称", "嘉立创ID", "淘宝链接", "值", "封装", "别名1", "别名2", "别名3", "别名4",
//         "别名5", "别名6", "别名7", "别名8", "别名9", "别名10", "对应mesh地址", "对应内部地址", "颜色"
//     };
//
//     for (int i = 0; i < headers.size(); ++i) {
//         xlsx.write(1, i + 1, headers[i]);
//     }
//
//     // 写入数据
//     for (int i = 0; i < rows.size(); ++i) {
//         for (int j = 0; j < rows[i].size(); ++j) {
//             xlsx.write(i + 2, j + 1, rows[i][j]);
//         }
//     }
//
//     // 添加数据验证
//     QXlsx::DataValidation jlcidValidation(QXlsx::DataValidation::Custom, QXlsx::DataValidation::Between,
//                                           "=AND(LEFT(B2,1)=\"C\", ISNUMBER(VALUE(MID(B2, 2, LEN(B2)-1))))");
//     jlcidValidation.addRange("B2:B1048576");
//     jlcidValidation.setErrorMessage("该单元格必须以'C'开头，后面跟随数字的嘉立创编号。", "无效输入");
//     xlsx.addDataValidation(jlcidValidation);
//
//     QXlsx::DataValidation tbLinkValidation(QXlsx::DataValidation::Custom, QXlsx::DataValidation::Between,
//                                            "=LEFT(C2,24)=\"https://item.taobao.com/\"");
//     tbLinkValidation.addRange("C2:C1048576");
//     tbLinkValidation.setErrorMessage("该单元格必须以'https://item.taobao.com/'开头的淘宝链接。", "无效输入");
//     xlsx.addDataValidation(tbLinkValidation);
//
//     // 保存为 Excel 文件
//     xlsx.saveAs("output.xlsx");
//
//     QMessageBox::information(this, "Success", "Excel 文件已成功创建。");
//
//     // 打开文件
//     QDesktopServices::openUrl(QUrl::fromLocalFile("output.xlsx"));
// }

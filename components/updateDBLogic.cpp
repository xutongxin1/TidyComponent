//
// Created by xtx on 24-9-13.
//
#include "mainwindow.h"

void MainWindow::loadData() {
    // 读取 JSON 文件
    QFile file("db.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Warning", "Cannot open db.json file");
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonObject records = QJsonDocument::fromJson(jsonData).object()["_default"].toObject();

    for (auto it = records.begin(); it != records.end(); ++it) {
        QJsonObject recordJson = it.value().toObject();
        QString name = recordJson.value("Name").toString();
        if (name.isEmpty())
            continue;

        QJsonObject info = recordJson.value("Info").toObject();

        // 创建一个 Record 对象并填充数据
        component_record record;
        record.name = name;
        record.color = info.value("Color").toString();
        record.jlcid = info.value("JLCID").toString();
        record.tbLink = info.value("tbLink").toString();
        record.value = info.value("value").toString();
        record.package = info.value("package").toString();

        // 读取别名（Alias1 到 Alias9）
        for (int i = 1; i <= 9; ++i) {
            QString aliasKey = QString("Alias%1").arg(i);
            record.aliases.append(info.value(aliasKey).toString());
        }

        // 将 Record 对象添加到 QVector 中
        recordsVector.append(record);
    }

    ShowAllComponents();//显示所有元器件
    // (void) QtConcurrent::run(
    //        [&] {
    //
    //        });

}


void MainWindow::importExcelToJson() {
    // 打开 Excel 文件
    QXlsx::Document xlsx("output.xlsx");
    if (!xlsx.load()) {
        qDebug() << "无法加载 Excel 文件。";
        return;
    }

    QJsonObject data;
    QJsonObject defaultObj;

    int rowCount = xlsx.dimension().rowCount(); // 获取行数
    int colCount = xlsx.dimension().columnCount(); // 获取列数

    // 遍历 Excel 数据并填充 JSON 结构
    for (int row = 2; row <= rowCount; ++row) {
        // 假设第一行是标题，从第二行开始
        QString itemId = QString::number(row - 1); // 从 1 开始编号
        QJsonObject infoObj;

        infoObj["Name"] = xlsx.read(row, 1).toString(); // 假设第一列是 "名称"
        QJsonObject info;

        info["JLCID"] = xlsx.read(row, 2).toString(); // "嘉立创ID"
        info["tbLink"] = xlsx.read(row, 3).toString(); // "淘宝链接"
        info["value"] = xlsx.read(row, 4).toString(); // "值"
        info["package"] = xlsx.read(row, 5).toString(); // "封装"
        info["Alias1"] = xlsx.read(row, 6).toString(); // "别名1"
        info["Alias2"] = xlsx.read(row, 7).toString(); // "别名2"
        info["Alias3"] = xlsx.read(row, 8).toString(); // "别名3"
        info["Alias4"] = xlsx.read(row, 9).toString(); // "别名4"
        info["Alias5"] = xlsx.read(row, 10).toString(); // "别名5"
        info["Alias6"] = xlsx.read(row, 11).toString(); // "别名6"
        info["Alias7"] = xlsx.read(row, 12).toString(); // "别名7"
        info["Alias8"] = xlsx.read(row, 13).toString(); // "别名8"
        info["Alias9"] = xlsx.read(row, 14).toString(); // "别名9"
        info["Alias10"] = xlsx.read(row, 15).toString(); // "别名10"
        info["MAC"] = xlsx.read(row, 16).toString(); // "对应mesh地址"
        info["INAddr"] = xlsx.read(row, 17).toString(); // "对应内部地址"
        info["Color"] = xlsx.read(row, 18).toString(); // "颜色"

        infoObj["Info"] = info;
        defaultObj[itemId] = infoObj;
    }

    data["_default"] = defaultObj;

    // 保存为 JSON 文件
    QFile file("db.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        // stream.setCodec("GBK"); // 设置编码
        QJsonDocument doc(data);
        stream << doc.toJson(QJsonDocument::Indented);
        file.close();
        qDebug() << "db.json 文件已成功创建。";
    } else {
        qDebug() << "无法创建 JSON 文件。";
    }

    // 重新加载数据
    loadData();
}

void MainWindow::exportJsonToExcel() {
    // 读取 JSON 文件
    QFile file("db.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Warning", "Cannot open db.json file");
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    QJsonObject jsonObject = document.object();
    QJsonObject defaultObject = jsonObject["_default"].toObject();

    QVector<QVector<QVariant> > rows;

    // 提取信息并组织成 QVector
    for (const QString &key : defaultObject.keys()) {
        QJsonObject itemData = defaultObject[key].toObject();
        QJsonObject info = itemData["Info"].toObject();
        QVector<QVariant> row = {
            itemData["Name"].toString(),
            info["JLCID"].toString(),
            info["tbLink"].toString(),
            info["value"].toString(),
            info["package"].toString(),
            info["Alias1"].toString(),
            info["Alias2"].toString(),
            info["Alias3"].toString(),
            info["Alias4"].toString(),
            info["Alias5"].toString(),
            info["Alias6"].toString(),
            info["Alias7"].toString(),
            info["Alias8"].toString(),
            info["Alias9"].toString(),
            info["Alias10"].toString(),
            info["MAC"].toString(),
            info["INAddr"].toString(),
            info["Color"].toString()
        };
        rows.append(row);
    }

    // 创建 Excel 文件并写入数据
    QXlsx::Document xlsx;
    QXlsx::Worksheet *worksheet = xlsx.currentWorksheet();

    // 设置表头
    QStringList headers = {
        "名称", "嘉立创ID", "淘宝链接", "值", "封装", "别名1", "别名2", "别名3", "别名4",
        "别名5", "别名6", "别名7", "别名8", "别名9", "别名10", "对应mesh地址", "对应内部地址", "颜色"
    };

    for (int i = 0; i < headers.size(); ++i) {
        xlsx.write(1, i + 1, headers[i]);
    }

    // 写入数据
    for (int i = 0; i < rows.size(); ++i) {
        for (int j = 0; j < rows[i].size(); ++j) {
            xlsx.write(i + 2, j + 1, rows[i][j]);
        }
    }

    // 添加数据验证
    QXlsx::DataValidation jlcidValidation(QXlsx::DataValidation::Custom, QXlsx::DataValidation::Between,
                                          "=AND(LEFT(B2,1)=\"C\", ISNUMBER(VALUE(MID(B2, 2, LEN(B2)-1))))");
    jlcidValidation.addRange("B2:B1048576");
    jlcidValidation.setErrorMessage("该单元格必须以'C'开头，后面跟随数字的嘉立创编号。", "无效输入");
    xlsx.addDataValidation(jlcidValidation);

    QXlsx::DataValidation tbLinkValidation(QXlsx::DataValidation::Custom, QXlsx::DataValidation::Between,
                                           "=LEFT(C2,24)=\"https://item.taobao.com/\"");
    tbLinkValidation.addRange("C2:C1048576");
    tbLinkValidation.setErrorMessage("该单元格必须以'https://item.taobao.com/'开头的淘宝链接。", "无效输入");
    xlsx.addDataValidation(tbLinkValidation);

    // 保存为 Excel 文件
    xlsx.saveAs("output.xlsx");

    QMessageBox::information(this, "Success", "Excel 文件已成功创建。");

    // 打开文件
    QDesktopServices::openUrl(QUrl::fromLocalFile("output.xlsx"));
}
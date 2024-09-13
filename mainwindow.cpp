#include "mainwindow.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QMap>
#include <algorithm>

#include "./ui_mainwindow.h"
#include "xlsxdatavalidation.h"
#include "thirdLib/QXlsx/QXlsx/header/xlsxdocument.h"

int record_DeviceNum = 0, record_WinNum = 0;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui_(new Ui::MainWindow) {
    ui_->setupUi(this);
    // 设置列数
    ui_->tableWidget->setColumnCount(16);

    // 设置水平表头标签
    QStringList headerLabels = {
        "显示", "名称", "嘉立创ID", "淘宝链接", "值", "封装", "别名1", "别名2", "别名3", "别名4",
        "别名5", "别名6", "别名7", "别名8", "别名9", "别名10", "淘宝链接"
    };
    ui_->tableWidget->setHorizontalHeaderLabels(headerLabels);

    // 禁用tableWidget修改
    ui_->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 初始化导入数据
    loadData();

    // 绑定导入导出按钮
    connect(ui_->btn_export, &QPushButton::clicked, this, &MainWindow::exportJsonToExcel);
    connect(ui_->btn_import, &QPushButton::clicked, this, &MainWindow::importExcelToJson);

    connect(ui_->input_search, &QLineEdit::returnPressed, this, &MainWindow::search);
}



// 计算两个字符串的相似度（简单实现，可以替换为更复杂的算法）
double MainWindow::calculateSimilarity(const QString& a, const QString& b) {
    if (a.isEmpty() || b.isEmpty()) return 0.0;
    int common = 0;
    QStringList wordsA = a.split(' ', Qt::SkipEmptyParts);
    QStringList wordsB = b.split(' ', Qt::SkipEmptyParts);

    for (const QString& wordA : wordsA) {
        for (const QString& wordB : wordsB) {
            if (wordA.contains(wordB, Qt::CaseInsensitive) || wordB.contains(wordA, Qt::CaseInsensitive)) {
                common++;
                break;
            }
        }
    }

    return static_cast<double>(common) / (wordsA.size() + wordsB.size() - common);
}

// 检查所有词是否都在记录字段中出现
bool MainWindow::isExactMatch(const component_record& record, const QStringList& searchWords) {
    int matchCount = 0;

    for (const auto& word : searchWords) {
        bool found = false;
        if (record.name.contains(word, Qt::CaseInsensitive) ||
            record.value.contains(word, Qt::CaseInsensitive) ||
            record.package.contains(word, Qt::CaseInsensitive)) {
            found = true;
        } else {
            for (const auto& alias : record.aliases) {
                if (alias.contains(word, Qt::CaseInsensitive)) {
                    found = true;
                    break;
                }
            }
        }

        if (found) {
            matchCount++;
        } else {
            return false; // 如果有一个词未找到，则不是完全匹配
        }
    }

    return true; // 所有词都找到了
}

// 查找最接近的记录
QVector<MainWindow::component_record> MainWindow::findClosestRecords(const QVector<component_record>& recordsVector, const QString& searchString) {
    QMap<double, component_record> similarityMap;
    QStringList searchWords = searchString.split(' ', Qt::SkipEmptyParts);
    QVector<component_record> exactMatches;

    for (const auto& record : recordsVector) {
        // 首先进行精确匹配检查
        if (isExactMatch(record, searchWords)) {
            exactMatches.append(record);
        } else {
            // 计算模糊相似度
            double totalSimilarity = 0.0;
            totalSimilarity += calculateSimilarity(record.name, searchString);
            totalSimilarity += calculateSimilarity(record.value, searchString);
            totalSimilarity += calculateSimilarity(record.package, searchString);

            for (const auto& alias : record.aliases) {
                totalSimilarity += calculateSimilarity(alias, searchString);
            }

            // 将相似度存入Map中，键是相似度，值是记录
            similarityMap.insert(totalSimilarity, record);
        }
    }

    // 最终结果
    QVector<component_record> result;

    // 添加精确匹配结果
    result += exactMatches;

    // 添加前5个最接近的模糊匹配结果
    auto it = similarityMap.end();
    int count = 0;

    while (it != similarityMap.begin() && count < 5) {
        --it;
        result.append(it.value());
        count++;
    }

    return result;
}

void MainWindow::search() {
    QString searchString = ui_->input_search->text();
    QVector<component_record> closestRecords = findClosestRecords(recordsVector, searchString);

    for (const auto& record : closestRecords) {
        qDebug() << "Closest Record: " << record.name << ", " << record.value << ", " << record.package;
    }
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

MainWindow::~MainWindow() {
    delete ui_;
}

void MainWindow::GetConstructConfig() {
}

void MainWindow::SaveConstructConfig() {
}

// 创建超链接标签的函数
QLabel *MainWindow::createHyperlinkLabel(const QString &text, const QString &url) {
    QLabel *label = new QLabel();

    // 设置超链接文本
    label->setText(QString("<a href=\"%1\">%2</a>").arg(url).arg(text));
    label->setTextFormat(Qt::RichText); // 设置文本格式为富文本
    label->setTextInteractionFlags(Qt::TextBrowserInteraction); // 设置文本交互标志为浏览器交互
    label->setOpenExternalLinks(true); // 允许打开外部链接

    // 如果需要居中对齐标签，可以取消以下注释
    // label->setAlignment(Qt::AlignCenter);

    return label;
}
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

    ui_->tableWidget->setRowCount(records.size());

    // 锁定列宽
    ui_->tableWidget->setColumnWidth(0, 64); // 设置第一列宽度为64像素
    ui_->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed); // 锁定第一列宽度

    int row = 0;
    for (auto it = records.begin(); it != records.end(); ++it, ++row) {
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

        // 更新界面
        ui_->tableWidget->setItem(row, 1, new QTableWidgetItem(record.name));

        // 设置控制按钮
        addButtonToTable(row, 0, record.color);

        // 处理JLCID
        if (record.jlcid.isEmpty()) {
            ui_->tableWidget->setItem(row, 2, new QTableWidgetItem(""));
        } else {
            QString url = QString("https://so.szlcsc.com/global.html?k=%1").arg(record.jlcid);
            QLabel *label = createHyperlinkLabel(record.jlcid, url);
            ui_->tableWidget->setCellWidget(row, 2, label);
        }

        // 处理TBLink
        if (record.tbLink.isEmpty()) {
            QString url = QString("https://shop131282813.taobao.com/search.htm?keyword=%1").arg(record.name);
            QLabel *label = createHyperlinkLabel("尝试优信搜索", url);
            ui_->tableWidget->setCellWidget(row, 3, label);
        } else {
            QLabel *label = createHyperlinkLabel(record.tbLink, record.tbLink);
            ui_->tableWidget->setCellWidget(row, 3, label);
        }

        // 设置其余单元格数据
        ui_->tableWidget->setItem(row, 4, new QTableWidgetItem(record.value));
        ui_->tableWidget->setItem(row, 5, new QTableWidgetItem(record.package));

        for (int col = 6; col <= 14; ++col) {
            ui_->tableWidget->setItem(row, col, new QTableWidgetItem(record.aliases[col - 6]));
        }
    }
}

void MainWindow::addButtonToTable(int row, int col, const QString &color) {
    // // 创建按钮
    // DropDownToolButton *button = new DropDownToolButton(this);
    // // connect(button, &QPushButton::clicked, [this, row, button]() { handleButtonClicked(row, button); });
    //
    // // 将十六进制颜色字符串转换为RGB
    // int r = color.mid(0, 2).toInt(nullptr, 16);
    // int g = color.mid(2, 2).toInt(nullptr, 16);
    // int b = color.mid(4, 2).toInt(nullptr, 16);
    // QString newBackgroundRgba = QString("background: rgba(%1, %2, %3, 0.7);").arg(r).arg(g).arg(b);
    //
    // // 设置按钮样式
    // button->setStyleSheet(button->styleSheet().replace("background: rgba(255, 255, 255, 0.7);", newBackgroundRgba));
    //
    // // 添加按钮到单元格
    // ui->tableWidget->setCellWidget(row, col, button);
    //
    // // 创建菜单（如果需要）
    // QMenu *menu = new QMenu(button);
    // QAction *sendAction = new QAction("Send", this);
    // QAction *saveAction = new QAction("Save", this);
    // connect(sendAction, &QAction::triggered, this, []() { qDebug() << "已发送"; });
    // connect(saveAction, &QAction::triggered, this, []() { qDebug() << "已保存"; });
    // menu->addAction(sendAction);
    // menu->addAction(saveAction);
    //
    // // 将菜单关联到按钮
    // button->setMenu(menu);
}

void MainWindow::InitConfig() {
    config_main_ini_ = new ConfigClass("main.ini", QSettings::IniFormat);
    device_count_ = config_main_ini_->value("/Device/device_num").toInt();
    config_device_ini_.emplace_back();
    for (int i = 1; i <= device_count_; i++) {
        config_device_ini_.push_back(new ConfigClass("Device" + QString::number(i) + ".ini",
                                                     QSettings::IniFormat));
        //        QSettings *tmp=new QSettings("config/Device" + QString::number(i) + ".ini",
        //                                       QSettings::IniFormat);
        //        config_device_ini_[i]->setValue("/ee/connect_mode",123);
        //        config_device_ini_[i]->sync();
    }
}

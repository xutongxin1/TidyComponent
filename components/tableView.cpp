//
// Created by xtx on 24-9-13.
//

#include "mainwindow.h"
// 创建超链接标签的函数
QLabel *MainWindow::createHyperlinkLabel(const QString &text, const QString &url) {
    auto *label = new QLabel();

    // 设置超链接文本
    label->setText(QString("<a href=\"%1\">%2</a>").arg(url).arg(text));
    label->setTextFormat(Qt::RichText); // 设置文本格式为富文本
    label->setTextInteractionFlags(Qt::TextBrowserInteraction); // 设置文本交互标志为浏览器交互
    label->setOpenExternalLinks(true); // 允许打开外部链接

    // 如果需要居中对齐标签，可以取消以下注释
    // label->setAlignment(Qt::AlignCenter);

    return label;
}
void MainWindow::ShowAllComponents() {

    auto start = std::chrono::high_resolution_clock::now();

    ui_->tableWidget->setRowCount(0); // 清空表格
    ui_->tableWidget->setRowCount(recordsVector.size());

    // 锁定列宽
    ui_->tableWidget->setColumnWidth(0, 64); // 设置第一列宽度为64像素
    ui_->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed); // 锁定第一列宽度

    int row = 0; // 初始化行数
    for (const component_record &record : recordsVector) {


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

        ++row; // 移动到下一行
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "渲染用时: " << duration << " ms\n";
}

void MainWindow::ShowSomeComponents() {
    ui_->tableWidget->setRowCount(0); // 清空表格

    //确定行数
    if (exacIndex.isEmpty() && fuzzyIndex.isEmpty()) {
        ui_->tableWidget->setRowCount(1); // 设置表格行数
        ui_->tableWidget->setSpan(0, 0, 1, ui_->tableWidget->columnCount()); // 合并整行
        ui_->tableWidget->setItem(0, 0, new QTableWidgetItem("未搜索到相关元器件"));
        return;
    }
    int totalRows = 0; // 总行数，包含两个空行
    ui_->tableWidget->setRowCount(totalRows); // 设置表格行数
    // 锁定列宽
    ui_->tableWidget->setColumnWidth(0, 64); // 设置第一列宽度为64像素
    ui_->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed); // 锁定第一列宽度
    int row = 0; // 初始化行数
    //检查有没有精确搜索结果
    if (!exacIndex.isEmpty()) {
        totalRows = exacIndex.size() + 1; // 总行数，包含两个空行
        ui_->tableWidget->setRowCount(totalRows); // 设置表格行数
        // 第一行合并显示“精确搜索结果”
        ui_->tableWidget->setSpan(row, 0, 1, ui_->tableWidget->columnCount()); // 合并整行
        ui_->tableWidget->setItem(row, 0, new QTableWidgetItem("精确搜索结果："));

        row++; // 移动到下一行

        // 显示精确搜索结果
        for (int index : exacIndex) {
            if (index >= 0 && index < recordsVector.size()) {
                // 检查索引范围有效性
                const component_record &record = recordsVector[index];

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

                row++; // 移动到下一行
            }
        }
    }

    //检查有没有模糊搜索结果
    if (!fuzzyIndex.isEmpty()) {

        totalRows=totalRows+fuzzyIndex.size()+1;
        ui_->tableWidget->setRowCount(totalRows); // 设置表格行数

        // 插入一个空行并合并显示“模糊搜索结果”
        ui_->tableWidget->setSpan(row, 0, 1, ui_->tableWidget->columnCount()); // 合并整行
        ui_->tableWidget->setItem(row, 0, new QTableWidgetItem("模糊搜索结果："));
        // ui_->tableWidget->item(row,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        row++; // 移动到下一行

        // 显示模糊搜索结果
        for (int index : fuzzyIndex) {
            if (index >= 0 && index < recordsVector.size()) {
                // 检查索引范围有效性
                const component_record &record = recordsVector[index];

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

                row++; // 移动到下一行
            }
        }
    }
}

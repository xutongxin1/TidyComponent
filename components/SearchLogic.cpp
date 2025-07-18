//
// Created by xtx on 24-9-13.
//
#include <iostream>

#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include "xlsxdocument.h"
#include "xlsxworksheet.h"
#include "xlsxcell.h"
#include "xlsxcellrange.h"
// 计算两个字符串的Levenshtein距离
double calculateSimilarity(const QString &a, const QString &b) {
    if (a.isEmpty() || b.isEmpty()) return 0.0;

    const int m = static_cast<int>(a.length());
    const int n = static_cast<int>(b.length());

    // 创建dp数组
    std::vector<std::vector<int> > dp(m + 1, std::vector<int>(n + 1, 0));

    // 初始化dp数组
    for (int i = 0; i <= m; ++i) dp[i][0] = i;
    for (int j = 0; j <= n; ++j) dp[0][j] = j;

    // 计算Levenshtein距离
    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            dp[i][j] = (std::min)({
                dp[i - 1][j] + 1, // 删除
                dp[i][j - 1] + 1, // 插入
                dp[i - 1][j - 1] + cost
            }); // 替换
        }
    }

    int distance = dp[m][n];
    int maxLength = (std::max)(m, n);

    // 防止越界
    if (maxLength == 0) return 1.0;

    // 计算相似度
    return 1.0 - static_cast<double>(distance) / maxLength;
}

// 检查所有词是否都在记录字段中出现
bool isExactMatch(const component_record_struct &record, const QStringList &searchWords) {
    int matchCount = 0;

    for (const auto &word : searchWords) {
        bool found = false;
        if (record.name.contains(word, Qt::CaseInsensitive) ||
            record.discription.contains(word, Qt::CaseInsensitive) ||
            record.package.contains(word, Qt::CaseInsensitive) ||
            record.jlcid.contains(word, Qt::CaseInsensitive) ||
            record.more_data.contains(word, Qt::CaseInsensitive)) {
            found = true;
        } else {
            for (const auto &alias : record.aliases) {
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
void MainWindow::fuzzy_search_records(const QString &searchString) const {
    auto start = std::chrono::high_resolution_clock::now();

    //清空记录
    model->exactPoint.clear();
    model->fuzzyPoint.clear();

    QMultiMap<double, component_record_struct *> similarityMap;
    const QStringList searchWords = searchString.split(' ', Qt::SkipEmptyParts);

    for (auto &record : model->component_record) {
        // 首先进行精确匹配检查
        if (isExactMatch(record, searchWords)) {
            model->exactPoint.append(&record);
        } else {
            // 将相似度存入Map中，键是相似度，值是记录
            if (double totalSimilarity = calculateSimilarity(record.searchKey, searchString); totalSimilarity > 0.0) {
                similarityMap.insert(totalSimilarity, &record);
            }
        }
    }

    // 添加前5个最接近的模糊匹配结果
    auto it = similarityMap.end();
    int count = 0;

    while (it != similarityMap.begin() && count < 5) {
        --it;
        model->fuzzyPoint.append(it.value());
        count++;
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "搜索用时: " << duration << " ms\n";
}
void MainWindow::exact_search_records(const QString &searchString) const {
    //清空记录
    model->exactPoint.clear();
    model->fuzzyPoint.clear();
    model->noExitString.clear();
    // 分解searchString，以分号，或者逗号（考虑全角和半角）分割
    QStringList searchTerms;
    QString currentTerm = "";

    for (int i = 0; i < searchString.length(); ++i) {
        QChar ch = searchString.at(i);

        // 检查是否为分隔符（半角和全角的逗号、分号）
        if (ch == QChar(0x002C) || ch == QChar(0x003B) || ch == QChar(0xFF0C) || ch == QChar(0xFF1B)) {
            if (!currentTerm.trimmed().isEmpty()) {
                searchTerms.append(currentTerm.trimmed());
                currentTerm = "";
            }
        } else {
            currentTerm += ch;
        }
    }

    // 添加最后一个词条
    if (!currentTerm.trimmed().isEmpty()) {
        searchTerms.append(currentTerm.trimmed());
    }

    // 处理每个搜索词条，确保格式正确
    QStringList validSearchTerms;

    for (const QString &term : searchTerms) {
        QString processedTerm = term.trimmed().toUpper();

        if (processedTerm.isEmpty()) {
            continue;
        }

        // 如果是小写c开头，转换为大写C
        if (processedTerm.startsWith('c') || processedTerm.startsWith('C')) {
            // 检查C后面是否跟着数字
            QString numberPart = processedTerm.mid(1);
            bool isValidNumber = false;
            numberPart.toInt(&isValidNumber);

            if (isValidNumber && !numberPart.isEmpty()) {
                validSearchTerms.append("C" + numberPart);
            }
        }
        // 如果是纯数字，在前面添加C
        else {
            bool isNumber = false;
            processedTerm.toInt(&isNumber);

            if (isNumber) {
                validSearchTerms.append("C" + processedTerm);
            }
            // 其他情况直接舍弃（不符合C+数字格式）
        }
    }
    qDebug() << "有效词条" << validSearchTerms;

    for (const QString &searchTerm : validSearchTerms) {
        if (model->component_record_Hash_cid.contains(searchTerm)) {
            model->exactPoint.append(model->component_record_Hash_cid[searchTerm]);
        } else {
            model->noExitString.append(searchTerm);
        }
    }
}
void MainWindow::search() const {
    // QString searchString = ui_->input_search->text();
    const QString searchString = _searchBox->text();
    if (searchString.isEmpty()) {
        model->showAll = true;
        model->updateData();
        // ShowAllComponents();
        return;
    }
    model->showAll = false;
    if (model->searchType == ComponentTableModel::SEARCH_FUZZY) {
        fuzzy_search_records(searchString);
    } else if (model->searchType == ComponentTableModel::SEARCH_EXACT) {
        exact_search_records(searchString);
    }

    // for (const auto &record : model.exacIndex) {
    //     qDebug() << "精确结果: " << model.component_record[record].name << ", " << model.component_record[record].value << ", " <<
    //         model.component_record[record].package;
    // }
    //
    // for (const auto &record : model.fuzzyIndex) {
    //     qDebug() << "模糊结果: " << model.component_record[record].name << ", " << model.component_record[record].value << ", " <<
    //         model.component_record[record].package;
    // }
    // qDebug() << "搜索结束";

    model->updateData();

    if (!model->exactPoint.isEmpty() || !model->fuzzyPoint.isEmpty()) {
        QModelIndex secondRowIndex = model->index(1, 0); // 第二行，第一列
        tableView->selectionModel()->setCurrentIndex(
            secondRowIndex,
            QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows
        );
    }
}

void MainWindow::searchBoxClear() {
    _searchBox->clear();
    tableView->selectionModel()->clear();
    model->showAll = true;
    model->updateData();
    updateComponentShowInfo_Clear();
}
void MainWindow::searchLogicInit() {
    connect(_searchBox, &QLineEdit::textChanged, this, &MainWindow::search);
    connect(_searchBox, &QLineEdit::editingFinished, this, [&] {
        if (_searchBox->text().isEmpty()) {
            search(); //防止刷新两次
        }
    });
    connect(_resetSearchButton, &ElaToolButton::clicked, this, [&] {
        if (model->searchType == ComponentTableModel::SEARCH_BOM) {
            model->searchType = ComponentTableModel::SEARCH_FUZZY;
            _searchTypeButton->setText("搜索模式：\n模糊");
        }
        searchBoxClear();
        _searchTypeButton->setEnabled(true);
        _searchBox->setEnabled(true);
        _searchBox->setToolTip("");
        _openEDAChromeButton->setEnabled(true);
    });
    connect(_searchTypeButton, &ElaToolButton::clicked, this, [&] {
        if (model->searchType == ComponentTableModel::SEARCH_FUZZY) {
            model->searchType = ComponentTableModel::SEARCH_EXACT;
            _searchTypeButton->setText("搜索模式：\nCID精确");
            search();
        } else if (model->searchType == ComponentTableModel::SEARCH_EXACT) {
            model->searchType = ComponentTableModel::SEARCH_FUZZY;
            _searchTypeButton->setText("搜索模式：\n模糊");
            search();
        } else {
            model->searchType = ComponentTableModel::SEARCH_FUZZY;
            _searchTypeButton->setText("搜索模式：\n模糊");
            searchBoxClear();
        }
    });
    connect(_bomSearchButton, &ElaToolButton::clicked, this, [&] {
        bomSearch();
    });
}

void MainWindow::bomSearch() {
    // 1. 弹出文件选择框
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("选择Excel文件"),
        "",
        tr("Excel Files (*.xlsx)")
    );

    if (fileName.isEmpty()) {
        return; // 用户取消了文件选择
    }

    // 2. 打开Excel文件
    QXlsx::Document xlsx(fileName);
    if (!xlsx.load()) {
        ShowErrorInfo("无法打开Excel文件");
        return;
    }

    // 获取当前工作表
    QXlsx::Worksheet *worksheet = static_cast<QXlsx::Worksheet *>(xlsx.currentWorksheet());
    if (!worksheet) {
        ShowErrorInfo("无法获取工作表");
        return;
    }

    // 3. 查找列索引
    int supplierPartCol = -1;
    int footprintCol = -1;
    int valueCol = -1;
    int nameCol = -1;

    // 获取第一行的范围
    QXlsx::CellRange usedRange = worksheet->dimension();
    int maxCol = usedRange.lastColumn();

    // 在第一行查找所需的列
    for (int col = 1; col <= maxCol; ++col) {
        QXlsx::Cell *cell = worksheet->cellAt(1, col).get();
        if (cell) {
            QString headerValue = cell->value().toString().trimmed();

            if (headerValue == "Supplier Part") {
                supplierPartCol = col;
            } else if (headerValue == "Footprint") {
                footprintCol = col;
            } else if (headerValue == "Value") {
                valueCol = col;
            } else if (headerValue == "Name") {
                nameCol = col;
            }
        }
    }

    // 检查是否找到了Supplier Part列
    if (supplierPartCol == -1) {
        ShowErrorInfo("未找到Supplier Part列");
        return;
    }

    QStringList validSearchTerms;

    // 4. 处理数据行（从第2行开始）
    int maxRow = usedRange.lastRow();
    for (int row = 2; row <= maxRow; ++row) {
        QString searchTerm;

        // 获取Supplier Part列的值
        QXlsx::Cell *supplierPartCell = worksheet->cellAt(row, supplierPartCol).get();
        QString supplierPartValue = supplierPartCell ? supplierPartCell->value().toString().trimmed() : "";

        if (!supplierPartValue.isEmpty()) {
            // 如果Supplier Part列有值，直接使用
            searchTerm = supplierPartValue;
        } else {
            // 如果Supplier Part列为空，尝试组合其他列
            QString footprintValue;
            QString valueValue;
            QString nameValue;

            // 获取Footprint列的值
            if (footprintCol != -1) {
                QXlsx::Cell *footprintCell = worksheet->cellAt(row, footprintCol).get();
                footprintValue = footprintCell ? footprintCell->value().toString().trimmed() : "";
            }

            // 获取Value列的值
            if (valueCol != -1) {
                QXlsx::Cell *valueCell = worksheet->cellAt(row, valueCol).get();
                valueValue = valueCell ? valueCell->value().toString().trimmed() : "";
            }

            // 获取Name列的值
            if (nameCol != -1) {
                QXlsx::Cell *nameCell = worksheet->cellAt(row, nameCol).get();
                nameValue = nameCell ? nameCell->value().toString().trimmed() : "";
            }

            // 按优先级组合
            if (!valueValue.isEmpty()) {
                // Footprint + Value
                searchTerm = footprintValue + " " + valueValue;
            } else if (!nameValue.isEmpty()) {
                // Footprint + Name
                searchTerm = footprintValue + " " + nameValue;
            } else {
                // 只有Footprint
                searchTerm = footprintValue;
            }
        }

        // 如果最终结果不为空，添加到列表
        if (!searchTerm.isEmpty()) {
            validSearchTerms.append(searchTerm);
        }
    }

    // 5. 处理validSearchTerms
    for (const QString &searchTerm : validSearchTerms) {
        if (model->component_record_Hash_cid.contains(searchTerm)) {
            model->exactPoint.append(model->component_record_Hash_cid[searchTerm]);
        } else {
            model->noExitString.append(searchTerm);
        }
    }
    if (model->exactPoint.isEmpty() && model->noExitString.isEmpty()) {
        ShowWarningInfo("未找到任何匹配的元件");
        return;
    }
    _searchTypeButton->setEnabled(false);
    _searchBox->setEnabled(false);
    _openEDAChromeButton->setEnabled(false);
    _searchBox->setToolTip("请先复位搜索");
    model->searchType = ComponentTableModel::SEARCH_BOM;
    _searchTypeButton->setText("搜索模式：\nBOM表");
    model->showAll = false;
    model->updateData();

    // 可选：显示处理结果
    qDebug() << model->exactPoint << model->noExitString;
}

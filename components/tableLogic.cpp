//
// Created by xtx on 24-9-13.
//
#include "mainwindow.h"

// 计算两个字符串的相似度（简单实现，可以替换为更复杂的算法）
// double MainWindow::calculateSimilarity(const QString &a, const QString &b) {
//     if (a.isEmpty() || b.isEmpty()) return 0.0;
//     int common = 0;
//     QStringList wordsA = a.split(' ', Qt::SkipEmptyParts);
//     QStringList wordsB = b.split(' ', Qt::SkipEmptyParts);
//
//     for (const QString &wordA : wordsA) {
//         for (const QString &wordB : wordsB) {
//             if (wordA.contains(wordB, Qt::CaseInsensitive) || wordB.contains(wordA, Qt::CaseInsensitive)) {
//                 common++;
//                 break;
//             }
//         }
//     }
//
//     return static_cast<double>(common) / (wordsA.size() + wordsB.size() - common);
// }
void MainWindow::AddComponentLogic() {


    //完成元器件添加
    // _infoDockhArea->show();
    // _addComponentDockhArea->hide();
    // _infoDockWidget->setWindowTitle("元件信息");
}
// 计算两个字符串的Levenshtein距离
double MainWindow::calculateSimilarity(const QString &a, const QString &b) {
    if (a.isEmpty() || b.isEmpty()) return 0.0;

    int m = a.length();
    int n = b.length();

    // 创建dp数组
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

    // 初始化dp数组
    for (int i = 0; i <= m; ++i) dp[i][0] = i;
    for (int j = 0; j <= n; ++j) dp[0][j] = j;

    // 计算Levenshtein距离
    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i - 1][j] + 1,    // 删除
                                 dp[i][j - 1] + 1,    // 插入
                                 dp[i - 1][j - 1] + cost});  // 替换
        }
    }

    int distance = dp[m][n];
    int maxLength = std::max(m, n);

    // 防止越界
    if (maxLength == 0) return 1.0;

    // 计算相似度
    return 1.0 - static_cast<double>(distance) / maxLength;
}

// 检查所有词是否都在记录字段中出现
bool MainWindow::isExactMatch(const component_record_struct &record, const QStringList &searchWords) {
    int matchCount = 0;

    for (const auto &word : searchWords) {
        bool found = false;
        if (record.name.contains(word, Qt::CaseInsensitive) ||
            record.discription.contains(word, Qt::CaseInsensitive) ||
            record.package.contains(word, Qt::CaseInsensitive)) {
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
void MainWindow::findClosestRecords(const QVector<component_record_struct> &component_record, const QString &searchString) {
    auto start = std::chrono::high_resolution_clock::now();

    //清空记录
    model->exacIndex.clear();
    model->fuzzyIndex.clear();

    QMultiMap<double, int> similarityMap;
    QStringList searchWords = searchString.split(' ', Qt::SkipEmptyParts);

    int index = 0;
    for (const auto &record : component_record) {
        // 首先进行精确匹配检查
        if (isExactMatch(record, searchWords)) {
            model->exacIndex.append(index);
        } else {
            // 计算模糊相似度
            double totalSimilarity = 0.0;
            totalSimilarity += calculateSimilarity(record.name, searchString);
            totalSimilarity += calculateSimilarity(record.discription, searchString);
            totalSimilarity += calculateSimilarity(record.package, searchString);

            for (const auto &alias : record.aliases) {
                totalSimilarity += calculateSimilarity(alias, searchString);
            }

            // 将相似度存入Map中，键是相似度，值是记录
            if (totalSimilarity > 0.0) {
                similarityMap.insert(totalSimilarity, index);
            }
        }
        index++;
    }

    // 添加前5个最接近的模糊匹配结果
    auto it = similarityMap.end();
    int count = 0;

    while (it != similarityMap.begin() && count < 5) {
        --it;
        model->fuzzyIndex.append(it.value());
        count++;
    }

    // 你的代码
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "搜索用时: " << duration << " ms\n";
}

void MainWindow::search() {
    // QString searchString = ui_->input_search->text();
    QString searchString = _searchBox->text();
    if(searchString.isEmpty()) {
        model->showAll=true;
        model->updateData();
        // ShowAllComponents();
        return;
    }
    model->showAll=false;
    findClosestRecords(model->component_record, searchString);

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
    //显示搜索结果
    // ShowSomeComponents();
}

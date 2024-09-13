//
// Created by xtx on 24-9-13.
//
#include "mainwindow.h"

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
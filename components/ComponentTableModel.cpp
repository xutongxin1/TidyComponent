//
// Created by xtx on 24-10-5.
//

#include "ComponentTableModel.h"
bool isExactMatch(const component_record_struct &record, const QStringList &searchWords);
double calculateSimilarity(const QString &a, const QString &b);
QVector<component_record_struct *> ComponentTableModel::search_records_with_strategy(const QString &searchString) {
    // 使用局部变量存储搜索结果
    QVector<component_record_struct *> localExactPoint;
    QVector<component_record_struct *> localFuzzyPoint;
    QMultiMap<double, component_record_struct *> similarityMap;

    const QStringList searchWords = searchString.split(' ', Qt::SkipEmptyParts);

    // 遍历所有记录进行搜索
    for (int i = 0; i < component_record.size(); ++i) {
        auto& record = component_record[i];  // 获取引用
        if (isExactMatch(record, searchWords)) {
            localExactPoint.append(&record);
        } else {
            if (double totalSimilarity = calculateSimilarity(record.searchKey, searchString); totalSimilarity > 0.0) {
                similarityMap.insert(totalSimilarity, &record);
            }
        }
    }

    // 从similarityMap中提取模糊匹配结果
    auto it = similarityMap.end();
    int count = 0;
    while (it != similarityMap.begin() && count < 5) {
        --it;
        localFuzzyPoint.append(it.value());
        count++;
    }

    // 根据新策略组合结果
    QVector<component_record_struct *> result;

    if (localExactPoint.size() >= 5) {
        // 如果精确匹配结果>=5个，只返回前5个精确匹配结果
        for (int i = 0; i < 5 && i < localExactPoint.size(); ++i) {
            result.append(localExactPoint[i]);
        }
    } else {
        // 如果精确匹配结果<5个，先添加所有精确匹配结果
        result.append(localExactPoint);

        // 然后添加模糊匹配结果，使总数达到5个
        int remaining = 5 - localExactPoint.size();
        for (int i = 0; i < remaining && i < localFuzzyPoint.size(); ++i) {
            result.append(localFuzzyPoint[i]);
        }
    }

    return result;
}

void ComponentTableModel::updateDisplayItems() {
    displayItems.clear();
    if (showAll) {
        for (int i = 0; i < component_record.size(); ++i) {
            DisplayItem item;
            item.type = DisplayItem::Data;
            item.dataPoint = &component_record[i];
            displayItems.append(item);
        }
    } else {
        if (searchType == SEARCH_FUZZY) {
            if (exactPoint.isEmpty() && fuzzyPoint.isEmpty()) {
                // 未搜索到结果
                DisplayItem item;
                item.type = DisplayItem::Label;
                item.label = "未找到结果";
                displayItems.append(item);
            } else {
                if (!exactPoint.isEmpty()) {
                    // 添加“精确搜索结果：”标签
                    DisplayItem labelItem;
                    labelItem.type = DisplayItem::Label;
                    labelItem.label = "精确搜索结果：";
                    displayItems.append(labelItem);

                    // 添加精确搜索结果的数据项
                    for (component_record_struct *idx : exactPoint) {
                        DisplayItem dataItem;
                        dataItem.type = DisplayItem::Data;
                        dataItem.dataPoint = idx;
                        displayItems.append(dataItem);
                    }
                }

                if (!fuzzyPoint.isEmpty()) {
                    // 添加“模糊搜索结果：”标签
                    DisplayItem labelItem;
                    labelItem.type = DisplayItem::Label;
                    labelItem.label = "模糊搜索结果：";
                    displayItems.append(labelItem);

                    // 添加模糊搜索结果的数据项
                    for (component_record_struct *idx : fuzzyPoint) {
                        DisplayItem dataItem;
                        dataItem.type = DisplayItem::Data;
                        dataItem.dataPoint = idx;
                        displayItems.append(dataItem);
                    }
                }
            }
        } else if (searchType == SEARCH_EXACT) {
            if (exactPoint.isEmpty()) {
                if (noExitString.isEmpty()) {
                    // 未搜索到结果
                    DisplayItem item;
                    item.type = DisplayItem::Label;
                    item.label = "未识别到\n有效CID";
                    displayItems.append(item);
                }
            } else {
                // 添加“精确搜索结果：”标签
                DisplayItem labelItem;
                labelItem.type = DisplayItem::Label;
                labelItem.label = "CID\n精确搜索结果：";
                displayItems.append(labelItem);

                // 添加精确搜索结果的数据项
                for (component_record_struct *idx : exactPoint) {
                    DisplayItem dataItem;
                    dataItem.type = DisplayItem::Data;
                    dataItem.dataPoint = idx;
                    displayItems.append(dataItem);
                }
            }

            if (!noExitString.isEmpty()) {
                DisplayItem labelItem;
                labelItem.type = DisplayItem::Label;
                labelItem.label = "以下CID\n未找到：";
                displayItems.append(labelItem);
                for (const QString &cid : noExitString) {
                    DisplayItem dataItem;
                    dataItem.type = DisplayItem::Label;
                    dataItem.label = cid;
                    displayItems.append(dataItem);
                }
            }
        } else if (searchType == SEARCH_BOM) {
            if (exactPoint.isEmpty()) {
                if (noExitString.isEmpty()) {
                    // 未搜索到结果
                    DisplayItem item;
                    item.type = DisplayItem::Label;
                    item.label = "未识别到有效信息";
                    displayItems.append(item);
                }
            } else {
                // 添加“精确搜索结果：”标签
                DisplayItem labelItem;
                labelItem.type = DisplayItem::Label;
                labelItem.label = "以下器件在库：";
                displayItems.append(labelItem);

                // 添加精确搜索结果的数据项
                for (component_record_struct *idx : exactPoint) {
                    DisplayItem dataItem;
                    dataItem.type = DisplayItem::Data;
                    dataItem.dataPoint = idx;
                    displayItems.append(dataItem);
                }
            }

            if (!noExitString.isEmpty()) {
                DisplayItem labelItem;
                labelItem.type = DisplayItem::Label;
                labelItem.label = "以下器件未入库：";
                displayItems.append(labelItem);
                for (const QString &string : noExitString) {
                    DisplayItem dataItem;
                    dataItem.type = DisplayItem::Label;
                    dataItem.label = string;
                    displayItems.append(dataItem);
                    for (QVector<component_record_struct *> suggestion=search_records_with_strategy(string); component_record_struct *suggestionPtr : suggestion) {
                        DisplayItem suggestionItem;
                        suggestionItem.type = DisplayItem::Suggestion;
                        suggestionItem.suggestionPoint = suggestionPtr;
                        displayItems.append(suggestionItem);
                    }
                }
            }
        }
    }
}

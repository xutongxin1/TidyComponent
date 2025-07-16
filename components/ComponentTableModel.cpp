//
// Created by xtx on 24-10-5.
//

#include "ComponentTableModel.h"
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
                for (const QString &cid : noExitString) {
                    DisplayItem dataItem;
                    dataItem.type = DisplayItem::Label;
                    dataItem.label = cid;
                    displayItems.append(dataItem);
                }
            }
        }
    }
}

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
            item.dataIndex = i;
            displayItems.append(item);
        }
    } else {
        if (searchType == SEARCH_FUZZY) {
            if (exacIndex.isEmpty() && fuzzyIndex.isEmpty()) {
                // 未搜索到结果
                DisplayItem item;
                item.type = DisplayItem::Label;
                item.label = "未搜索到相关元器件";
                displayItems.append(item);
            } else {
                if (!exacIndex.isEmpty()) {
                    // 添加“精确搜索结果：”标签
                    DisplayItem labelItem;
                    labelItem.type = DisplayItem::Label;
                    labelItem.label = "精确搜索结果：";
                    displayItems.append(labelItem);

                    // 添加精确搜索结果的数据项
                    for (int idx : exacIndex) {
                        if (idx >= 0 && idx < component_record.size()) {
                            DisplayItem dataItem;
                            dataItem.type = DisplayItem::Data;
                            dataItem.dataIndex = idx;
                            displayItems.append(dataItem);
                        }
                    }
                }

                if (!fuzzyIndex.isEmpty()) {
                    // 添加“模糊搜索结果：”标签
                    DisplayItem labelItem;
                    labelItem.type = DisplayItem::Label;
                    labelItem.label = "模糊搜索结果：";
                    displayItems.append(labelItem);

                    // 添加模糊搜索结果的数据项
                    for (int idx : fuzzyIndex) {
                        if (idx >= 0 && idx < component_record.size()) {
                            DisplayItem dataItem;
                            dataItem.type = DisplayItem::Data;
                            dataItem.dataIndex = idx;
                            displayItems.append(dataItem);
                        }
                    }
                }
            }
        } else if (searchType == SEARCH_EXACT) {
            if (exacIndex.isEmpty()) {
                // 未搜索到结果
                DisplayItem item;
                item.type = DisplayItem::Label;
                item.label = "未搜索到相关元器件";
                displayItems.append(item);
            } else {
                // 添加“精确搜索结果：”标签
                DisplayItem labelItem;
                labelItem.type = DisplayItem::Label;
                labelItem.label = "精确搜索结果：";
                displayItems.append(labelItem);

                // 添加精确搜索结果的数据项
                for (int idx : exacIndex) {
                    if (idx >= 0 && idx < component_record.size()) {
                        DisplayItem dataItem;
                        dataItem.type = DisplayItem::Data;
                        dataItem.dataIndex = idx;
                        displayItems.append(dataItem);
                    }
                }
            }
        }
        else if (searchType == SEARCH_BOM) {
            if (BomIndex_Exist.isEmpty()&& BomIndex_NOTExist.isEmpty()) {
                // 未搜索到结果
                DisplayItem item;
                item.type = DisplayItem::Label;
                item.label = "未搜索到相关元器件";
                displayItems.append(item);
            } else {
                // 添加“精确搜索结果：”标签
                DisplayItem labelItem;
                labelItem.type = DisplayItem::Label;
                labelItem.label = "精确搜索结果：";
                displayItems.append(labelItem);

                // 添加精确搜索结果的数据项
                for (int idx : exacIndex) {
                    if (idx >= 0 && idx < component_record.size()) {
                        DisplayItem dataItem;
                        dataItem.type = DisplayItem::Data;
                        dataItem.dataIndex = idx;
                        displayItems.append(dataItem);
                    }
                }
            }
        }
    }
}

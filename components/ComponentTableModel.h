//
// Created by xtx on 24-10-5.
//

#ifndef COMPONENTTABLEMODEL_H
#define COMPONENTTABLEMODEL_H
#include <QAbstractTableModel>

#include "ColorDelegate.h"

struct component_record_struct {
    QString name;
    QString color;
    QString jlcid;
    QString tbLink;
    QString discription;
    QString package;
    QVector<QString> aliases;
};
const QStringList titles = {
    "显示状态", "名称", "描述", "封装", "立创编号", "别名1", "别名2", "别名3", "别名4",
    "别名5", "别名6", "别名7", "别名8", "别名9", "别名10"
};

class ComponentTableModel : public QAbstractTableModel {
        Q_OBJECT

    public:
        explicit ComponentTableModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {
        }
        ~ComponentTableModel() {
        }

        int rowCount(const QModelIndex &parent = QModelIndex()) const override {
            return displayItems.size();
        }

        int columnCount(const QModelIndex &/*parent*/ = QModelIndex()) const override {
            return titles.size();
        }

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
            if (!index.isValid() || index.column() >= columnCount() || index.row() >= rowCount())
                return QVariant();

            const DisplayItem &item = displayItems[index.row()];

            switch (role) {
                case Qt::DisplayRole:
                    if (item.type == DisplayItem::Label) {
                        // 如果是标签行，只在第一列显示标签
                        if (index.column() == 0)
                            return item.label;
                        else
                            return QVariant(); // 其他列为空
                    } else if (item.type == DisplayItem::Data) {
                        const component_record_struct &record = component_record[item.dataIndex];
                        switch (index.column()) {
                            case 0:
                                return record.color;
                            case 1:
                                return record.name;
                            case 2:
                                return record.discription;
                            case 3:
                                return record.package;
                            case 4:
                                return record.jlcid;
                            default:
                                if (index.column() >= 5 && index.column() <= 14) {
                                    int aliasIndex = index.column() - 5;
                                    if (aliasIndex < record.aliases.size())
                                        return record.aliases[aliasIndex];
                                    else
                                        return QVariant();
                                } else {
                                    return QVariant();
                                }
                        }
                    } else {
                        return QVariant();
                    }
                    break;

                case Qt::TextAlignmentRole:
                    // 对齐方式
                    return Qt::AlignCenter;
                    break;
                // case Qt::DecorationRole:
                //     if (index.column() == 0) {
                //         QColor color(component_record[item.dataIndex].color);
                //         if (color.isValid()) {
                //             QPixmap pixmap(16, 16);
                //             pixmap.fill(color);
                //             return pixmap;
                //         } else {
                //             return QVariant();
                //         }
                //     }
                case Qt::BackgroundRole:
                    if (index.column() == 0) {
                        if (item.type == DisplayItem::Data)
                            return QBrush(QColor(component_record[item.dataIndex].color));
                        else
                            return QVariant();
                    } else {
                        return QVariant();
                    }
                default:
                    return QVariant();
            }
        }

        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const {
            //        水平表头显示信息
            switch (role) {
                case Qt::DisplayRole:
                    if (orientation == Qt::Horizontal && section >= 0 && section <= columnCount())
                        return titles.at(section);
                    break;
                default:
                    break;
            }

            return QAbstractItemModel::headerData(section, orientation, role);
        }

        Qt::ItemFlags flags(const QModelIndex &index) const override {
            if (!index.isValid())
                return Qt::NoItemFlags;

            Qt::ItemFlags flags = QAbstractTableModel::flags(index);

            // 设置所有单元格为可编辑
            flags |= Qt::ItemIsEditable;

            return flags;
        }

        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
            if (value.toString().isEmpty()) {
                // 数据不能为空，返回 false 表示修改失败
                return false;
            }

            if (!index.isValid() || index.column() >= columnCount() || index.row() >= rowCount())
                return false;

            if (role == Qt::EditRole) {
                switch (index.column()) {
                    case 0:
                        component_record[index.row()].color = value.toString();
                        break;
                    case 1:
                        component_record[index.row()].name = value.toString();
                        break;
                    case 2:
                        component_record[index.row()].discription = value.toString();
                        break;
                    case 3:
                        component_record[index.row()].package = value.toString();
                        break;
                    case 4:
                        component_record[index.row()].jlcid = value.toString();
                        break;
                    default:
                        if (index.column() >= 5 && index.column() <= 14) {
                            int aliasIndex = index.column() - 5;
                            // Ensure the aliases vector is large enough
                            if (aliasIndex >= component_record[index.row()].aliases.size()) {
                                component_record[index.row()].aliases.resize(aliasIndex + 1);
                            }
                            component_record[index.row()].aliases[aliasIndex] = value.toString();
                        } else {
                            return false;
                        }
                        break;
                }

                // Notify the view that the data has changed
                emit dataChanged(index, index, {role});

                // Indicate that the data was successfully set
                return true;
            }

            // For other roles, return false
            return false;
        }

    private:
        //    行修改函数：添加多行和删除多行
        virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override {
            //        起始行row超限时，修正到两端插入
            if (row > rowCount()) row = rowCount();
            if (row < 0) row = 0;

            //        需要将修改部分的代码使用begin和end函数包起来
            beginInsertRows(parent, row, row + count - 1);

            //        添加数据
            for (int i = 0; i < count; ++i)
                component_record.insert(component_record.begin() + row + i,
                                        component_record_struct());

            endInsertRows();

            emit dataChanged(createIndex(row, 0), createIndex(row + count - 1, columnCount() - 1));
            return true;
        }

        virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) {
            if (row < 0 || row >= rowCount() || row + count > rowCount()) return false;

            //        需要将修改部分的代码使用begin和end函数包起来
            beginRemoveRows(parent, row, row + count - 1);

            //        删除数据
            for (int i = 0; i < count; ++i) {
                component_record.remove(row);
            }

            endRemoveRows();

            return true;
        }

        // 更新显示项目列表
        void updateDisplayItems() {
            displayItems.clear();
            if (showAll) {
                for (int i = 0; i < component_record.size(); ++i) {
                    DisplayItem item;
                    item.type = DisplayItem::Data;
                    item.dataIndex = i;
                    displayItems.append(item);
                }
            } else {
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
            }
        }

    public:
        // 布尔变量，决定显示全部还是部分内容
        bool showAll;
        // 存储精确和模糊搜索的索引
        QVector<int> exacIndex;
        QVector<int> fuzzyIndex;
        QVector<component_record_struct> component_record;
        struct DisplayItem {
            enum Type { Label, Data } type;
            QString label; // 对于 Label 类型
            int dataIndex; // 对于 Data 类型，对应 component_records 中的索引
        };
        // 存储要显示的项目列表
        QVector<DisplayItem> displayItems;
        void updateData() {
            auto start = std::chrono::high_resolution_clock::now();
            beginResetModel();
            updateDisplayItems();
            endResetModel();
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            qDebug() << "渲染用时: " << duration << " ms\n";
        }
};

#endif //COMPONENTTABLEMODEL_H

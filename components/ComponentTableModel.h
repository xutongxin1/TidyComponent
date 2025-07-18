//
// Created by xtx on 24-10-5.
//

#ifndef COMPONENTTABLEMODEL_H
#define COMPONENTTABLEMODEL_H
#include <QAbstractTableModel>

#include "ColorDelegate.h"
enum ComponentState {
    ComponentState_Ready = 1, // 就绪
    ComponentState_APPLYOUT, // 申请出库
    ComponentState_OUT, // 已出库
    ComponentState_APPLYIN, // 申请归还
};

enum DeviceType {
    DeviceType_A42 = 1,
    DeviceType_A21,
    DeviceType_B53
};
struct component_record_struct {
    //以下为展示的部分
    QString name = QString(); //元器件名称
    QString jlcid = QString(); //嘉立创CID
    QString discription = QString(); //元器件描述
    QString more_data = QString(); //元器件参数
    QString package = QString(); //元器件封装
    QVector<QString> aliases = QVector<QString>(5, QString()); //元器件别名
    bool isAlias = false; //是否有别名
    QVector<QString> png_FileUrl = {}; //元器件实物图文件路径
    QVector<QString> sch_svg_FileUrl = {}; //元器件sch_svg文件路径
    QVector<QString> pcb_svg_FileUrl = {}; //元器件pcb_svg文件路径
    QString pdf_name = QString(); //元器件pdf文件名称
    QString pdf_FileUrl = QString(); //元器件pdf文件路径
    QString price = QString(); //元器件价格
    QString inventory = QString(); //元器件库存
    QString PID = QString(); //元器件库存

    //以下为逻辑部分
    QString pdf_url = QString();
    QString searchKey = QString();
    QString color = QString("就绪");
    ComponentState isApply = ComponentState_Ready; //是否被申请
    // int applyType
    float weight = 0.0f; //器件重量
    QString MAC = QString(); //对应mesh地址
    int coordinate = 0; //对应内部地址
    DeviceType device_type = DeviceType_B53; //设备类型
};
const QStringList titles = {
    "显示状态", "名称", "描述", "封装", "立创编号", "商品参数"
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
            if (!index.isValid() || index.column() >= columnCount() || index.row() >= rowCount()||index.row()<0)
                return QVariant();

            const DisplayItem &item = displayItems[index.row()];
            QString text;
            switch (role) {
                case Qt::DisplayRole:
                    if (item.type == DisplayItem::Label) {
                        // 如果是标签行，只在第一列显示标签
                        if (index.column() == 0)
                            return item.label;
                        else
                            return QVariant(); // 其他列为空
                    } else if (item.type == DisplayItem::Data || item.type == DisplayItem::Suggestion) {
                        component_record_struct *record=nullptr;
                        if (item.type == DisplayItem::Data)
                        {
                            record = item.dataPoint;
                        }
                        else {
                            record = item.suggestionPoint;
                        }
                        switch (index.column()) {
                            case 0:
                                if (record->isApply == ComponentState_Ready) {
                                    return "就绪";
                                } else if (record->isApply == ComponentState_APPLYOUT) {
                                    return "正在申请出库";
                                } else if (record->isApply == ComponentState_OUT) {
                                    return "已取出";
                                } else if (record->isApply == ComponentState_APPLYIN) {
                                    return "正在申请归还";
                                }
                            case 1:
                                return record->name;
                            case 2:
                                text = record->discription;
                                if (record->isAlias) {
                                    text += " (";
                                    for (const auto &aliase : record->aliases) {
                                        if (!aliase.isEmpty()) {
                                            text += aliase;
                                            text += "，";
                                        }
                                    }
                                    text.removeLast();
                                    text += ")";
                                }
                                return text;
                            case 3:
                                return record->package;
                            case 4:
                                return record->jlcid;
                            case 5:
                                return record->more_data;
                            default:
                                return QVariant();
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
                    if (item.type == DisplayItem::Data) {
                        if (index.column() == 0) {
                            if (item.dataPoint->color == "就绪" || item.dataPoint->color
                                == "已取出") {
                                return QVariant();
                            } else {
                                return QBrush(QColor(item.dataPoint->color));
                            }
                        } else { return QVariant(); }
                    } else if (item.type == DisplayItem::Suggestion){
                        return QBrush(QColor("Gray"));
                    }else {
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

            // Qt::ItemFlags flags = QAbstractTableModel::flags(index);
            //
            // // 设置所有单元格为可编辑
            // flags |= Qt::ItemIsEditable;
            //
            // return flags;

            // 只允许第一列(列索引为0)可编辑
            if (index.column() == 0) {
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
            }

            // 其他列只能选择，不能编辑
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
            if (value.toString().isEmpty()) {
                // 数据不能为空，返回 false 表示修改失败
                return false;
            }

            if (!index.isValid() || index.column() >= columnCount() || index.row() >= rowCount())
                return false;

            if (index.column() == 0) {
                // 只允许修改第一列的颜色
                if (role == Qt::EditRole) {
                    QString newColor = value.toString();
                    if (newColor.isEmpty()) {
                        return false; // 颜色不能为空
                    }
                    component_record[index.row()].color = newColor;
                    emit dataChanged(index, index, {role}); // 通知视图数据已更改
                    return true;
                }
            } else {
                // 其他列不允许编辑
                return false;
            }
            return false;
        }
        // 更新某一列数据的方法（带角色参数）
        void updateColumnWithRoles(int column, const QVector<int> &roles = {Qt::DisplayRole}) {
            // 更新你的数据...
            QModelIndex topLeft = index(0, column);
            QModelIndex bottomRight = index(rowCount() - 1, column);

            // 指定更新的角色（如DisplayRole, BackgroundRole等）
            emit dataChanged(topLeft, bottomRight, roles);
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

        QVector<component_record_struct *> search_records_with_strategy(const QString &searchString);
        // 更新显示项目列表
        void updateDisplayItems();

        void onRowClicked(const QModelIndex &index) {
            if (index.column() == 4) {
                // Check if the clicked column is CID column (5th column)
                const auto cid = index.data(Qt::DisplayRole).toString();
                emit(RowClickedWithCID(cid));
            }
        }

    public:
        // 布尔变量，决定显示全部还是部分内容
        bool showAll;
        enum SEARCH_TYPE {
            SEARCH_FUZZY = 1,
            SEARCH_EXACT,
            SEARCH_BOM
        };
        SEARCH_TYPE searchType = SEARCH_FUZZY;
        // 存储精确和模糊搜索的索引
        QVector<component_record_struct *> exactPoint;
        QVector<component_record_struct *> fuzzyPoint;
        QStringList noExitString;
        // QVector<int> BomIndex_NOTExist;
        // QVector<int> BomIndex_Exist;
        QVector<component_record_struct> component_record;
        QHash<QString, component_record_struct *> component_record_Hash_cid;
        QHash<QString, component_record_struct *> component_record_Hash_MACD;
        struct DisplayItem {
            enum Type { Label, Data, Suggestion } type = Data;
            QString label = QString(); // 对于 Label 类型
            component_record_struct *dataPoint = nullptr; // 对于 Data 类型，对应 component_records 中的索引
            component_record_struct *suggestionPoint = nullptr; // 对于 Suggestion 类型，对应 component_records 中的索引
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
        void updateDataWithNoRowChange() {
            auto start = std::chrono::high_resolution_clock::now();

            // 更新数据但不重置模型
            updateDisplayItems();

            // 发送数据改变信号
            if (rowCount() > 0 && columnCount() > 0) {
                emit dataChanged(index(0, 0),
                                 index(rowCount() - 1, columnCount() - 1));
            }

            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            qDebug() << "渲染用时: " << duration << " ms\n";
        }
        Q_SIGNAL
        void RowClickedWithCID(QString CID);
};

#endif //COMPONENTTABLEMODEL_H

//
// Created by xtx on 24-11-18.
//

#ifndef SHOWINFOMODEL_H
#define SHOWINFOMODEL_H
#include "structH.h"
#include <QAbstractTableModel>
#include <QVariant>

#include "ComponentTableModel.h"
#include "mainwindow.h"
class ShowInfoModel : public QAbstractTableModel {
        Q_OBJECT

    public:
        // 构造函数，接收数据（CID对应的元器件结构体）
        explicit ShowInfoModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {
        }

        void setComponentData(component_record_struct *record);

        [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override; // NOLINT(*-default-arguments)

        [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override; // NOLINT(*-default-arguments)

        [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

        [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

        bool setData(const QModelIndex &index, const QVariant &value, int role);
        bool m_editModeEnabled = false;
        component_record_struct *m_record = nullptr;
};

inline void ShowInfoModel::setComponentData(component_record_struct *record) {
    // const auto start = std::chrono::high_resolution_clock::now();
    beginResetModel();
    m_record = record;
    endResetModel();
    // auto stop = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    // qDebug() << "渲染用时: " << duration << " ms\n";
}
inline int ShowInfoModel::rowCount(const QModelIndex &parent) const {
    if (m_record == nullptr)
        return 0;
    return static_cast<int>(m_record->aliases.size()) + 7 + 2; // 5 fields + aliases rows+2 info
}
inline int ShowInfoModel::columnCount(const QModelIndex &parent) const {
    return 2; // Field, Value
}
inline QVariant ShowInfoModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() == 0) {
            switch (index.row()) {
                case 0:
                    return "器件名称";
                case 1:
                    return "嘉立创CID";
                case 2:
                    return "描述";
                case 3:
                    return "商品参数";
                case 4:
                    return "封装";
                case 5:
                    return "立创参考价";
                case 6:
                    return "立创库存";
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                    return "别名" + QString::number(index.row() - 6);
                case 12:
                    return "MAC地址";
                case 13:
                    return "存储位置";
                default:
                    return QVariant();
            }
        } else {
            switch (index.row()) {
                case 0:
                    return m_record->name; // Name
                case 1:
                    return m_record->jlcid; // CID
                case 2:
                    return m_record->discription; // Description
                case 3:
                    return m_record->more_data; // More Data
                case 4:
                    return m_record->package; // Package
                case 5:
                    return m_record->price; // price
                case 6:
                    return m_record->inventory; // inventory
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                    return m_record->aliases[index.row() - 7]; // Aliases start from row 5
                case 12:
                    return m_record->MAC;
                case 13:
                    return m_record->coordinate;
                default:
                    return QVariant();
            }
        }
    }
    return QVariant();
}
inline Qt::ItemFlags ShowInfoModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    // 只有在编辑模式开启且是指定行时才可编辑
    if (index.row() >= 7 && index.row() < 12) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

// 重写setData方法，处理编辑后的数据
inline bool ShowInfoModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole || !index.isValid()) {
        return false;
    }

    m_record->aliases[index.row() - 7] = value.toString();

    //更新搜索索引
    m_record->searchKey = m_record->name + m_record->jlcid + m_record->
        discription + m_record->package + m_record->more_data;
    bool isAlias = false;
    for (const auto &alias : m_record->aliases) {
        if (!alias.isEmpty()) {
            isAlias = true;
            m_record->searchKey += alias;
        }
    }
    m_record->isAlias = isAlias;

    emit dataChanged(index, index, {role});
    return true;
}

#endif //SHOWINFOMODEL_H

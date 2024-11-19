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

        void setComponentData(const component_record_struct &record);

        [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override; // NOLINT(*-default-arguments)

        [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override; // NOLINT(*-default-arguments)

        [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

        [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

    private:
        component_record_struct m_record;
};

inline void ShowInfoModel::setComponentData(const component_record_struct &record) {
    // const auto start = std::chrono::high_resolution_clock::now();
    beginResetModel();
    m_record = record;
    endResetModel();
    // auto stop = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    // qDebug() << "渲染用时: " << duration << " ms\n";
}
inline int ShowInfoModel::rowCount(const QModelIndex &parent) const {
    return static_cast<int>(m_record.aliases.size()) + 5; // 5 fields + aliases rows
}
inline int ShowInfoModel::columnCount(const QModelIndex &parent) const {
    return 2; // Field, Value
}
inline QVariant ShowInfoModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
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
                default:
                    if (index.row() < 10)
                        return "别名" + QString::number(index.row() - 4);
                    else
                        return QVariant();
            }
        } else {
            switch (index.row()) {
                case 0:
                    return m_record.name; // Name
                case 1:
                    return m_record.jlcid; // CID
                case 2:
                    return m_record.discription; // Description
                case 3:
                    return m_record.more_data; // More Data
                case 4:
                    return m_record.package; // Package
                default:
                    if (index.row() < 10)
                        // For aliases
                        return m_record.aliases[index.row() - 5]; // Aliases start from row 5
                    else
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
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled; // Cells are selectable but not editable
}


#endif //SHOWINFOMODEL_H

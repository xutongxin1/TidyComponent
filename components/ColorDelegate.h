//
// Created by xtx on 24-10-7.
//

#ifndef COLORDELEGATE_H
#define COLORDELEGATE_H
#include <QColorDialog>
#include <QStyledItemDelegate>

#include "ElaColorDialog.h"

class ColorDelegate : public QStyledItemDelegate {
        Q_OBJECT

    public:
        explicit ColorDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {

        }
        // 创建编辑器（颜色选择器）
        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override {
            if (index.column() == 0) {
                // 检查单元格文本，如果是"就绪"则不创建颜色选择器
                QString cellText = index.model()->data(index, Qt::DisplayRole).toString();
                if (cellText == "就绪") {
                    return nullptr; // 不创建编辑器
                }
                // QString CID = index.model()->data(index.sibling(index.row(), index.column() + 4), Qt::DisplayRole).
                //     toString();
                // 使用 ElaColorDialog 替代 QColorDialog
                ElaColorDialog * editor = new ElaColorDialog();
                QColor color(cellText);
                if (color.isValid()) {
                    editor->setCurrentColor(color);
                }
                // connect(editor, &ElaColorDialog::colorSelected, this, [&,index](const QColor &newColor) {
                //     // 当颜色变化时，发出信号
                //     QAbstractItemModel *model = const_cast<QAbstractItemModel *>(index.model());
                //     model->setData(index, newColor.name(), Qt::EditRole);
                // });
                return editor;
            } else {
                return QStyledItemDelegate::createEditor(parent, option, index);
            }
        }

        // // 设置编辑器数据（当前颜色）
        // void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        //     if (index.column() == 0) {
        //         ElaColorDialog *colorDialog = qobject_cast<ElaColorDialog *>(editor);
        //         if (colorDialog) {
        //             QString colorName = index.model()->data(index, Qt::EditRole).toString();
        //             QColor color(colorName);
        //             if (color.isValid()) {
        //                 colorDialog->setCurrentColor(color);
        //             }
        //         }
        //     } else {
        //         QStyledItemDelegate::setEditorData(editor, index);
        //     }
        // }

        void setModelData(QWidget *editor, QAbstractItemModel *model,
                          const QModelIndex &index) const override {
            if (index.column() == 0) {
                ElaColorDialog *colorDialog = qobject_cast<ElaColorDialog *>(editor);
                if (colorDialog) {
                    QColor color = colorDialog->getCurrentColor();
                    if (color.isValid()) {
                        // // 获取原来的颜色进行比较
                        // QString oldColorName = model->data(index, Qt::EditRole).toString();
                        // QColor oldColor(oldColorName);

                        // 设置新颜色到模型
                        model->setData(index, color.name(), Qt::EditRole);

                        // // 如果颜色发生了变化，发出信号
                        // if (oldColor != color) {
                        //     emit colorChanged(index, color);
                        // }
                    }
                }
            } else {
                QStyledItemDelegate::setModelData(editor, model, index);
            }
        }
};

#endif //COLORDELEGATE_H

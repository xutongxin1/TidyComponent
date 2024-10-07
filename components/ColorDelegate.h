//
// Created by xtx on 24-10-7.
//

#ifndef COLORDELEGATE_H
#define COLORDELEGATE_H
#include <QColorDialog>
#include <QStyledItemDelegate>

// 创建自定义委托类，继承自 QStyledItemDelegate
class ColorDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ColorDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    // 创建编辑器（颜色选择器）
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override {
        if (index.column() == 0) {
            QColorDialog *editor = new QColorDialog(parent);
            editor->setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
            return editor;
        } else {
            return QStyledItemDelegate::createEditor(parent, option, index);
        }
    }

    // 设置编辑器数据（当前颜色）
    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        if (index.column() == 0) {
            QColorDialog *colorDialog = qobject_cast<QColorDialog *>(editor);
            QString colorName = index.model()->data(index, Qt::EditRole).toString();
            QColor color(colorName);
            if (color.isValid()) {
                colorDialog->setCurrentColor(color);
            }
        } else {
            QStyledItemDelegate::setEditorData(editor, index);
        }
    }

    // 将编辑器中的数据设置回模型
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override {
        if (index.column() == 0) {
            QColorDialog *colorDialog = qobject_cast<QColorDialog *>(editor);
            QColor color = colorDialog->currentColor();
            if (color.isValid()) {
                model->setData(index, color.name(), Qt::EditRole);
                // QLinearGradient  back(0,0,100,30);
                // back.setColorAt(0,Qt::white);
                // back.setColorAt(0.5,Qt::green);
                // back.setColorAt(1,Qt::blue);
                // model->setData(index, QBrush(back), Qt::BackgroundRole);
            }
        } else {
            QStyledItemDelegate::setModelData(editor, model, index);
        }
    }

    // // **重写 paint 方法，确保背景颜色被正确绘制**
    // void paint(QPainter *painter, const QStyleOptionViewItem &option,
    //            const QModelIndex &index) const override {
    //     QStyleOptionViewItem opt = option;
    //     initStyleOption(&opt, index);
    //
    //     // 检查模型是否提供了背景颜色
    //     QVariant background = index.data(Qt::BackgroundRole);
    //     if (background.canConvert<QBrush>()) {
    //         opt.backgroundBrush = qvariant_cast<QBrush>(background);
    //     }
    //
    //     // 调用基类的 paint 方法进行绘制
    //     QStyledItemDelegate::paint(painter, opt, index);
    // }
};






#endif //COLORDELEGATE_H

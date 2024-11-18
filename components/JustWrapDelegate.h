//
// Created by xtx on 24-11-17.
//

#ifndef WRAPDELEGATE_H
#define WRAPDELEGATE_H
#include <QPainter>
#include<QStyledItemDelegate>
class JustWrapDelegate final : public QStyledItemDelegate {
        Q_OBJECT

    public:
        explicit JustWrapDelegate(QObject *parent) : QStyledItemDelegate(parent) {
        }

        ~JustWrapDelegate() override = default;

        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override {
            const auto text = index.model()->data(index, Qt::DisplayRole).toString();
            //注意，这里必须用TextWrapAnywhere!  TextWordWrap无效
            painter->drawText(option.rect, Qt::TextWrapAnywhere | Qt::AlignVCenter, text);
        }
};

#endif //WRAPDELEGATE_H

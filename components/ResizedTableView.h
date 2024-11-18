//
// Created by xtx on 24-11-17.
//

#ifndef RESIZEDTABLEVIEW_H
#define RESIZEDTABLEVIEW_H
#include <QHeaderView>
#include "ElaTableView.h"

class ResizedTableView : public ElaTableView {
        Q_OBJECT

    public:
        explicit ResizedTableView(QWidget *parent = nullptr) : ElaTableView(parent) {
            connect(horizontalHeader(), &QHeaderView::sectionResized, this, [&] {
                verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
                resizeRowToContents(0);
            });
        }
        ~ResizedTableView() override = default;
};

#endif //RESIZEDTABLEVIEW_H

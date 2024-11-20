//
// Created by xtx on 24-11-19.
//
#include "ShowInfoModel.h"
void MainWindow::updateContent(const QItemSelection &selected, const QItemSelection &deselected) const {
    // qDebug() << selected.indexes();
    QModelIndexList selectedIndexes = selected.indexes();
    if (selectedIndexes.isEmpty()) {
        //没有选中任何行
        _showInfo_tableView->hide();
        _showInfo_PNGView->hide();
        _showInfo_SCHPCBview->hide();
        _showInfo_OpenPDFButton->hide();
        _showInfo_NoComponentTips->show();
        return;
    }
    if (selectedIndexes[0].isValid()) {
        if (const QString cid = selectedIndexes[0].sibling(selectedIndexes[0].row(), 4).data(Qt::DisplayRole).toString()
            ; model->
            component_record_Hash.contains(cid)) {
            const component_record_struct record = model->component_record_Hash.value(cid);

            _showInfo_model->setComponentData(record);

            // // Set image cards (ElaPromotionView)
            if (record.png_FileUrl.empty()) {
                _showInfo_PNGView->hide();
            } else {
                _showInfo_PNGView->clearPromotionCard();
                for (int j = 0; j < record.png_FileUrl.size(); ++j) {
                    switch (j) {
                        case 0:
                            AddCardToShow(_showInfo_PNGView, _showInfo_PNGCard1, record.png_FileUrl[j]);
                            break;
                        case 1:
                            AddCardToShow(_showInfo_PNGView, _showInfo_PNGCard2, record.png_FileUrl[j]);
                            break;
                        case 2:
                            AddCardToShow(_showInfo_PNGView, _showInfo_PNGCard3, record.png_FileUrl[j]);
                            break;
                        case 3:
                            AddCardToShow(_showInfo_PNGView, _showInfo_PNGCard4, record.png_FileUrl[j]);
                            break;
                        case 4:
                            AddCardToShow(_showInfo_PNGView, _showInfo_PNGCard5, record.png_FileUrl[j]);
                            break;

                        default: ;
                    }
                }
            }
            // for (int j = 0; j < record.sch_svg_FileUrl.size(); ++j) {
            //     _showInfo_PNGCard2->setCardPixmap(QPixmap(record.sch_svg_FileUrl[j]));
            // }
            // for (int j = 0; j < record.pcb_svg_FileUrl.size(); ++j) {
            //     _showInfo_PNGCard3->setCardPixmap(QPixmap(record.pcb_svg_FileUrl[j]));
            // }

            //显示信息
            _showInfo_tableView->show();
            _showInfo_PNGView->show();
            _showInfo_SCHPCBview->show();
            _showInfo_OpenPDFButton->show();
            _showInfo_NoComponentTips->hide();

            //要先显示表格才能根据内容调整高度
            _showInfo_tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
            int totalHeight = 2; //疑似是border的宽度
            for (int row = 0; row < _showInfo_tableView->model()->rowCount(); row++) {
                totalHeight += _showInfo_tableView->rowHeight(row);
            }

            // 设置 QTableView 的高度为内容的高度
            _showInfo_tableView->setFixedHeight(totalHeight);
        }
    }
}
void MainWindow::AddCardToShow(ElaPromotionView *view, ElaPromotionCard *card, const QString &fileURL) {
    card->setCardPixmap(QPixmap(fileURL));
    view->appendPromotionCard(card);
}

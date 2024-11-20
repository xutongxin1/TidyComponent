//
// Created by xtx on 24-11-19.
//
#include <QSvgRenderer>

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

            //设置png
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

            //设置sch和pcb
            if (record.sch_svg_FileUrl.empty()) {
                _showInfo_PNGView->hide();
            } else {
                int sch_pcb_count = 0;
                _showInfo_SCHPCBview->clearPromotionCard();
                for (const auto &j : record.sch_svg_FileUrl) {
                    switch (sch_pcb_count) {
                        case 0:
                            AddCardToShow(_showInfo_SCHPCBview, _showInfo_SCHCard1, j, true);
                            break;
                        case 1:
                            AddCardToShow(_showInfo_SCHPCBview, _showInfo_SCHCard2, j, true);
                            break;
                        case 2:
                            AddCardToShow(_showInfo_SCHPCBview, _showInfo_SCHCard3, j, true);
                            break;
                        case 3:
                            AddCardToShow(_showInfo_SCHPCBview, _showInfo_SCHCard4, j, true);
                            break;
                        case 4:
                            AddCardToShow(_showInfo_SCHPCBview, _showInfo_SCHCard5, j, true);
                            break;
                        default: ;
                    }
                    sch_pcb_count++;
                }
                sch_pcb_count = 0;
                for (const auto &j : record.pcb_svg_FileUrl) {
                    switch (sch_pcb_count) {
                        case 0:
                            AddCardToShow(_showInfo_SCHPCBview, _showInfo_PCBCard1, j, true);
                            break;
                        case 1:
                            AddCardToShow(_showInfo_SCHPCBview, _showInfo_PCBCard2, j, true);
                            break;
                        case 2:
                            AddCardToShow(_showInfo_SCHPCBview, _showInfo_PCBCard3, j, true);
                            break;
                        case 3:
                            AddCardToShow(_showInfo_SCHPCBview, _showInfo_PCBCard4, j, true);
                            break;
                        case 4:
                            AddCardToShow(_showInfo_SCHPCBview, _showInfo_PCBCard5, j, true);
                            break;
                        default: ;
                    }
                    sch_pcb_count++;
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
QPixmap renderSvgToPixmap(const QString &svgFilePath, const QSize &targetSize, int xOffset = 0, int yOffset = 0) {
    // 创建QSvgRenderer来解析SVG
    QSvgRenderer svgRenderer(svgFilePath);

    // 获取SVG的原始尺寸
    QSize svgSize = svgRenderer.defaultSize();

    // 计算目标尺寸，使其按原始比例缩放
    QSize scaledSize = targetSize;
    float aspectRatio = static_cast<float>(svgSize.width()) / svgSize.height();
    float targetAspectRatio = static_cast<float>(targetSize.width()) / targetSize.height();

    if (aspectRatio > targetAspectRatio) {
        // 宽度优先缩放
        scaledSize.setWidth(targetSize.width());
        scaledSize.setHeight(targetSize.width() / aspectRatio);
    } else {
        // 高度优先缩放
        scaledSize.setHeight(targetSize.height());
        scaledSize.setWidth(targetSize.height() * aspectRatio);
    }

    // 创建白色背景的QPixmap
    QPixmap pixmap(targetSize);
    pixmap.fill(Qt::white); // 填充白色背景

    // 使用QPainter将SVG渲染到QPixmap上
    QPainter painter(&pixmap);
    // 计算偏移量，使图像居中或者手动调整
    int centerX = (targetSize.width() - scaledSize.width()) / 2 + xOffset;
    int centerY = (targetSize.height() - scaledSize.height()) / 2 + yOffset;

    // 使用计算的偏移量来调整图像的位置
    painter.translate(centerX, centerY);
    svgRenderer.render(&painter);
    painter.end();

    return pixmap;
}
QPixmap renderSvgToPixmap(const QString &svgFilePath, const QSize &targetSize) {
    // 创建QSvgRenderer来解析SVG
    QSvgRenderer svgRenderer(svgFilePath);

    // 获取SVG的原始尺寸
    QSize svgSize = svgRenderer.defaultSize();

    // 计算目标尺寸，使其按原始比例缩放
    QSize scaledSize = targetSize;
    float aspectRatio = static_cast<float>(svgSize.width()) / svgSize.height();
    float targetAspectRatio = static_cast<float>(targetSize.width()) / targetSize.height();

    if (aspectRatio > targetAspectRatio) {
        // 宽度优先缩放
        scaledSize.setWidth(targetSize.width());
        scaledSize.setHeight(targetSize.width() / aspectRatio);
    } else {
        // 高度优先缩放
        scaledSize.setHeight(targetSize.height());
        scaledSize.setWidth(targetSize.height() * aspectRatio);
    }

    // 创建白色背景的QPixmap
    QPixmap pixmap(targetSize);
    pixmap.fill(Qt::white);  // 填充白色背景

    // 使用QPainter将SVG渲染到QPixmap上
    QPainter painter(&pixmap);
    // 计算绘制时的位置，使得图像居中
    int xOffset = (targetSize.width() - scaledSize.width()) / 2;
    int yOffset = (targetSize.height() - scaledSize.height()) / 2;
    painter.translate(xOffset, yOffset);
    svgRenderer.render(&painter);
    painter.end();

    return pixmap;
}
void MainWindow::AddCardToShow(ElaPromotionView *view, ElaPromotionCard *card, const QString &fileURL, bool isSVG) {
    if (isSVG) {
        card->setCardPixmap(renderSvgToPixmap(fileURL, QSize(1600, 1600), -100, -400));
    } else {
        card->setCardPixmap(QPixmap(fileURL));
    }

    view->appendPromotionCard(card);
}

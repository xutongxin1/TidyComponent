#include <QSvgRenderer>

#include "ElaScrollArea.h"
#include "GetFileRequestHandler.h"
#include "GetRequestHandler.h"
#include "mainwindow.h"
//
// Created by xtx on 24-11-14.
//
QString GetCIDPath(const QString &CID) {
    return INFOPATH + "/" + CID;
}

QByteArray svgToPng(const QString &svgContent, int width = 0, int height = 0) {
    // 使用SVG内容创建渲染器
    QSvgRenderer renderer(svgContent.toUtf8());

    // 获取默认尺寸或使用指定尺寸
    QSize size = renderer.defaultSize();
    if (width > 0 && height > 0) {
        size = QSize(width, height);
    } else if (width > 0) {
        double aspectRatio = static_cast<double>(size.height()) / size.width();
        size = QSize(width, static_cast<int>(width * aspectRatio));
    } else if (height > 0) {
        double aspectRatio = static_cast<double>(size.width()) / size.height();
        size = QSize(static_cast<int>(height * aspectRatio), height);
    }

    // 创建QImage用于渲染
    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    // 渲染SVG
    QPainter painter(&image);
    renderer.render(&painter);
    painter.end();

    // 转换为PNG
    QByteArray pngData;
    QBuffer buffer(&pngData);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    return pngData;
}

QPixmap resizeImage(const QString &filename, int width = -1, int height = -1) {
    // 加载图片
    QPixmap originalPixmap(filename);

    // 如果图片加载失败
    if (originalPixmap.isNull()) {
        qDebug() << "Failed to load image:" << filename;
        return QPixmap(); // 返回空的 QPixmap
    }

    // 根据输入宽度或高度调整图片
    if (width > 0 && height == -1) {
        // 以宽度为固定值，计算等比例缩放后的高度
        int newHeight = static_cast<int>(originalPixmap.height() * (width / static_cast<float>(originalPixmap.
            width())));
        return originalPixmap.scaled(width, newHeight, Qt::KeepAspectRatio);
    } else if (height > 0 && width == -1) {
        // 以高度为固定值，计算等比例缩放后的宽度
        int newWidth = static_cast<int>(originalPixmap.width() * (height / static_cast<float>(originalPixmap.
            height())));
        return originalPixmap.scaled(newWidth, height, Qt::KeepAspectRatio);
    } else {
        // 如果两个宽度和高度都传入了，直接裁剪到给定大小
        return originalPixmap.scaled(width, height, Qt::IgnoreAspectRatio); // 强制裁剪
    }
}

QString SaveFile(const QString &directory, const QString &filename, const QString &content) {
    QString dirPath = directory.isEmpty() ? QDir::tempPath() : directory;
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath("."); // 创建目录
    }
    // qDebug() << "Saving file to:" << dir.filePath(filename);
    QFile file(dir.filePath(filename));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(content.toUtf8());
        file.close();
    }
    return dir.filePath(filename);
}

QString SavePngFile(const QString &directory, const QString &filename, const QString &content) {
    QString dirPath = directory.isEmpty() ? QDir::tempPath() : directory;
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath("."); // 创建目录
    }
    // 移除可能存在的base64头部信息(如果有的话)
    QString base64Data = content;
    if (base64Data.contains(","))
        base64Data = base64Data.split(",").at(1);

    // 将base64字符串转换为QByteArray
    QByteArray imageData = QByteArray::fromBase64(base64Data.toLatin1());

    // qDebug() << "Saving file to:" << dir.filePath(filename);
    QFile file(dir.filePath(filename));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(imageData);
        file.close();
    }
    return dir.filePath(filename);
}

//仅用于更新数据
void MainWindow::AnalyzeComponentData(const QString &CID, const QJsonObject &json,
                                      component_record_struct &component) {
    // 1. Extract basic data fields
    component.name = json["商品型号"].toString();

    // Randomly assign a color (for demonstration purposes, using a placeholder value)
    component.color = "就绪"; // You can replace this with a function to assign a real random color if needed

    component.jlcid = json["商品编号"].toString();

    // Extract weight and convert to float (extract number before "克(g)")
    QString weightStr = json["商品毛重"].toString();
    static QRegularExpression re("[^0-9.]");
    weightStr.remove(re); // Remove non-numeric characters except for the dot
    component.weight = weightStr.toFloat();

    component.discription = json["描述"].toString();
    if (component.discription.isEmpty()) {
        component.discription = component.name;
    }
    component.more_data = json["商品参数"].toString();
    component.package = json["商品封装"].toString();

    component.pdf_url = json["数据手册"].toString();
    component.pdf_name = json["数据手册名称"].toString();

    component.price = json["价格"].toString();
    component.inventory = json["库存"].toString();
    component.PID = json["PID"].toString();

    QJsonArray imageLinks = json["图片链接"].toArray();
    qDebug() << imageLinks << imageLinks.size();
    for (int i = 0; i < imageLinks.size(); ++i) {
        if (imageLinks[i].toString().isEmpty())continue;
        getFileRequest(imageLinks[i].toString(), [=](const QString &png_FileUrl) {
                           switch (i) {
                               case 0:
                                   AddCardToShow(_showInfo_PNGView, _showInfo_PNGCard1, png_FileUrl);
                                   break;
                               case 1:
                                   AddCardToShow(_showInfo_PNGView, _showInfo_PNGCard2, png_FileUrl);
                                   break;
                               case 2:
                                   AddCardToShow(_showInfo_PNGView, _showInfo_PNGCard3, png_FileUrl);
                                   break;
                               case 3:
                                   AddCardToShow(_showInfo_PNGView, _showInfo_PNGCard4, png_FileUrl);
                                   break;
                               case 4:
                                   AddCardToShow(_showInfo_PNGView, _showInfo_PNGCard5, png_FileUrl);
                                   break;

                               default: ;
                           }
                       }, nullptr,
                       "img_" + QString::number(i) + ".jpg", GetCIDPath(CID));
        component.png_FileUrl.append(GetCIDPath(CID) + "/img_" + QString::number(i) + ".jpg");
    }
    QJsonObject schSvg = json["sch_svg"].toObject();
    for (auto it = schSvg.begin(); it != schSvg.end(); ++it) {
        QString filename = "sch_svg_" + it.key() + ".png";
        QString content = it.value().toString();
        filename = SavePngFile(GetCIDPath(CID), filename, content);
        qDebug() << "Saving sch_svg to file:" << filename;
        component.sch_svg_FileUrl.append(filename);
    }

    QJsonObject pcbSvg = json["pcb_svg"].toObject();
    for (auto it = pcbSvg.begin(); it != pcbSvg.end(); ++it) {
        QString filename = "pcb_svg_" + it.key() + ".png";
        QString content = it.value().toString();
        filename = SavePngFile(GetCIDPath(CID), filename, content);
        qDebug() << "Saving pcb_svg to file:" << filename;
        component.pcb_svg_FileUrl.append(filename);
    }
}

//仅用于添加数据
void MainWindow::AnalyzeAddingComponentData(const QString &CID, const QJsonObject &json,
                                            component_record_struct &component) {
    // 1. Extract basic data fields
    component.name = json["商品型号"].toString();

    // Randomly assign a color (for demonstration purposes, using a placeholder value)
    component.color = "就绪"; // You can replace this with a function to assign a real random color if needed

    component.jlcid = json["商品编号"].toString();

    // Extract weight and convert to float (extract number before "克(g)")
    QString weightStr = json["商品毛重"].toString();
    static QRegularExpression re("[^0-9.]");
    weightStr.remove(re); // Remove non-numeric characters except for the dot
    component.weight = weightStr.toFloat();

    component.discription = json["描述"].toString();
    if (component.discription.isEmpty()) {
        component.discription = component.name;
    }
    component.more_data = json["商品参数"].toString();
    component.package = json["商品封装"].toString();

    component.pdf_url = json["数据手册"].toString();
    component.pdf_name = json["数据手册名称"].toString();

    component.price = json["价格"].toString();
    component.inventory = json["库存"].toString();
    component.PID = json["PID"].toString();

    // 2. Extract image URLs and store them in aliases (QVector<QString>)
    QJsonArray imageLinks = json["图片链接"].toArray();
    qDebug() << imageLinks << imageLinks.size();
    // QVector<QString> img_url;
    for (int i = 0; i < imageLinks.size(); ++i) {
        if (imageLinks[i].toString().isEmpty())continue;
        // qDebug() << "Processing image " << i << " URL:" << imageLinks[i].toString();
        getFileRequest(imageLinks[i].toString(), [=](const QString &) {
                           if (i == 0) {
                               _addComponent_CheckInfoWidget_Card1->
                                   setCardPixmap(QPixmap(_addingComponentObj->png_FileUrl[i]));
                           } else if (i == 1) {
                               _addComponent_CheckInfoWidget_Card2->
                                   setCardPixmap(QPixmap(_addingComponentObj->png_FileUrl[i]));
                           } else if (i == 2) {
                               _addComponent_CheckInfoWidget_Card3->
                                   setCardPixmap(QPixmap(_addingComponentObj->png_FileUrl[i]));
                           }
                       }
                       , nullptr,
                       "img_" + QString::number(i) + ".jpg", GetCIDPath(CID));
        component.png_FileUrl.append(GetCIDPath(CID) + "/img_" + QString::number(i) + ".jpg");
    }

    // qDebug() << img_url;
    // 3. Extract sch_svg and pcb_svg
    // For sch_svg (e.g., storing in files)
    QJsonObject schSvg = json["sch_svg"].toObject();
    for (auto it = schSvg.begin(); it != schSvg.end(); ++it) {
        QString filename = "sch_svg_" + it.key() + ".png";
        QString content = it.value().toString();
        filename = SavePngFile(GetCIDPath(CID), filename, content);
        qDebug() << "Saving sch_svg to file:" << filename;
        component.sch_svg_FileUrl.append(filename);
    }

    // For pcb_svg (e.g., storing in files)
    QJsonObject pcbSvg = json["pcb_svg"].toObject();
    for (auto it = pcbSvg.begin(); it != pcbSvg.end(); ++it) {
        QString filename = "pcb_svg_" + it.key() + ".png";
        QString content = it.value().toString();
        filename = SavePngFile(GetCIDPath(CID), filename, content);
        qDebug() << "Saving pcb_svg to file:" << filename;
        component.pcb_svg_FileUrl.append(filename);
    }
}

void MainWindow::AddComponentLogic_1() {
    _addComponent_EditBoxText->hide();

    _addComponent_EditBox->hide();
    _addComponent_busyRing->show();
    _addComponent_CheckInfoText->show();
    //请求服务器获取元器件信息
    _addingComponent_CID = _addComponent_EditBox->text();
    if (_addingComponent_CID[0] != 'C' && _addingComponent_CID[0] != 'c') {
        _addingComponent_CID = "C" + _addingComponent_CID;
    } else if (_addingComponent_CID[0] == 'c') {
        _addingComponent_CID[0] = 'C';
    }
    // _addComponent_EditBox->setText(CID);

    if (isExistingComponent(_addingComponent_CID)) {
        cancelAddComponentLogic(); //退出逻辑
        ShowWarningInfo("该元器件已经存在");
        return;
    }
    getRequest("http://127.0.0.1:8000/item/" + _addingComponent_CID, [&](const QJsonObject &jsonObj) {
                   qDebug() << jsonObj;

                   AnalyzeAddingComponentData(_addingComponent_CID, jsonObj, *_addingComponentObj);
                   // component_record_struct component1;

                   _addComponent_busyRing->hide();
                   _addComponent_CheckInfoWidget_Text->setText(
                       "<b>元器件名称：</b>" + _addingComponentObj->name + "<br><b>元器件编号：</b>" + _addingComponent_CID +
                       "<br><b>元器件描述：</b>"
                       + _addingComponentObj->discription +
                       "<br><b>元器件封装：</b>" + _addingComponentObj->package);
                   // for (int j = 0; j < _addingComponentObj->png_FileUrl.size(); j++) {
                   //     if (j == 0) {
                   //         _addComponent_CheckInfoWidget_Card1->
                   //             setCardPixmap(QPixmap(_addingComponentObj->png_FileUrl[j]));
                   //     } else if (j == 1) {
                   //         _addComponent_CheckInfoWidget_Card2->
                   //             setCardPixmap(QPixmap(_addingComponentObj->png_FileUrl[j]));
                   //     } else if (j == 2) {
                   //         _addComponent_CheckInfoWidget_Card3->
                   //             setCardPixmap(QPixmap(_addingComponentObj->png_FileUrl[j]));
                   //     }
                   // }
                   _addComponent_CheckInfoWidget->show();
                   // _promotionView->show();

                   // _promotionView->setCurrentIndex(0);
                   _addComponent_ProgressBar->setValue(60);
                   _addComponentButtonNext->setEnabled(true);
                   _addComponentStep = 2;
               }, [&](QNetworkReply::NetworkError error) {
                   if (error == QNetworkReply::NetworkError::InternalServerError) {
                       ShowErrorInfo("服务端错误，请联系服务器管理员");
                   }
                   // qWarning() << "无法获取元器件信息 " << error;
                   ShowErrorInfo("？");
                   cancelAddComponentLogic();
               }, [&] {
                   ShowErrorInfo("请求超时，请检查网络连接");
                   cancelAddComponentLogic();
               });
    _addComponent_ProgressBar->setValue(40);
    _addComponentButtonNext->setEnabled(false);
    // _addComponentStep = 2;
}
void MainWindow::AddComponentLogic_2() {
    _addComponent_CheckInfoWidget->hide();
    _addComponent_CheckInfoText->hide();

    if (!isDownloadPDF) {
        _addComponentStep = 3;
        AddComponentLogic_3();
    } else {
        _addComponent_DownloadText->show();
        _addComponent_DownloadProgressRing->show();
        _addComponentButtonNext->setEnabled(false);
        _addComponent_DownloadProgressRing->setCurValue(0);
        getFileRequest(_addingComponentObj->pdf_url, [&](const QString &fileUrl) {
                           qDebug() << "下载完成" << fileUrl;
                           _addComponent_ProgressBar->setValue(80);
                           _addingComponentObj->pdf_FileUrl = fileUrl;
                           _addComponentStep = 3;
                           AddComponentLogic_3();
                       }, [&](const QNetworkReply::NetworkError error) {
                           qWarning() << "无法获取元器件数据手册 " << error;
                       }, _addingComponentObj->pdf_name, GetCIDPath(_addingComponent_CID),
                       [&](qint64 bytesReceived, qint64 bytesTotal) {
                           _addComponent_DownloadProgressRing->setCurValue(bytesReceived * 100 / bytesTotal);
                       });
    }
    // _addComponentStep = 3;
}
void MainWindow::AddComponentLogic_3() {
    _addComponent_DownloadText->hide();
    _addComponent_DownloadProgressRing->hide();
    _addComponent_WaitText->show();
    _addComponent_OpenText->show();

    _addComponent_timeLeft = 600; // 剩余时间60秒
    // 每秒更新一次剩余时间

    _addComponent_timer->start(100);
    if (DEBUG) {
        QTimer::singleShot(3000, [&]() {
            isAddingComponent = false;
        });
    }
}
void MainWindow::AddComponentLogic_4() {
    //清理现场
    _addComponent_WaitText->setText("已完成添加向导");
    _addComponent_ProgressBar->setValue(100);
    _addComponent_CancelButton->setEnabled(false);
    ShowSuccessInfo("添加元器件成功");
    //TODO: 上传数据
    addComponentToLib(*_addingComponentObj);
    model->updateData();
    SaveSingleComponent(_addingComponent_CID); //触发保存
    delete _addingComponentObj;
    _addingComponentObj = nullptr;
    QTimer::singleShot(2000, [&]() {
        //关闭向导
        _addComponent_WaitText->hide();
        _addComponent_BeginButton->setEnabled(true);
        _addComponent_DockhArea->hide();
        _showInfo_Widget->show();
        _infoDockWidget->setWindowTitle("元器件信息");
        _infoDockWidget->setWidget(_showInfo_scrollArea);
        // resizeDocks({_infoDockWidget}, {600}, Qt::Vertical);
        resizeDocks({_infoDockWidget}, {400}, Qt::Horizontal);
    });
    _addComponentStep = 1;
}
void MainWindow::cancelAddComponentLogic() {
    _addComponent_ProgressBar->setValue(20);
    _addComponent_timer->stop();
    _addComponent_BeginButton->setEnabled(true);
    _addComponent_DockhArea->hide();
    _showInfo_Widget->show();
    _infoDockWidget->setWindowTitle("元器件信息");
    _infoDockWidget->setWidget(_showInfo_scrollArea);
    // resizeDocks({_infoDockWidget}, {600}, Qt::Vertical);
    resizeDocks({_infoDockWidget}, {400}, Qt::Horizontal);
    _addComponentStep = 1;
    if (_addingComponentObj != nullptr) {
        delete _addingComponentObj;
        _addingComponentObj = nullptr;
    }
}
void MainWindow::initAddComponentLogic() {
    connect(_addComponent_CancelButton, &ElaToolButton::clicked, this, [&] {
        cancelAddComponentLogic();
    });
    connect(_addComponent_BeginButton, &ElaToolButton::clicked, this, [&] {
        _addComponent_DockhArea->show();
        _showInfo_Widget->hide();
        _infoDockWidget->setWindowTitle("新增元器件向导");
        _infoDockWidget->setWidget(_addComponent_DockhArea);
        _addComponent_WaitText->hide();
        _addComponent_busyRing->hide();
        _addComponent_DownloadProgressRing->hide();
        _addComponent_CheckInfoText->hide();
        _addComponent_DownloadText->hide();
        _addComponent_OpenText->hide();
        _addComponent_EditBox->show();
        _addComponent_EditBoxText->show();
        // resizeDocks({_infoDockWidget}, {600}, Qt::Vertical);
        resizeDocks({_infoDockWidget}, {400}, Qt::Horizontal);
        _addComponent_ProgressBar->setValue(20);

        isAddingComponent = true;
        _addComponentStep = 1;
        _addComponent_CancelButton->setEnabled(true);
        _addComponent_BeginButton->setEnabled(false);
        _addComponent_EditBox->clear();
        _addComponent_EditBox->setFocus();
        _addComponentButtonNext->setEnabled(false);
        _addComponent_CheckInfoWidget->hide();

        _addingComponentObj = new component_record_struct;
        // if (DEBUG)
        //     _addComponent_EditBox->setText("52717");
        // AddComponentLogic();
    });
    connect(_addComponent_EditBox, &ElaLineEdit::textChanged, this, [&] {
        if (_addComponentStep == 1) {
            if (_addComponent_EditBox->text().isEmpty()) {
                _addComponentButtonNext->setEnabled(false);
            } else {
                _addComponentButtonNext->setEnabled(true);
            }
        }
    });
    connect(_addComponentButtonNext, &ElaToolButton::clicked, this, [&] {
        switch (_addComponentStep) {
            case 1:
                AddComponentLogic_1();
                break;
            case 2:
                AddComponentLogic_2();
                break;
            case 3:
            case 4:
            default:
                break;
        }
    });
    connect(_addComponent_timer, &QTimer::timeout, this, [&] {
        if (!isAddingComponent) {
            _addComponent_timer->stop(); // 停止计时器
            _addComponentStep = 4;
            AddComponentLogic_4(); //处理下一个逻辑
            return;
        }
        _addComponent_timeLeft--; // 每秒减少1秒
        _addComponent_WaitText->setText("请在" + QString::number((_addComponent_timeLeft / 10)) + "s内打开闪蓝灯的格子");
        if (_addComponent_timeLeft == 0) {
            qDebug() << "计时结束!";
            _addComponent_timer->stop(); // 停止计时器
            cancelAddComponentLogic();
        }
    });
}

#include "GetFileRequestHandler.h"
#include "GetRequestHandler.h"
#include "mainwindow.h"
//
// Created by xtx on 24-11-7.
//
void MainWindow::getDailySection() const {
    //获取
    getRequest("https://whyta.cn/api/tx/one?key=cc8cba0a7069",
               [&](const QJsonObject &jsonObj) {
                   QJsonObject resultObj = jsonObj.value("result").toObject();

                   // 提取字段
                   QString word = resultObj.value("word").toString();
                   QString imgurl = resultObj.value("imgurl").toString();
                   if (word.isEmpty() || imgurl.isEmpty())
                       qWarning() << "无法解析每日一言";
                   // 输出字段
                   // qDebug() << "Word:" << word;
                   // qDebug() << "Image URL:" << imgurl;
                   // qDebug() << "系统缓存目录路径:" << QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
                   getFileRequest(imgurl,
                                  [&](const QString &filePath) {
                                      // qDebug() << "File downloaded to:" << filePath;
                                      QImageReader reader(filePath);
                                      reader.setAutoTransform(true);
                                      const QImage img = reader.read();
                                      _promotionCard->setFixedSize(img.size().width() * 350.0 / img.size().height(),
                                                                   350);
                                      _promotionCard->setCardPixmap(QPixmap(filePath));
                                  },
                                  [](QNetworkReply::NetworkError error) {
                                      // 处理失败的错误信息
                                      qWarning() << "无法获取每日一言 " << error;
                                  },
                                  "Daily.jpg",
                                  CONFIGPATH);
                   _promotionCard->setSubTitle(word);
               },
               [](QNetworkReply::NetworkError error) {
                   // 处理失败的错误信息
                   qWarning() << "无法获取每日一言 " << error;
               });
}

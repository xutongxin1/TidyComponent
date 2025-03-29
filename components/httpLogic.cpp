#include <QImageReader>

#include "GetFileRequestHandler.h"
#include "GetRequestHandler.h"
#include "mainwindow.h"
//
// Created by xtx on 24-11-7.
//
void MainWindow::getDailySection() const {
    //获取
    getRequest("http://v3.wufazhuce.com:8000/api/channel/one/0/0",
               [&](const QJsonObject &jsonObj) {
                   if (!jsonObj.contains("data") || !jsonObj["data"].isObject()) {
                       qWarning() << "无法解析每日一言";
                       return;
                   }

                   QJsonObject dataObj = jsonObj["data"].toObject();

                   // 检查是否包含"content_list"数组
                   if (!dataObj.contains("content_list") || !dataObj["content_list"].isArray()) {
                       qWarning() << "无法解析每日一言";
                       return;
                   }

                   QJsonArray contentList = dataObj["content_list"].toArray();

                   // 检查数组是否为空
                   if (contentList.isEmpty()) {
                       qWarning() << "无法解析每日一言";
                       return;
                   }

                   // 获取第一个元素
                   QJsonObject firstContent = contentList.at(0).toObject();

                   // word
                   QString imgurl = firstContent["img_url"].toString();
                   QString word = firstContent["forward"].toString();

                   if (word.isEmpty() || imgurl.isEmpty()) {
                       qWarning() << "无法解析每日一言";
                       return;
                   }
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
                                  TEMP_PATH);
                   _promotionCard->setSubTitle(word);
               },
               [](QNetworkReply::NetworkError error) {
                   // 处理失败的错误信息
                   qWarning() << "无法获取每日一言 " << error;
               });
}

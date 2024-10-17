//
// Created by xtx on 24-10-17.
//

#ifndef GETFILEREQUEST_H
#define GETFILEREQUEST_H

/*
使用默认的文件名和存储路径
 getFileRequest("http://example.com/file.zip",
               [](QString filePath) {
                   qDebug() << "File downloaded to:" << filePath;
               },
               [](QNetworkReply::NetworkError error) {
                   qDebug() << "Download error:" << error;
               });
自定义文件名
getFileRequest("http://example.com/file.zip",
               [](QString filePath) {
                   qDebug() << "File downloaded to:" << filePath;
               },
               nullptr,
               "my_custom_name.zip");
自定义存储路径
getFileRequest("http://example.com/file.zip",
               [](QString filePath) {
                   qDebug() << "File downloaded to:" << filePath;
               },
               nullptr,
               QString(),
               "/path/to/save/directory");
自定义文件名和存储路径
getFileRequest("http://example.com/file.zip",
               [](QString filePath) {
                   qDebug() << "File downloaded to:" << filePath;
               },
               nullptr,
               "my_custom_name.zip",
               "/path/to/save/directory");

*/

#include <QtNetwork>
#include <functional>
#include <QFile>
#include <QDir>
#include <QDateTime>

class GetFileRequestHandler : public QObject {
        Q_OBJECT

    public:
        GetFileRequestHandler(const QString &url,
                              std::function<void(QString)> onSuccess,
                              std::function<void(QNetworkReply::NetworkError)> onError = nullptr,
                              const QString &fileName = QString(),
                              const QString &directory = QString(),
                              std::function<void(qint64, qint64)> onProgress = nullptr,
                              QObject *parent = nullptr)
            : QObject(parent), m_onSuccess(onSuccess), m_onError(onError), m_onProgress(onProgress) {
            // 创建网络访问管理器
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);

            // 创建请求
            QNetworkRequest request(QUrl(url, QUrl::TolerantMode));

            // 发送GET请求
            QNetworkReply *reply = manager->get(request);

            // 生成保存文件的路径
            m_savePath = generateSavePath(url, fileName, directory);

            // 连接请求完成的信号到槽函数
            connect(reply, &QNetworkReply::finished, this, [this, reply]() {
                // 检查是否有错误
                if (reply->error() == QNetworkReply::NoError) {
                    // 读取响应数据并保存到文件
                    QByteArray data = reply->readAll();

                    QFile file(m_savePath);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(data);
                        file.close();
                        // 调用成功的回调函数，传递文件路径
                        m_onSuccess(m_savePath);
                    } else {
                        // 如果无法写入文件，调用错误回调
                        if (m_onError) {
                            m_onError(QNetworkReply::ContentAccessDenied);
                        }
                    }
                } else {
                    // 调用失败的回调函数（如果提供了）
                    if (m_onError) {
                        m_onError(reply->error());
                    }
                }
                // 清理
                reply->deleteLater();
                this->deleteLater();
            });
            // 连接下载进度信号
            if (m_onProgress) {
                connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal) {
                    m_onProgress(bytesReceived, bytesTotal);
                });
            }
        }

        std::function<void(qint64, qint64)> m_onProgress;

    private:
        QString generateSavePath(const QString &url, const QString &fileName, const QString &directory) {
            QString dirPath = directory.isEmpty() ? QDir::tempPath() : directory;
            QString name = fileName;

            if (name.isEmpty()) {
                // 从URL中提取文件名
                QUrl qurl(url);
                name = QFileInfo(qurl.path()).fileName();
                if (name.isEmpty()) {
                    // 如果无法从URL获取文件名，使用时间戳
                    name = QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz");
                }
            }

            QDir dir(dirPath);
            if (!dir.exists()) {
                dir.mkpath("."); // 创建目录
            }

            return dir.filePath(name);
        }

        std::function<void(QString)> m_onSuccess; // 成功回调，传递文件路径
        std::function<void(QNetworkReply::NetworkError)> m_onError;
        QString m_savePath; // 保存文件的路径
};

// 定义 getFileRequest 函数
inline void getFileRequest(const QString &url,
                           std::function<void(QString)> onSuccess,
                           std::function<void(QNetworkReply::NetworkError)> onError = nullptr,
                           const QString &fileName = QString(),
                           const QString &directory = QString()) {
    // 创建请求处理器
    new GetFileRequestHandler(url, onSuccess, onError, fileName, directory);
}

#endif //GETFILEREQUEST_H

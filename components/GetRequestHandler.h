//
// Created by xtx on 24-10-17.
//

#ifndef GETREQUESTHANDLER_H
#define GETREQUESTHANDLER_H

/*
 *如果您希望成功回调函数接收 QString 类型的数据，可以这样调用：
getRequest("http://example.com",
           [](QString data) {
               // 处理成功的响应数据（QString）
               qDebug() << "Success:" << data;
           },
           [](QNetworkReply::NetworkError error) {
               // 处理失败的错误信息
               qDebug() << "Error:" << error;
           });
如果您希望成功回调函数接收 QByteArray 类型的数据，可以这样调用：
getRequest("http://example.com",
           [](QByteArray data) {
               // 处理成功的响应数据（QByteArray）
               qDebug() << "Success:" << data;
           },
           [](QNetworkReply::NetworkError error) {
               // 处理失败的错误信息
               qDebug() << "Error:" << error;
           });


 */
#include <QtNetwork>
#include <functional>
#include <type_traits>
#include <QJsonObject>
#include <QJsonDocument>

class GetRequestHandler : public QObject {
        Q_OBJECT

    public:
        template<typename SuccessHandler>
        GetRequestHandler(const QString &url,
                          SuccessHandler onSuccess,
                          std::function<void(QNetworkReply::NetworkError,QString)> onError = nullptr,
                          std::function<void()> onTimeOut = nullptr,
                          int timeout = 5000, // 默认超时时间为 5000 毫秒
                          QObject *parent = nullptr)
            : QObject(parent), m_onError(onError), m_onTimeOut(onTimeOut), m_timeout(timeout) {
            // 创建网络访问管理器
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);

            // 创建请求
            QNetworkRequest request(QUrl(url, QUrl::TolerantMode));

            // 设置超时
            QTimer::singleShot(m_timeout, this, [this, manager, request]() {
                if (m_onTimeOut) {
                    m_onTimeOut();
                }
                manager->deleteLater();
            });

            // 发送GET请求
            QNetworkReply *reply = manager->get(request);

            // 连接请求完成的信号到槽函数
            connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess]() {
                // 检查是否有错误
                if (reply->error() == QNetworkReply::NoError) {
                    // 读取响应数据
                    QByteArray data = reply->readAll();

                    // 判断返回类型并调用相应的成功回调
                    if constexpr (std::is_invocable_v<SuccessHandler, QString>) {
                        QString strData = QString::fromUtf8(data);
                        onSuccess(strData);
                    } else if constexpr (std::is_invocable_v<SuccessHandler, QByteArray>) {
                        onSuccess(data);
                    } else if constexpr (std::is_invocable_v<SuccessHandler, QJsonObject>) {
                        QJsonParseError parseError;
                        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);
                        if (parseError.error == QJsonParseError::NoError) {
                            onSuccess(jsonDoc.object());
                        } else {
                            // 无法转换为 QJsonObject 时调用错误处理
                            if (m_onError) {
                                m_onError(QNetworkReply::UnknownNetworkError,QString()); // 使用HTTP 202
                            }
                        }
                    } else {
                        static_assert(false, "onSuccess must accept either QByteArray, QString, or QJsonObject");
                    }
                } else {
                    // 调用失败的回调函数（如果提供了）
                    if (m_onError) {
                        m_onError(reply->error(),reply->readAll());
                    }
                }
                // 清理
                reply->deleteLater();
                this->deleteLater();
            });
        }

    private:
        std::function<void(QNetworkReply::NetworkError,QString)> m_onError;
        std::function<void()> m_onTimeOut;
        int m_timeout;
};

// 定义模板异步GET请求函数
template<typename SuccessHandler>
void getRequest(const QString &url,
                SuccessHandler onSuccess,
                std::function<void(QNetworkReply::NetworkError,QString)> onError = nullptr,
                std::function<void()> onTimeOut = nullptr,
                int timeout = 8000) {
    // 创建请求处理器
    new GetRequestHandler(url, onSuccess, onError, onTimeOut, timeout);
}

#endif //GETREQUESTHANDLER_H

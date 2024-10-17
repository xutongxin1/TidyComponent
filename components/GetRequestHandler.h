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

class GetRequestHandler : public QObject
{
    Q_OBJECT
public:
    template<typename SuccessHandler>
    GetRequestHandler(const QString& url,
                      SuccessHandler onSuccess,
                      std::function<void(QNetworkReply::NetworkError)> onError = nullptr,
                      QObject* parent = nullptr)
        : QObject(parent), m_onError(onError)
    {
        // 创建网络访问管理器
        QNetworkAccessManager* manager = new QNetworkAccessManager(this);

        // 创建请求
        QNetworkRequest request(QUrl(url, QUrl::TolerantMode));

        // 发送GET请求
        QNetworkReply* reply = manager->get(request);

        // 连接请求完成的信号到槽函数
        connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess]() {
            // 检查是否有错误
            if (reply->error() == QNetworkReply::NoError) {
                // 读取响应数据
                QByteArray data = reply->readAll();
                // 根据回调函数的参数类型处理数据
                if constexpr (std::is_invocable_v<SuccessHandler, QString>) {
                    QString strData = QString::fromUtf8(data);
                    onSuccess(strData);
                } else if constexpr (std::is_invocable_v<SuccessHandler, QByteArray>) {
                    onSuccess(data);
                } else {
                    static_assert(false, "onSuccess must accept either QByteArray or QString");
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
    }

private:
    std::function<void(QNetworkReply::NetworkError)> m_onError;
};

// 定义模板异步GET请求函数
template<typename SuccessHandler>
void getRequest(const QString& url,
                SuccessHandler onSuccess,
                std::function<void(QNetworkReply::NetworkError)> onError = nullptr)
{
    // 创建请求处理器
    new GetRequestHandler(url, onSuccess, onError);
}






#endif //GETREQUESTHANDLER_H

#ifndef HTTPSHORTCONNECTION_H
#define HTTPSHORTCONNECTION_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPixmap>
#include <QTimer>
#include "../utils/userinfo.h"

class TcpLongConnection;
class UserInfo;

class HttpShortConnection : public QObject
{
    Q_OBJECT
public:
    enum class ImageError
    {
        NoError,
        NotFound
    };

    HttpShortConnection(const HttpShortConnection&) = delete;
    HttpShortConnection& operator=(const HttpShortConnection&) = delete;

    static HttpShortConnection& getHttpClient();

    void uploadAvatar(const QString& filePath);
    void uploadImage(const QString& filePath, std::function<void(const QString& url)> cb_success, bool failed_notice = false, std::function<void(const QString& info)> cb_failed = nullptr);
    void getImage(const QString& url, size_t retryTime, std::function<void(const QByteArray&, ImageError)> onSuccess = nullptr, bool failed_notice = true);

signals:
    void mainState(bool isSuccess, QString info);
    void refreshExpiredExit();

private:
    explicit HttpShortConnection(QObject *parent = nullptr);
    QByteArray getImageFormat(const QByteArray& data) const;

    QNetworkAccessManager* httpmanager;
};

#endif // HTTPSHORTCONNECTION_H

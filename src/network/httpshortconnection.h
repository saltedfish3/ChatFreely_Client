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
    HttpShortConnection(const HttpShortConnection&) = delete;
    HttpShortConnection& operator=(const HttpShortConnection&) = delete;

    static HttpShortConnection& getHttpClient();

    void uploadAvatar(const QString& filePath);
    void getImage(const QString& url, size_t retryTime, std::function<void(const QPixmap&)> onSuccess = nullptr, bool failed_notice = true);

signals:
    void mainState(bool isSuccess, QString info);
    void refreshExpiredExit();

private:
    explicit HttpShortConnection(QObject *parent = nullptr);

    QNetworkAccessManager* httpmanager;
};

#endif // HTTPSHORTCONNECTION_H

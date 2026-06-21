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
#include "tcplongconnection.h"

class HttpShortConnection : public QObject
{
    Q_OBJECT
public:
    HttpShortConnection(const HttpShortConnection&) = delete;
    HttpShortConnection& operator=(const HttpShortConnection&) = delete;

    static HttpShortConnection& getHttpClient();

    void uploadAvatar(const QString& filePath);
    void getAvatar(const QString& url, size_t retryTime);

signals:
    void AvatarReady(QPixmap pixmap);

private:
    explicit HttpShortConnection(QObject *parent = nullptr);

    QNetworkAccessManager* httpmanager;
};

#endif // HTTPSHORTCONNECTION_H

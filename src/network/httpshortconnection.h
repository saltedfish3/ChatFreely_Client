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

    enum class MediaType
    {
        Image,
        Video,
        File
    };

    HttpShortConnection(const HttpShortConnection&) = delete;
    HttpShortConnection& operator=(const HttpShortConnection&) = delete;

    static HttpShortConnection& getHttpClient();

    void uploadAvatar(const QString& filePath);
    void uploadMedia(MediaType type, const QString& filePath, std::function<void(const QString& url)> cb_success, bool failed_notice = false, std::function<void(const QString& info)> cb_failed = nullptr);
    void getImage(const QString& url, size_t retryTime, std::function<void(const QByteArray&, ImageError)> onSuccess = nullptr, bool failed_notice = true);

signals:
    void mainState(bool isSuccess, QString info);
    void refreshExpiredExit();

private:
    explicit HttpShortConnection(QObject *parent = nullptr);
    QByteArray getImageFormat(const QByteArray& data) const;
    QString isVideo(const QByteArray& data) const;

    void sendUploadInit(MediaType type, const QString& filePath, qint64 fileSize, const QByteArray& md5, const QString& suffix,
                        const QString& mimeType, const QByteArray& fileData, std::function<void(const QString&)> cb_success, bool failed_notice, std::function<void(const QString&)> cb_failed);
    void multipartUpload(const QString& filePath, qint64 everyPartSize, const QString& objectKey, const QString& uploadId,
                         const QString& finalUrl, std::function<void(const QString&)> cb_success, bool failed_notice, std::function<void(const QString&)> cb_failed);
    void uploadOnePart(const QString& filePath, qint64 partSize, const QString& objectKey, const QString& uploadId, int partNumber, int totalParts,
                       QSharedPointer<QJsonArray> parts, QSharedPointer<std::function<void()>> next, const QString& finalUrl,
                       std::function<void(const QString&)> cb_success, bool failed_notice, std::function<void(const QString&)> cb_failed);
    void sendComplete(const QString& objectKey, const QString& uploadId, const QJsonArray& parts, const QString& finalUrl, std::function<void(const QString&)> cb_success, bool failed_notice, std::function<void(const QString&)> cb_failed);

    QNetworkAccessManager* httpmanager;
};

#endif // HTTPSHORTCONNECTION_H

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
#include <QHash>
#include <QFile>
#include <QElapsedTimer>
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

    struct DownloadStatus
    {
        bool isDownloading = false;
        bool isFinished = false;
        qint64 receivedSize = 0;
        qint64 totalSize = 0;

        int getDownloadPercent() const
        {
            return totalSize > 0 ? static_cast<int>(receivedSize * 100 / totalSize) : 0;
        }
    };

    HttpShortConnection(const HttpShortConnection&) = delete;
    HttpShortConnection& operator=(const HttpShortConnection&) = delete;

    static HttpShortConnection& getHttpClient();

    //上传
    void uploadAvatar(const QString& filePath);
    void uploadMedia(MediaType type, const QString& filePath, std::function<void(const QString& url)> cb_success, bool failed_notice = false, std::function<void(const QString& info)> cb_failed = nullptr);

    //下载
    void getImage(const QString& url, size_t retryTime, std::function<void(const QByteArray&, ImageError)> onSuccess = nullptr, bool failed_notice = true);
    void downloadFile(const QString& url, const QString& targetPath);
    void cancelDownloadFile(const QString& url);
    void cancelAllDownloads();
    DownloadStatus getDownloadStatus(const QString& url) const;
    QString resolveTargetPath(const QString& path);

signals:
    void mainState(bool isSuccess, QString info);
    void refreshExpiredExit();

    void downloadStarted(const QString& url);
    void downloadProgressChanged(const QString& url, qint64 received, qint64 total);
    void downloadFinished(const QString& url, bool isSuccess, const QString& info);

private:
    struct DownloadTask
    {
        QNetworkReply* reply = nullptr;
        QFile* file = nullptr;
        QString url;
        QString targetPath;
        QString partPath;
        qint64 receivedSize = 0;
        qint64 totalSize = 0;
        int retryCount = 0;

        QByteArray buffer;
        QElapsedTimer throttle;
    };

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

    void startDownload(const QString& url);
    void finishDownload(const QString& url, bool isSuccess, const QString& info);

    //下载去重
    QHash<QString, DownloadTask*> hash_downloadTasks;
    QHash<QString, DownloadStatus> hash_downloadFinishedStatus;

    QNetworkAccessManager* httpmanager;
};

#endif // HTTPSHORTCONNECTION_H

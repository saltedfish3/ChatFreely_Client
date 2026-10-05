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
            if(totalSize <= 0)
                return 0;
            return qBound(0, totalSize > 0 ? static_cast<int>(receivedSize * 100 / totalSize) : 0, 100);
        }
    };

    struct UploadResult
    {
        QString url;
        QString thumbnailUrl;
    };

    HttpShortConnection(const HttpShortConnection&) = delete;
    HttpShortConnection& operator=(const HttpShortConnection&) = delete;

    static HttpShortConnection& getHttpClient();

    //上传
    void uploadAvatar(const QString& filePath);
    void uploadMedia(MediaType type, const QString& filePath, std::function<void(const UploadResult& result)> cb_success, bool failed_notice = false, std::function<void(const QString& info)> cb_failed = nullptr, bool needServerThumbnail = true);

    //下载
    void getImage(const QString& url, size_t retryTime, std::function<void(const QByteArray&, ImageError)> onSuccess = nullptr, bool failed_notice = true);
    void downloadFile(const QString& url, const QString& targetPath);
    void cancelDownloadFile(const QString& url);
    void cancelAllDownloads();
    void cancelUpload(const QString& filePath);
    DownloadStatus getDownloadStatus(const QString& url) const;
    QString resolveTargetPath(const QString& path);

    void checkUrlExists(const QString& url, std::function<void(bool isExists)> callback);

signals:
    void mainState(bool isSuccess, QString info);
    void refreshExpiredExit();

    void downloadStarted(const QString& url);
    void downloadProgressChanged(const QString& url, qint64 received, qint64 total);
    void downloadFinished(const QString& url, bool isSuccess, const QString& info);

    void uploadProgressChanged(const QString& filePath, qint64 sendSize, qint64 totalSize, int percent);

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

    struct UploadTask
    {
        MediaType type;
        QString filePath;
        qint64 totalSize = 0;
        qint64 sendSize = 0;
        int lastPercent = -1;
        QElapsedTimer throttle;
        QList<std::function<void(const UploadResult& result)>> successCallbacks;
        QList<std::function<void(const QString&)>> failedCallbacks;

        int getPercent() const
        {
            if(totalSize <= 0)
                return 0;
            int percent = static_cast<int>(sendSize * 100 / totalSize);
            return qBound(0, percent, 100);
        }
    };

    explicit HttpShortConnection(QObject *parent = nullptr);
    QByteArray getImageFormat(const QByteArray& data) const;
    QString isVideo(const QByteArray& data) const;

    void sendUploadInit(MediaType type, const QString& filePath, qint64 fileSize, const QByteArray& md5, const QString& suffix,
                        const QString& mimeType, const QByteArray& fileData, std::function<void(const UploadResult&)> cb_success, bool failed_notice, std::function<void(const QString&)> cb_failed, bool needServerThumbnail);
    void finishUpload(const QString& filePath, const QString& info);//失败
    void finishUpload(const QString& filePath, const UploadResult& result);//成功
    void multipartUpload(const QString& filePath, qint64 everyPartSize, const QString& objectKey, const QString& uploadId,
                         const QString& finalUrl, const QJsonArray& initialParts, std::function<void(const UploadResult&)> cb_success, bool failed_notice, std::function<void(const QString&)> cb_failed);
    void uploadOnePart(const QString& filePath, qint64 partSize, const QString& objectKey, const QString& uploadId, int partNumber, int totalParts,
                       QSharedPointer<QJsonArray> parts, QSharedPointer<std::function<void()>> next, const QString& finalUrl,
                       std::function<void(const UploadResult&)> cb_success, bool failed_notice, std::function<void(const QString&)> cb_failed);
    void sendComplete(const QString& filePath, const QString& objectKey, const QString& uploadId, const QJsonArray& parts, const QString& finalUrl, std::function<void(const UploadResult&)> cb_success, bool failed_notice, std::function<void(const QString&)> cb_failed);
    void handleUploadProgress(const QString& filePath, qint64 alreadySend);

    void startDownload(const QString& url);
    void finishDownload(const QString& url, bool isSuccess, const QString& info);

    void cleanALL();
    bool isCleaning = false;

    //下载去重
    QHash<QString, DownloadTask*> hash_downloadTasks;
    QHash<QString, DownloadStatus> hash_downloadFinishedStatus;

    //上传去重
    QHash<QString, UploadTask*> hash_uploadTasks;

    QHash<QString, QSet<QNetworkReply*>> hash_uploadReply;

    QNetworkAccessManager* httpmanager;
};

#endif // HTTPSHORTCONNECTION_H

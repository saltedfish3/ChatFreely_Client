#ifndef IMAGECACHEMANAGER_H
#define IMAGECACHEMANAGER_H

#include <QObject>
#include <QHash>
#include <QPixmap>
#include <QImage>
#include <QMutex>
#include <QThread>
#include <QCache>
#include <QtConcurrent/QtConcurrent>
#include <QCoreApplication>
#include <QPainter>
#include <QPainterPath>
#include <QThreadPool>
#include "../network/httpshortconnection.h"
#include "GlobalVariable.h"
#include "functionrunnable.h"

class ImageCacheManager : public QObject
{
    Q_OBJECT
public:
    enum class ImageState
    {
        Loading = 0,
        Failed,
        Success,
        NotExist
    };

    static ImageCacheManager& getManager();
    ImageCacheManager& operator=(const ImageCacheManager&) = delete;
    ImageCacheManager(const ImageCacheManager&) = delete;

    void loadImage(const QString& url, std::function<void(const QPixmap&)> callback, bool failed_notice = false,
                   qreal dpr = -1, int radius = 0, int padding = 0, QSize size = QSize());
    void insertCache(const QString& url, const QImage& img, qreal dpr = -1, int radius = 0, int padding = 0, QSize size = QSize());
    void insertCache(const QString& url, const QPixmap& pix, qreal dpr = -1, int radius = 0, int padding = 0, QSize size = QSize());
    void insertCache(const QString& url, const QByteArray& data, qreal dpr = -1, int radius = 0, int padding = 0, QSize size = QSize());
    void migrateCache(const QString& oldUrl, const QString& newUrl, qreal dpr = -1, int radius = 0, int padding = 0, QSize size = QSize());
    void removeCache(const QString& url, qreal dpr = -1, int radius = 0, int padding = 0, QSize size = QSize());
    QString getCacheFilePath(const QString& url, qreal dpr = -1, int radius = 0, int padding = 0, QSize size = QSize()) const;
    QPixmap fastLoadImage(const QString& url, qreal dpr = -1, int radius = 0, int padding = 0, QSize size = QSize());

    ImageState getImageState(const QString& url, qreal dpr = -1, int radius = 0, int padding = 0, QSize size = QSize());

signals:

private:
    struct imageTask
    {
        qreal dpr;
        int radius;
        int padding;
        QSize size;
        std::function<void(const QPixmap&)> callback;
    };

    explicit ImageCacheManager(QObject *parent = nullptr);
    //加载图片
    bool tryLoadOriginalFromCache(const QString& url, QPixmap& outPixmap);
    bool loadCacheFromMemory(const QString& url, QPixmap& outPixmap, qreal dpr, int radius, int padding, QSize size);
    bool loadCacheFromDisk(const QString& url, QPixmap& outPixmap, qreal dpr, int radius, int padding, QSize size);
    void loadCacheFromServer(const QString& url, imageTask task, bool failed_notice);

    void handleDownloadFinished(const QString& url, const QByteArray& data);
    QString getUrlKey(const QString& url, qreal dpr = -1, int radius = 0, int padding = 0, QSize size = QSize()) const;
    QString getFilenameFromKey(const QString& key) const;
    void invokeCallbacks(const QList<std::function<void(const QPixmap&)>>& callbacks, const QPixmap& pic);

    //图片处理
    void handleRounded(const QString& url, const QPixmap& pix, imageTask task);
    QImage addRoundedAndPadding(const QImage &image, int radius, int padding, qreal dpr, QSize displaySize) const;

    QString pos_imageCache;
    QCache<QString, QPixmap> cache_memoryCache;

    //图片状态
    QHash<QString, ImageState> hash_imageState;

    //下载任务去重
    QThreadPool threadPool;
    QSet<QString> set_waitingUrls;
    QHash<QString, QList<imageTask>> hash_waitingCallback;
    QHash<QString, QTimer*> hash_timeoutTimer;

    //圆角绘制任务去重
    QSet<QString> set_handleRounded;
    QHash<QString, QList<std::function<void(const QPixmap&)>>> hash_roundedCallback;

    QReadWriteLock rwLock;
};

#endif // IMAGECACHEMANAGER_H

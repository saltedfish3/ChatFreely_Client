#ifndef IMAGECACHEMANAGER_H
#define IMAGECACHEMANAGER_H

#include <QObject>
#include <QHash>
#include <QPixmap>
#include <QImage>
#include <QMutex>
#include <QThread>
#include <QCache>
#include <QCoreApplication>
#include "../network/httpshortconnection.h"
#include "../utils/GlobalVariable.h"

class ImageCacheManager : public QObject
{
    Q_OBJECT
public:
    static ImageCacheManager& getManager();
    ImageCacheManager& operator=(const ImageCacheManager&) = delete;
    ImageCacheManager(const ImageCacheManager&) = delete;

    void loadImage(const QString& url, std::function<void(const QPixmap&)> callback, bool failed_notice = false);

signals:

private:
    explicit ImageCacheManager(QObject *parent = nullptr);

    bool tryLoadFromCache(const QString& url, QPixmap& outPixmap);

    void handleDownloadFinished(const QString& url, const QPixmap& pic);

    QString getFilenameFromUrl(const QString& url) const;

    void invokeCallbacks(const QList<std::function<void(const QPixmap&)>>& callbacks, const QPixmap& pic);

    QString pos_imageCache;

    QCache<QString, QPixmap> cache_memoryCache;

    QSet<QString> set_waitingUrls;
    QHash<QString, QList<std::function<void(const QPixmap&)>>> hash_waitingCallback;
    QHash<QString, QTimer*> hash_timeoutTimer;

    QMutex mutex;
};

#endif // IMAGECACHEMANAGER_H

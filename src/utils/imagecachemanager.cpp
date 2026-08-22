#include "imagecachemanager.h"

ImageCacheManager &ImageCacheManager::getManager()
{
    static ImageCacheManager icm;
    return icm;
}

void ImageCacheManager::loadImage(const QString &url, std::function<void (const QPixmap &)> callback, bool failed_notice)
{
    QPixmap cache;
    if(tryLoadFromCache(url, cache))
    {
        if(callback)
            invokeCallbacks({callback}, cache);
        return;
    }

    //未命中缓存
    bool needDownload = false;
    {
        QMutexLocker locker(&(this->mutex));
        if(this->set_waitingUrls.contains(url))
        {
            //若同一图片下载任务存在，只添加等待回调
            this->hash_waitingCallback[url].append(callback);
            return;
        }
        else
        {
            this->set_waitingUrls.insert(url);
            this->hash_waitingCallback[url].append(callback);

            QTimer* timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setInterval(30000);

            connect(timer, &QTimer::timeout, this, [this, url](){
                handleDownloadFinished(url, QPixmap());
            });

            timer->start();
            this->hash_timeoutTimer[url] = timer;

            needDownload = true;
        }
    }

    if(needDownload)
    {
        HttpShortConnection::getHttpClient().getImage(url, 3, [this, url](const QPixmap& pix){
            handleDownloadFinished(url, pix);
        }, failed_notice);
    }
}

ImageCacheManager::ImageCacheManager(QObject *parent)
    : QObject{parent}
{
    this->pos_imageCache = GlobalVariable::getPosOfImageCache();
    this->cache_memoryCache.setMaxCost(100 * 1024 * 1024);//100MB
}

bool ImageCacheManager::tryLoadFromCache(const QString &url, QPixmap &outPixmap)
{
    {
        QMutexLocker locker(&(this->mutex));
        QPixmap* cache = this->cache_memoryCache.object(url);
        if(cache)
        {
            outPixmap = *cache;
            return true;
        }
    }

    //尝试从硬盘加载
    const QString filePath = getFilenameFromUrl(url);
    if(!QFile::exists(filePath))
        return false;

    QPixmap pix;
    if(!pix.load(filePath))
    {
        QFile::remove(filePath);
        return false;
    }
    qDebug()<<"硬盘加载";
    outPixmap = pix;

    QPixmap* ptr(new QPixmap(pix));
    int cost = pix.toImage().sizeInBytes();
    if(cost < 1)
        cost = 1;
    QMutexLocker locker(&(this->mutex));
    this->cache_memoryCache.insert(url, ptr, cost);

    return true;
}

void ImageCacheManager::handleDownloadFinished(const QString &url, const QPixmap &pic)
{
    QList<std::function<void(const QPixmap&)>> callbacks;
    QTimer* timer = nullptr;

    {
        QMutexLocker locker(&(this->mutex));
        this->set_waitingUrls.remove(url);

        if(this->hash_timeoutTimer.contains(url))
        {
            timer = this->hash_timeoutTimer.take(url);
            timer->stop();
            timer->deleteLater();
        }

        if(!pic.isNull())
        {
            QPixmap* ptr(new QPixmap(pic));
            int cost = pic.toImage().sizeInBytes();

            if(cost < 1)
                cost = 1;

            this->cache_memoryCache.insert(url, ptr, cost);
        }

        callbacks = this->hash_waitingCallback.take(url);
    }

    if(!pic.isNull())
    {
        QDir().mkpath(this->pos_imageCache);
        QFile file(getFilenameFromUrl(url));
        if(file.open(QIODevice::WriteOnly))
        {
            pic.save(&file, "PNG");
            file.close();
        }
    }

    invokeCallbacks(callbacks, pic);
}

QString ImageCacheManager::getFilenameFromUrl(const QString &url) const
{
    QString hash = QString(QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md5).toHex());
    return this->pos_imageCache + "/" + hash;
}

void ImageCacheManager::invokeCallbacks(const QList<std::function<void (const QPixmap &)> > &callbacks, const QPixmap &pic)
{
    if(QThread::currentThread() == QCoreApplication::instance()->thread())
    {
        for(const auto& cb : callbacks)
        {
            if(cb)
                cb(pic);
        }
    }
    else
    {
        QMetaObject::invokeMethod(QCoreApplication::instance(), [callbacks, pic](){
            for(const auto& cb : callbacks)
            {
                if(cb)
                    cb(pic);
            }
        }, Qt::QueuedConnection);
    }
}

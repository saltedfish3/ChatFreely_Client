#include "imagecachemanager.h"

ImageCacheManager &ImageCacheManager::getManager()
{
    static ImageCacheManager icm;
    return icm;
}

void ImageCacheManager::loadImage(const QString &url, std::function<void (const QPixmap &)> callback, bool failed_notice,
                                  int radius, int padding, QSize size)
{
    //从内存加载
    QPixmap cache;
    if(loadCacheFromMemory(url, cache, radius, padding, size))
    {
        if(callback)
            invokeCallbacks({callback}, cache);
        return;
    }

    this->threadPool.start(new FunctionRunnable([this, callback, failed_notice, radius, padding, size, url](){
        QPixmap cache;
        if(loadCacheFromMemory(url, cache, radius, padding, size))
        {
            if(callback)
                invokeCallbacks({callback}, cache);
            return;
        }

        if(url.startsWith(":/") || url.startsWith("qrc:/"))
        {
            QImage img(url);
            if(img.isNull())
            {
                invokeCallbacks({callback}, QPixmap("qrc:/default/images/defaultAvatar.png"));
                return;
            }

            QPixmap pix = QPixmap::fromImage(img);
            if(radius != 0 || padding != 0 || !size.isEmpty())
            {
                imageTask task;
                task.callback = callback;
                task.radius = radius;
                task.padding = padding;
                task.size = size;

                handleRounded(url, pix, task);
            }
            else
            {
                insertCache(url, pix, radius, padding, size);
                invokeCallbacks({callback}, pix);
            }
            return;
        }

        //内存没有磁盘没有直接标不存在
        if(url.startsWith("local://"))
        {
            QPixmap diskPix;
            if(loadCacheFromDisk(url, diskPix, radius, padding, size))
            {
                if(radius != 0 || padding != 0 || !size.isEmpty())
                    handleRounded(url, diskPix, {radius, padding, size, callback});
                else
                    invokeCallbacks({callback}, diskPix);
                return;
            }

            {
                QString key = getUrlKey(url, radius, padding, size);
                QWriteLocker locker(&(this->rwLock));
                this->hash_imageState[key] = ImageState::NotExist;
            }
            if(callback)
                invokeCallbacks({callback}, QPixmap());
            return;
        }

        //尝试获取原图拓展
        QPixmap pix;
        if(tryLoadOriginalFromCache(url, pix))
        {
            //处理圆角并且调用回调
            handleRounded(url, pix, {radius, padding, size, callback});
            return;
        }

        //未命中缓存
        imageTask task;
        task.callback = callback;
        task.radius = radius;
        task.padding = padding;
        task.size = size;
        QMetaObject::invokeMethod(QCoreApplication::instance(), [this, url, task, failed_notice](){
            loadCacheFromServer(url, task, failed_notice);
        }, Qt::QueuedConnection);
    }));
}

void ImageCacheManager::loadThumbnail(const QString &url, std::function<void (const QPixmap &)> callback)
{
    if(url.isEmpty())
    {
        if(callback)
            callback(QPixmap());
        return;
    }

    QPixmap thumbnail = fastLoadThumbnail(url);
    if(!thumbnail.isNull())
    {
        if(callback)
            callback(thumbnail);
        return;
    }

    loadImage(url, [this, url, callback](const QPixmap& pix){
        if(pix.isNull())
        {
            if(callback)
                callback(pix);
            return;
        }

        insertThumbnail(url, pix);

        if(callback)
            callback(pix);
    }, false, 0, 0, QSize());
}

void ImageCacheManager::insertCache(const QString &url, const QImage &img, int radius, int padding, QSize size)
{
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);

    if(img.hasAlphaChannel())
        img.save(&buffer, "PNG");
    else
        img.save(&buffer, "JPEG", 80);
    buffer.close();

    if(data.isEmpty())
    {
        qWarning() << "insertCache: save pixmap error:" << url;
        return;
    }

    insertCache(url, data, radius, padding, size);
}

void ImageCacheManager::insertCache(const QString &url, const QPixmap &pix, int radius, int padding, QSize size)
{
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);

    if(pix.hasAlphaChannel())
        pix.save(&buffer, "PNG");
    else
        pix.save(&buffer, "JPEG", 80);
    buffer.close();

    if(data.isEmpty())
    {
        qWarning() << "insertCache: save pixmap error:" << url;
        return;
    }

    insertCache(url, data, radius, padding, size);
}

void ImageCacheManager::insertCache(const QString &url, const QByteArray &data, int radius, int padding, QSize size)
{
    QDir().mkpath(this->pos_imageCache);
    QString key = getUrlKey(url, radius, padding, size);

    QFile file(getFilePathFromKey(key));
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(data);
        file.close();
    }
    else
        return;

    QPixmap pix;
    if(pix.loadFromData(data))
    {
        QPixmap* ptr = new QPixmap(pix);
        int cost = pix.toImage().sizeInBytes();
        if(cost < 1)
            cost = 1;

        QWriteLocker locker(&(this->rwLock));
        this->cache_memoryCache.insert(key, ptr, cost);
        this->hash_imageState.insert(key, ImageState::Success);
    }
}

void ImageCacheManager::insertThumbnail(const QString &url, const QPixmap &pix)
{
    if(url.isEmpty() || pix.isNull())
        return;

    putThumbnail(url, pix);

    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    pix.save(&buffer, pix.hasAlphaChannel() ? "PNG" : "JPEG", 80);
    buffer.close();

    if(data.isEmpty())
        return;

    QDir().mkpath(this->pos_imageCache);
    QFile file(this->getFilePathFromKey(getUrlKey(url)));
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(data);
        file.close();
    }
}

void ImageCacheManager::migrateCache(const QString &oldUrl, const QString &newUrl, int radius, int padding, QSize size)
{
    QString oldKey = getUrlKey(oldUrl, radius, padding, size);
    QString newKey = getUrlKey(newUrl, radius, padding, size);

    QString oldPath = getFilePathFromKey(oldKey);
    QString newPath = getFilePathFromKey(newKey);

    if(!QFile::exists(oldPath))
        return;

    if(QFile::copy(oldPath, newPath))
    {
        QFile::remove(oldPath);
        QPixmap newPix;
        if(newPix.load(newPath))
        {
            QPixmap* ptr = new QPixmap(newPix);
            int cost = ptr->toImage().sizeInBytes();
            if(cost < 1)
                cost = 1;

            QWriteLocker locker(&(this->rwLock));
            this->cache_memoryCache.insert(newKey, ptr, cost);
            this->hash_imageState.insert(newKey, ImageState::Success);
        }
    }
    else
    {
        QPixmap pix;
        if(!loadCacheFromMemory(oldUrl, pix, radius, padding, size) && !loadCacheFromDisk(oldUrl, pix, radius, padding, size))
            return;

        insertCache(newUrl, pix, radius, padding, size);
        QFile::remove(oldPath);
    }

    QWriteLocker locker(&(this->rwLock));
    this->cache_memoryCache.remove(oldKey);
    this->hash_imageState.remove(oldKey);
}

void ImageCacheManager::migrateThumbnail(const QString &oldThumbnailUrl, const QString &newThumbnailUrl)
{
    if(oldThumbnailUrl.isEmpty() || newThumbnailUrl.isEmpty() || oldThumbnailUrl == newThumbnailUrl)
        return;

    {
        QWriteLocker locker(&(this->rwLock));
        auto it = this->hash_thumbnails.find(oldThumbnailUrl);
        if(it != this->hash_thumbnails.end())
        {
            QPixmap pix = it.value();
            this->hash_thumbnails.erase(it);
            this->list_thumbLRU.removeAll(oldThumbnailUrl);

            this->hash_thumbnails[newThumbnailUrl] = pix;
            this->list_thumbLRU.append(newThumbnailUrl);
        }
    }

    QString oldPath = getFilePathFromKey(getUrlKey(oldThumbnailUrl));
    QString newPath = getFilePathFromKey(getUrlKey(newThumbnailUrl));

    if(QFile::exists(oldPath))
    {
        if(QFile::exists(newPath))
            QFile::remove(newPath);
        QFile::rename(oldPath, newPath);
    }
}

void ImageCacheManager::removeCache(const QString &url, int radius, int padding, QSize size)
{
    QString key = getUrlKey(url, radius, padding, size);
    {
        QWriteLocker locker(&(this->rwLock));
        this->cache_memoryCache.remove(key);
        this->hash_imageState.remove(key);
    }
    QFile::remove(getFilePathFromKey(key));
}

QString ImageCacheManager::getCacheFilePath(const QString &url, int radius, int padding, QSize size) const
{
    QString key = getUrlKey(url, radius, padding, size);
    QString filePath = getFilePathFromKey(key);
    if(QFile::exists(filePath))
        return filePath;

    return QString();
}

QString ImageCacheManager::getFilenameFromUrl(const QString &url, int radius, int padding, QSize size) const
{
    QString key = getUrlKey(url, radius, padding, size);
    return QString(QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Md5).toHex()) + ".png";
}

bool ImageCacheManager::saveTo(const QString &url, const QString &targetDir)
{
    if(url.isEmpty() || targetDir.isEmpty())
        return false;

    QString cachePath = getCacheFilePath(url);
    if(cachePath.isEmpty())
        return false;

    QString filename = getFilenameFromUrl(url);
    if(filename.isEmpty())
        return false;

    QDir dir(targetDir);
    if(!dir.exists() && !QDir().mkpath(targetDir))
        return false;

    if(QFile::exists(dir.filePath(filename)))
        QFile::remove(dir.filePath(filename));

    return QFile::copy(cachePath, dir.filePath(filename));
}

QPixmap ImageCacheManager::fastLoadImage(const QString &url,int radius, int padding, QSize size)
{
    QPixmap pix;
    if(!loadCacheFromMemory(url, pix, radius, padding, size))
        return QPixmap();

    return pix;
}

QPixmap ImageCacheManager::fastLoadThumbnail(const QString &url)
{
    if(url.isEmpty())
        return QPixmap();

    QWriteLocker locker(&(this->rwLock));
    auto it = this->hash_thumbnails.find(url);
    if(it == this->hash_thumbnails.end())
        return QPixmap();

    this->list_thumbLRU.removeAll(url);
    this->list_thumbLRU.append(url);
    return it.value();
}

void ImageCacheManager::putThumbnail(const QString &url, const QPixmap &pix)
{
    if(pix.isNull() || url.isEmpty())
        return;

    QWriteLocker locker(&(this->rwLock));
    if(this->hash_thumbnails.contains(url))
    {
        this->hash_thumbnails[url] = pix;
        this->list_thumbLRU.removeAll(url);
        this->list_thumbLRU.append(url);
        return;
    }

    while(this->list_thumbLRU.size() >= 3000)
    {
        QString older = this->list_thumbLRU.takeFirst();
        this->hash_thumbnails.remove(older);
    }

    this->hash_thumbnails[url] = pix;
    this->list_thumbLRU.append(url);
}

void ImageCacheManager::removeThumbnail(const QString &url)
{
    QWriteLocker locker(&(this->rwLock));

    if(this->hash_thumbnails.remove(url))
        this->list_thumbLRU.removeAll(url);
}

ImageCacheManager::ImageState ImageCacheManager::getImageState(const QString &url, int radius, int padding, QSize size)
{
    QString key = getUrlKey(url, radius, padding, size);
    QReadLocker locker(&(this->rwLock));
    if(this->hash_imageState.contains(key))
    {
        return this->hash_imageState.value(key);
    }

    return ImageState::Failed;
}

ImageCacheManager::ImageCacheManager(QObject *parent)
    : QObject{parent}
{
    this->pos_imageCache = GlobalVariable::getPosOfImageCache();
    this->cache_memoryCache.setMaxCost(100 * 1024 * 1024);//100MB
    this->threadPool.setMaxThreadCount(4);

    this->hash_thumbnails.reserve(3000);
    this->list_thumbLRU.reserve(3000);
}

bool ImageCacheManager::tryLoadOriginalFromCache(const QString &url, QPixmap &outPixmap)
{
    return loadCacheFromMemory(url, outPixmap, 0, 0, QSize()) || loadCacheFromDisk(url, outPixmap, 0, 0, QSize());
}

bool ImageCacheManager::loadCacheFromMemory(const QString &url, QPixmap &outPixmap, int radius, int padding, QSize size)
{
    QString key = getUrlKey(url, radius, padding, size);
    QReadLocker locker(&(this->rwLock));
    if(this->cache_memoryCache.contains(key))
    {
        outPixmap = *(this->cache_memoryCache.object(key));
        return true;
    }
    return false;
}

bool ImageCacheManager::loadCacheFromDisk(const QString &url, QPixmap &outPixmap, int radius, int padding, QSize size)
{
    QString key = getUrlKey(url, radius, padding, size);
    QString path = getFilePathFromKey(key);
    if(QFile::exists(path))
    {
        QPixmap pix;
        if(!pix.load(path))
        {
            qWarning() << "loadCacheFromDisk failed:" << path;
            return false;
        }

        outPixmap = pix;

        QPixmap* ptr(new QPixmap(pix));
        int cost = pix.toImage().sizeInBytes();
        if(cost < 1)
            cost = 1;

        QWriteLocker locker(&(this->rwLock));
        this->cache_memoryCache.insert(key, ptr, cost);
        this->hash_imageState.insert(key, ImageState::Success);
        return true;
    }
    return false;
}

void ImageCacheManager::loadCacheFromServer(const QString &url, imageTask task, bool failed_notice)
{
    bool needDownload = false;
    {
        QWriteLocker locker(&(this->rwLock));
        if(this->set_waitingUrls.contains(url))
        {
            //若同一图片下载任务存在，只添加等待回调
            this->hash_waitingCallback[url].append(task);
            return;
        }
        else
        {
            this->set_waitingUrls.insert(url);
            this->hash_waitingCallback[url].append(task);

            QTimer* timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setInterval(10000);

            connect(timer, &QTimer::timeout, this, [this, url, task](){
                {
                    QString key = getUrlKey(url, task.radius, task.padding, task.size);
                    QWriteLocker locker(&(this->rwLock));
                    this->hash_imageState[key] = ImageState::Failed;
                }
                handleDownloadFinished(url, QByteArray());
            });

            timer->start();
            this->hash_timeoutTimer[url] = timer;

            needDownload = true;
        }
    }

    if(needDownload)
    {
        QString key = getUrlKey(url, task.radius, task.padding, task.size);
        {
            QWriteLocker locker(&(this->rwLock));
            this->hash_imageState[key] = ImageState::Loading;
        }

        HttpShortConnection::getHttpClient().getImage(url, 3, [this, url, task](const QByteArray& data, HttpShortConnection::ImageError error){
            if(error == HttpShortConnection::ImageError::NotFound)
            {
                QString key = getUrlKey(url, task.radius, task.padding, task.size);
                QWriteLocker locker(&(this->rwLock));
                this->hash_imageState[key] = ImageState::NotExist;
            }
            handleDownloadFinished(url, data);
        }, failed_notice);
    }
}

void ImageCacheManager::handleDownloadFinished(const QString &url, const QByteArray& data)
{
    QList<imageTask> tasks;
    QImage originalImage;
    QTimer* timer = nullptr;
    bool isVaild = false;

    {
        QWriteLocker locker(&(this->rwLock));
        this->set_waitingUrls.remove(url);

        if(this->hash_timeoutTimer.contains(url))
        {
            timer = this->hash_timeoutTimer.take(url);
            timer->stop();
            timer->deleteLater();
        }

        if(!data.isEmpty() && originalImage.loadFromData(data))
            isVaild = true;

        tasks = this->hash_waitingCallback.take(url);
    }

    this->threadPool.start(new FunctionRunnable([this, isVaild, url, data, tasks, originalImage](){
        if(isVaild)
            insertCache(url, data, 0, 0, QSize());//默认标准图

        for(const auto& task : std::as_const(tasks))
        {
            if(!isVaild)
            {
                invokeCallbacks({task.callback}, QPixmap());
                continue;
            }
            if(task.radius == 0 && task.padding == 0 && task.size.isEmpty())
                invokeCallbacks({task.callback}, QPixmap::fromImage(originalImage));
            else
                handleRounded(url, QPixmap::fromImage(originalImage), task);//开始异步画圆角
        }
    }));
}

QString ImageCacheManager::getUrlKey(const QString &url, int radius, int padding, QSize size) const
{
    QString sizeStr = size.isValid() && !size.isEmpty() ? QString("%1x%2").arg(size.width()).arg(size.height()) : QStringLiteral("orig");
    return QString("%1rounded%2_p%3_s%4").arg(url).arg(radius).arg(padding).arg(sizeStr);
}

QString ImageCacheManager::getFilePathFromKey(const QString& key) const
{
    QString hash = QString(QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Md5).toHex());
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

void ImageCacheManager::handleRounded(const QString &url, const QPixmap &pix, imageTask task)
{
    QString key = getUrlKey(url, task.radius, task.padding, task.size);
    {
        QWriteLocker locker(&(this->rwLock));
        if(this->set_handleRounded.contains(key))
        {
            this->hash_roundedCallback[key].append(task.callback);
            return;
        }
        this->set_handleRounded.insert(key);
        this->hash_roundedCallback[key].append(task.callback);
    }

    QImage image = pix.toImage();

    QImage result = addRoundedAndPadding(image, task.radius, task.padding, GlobalVariable::getMaxDevicePixelRatio(), task.size);
    QPixmap resultPix = QPixmap::fromImage(result);

    QList<std::function<void(const QPixmap&)>> callbacks;
    {
        QWriteLocker locker(&(this->rwLock));
        this->set_handleRounded.remove(key);
        callbacks = this->hash_roundedCallback.take(key);
    }
    insertCache(url, result, task.radius, task.padding, task.size);
    invokeCallbacks(callbacks, resultPix);
}

QImage ImageCacheManager::addRoundedAndPadding(const QImage &image, int radius, int padding, qreal dpr, QSize displaySize) const
{
    if(image.isNull())
        return QImage();

    QImage pic = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    QImage content = pic;
    qreal actualDpr = dpr;
    if(displaySize.isValid() && !displaySize.isEmpty())
    {
        QSize want(qRound(displaySize.width()*dpr), qRound(displaySize.height()*dpr));
        if(pic.width() > want.width() && pic.height() > want.height())
        {
            content = pic.scaled(want, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            actualDpr = dpr;
        }
        else
            actualDpr = qMin(static_cast<qreal>(content.width()) / displaySize.width(),
                             static_cast<qreal>(content.height()) / displaySize.height());
    }

    int paddingPhysical = qRound(padding * actualDpr);
    QSize physicalSize = content.size() + QSize(2 * paddingPhysical, 2 * paddingPhysical);

    const int ss = 2;
    QImage mask(physicalSize * ss, QImage::Format_ARGB32_Premultiplied);
    mask.fill(Qt::transparent);
    {
        QPainter maskPainter(&mask);
        maskPainter.setRenderHint(QPainter::Antialiasing);
        QRectF maskRect(paddingPhysical * ss, paddingPhysical * ss,
                        content.width() * ss, content.height() * ss);
        QPainterPath path;
        path.addRoundedRect(maskRect, radius * actualDpr * ss, radius * actualDpr * ss);
        maskPainter.fillPath(path, Qt::black);
    }
    mask = mask.scaled(physicalSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QImage result(physicalSize, QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawImage(QPointF(paddingPhysical, paddingPhysical), content);

    //合成模式
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawImage(0, 0, mask);
    painter.end();

    return result;
}

#include "imagecachemanager.h"

ImageCacheManager &ImageCacheManager::getManager()
{
    static ImageCacheManager icm;
    return icm;
}

void ImageCacheManager::loadImage(const QString &url, std::function<void (const QPixmap &)> callback, bool failed_notice,
                                  qreal dpr, int radius, int padding, QSize size)
{
    dpr = dpr == -1 ? GlobalVariable::getMaxDevicePixelRatio() : dpr;

    //从内存加载
    QPixmap cache;
    if(loadCacheFromMemory(url, cache, dpr, radius, padding, size))
    {
        if(callback)
            invokeCallbacks({callback}, cache);
        return;
    }

    this->threadPool.start(new FunctionRunnable([this, callback, failed_notice, dpr, radius, padding, size, url](){
        QPixmap cache;
        if(loadCacheFromMemory(url, cache, dpr, radius, padding, size))
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
                QString key = getUrlKey(url, dpr, radius, padding, size);
                invokeCallbacks({callback}, QPixmap("qrc:/default/images/defaultAvatar.png"));
                return;
            }

            QPixmap pix = QPixmap::fromImage(img);
            if(radius != 0 || padding != 0 || dpr != 1.0 || !size.isEmpty())
            {
                imageTask task;
                task.callback = callback;
                task.dpr = dpr;
                task.radius = radius;
                task.padding = padding;
                task.size = size;

                handleRounded(url, pix, task);
            }
            else
            {
                insertCache(url, pix, dpr, radius, padding, size);
                invokeCallbacks({callback}, pix);
            }
            return;
        }
        //内存没有磁盘没有直接标不存在
        if(url.startsWith("local://"))
        {
            {
                QString key = getUrlKey(url, dpr, radius, padding, size);
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
            handleRounded(url, pix, {dpr, radius, padding, size, callback});
            return;
        }

        //未命中缓存
        imageTask task;
        task.callback = callback;
        task.dpr = dpr;
        task.radius = radius;
        task.padding = padding;
        task.size = size;
        QMetaObject::invokeMethod(QCoreApplication::instance(), [this, url, task, failed_notice](){
            loadCacheFromServer(url, task, failed_notice);
        }, Qt::QueuedConnection);
    }));
}

void ImageCacheManager::insertCache(const QString &url, const QImage &img, qreal dpr, int radius, int padding, QSize size)
{
    dpr = dpr == -1 ? GlobalVariable::getMaxDevicePixelRatio() : dpr;
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    buffer.close();

    insertCache(url, data, dpr, radius, padding, size);
}

void ImageCacheManager::insertCache(const QString &url, const QPixmap &pix, qreal dpr, int radius, int padding, QSize size)
{
    dpr = dpr == -1 ? GlobalVariable::getMaxDevicePixelRatio() : dpr;
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    pix.save(&buffer, "PNG");
    buffer.close();

    insertCache(url, data, dpr, radius, padding, size);
}

void ImageCacheManager::insertCache(const QString &url, const QByteArray &data, qreal dpr, int radius, int padding, QSize size)
{
    dpr = dpr == -1 ? GlobalVariable::getMaxDevicePixelRatio() : dpr;
    QDir().mkpath(this->pos_imageCache);
    QString key = getUrlKey(url, dpr, radius, padding, size);

    QFile file(getFilenameFromKey(key));
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
        pix.setDevicePixelRatio(dpr);
        QPixmap* ptr = new QPixmap(pix);
        int cost = pix.toImage().sizeInBytes();
        if(cost < 1)
            cost = 1;

        QWriteLocker locker(&(this->rwLock));
        this->cache_memoryCache.insert(key, ptr, cost);
        this->hash_imageState.insert(key, ImageState::Success);
    }
}

void ImageCacheManager::migrateCache(const QString &oldUrl, const QString &newUrl, qreal dpr, int radius, int padding, QSize size)
{
    dpr = dpr == -1 ? GlobalVariable::getMaxDevicePixelRatio() : dpr;
    QString oldKey = getUrlKey(oldUrl, dpr, radius, padding, size);
    QString newKey = getUrlKey(newUrl, dpr, radius, padding, size);

    QString oldPath = getFilenameFromKey(oldKey);
    QString newPath = getFilenameFromKey(newKey);

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
        if(!loadCacheFromMemory(oldUrl, pix, dpr, radius, padding, size) && !loadCacheFromDisk(oldUrl, pix, dpr, radius, padding, size))
            return;

        insertCache(newUrl, pix, dpr, radius, padding, size);
        QFile::remove(oldPath);
    }

    QWriteLocker locker(&(this->rwLock));
    this->cache_memoryCache.remove(oldKey);
    this->hash_imageState.remove(oldKey);
}

void ImageCacheManager::removeCache(const QString &url, qreal dpr, int radius, int padding, QSize size)
{
    dpr = dpr == -1 ? GlobalVariable::getMaxDevicePixelRatio() : dpr;
    QString key = getUrlKey(url, dpr, radius, padding, size);
    {
        QWriteLocker locker(&(this->rwLock));
        this->cache_memoryCache.remove(key);
        this->hash_imageState.remove(key);
    }
    QFile::remove(getFilenameFromKey(key));
}

QString ImageCacheManager::getCacheFilePath(const QString &url, qreal dpr, int radius, int padding, QSize size) const
{
    dpr = dpr == -1 ? GlobalVariable::getMaxDevicePixelRatio() : dpr;
    QString key = getUrlKey(url, dpr, radius, padding, size);
    QString filePath = getFilenameFromKey(key);
    if(QFile::exists(filePath))
        return filePath;

    return QString();
}

QPixmap ImageCacheManager::fastLoadImage(const QString &url, qreal dpr, int radius, int padding, QSize size)
{
    dpr = dpr == -1 ? GlobalVariable::getMaxDevicePixelRatio() : dpr;
    QPixmap pix;
    if(!loadCacheFromMemory(url, pix, dpr, radius, padding, size))
        return QPixmap();
    return pix;
}

ImageCacheManager::ImageState ImageCacheManager::getImageState(const QString &url, qreal dpr, int radius, int padding, QSize size)
{
    dpr = dpr == -1 ? GlobalVariable::getMaxDevicePixelRatio() : dpr;
    QString key = getUrlKey(url, dpr, radius, padding, size);
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
}

bool ImageCacheManager::tryLoadOriginalFromCache(const QString &url, QPixmap &outPixmap)
{
    return loadCacheFromMemory(url, outPixmap, 1.0, 0, 0, QSize()) || loadCacheFromDisk(url, outPixmap, 1.0, 0, 0, QSize());
}

bool ImageCacheManager::loadCacheFromMemory(const QString &url, QPixmap &outPixmap, qreal dpr, int radius, int padding, QSize size)
{
    QString key = getUrlKey(url, dpr, radius, padding, size);
    QReadLocker locker(&(this->rwLock));
    if(this->cache_memoryCache.contains(key))
    {
        outPixmap = *(this->cache_memoryCache.object(key));
        return true;
    }
    return false;
}

bool ImageCacheManager::loadCacheFromDisk(const QString &url, QPixmap &outPixmap, qreal dpr, int radius, int padding, QSize size)
{
    QString key = getUrlKey(url, dpr, radius, padding, size);
    QString path = getFilenameFromKey(key);
    if(QFile::exists(path))
    {
        QPixmap pix;
        if(!pix.load(path))
        {
            QFile::remove(path);
            return false;
        }
        pix.setDevicePixelRatio(dpr);
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
                    QString key = getUrlKey(url, task.dpr, task.radius, task.padding, task.size);
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
        QString key = getUrlKey(url, task.dpr, task.radius, task.padding, task.size);
        {
            QWriteLocker locker(&(this->rwLock));
            this->hash_imageState[key] = ImageState::Loading;
        }

        HttpShortConnection::getHttpClient().getImage(url, 3, [this, url, task](const QByteArray& data, HttpShortConnection::ImageError error){
            if(error == HttpShortConnection::ImageError::NotFound)
            {
                QString key = getUrlKey(url, task.dpr, task.radius, task.padding, task.size);
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
            insertCache(url, data, 1.0, 0, 0, QSize());//默认标准图

        for(const auto& task : std::as_const(tasks))
        {
            if(!isVaild)
            {
                invokeCallbacks({task.callback}, QPixmap());
                continue;
            }
            if(task.radius == 0 && task.padding == 0 && task.dpr == 1.0 && task.size.isEmpty())
                invokeCallbacks({task.callback}, QPixmap::fromImage(originalImage));
            else
                handleRounded(url, QPixmap::fromImage(originalImage), task);//开始异步画圆角
        }
    }));
}

QString ImageCacheManager::getUrlKey(const QString &url, qreal dpr, int radius, int padding, QSize size) const
{
    dpr = dpr == -1 ? GlobalVariable::getMaxDevicePixelRatio() : dpr;
    QString sizeStr = size.isValid() && !size.isEmpty() ? QString("%1x%2").arg(size.width()).arg(size.height()) : QStringLiteral("orig");
    return QString("%1rounded%2_p%3_d%4_s%5").arg(url).arg(radius).arg(padding).arg(dpr, 0, 'f', 3).arg(sizeStr);
}

QString ImageCacheManager::getFilenameFromKey(const QString& key) const
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
    QString key = getUrlKey(url, task.dpr, task.radius, task.padding, task.size);
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

    QImage result = addRoundedAndPadding(image, task.radius, task.padding, task.dpr, task.size);
    QPixmap resultPix = QPixmap::fromImage(result);
    resultPix.setDevicePixelRatio(task.dpr);

    QList<std::function<void(const QPixmap&)>> callbacks;
    {
        QWriteLocker locker(&(this->rwLock));
        this->set_handleRounded.remove(key);
        callbacks = this->hash_roundedCallback.take(key);
    }
    insertCache(url, result, task.dpr, task.radius, task.padding, task.size);
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

    result.setDevicePixelRatio(actualDpr);
    return result;
}

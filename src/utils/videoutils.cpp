#include "videoutils.h"
#include "GlobalVariable.h"

namespace
{
    struct Context : public QObject
    {
        QMediaPlayer* player = nullptr;
        QAudioOutput* audio = nullptr;
        QVideoSink* sink = nullptr;
        QTimer* timeout = nullptr;

        QString filePath;
        qint64 positionMs = 1000;
        QList<std::function<void(VideoUtils::VideoInfo)>> callbacks;
        bool isFinished = false;

        void finish(VideoUtils::VideoInfo info)
        {
            if(isFinished)
                return;
            isFinished = true;

            if(timeout && timeout->isActive())
                timeout->stop();

            for(auto& cb : callbacks)
            {
                if(cb)
                    cb(info);
            }

            this->deleteLater();
        }
    };

    QHash<QString, Context*> waitingContexts;
}

void VideoUtils::extractAsync(const QString &filePath, std::function<void (const VideoInfo &)> callback, qint64 posistionMs, int timeoutMs)
{
    if(!QFileInfo::exists(filePath))
    {
        VideoInfo info;
        info.valid = false;
        if(callback)
            callback(info);
        return;
    }

    auto it = waitingContexts.find(filePath);
    if(it != waitingContexts.end())
    {
        it.value()->callbacks.append(callback);
        return;
    }

    auto* arg = new Context();
    arg->filePath = filePath;
    arg->positionMs = posistionMs;
    arg->callbacks.append(callback);
    arg->player = new QMediaPlayer(arg);
    arg->audio = new QAudioOutput(arg);
    arg->sink = new QVideoSink(arg);
    arg->player->setAudioOutput(arg->audio);
    arg->player->setVideoSink(arg->sink);

    arg->audio->setMuted(true);
    arg->timeout = new QTimer(arg);
    arg->timeout->setSingleShot(true);
    arg->timeout->setInterval(timeoutMs);

    QObject::connect(arg->timeout, &QTimer::timeout, arg->timeout, [arg](){
        qWarning() << "获取视频帧数据超时:" << arg->filePath;
        waitingContexts.remove(arg->filePath);
        arg->finish({});
    });

    QObject::connect(arg->sink, &QVideoSink::videoFrameChanged, arg->sink, [arg](const QVideoFrame& frame){
        if(arg->isFinished)
            return;
        if(!frame.isValid())
            return;

        QImage image = frame.toImage();
        if(image.isNull())
            return;

        VideoInfo info;
        info.valid = true;
        info.width = image.width();
        info.height = image.height();
        info.duration = arg->player->duration();

        if(image.width() > 200 || image.height() > 200)
            image = image.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        image = image.convertToFormat(QImage::Format_RGB888);
        info.thumbnail = image;

        arg->player->pause();
        waitingContexts.remove(arg->filePath);
        arg->finish(info);
    });

    QObject::connect(arg->player, &QMediaPlayer::mediaStatusChanged, arg->player, [arg](QMediaPlayer::MediaStatus status){
        if(arg->isFinished)
            return;

        if(status == QMediaPlayer::LoadedMedia)
        {
            qint64 duration = arg->player->duration();

            qint64 target = arg->positionMs;
            if(duration > 0 && duration < target)
                target = qMax<qint64>(0, duration / 2);

            arg->player->setPosition(target);
            arg->player->play();
        }
        else if(status == QMediaPlayer::InvalidMedia)
        {
            qWarning()<< "播放错误:" << arg->player->errorString();
            waitingContexts.remove(arg->filePath);
            arg->finish({});
        }
    });

    QObject::connect(arg->player, &QMediaPlayer::errorOccurred, arg->player, [arg](QMediaPlayer::Error err, const QString& errStr){
        if(arg->isFinished)
            return;
        qWarning() << "播放错误:" << err << errStr;
        waitingContexts.remove(arg->filePath);
        arg->finish({});
    });

    waitingContexts[filePath] = arg;
    arg->timeout->start();
    arg->player->setSource(QUrl::fromLocalFile(filePath));
}

QString VideoUtils::getLocalUrlPath(const QString &url)
{
    if(url.isEmpty())
        return {};

    QString dir = GlobalVariable::getPosOfDownloadFile();
    if(dir.isEmpty())
        return QString();

    QUrl url_(url);
    QString filename = QFileInfo(url_.path()).fileName();

    if(filename.isEmpty())
        return {};

    if(QFileInfo(filename).suffix().isEmpty())
        filename += ".mp4";

    return QDir(dir).filePath(filename);
}

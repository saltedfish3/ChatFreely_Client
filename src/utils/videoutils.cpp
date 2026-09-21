#include "videoutils.h"

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
        std::function<void(VideoUtils::VideoInfo)> callback;
        bool isFinished = false;

        void finish(VideoUtils::VideoInfo info)
        {
            if(isFinished)
                return;
            isFinished = true;

            if(timeout && timeout->isActive())
                timeout->stop();

            if(callback)
                callback(info);

            this->deleteLater();
        }
    };
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

    auto* arg = new Context();
    arg->filePath = filePath;
    arg->positionMs = posistionMs;
    arg->callback = callback;
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
        info.thumbnail = image;

        arg->player->pause();
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
            arg->finish({});
        }
    });

    QObject::connect(arg->player, &QMediaPlayer::errorOccurred, arg->player, [arg](QMediaPlayer::Error err, const QString& errStr){
        if(arg->isFinished)
            return;
        qWarning() << "播放错误:" << err << errStr;
        arg->finish({});
    });

    arg->timeout->start();
    arg->player->setSource(QUrl::fromLocalFile(filePath));
}

#ifndef VIDEOUTILS_H
#define VIDEOUTILS_H

#include <QObject>
#include <QMediaPlayer>
#include <QTimer>
#include <QAudioOutput>
#include <QVideoSink>
#include <QFileInfo>
#include <QImage>
#include <QVideoFrame>

class VideoUtils : public QObject
{
    Q_OBJECT
public:
    struct VideoInfo
    {
        bool valid = false;
        int width = 0;
        int height = 0;
        qint64 duration = 0;
        QImage thumbnail;

        void clear()
        {
            *this = VideoInfo();
        }
    };
    static void extractAsync(const QString& filePath, std::function<void(const VideoInfo&)> callback, qint64 posistionMs = 1000, int timeoutMs = 15000);

signals:
};

#endif // VIDEOUTILS_H

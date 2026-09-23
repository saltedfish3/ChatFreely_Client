#ifndef VIDEOPREVIEWWIDGET_H
#define VIDEOPREVIEWWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoSink>
#include <QVideoFrame>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include "imageview.h"
#include "closebutton.h"
#include "minimizebutton.h"
#include "titlebarwidget.h"
#include "videocontentview.h"

class VideoPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    enum class Edge
    {
        Left,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        None
    };
    static VideoPreviewWidget& getPreviewWidget();
    VideoPreviewWidget& operator=(const VideoPreviewWidget&) = delete;
    VideoPreviewWidget(const VideoPreviewWidget&) = delete;

    void setVideoUrl(const QUrl& url);

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* obj,QEvent* ev) override;
    void resizeEvent(QResizeEvent* event) override;

signals:

private:
    explicit VideoPreviewWidget(int width = 900, int height = 600, QWidget *parent = nullptr);
    VideoPreviewWidget::Edge edgeAt(const QPoint& pos);
    void updateCursor(Edge edge);

    Edge edge = Edge::None;
    QPoint pos_startScaleGlobal;
    QRect rect_startGeometry;

    TitleBarWidget* widget_titleBar;
    VideoContentView* view_video;

    QMediaPlayer* player;
    QAudioOutput* output;
    QVideoSink* sink;

    QPoint pos_widget;

    QAtomicInt isFrameBusy{0};
    QSize currentFrameSize;
};

#endif // VIDEOPREVIEWWIDGET_H

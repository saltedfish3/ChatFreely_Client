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
#include <QMediaMetaData>
#include <QtConcurrent/QtConcurrent>
#include "imageview.h"
#include "closebutton.h"
#include "minimizebutton.h"
#include "titlebarwidget.h"
#include "videocontentview.h"
#include "savebutton.h"
#include "toastmanager.h"
#include "../network/httpshortconnection.h"
#include "../network/tcplongconnection.h"

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

    void setVideoUrl(const QUrl& url, qint64 totalSize = 0);

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* obj,QEvent* ev) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

signals:

private:
    explicit VideoPreviewWidget(int width = 900, int height = 600, QWidget *parent = nullptr);
    VideoPreviewWidget::Edge edgeAt(const QPoint& pos);
    void updateCursor(Edge edge);
    void updateSaveButton();
    void trySetVideoSize();
    QString getSavePath();

    Edge edge = Edge::None;
    QPoint pos_startScaleGlobal;
    QRect rect_startGeometry;

    TitleBarWidget* widget_titleBar;
    VideoContentView* view_video;
    SaveButton* btn_save;
    QString currentVideoUrl;
    qint64 currentVideoSize = 0;

    QMediaPlayer* player;
    QAudioOutput* output;

    QPoint pos_widget;
};

#endif // VIDEOPREVIEWWIDGET_H

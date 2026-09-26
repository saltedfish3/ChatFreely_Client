#ifndef VIDEOCONTENTVIEW_H
#define VIDEOCONTENTVIEW_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QOpenGLWidget>
#include "videocontrolbar.h"
#include "../utils/GlobalVariable.h"

class VideoContentView : public QGraphicsView
{
    Q_OBJECT
public:
    enum class State
    {
        None,
        Loading,
        Playing,
        Paused,
        Error
    };

    explicit VideoContentView(QMediaPlayer* player, QAudioOutput* output, QWidget *parent = nullptr);

    void setState(State state);
    State getState() const;
    void setErrorText(const QString& text);
    void setVideoNativeSize(const QSize& size);

signals:
    void clicked();

protected:
    void drawForeground(QPainter* painter, const QRectF& rect) override;
    void resizeEvent(QResizeEvent *event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;


private:
    void updateViewTransform();
    void updateMask();
    void updateControlBarGeometry();

    QMediaPlayer* player;
    QAudioOutput* output;
    QGraphicsScene* scene;
    QGraphicsVideoItem* item_video;

    QSize videoNativeSize;
    State currentState = State::None;
    QString errorText;
    QPoint pressPos = QPoint(-1, -1);

    VideoControlBar* bar;
};

#endif // VIDEOCONTENTVIEW_H

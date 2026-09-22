#ifndef VIDEOCONTENTVIEW_H
#define VIDEOCONTENTVIEW_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QtOpenGLWidgets/QtOpenGLWidgets>

class VideoContentView : public QOpenGLWidget
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

    explicit VideoContentView(QWidget *parent = nullptr);

    void setState(State state);
    State getState() const;
    void setFrame(const QImage& frame);
    void setErrorText(const QString& text);
    void clearFrame();

signals:
    void clicked();

protected:
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void rebuildPath();

    State currentState = State::None;
    QString errorText;

    QPoint pressPos = QPoint(-1, -1);

    QImage frame;

    //缓存路径
    QPainterPath path;
};

#endif // VIDEOCONTENTVIEW_H

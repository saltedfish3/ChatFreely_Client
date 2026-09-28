#ifndef VIDEOCONTROLBAR_H
#define VIDEOCONTROLBAR_H

#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QLayout>
#include <QToolButton>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QTimer>
#include "ratewidget.h"

class VideoControlBar : public QWidget
{
    Q_OBJECT
public:
    explicit VideoControlBar(QMediaPlayer* player, QAudioOutput* output, QWidget *parent = nullptr);

signals:
    void transAreaClicked();
    void userActivity();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    void mouseMoveEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;

private:
    QString formatTime(qint64 ms);

    void initStyle();
    void updateAudioSlierPosistion();
    QIcon colorToIcon(const QIcon& icon, const QColor& color, const QSize& size = QSize(18, 18));

    QMediaPlayer* player;
    QAudioOutput* output;

    QHBoxLayout* layout;
    QToolButton* btn_play;
    QToolButton* btn_audio;
    QToolButton* btn_rate;
    RateWidget* menu_rate;

    QLabel* label_currentTime;
    QLabel* label_totalTime;

    QSlider* slider_video;
    QSlider* slider_audio;
    int lastVolume = 100;
    QTimer* timer_audioSlider;

    QWidget* widget_control;

    QPoint pressPos = QPoint(-1, -1);
};

#endif // VIDEOCONTROLBAR_H

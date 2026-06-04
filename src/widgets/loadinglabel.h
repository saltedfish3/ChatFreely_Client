#ifndef LOADINGLABEL_H
#define LOADINGLABEL_H

#include <QWidget>
#include <QPainter>

class LoadingLabel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal angle READ angle WRITE setAngle)
public:
    explicit LoadingLabel(QWidget *parent = nullptr);

    qreal angle() const;
    void setAngle(qreal angle);

    void setPixmap(const QPixmap &pixmap);
signals:

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal m_angle = 0;
    QPixmap m_pix;
};

#endif // LOADINGLABEL_H

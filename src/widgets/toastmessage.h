#ifndef TOASTMESSAGE_H
#define TOASTMESSAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QRgb>

class ToastMessage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    enum Type
    {
        Success, Error, Info, Warning
    };

    explicit ToastMessage(Type type, const QString& msgs, QWidget *parent = nullptr);
    qreal opacity() const;
    void setOpacity(qreal val);
    void showToast();

signals:

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    qreal m_opacity;
    QLabel* icon;
    QLabel* msg;
    QPropertyAnimation* fadeIn;
    QPropertyAnimation* fadeOut;
    QTimer* autoClose;

    QColor bgColor;
    QColor borderColor;
};

#endif // TOASTMESSAGE_H

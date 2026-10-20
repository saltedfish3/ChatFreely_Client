#ifndef MOREWIDGET_H
#define MOREWIDGET_H

#include <QWidget>
#include <QFrame>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>

class MoreWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MoreWidget(bool isFriend = true, QWidget *parent = nullptr);

signals:
    void closed();
    void switchToInfoWidget();

protected:
    void focusOutEvent(QFocusEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void initMore(bool isFriend);
    void initStyle();
};

#endif // MOREWIDGET_H

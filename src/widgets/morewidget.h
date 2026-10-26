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
    explicit MoreWidget(bool isFriend = true, bool isOwner = false, QWidget *parent = nullptr);
    void setIsGroupOwner(bool isOwner);

signals:
    void closed();
    void switchToInfoWidget();
    void clearMessages();
    void deletedConversation();

protected:
    void focusOutEvent(QFocusEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void initMore(bool isFriend, bool isOwner);
    void initStyle();
};

#endif // MOREWIDGET_H

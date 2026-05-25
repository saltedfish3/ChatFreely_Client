#ifndef CONTACTSINFOWIDGET_H
#define CONTACTSINFOWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

class ContactsInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContactsInfoWidget(int width, int height, QWidget *parent = nullptr);

signals:

private:
    void init();
    void initStyle();

    void setRadius(QIcon pic, QLabel* label, int hei_wid);

    QLabel* label_avatar;
    QLabel* label_username;
    QLabel* label_userID;
    QLabel* label_status_icon;
    QLabel* label_status;

    QPushButton* btn_sendMsg;
    QLabel* label_sendMsg_icon;
    QPushButton* btn_deleteFriend;
    QLabel* label_deleteFriend;

    QWidget* widget_emailRegion;
    QLabel* label_emailChinese;
    QLabel* label_email;
};

#endif // CONTACTSINFOWIDGET_H

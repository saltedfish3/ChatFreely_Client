#ifndef CONTACTSINFOWIDGET_H
#define CONTACTSINFOWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>

class ContactsInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContactsInfoWidget(int width, int height, QWidget *parent = nullptr);
    void changeInfo(QPixmap avatar, QString username, QString uid, QString sid, QString email, bool isOnline);
    void changeOnlineStatus(bool isOnline);

signals:
    void openConversation(const QString &uid, const QString &username, const QPixmap &avatar, bool isOnline);
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

    QString username;
    QString sid;
    QString email;
    QString uid;
    QPixmap avatar;
    bool isOnline = false;
};

#endif // CONTACTSINFOWIDGET_H

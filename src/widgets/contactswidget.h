#ifndef CONTACTSWIDGET_H
#define CONTACTSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QIcon>
#include <QStackedWidget>
#include <QStandardItem>
#include <QPropertyAnimation>
#include <QList>
#include <QCollator>
#include <QScrollBar>
#include "newfriendbutton.h"
#include "newfriendmanagewidget.h"
#include "addnewfriendwidget.h"
#include "contactsinfowidget.h"
#include "frienddelegate.h"
#include "loadinglabel.h"

class ContactsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContactsWidget(int width, int height, QWidget *parent = nullptr);
    void addFriendItem(QString uid, QString sid, QString username, QString avatar_url, QString email, bool isOnline);

signals:

private:
    enum Status
    {
        Loading = 0,
        Empty,
        Hide
    };
    struct FriendInfo
    {
        QString uid;
        QString sid;
        QString username;
        QString avatar_url;
        QString email;
        bool isOnline;
    };

    void initSideBar();
    void initSideBarStyle();
    void initSideUn();
    void initSideUnStyle();

    void initStacked();

    void changeStatus(Status status);
    void sortedFriendList();
    void searchFriend(const QString& text);

    QString getFirstLetter(const QString& str);

    QWidget* widget_sideBar;
    QWidget* widget_sideUn;
    QWidget* widget_noSelect;
    NewFriendManageWidget* widget_newFriend;
    ContactsInfoWidget* widget_contactsInfo;

    QPushButton* btn_addFriend;
    QLineEdit* edit_searchFriend;

    NewFriendButton* btn_newFriend;
    QLabel* label_newFriend_icon;

    QLabel* label_myFriend;
    QLabel* label_noSelect;

    QListView* listView_myFriend;
    QStandardItemModel* model;
    FriendDelegate* delegate;

    QWidget* widget_status;
    QLabel* label_empty;
    QLabel* label_loading;
    LoadingLabel* label_loadingIcon;
    QPropertyAnimation* animation_loading;

    QList<FriendInfo> list_friend;

    QStackedWidget* stackedWidget;
};

#endif // CONTACTSWIDGET_H

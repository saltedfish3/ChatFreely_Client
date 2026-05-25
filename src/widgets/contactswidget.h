#ifndef CONTACTSWIDGET_H
#define CONTACTSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QIcon>
#include <QStackedWidget>
#include "newfriendbutton.h"
#include "newfriendmanagewidget.h"
#include "contactsinfowidget.h"

class ContactsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContactsWidget(int width, int height, QWidget *parent = nullptr);

signals:

private:
    void initSideBar();
    void initSideBarStyle();
    void initSideUn();
    void initSideUnStyle();

    void initStacked();

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

    QStackedWidget* stackedWidget;
};

#endif // CONTACTSWIDGET_H

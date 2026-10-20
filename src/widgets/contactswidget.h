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
#include <QButtonGroup>
#include "newfriendbutton.h"
#include "newfriendmanagewidget.h"
#include "addnewfriendwidget.h"
#include "contactsinfowidget.h"
#include "frienddelegate.h"
#include "loadinglabel.h"
#include "../utils/friendmanage.h"
#include "groupdelegate.h"
#include "../utils/groupmanager.h"
#include "groupinfowidget.h"

class ContactsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContactsWidget(int width, int height, QWidget *parent = nullptr);
    void addFriendItem(QString uid, QString sid, QString username, QPixmap avatar, QString email, bool isOnline);
    void addGroupItem(const QString& conversationID, const QString& groupID, const QString& groupName,
                      const QString& ownerUID, const QHash<QString, GroupManager::GroupMemberInfo>& memberInfos, const QPixmap& avatar);
    void switchToInfoWidget(const QString& conversaionID);

signals:
    void openConversation(const QString &conversationID);

private:
    enum Status
    {
        Loading = 0,
        Empty,
        Hide
    };

    void initSideBar();
    void initSideBarStyle();
    void initSideUn();
    void initSideUnStyle();

    void initStacked();

    void changeStatus(Status status);
    void sortedFriendList();
    void sortedGroupList();
    void searchFriend(const QString& text);
    void searchGroup(const QString& text);
    void restoreSelect();

    QString getFirstLetter(const QString& str);

    QWidget* widget_sideBar;
    QWidget* widget_sideUn;
    QWidget* widget_noSelect;
    NewFriendManageWidget* widget_newFriend;
    ContactsInfoWidget* widget_contactsInfo;
    GroupInfoWidget* widget_groupInfo;

    QPushButton* btn_addFriend;
    QLineEdit* edit_searchFriend;

    NewFriendButton* btn_newFriend;
    QLabel* label_newFriend_icon;

    QButtonGroup* group;
    QPushButton* btn_myFriend;
    QPushButton* btn_myGroups;
    QLabel* label_noSelect;

    QListView* listView_myFriend;
    QStandardItemModel* model;
    FriendDelegate* delegate;

    QStandardItemModel* model_group;
    GroupDelegate* delegate_group;

    QWidget* widget_status;
    QLabel* label_empty;
    QLabel* label_loading;
    LoadingLabel* label_loadingIcon;
    QPropertyAnimation* animation_loading;

    //操作好友列表
    QString selectedUID;

    QStackedWidget* stackedWidget;
};

#endif // CONTACTSWIDGET_H

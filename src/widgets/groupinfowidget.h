#ifndef GROUPINFOWIDGET_H
#define GROUPINFOWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QScrollArea>
#include <QListView>
#include <QStandardItemModel>
#include <QScrollBar>
#include <QLineEdit>
#include <QFileDialog>
#include "../utils/imagecachemanager.h"
#include "groupmemberdelegate.h"
#include "../utils/groupmanager.h"
#include "../network/tcplongconnection.h"
#include "../utils/friendmanage.h"
#include "avatarview.h"

class GroupInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GroupInfoWidget(int width, int height, QWidget *parent = nullptr);
    void changeInfo(const QString& conversationID, const QString& groupID, const QString& groupName,
                    const QString& ownerUID, const QHash<QString, GroupManager::GroupMemberInfo>& memberInfos, const QPixmap& pix);
    void changeSelectedGroupName(const QString& groupName);
    void changeSelectedAvatar(const QPixmap& avatar);
    void changeMemberAvatar(const QString& uid, const QPixmap& avatar);

signals:
    void openConversation(const QString &conversationID);
    void exitGroup(const QString& conversationID);

private:
    void init();
    void initStyle();

    void setRadius(QPixmap& pix, AvatarView* view, int radius);
    void setRadiusOnLabel(QPixmap& pix, QLabel* label, int radius);
    void addGroupMemberItem(const QString& username, const QString& sid, const QString& uid, const QPixmap& avatar);
    void updateIsFriend();
    void updateEditPos();

    QScrollArea* area;
    QWidget* widget_holy;

    // QLabel* label_avatar;
    AvatarView* view_avatar;
    QLabel* label_camera_icon;

    QLineEdit* edit_groupName;
    QPushButton* btn_doneOrEdit;
    QLabel* label_groupID;

    QPushButton* btn_sendMsg;
    QLabel* label_sendMsg_icon;
    QPushButton* btn_exitGroup;
    QLabel* label_exitGroup;

    QWidget* widget_memberRegion;

    QLabel* label_ownerName;
    QLabel* label_ownerAvatar;
    QLabel* label_ownerUsername;
    QLabel* label_ownerSID;
    QLabel* label_memberName;

    QListView* view;
    QStandardItemModel* model;
    GroupMemberDelegate* delegate;

    QPushButton* btn_addFriend;

    QString conversationID;
    QString currentGroupName;
    int64_t currentMemberCount;
};

#endif // GROUPINFOWIDGET_H

#ifndef NEWFRIENDMANAGEWIDGET_H
#define NEWFRIENDMANAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QListView>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QMetaObject>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QScrollBar>
#include <QPersistentModelIndex>
#include "friendapplydelegate.h"
#include "../network/tcplongconnection.h"
#include "../network/httpshortconnection.h"
#include "../utils/userinfo.h"
#include "loadinglabel.h"

class NewFriendManageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NewFriendManageWidget(int width, int height, QWidget *parent = nullptr);
    void addFriendRequestsItem(QString uid, QString sid, QString username, QString avatar_url, QString verMsg);
    void addGroupRequestsItem(const QString& requestID, const QString& groupName, int source,
                              const QString& displayName, const QString& displayAvatarUrl,
                              const QString& verMsg, const QString& applicantSID, const QString& inviterName);


signals:
    void RequestsNumberChange(size_t number);

private:
    enum Status
    {
        Loading = 0,
        Empty,
        Hide
    };
    void initWidget();
    void initstyle();
    void changeStatus(Status status);

    QLabel* label_friendApply;
    LoadingLabel* label_loadingIcon;
    QLabel* label_loading;
    QLabel* label_empty;

    QWidget* widget_friendApply;
    QWidget* widget_status;

    QPushButton* btn_refresh;
    QPropertyAnimation* animation_loading;

    QListView* listView_friendApplyManage;
    QStandardItemModel* model;
    FriendApplyDelegate* delegate;

    QString handle_uid;

    size_t unProcessedRequests = 0;
};

#endif // NEWFRIENDMANAGEWIDGET_H

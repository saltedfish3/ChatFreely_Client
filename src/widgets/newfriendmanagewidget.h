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
    void addRequestsItem(QString uid, QString sid, QString username, QString avatar_url, QString verMsg);

signals:

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

    QPersistentModelIndex waitingHandleIndex;
};

#endif // NEWFRIENDMANAGEWIDGET_H

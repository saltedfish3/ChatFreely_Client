#ifndef NEWFRIENDMANAGEWIDGET_H
#define NEWFRIENDMANAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QListView>
#include <QStandardItemModel>
#include <QModelIndex>
#include "friendapplydelegate.h"
#include "../network/tcplongconnection.h"
#include "../utils/userinfo.h"

class NewFriendManageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NewFriendManageWidget(int width, int height, QWidget *parent = nullptr);
    void addRequestsItem(QString uid, QString sid, QString username, QString avatar_url, QString verMsg);

signals:

private:
    void initWidget();
    void initstyle();

    QLabel* label_friendApply;

    QWidget* widget_friendApply;

    QListView* listView_friendApplyManage;
    QStandardItemModel* model;
    FriendApplyDelegate* delegate;

    QString handle_uid;
};

#endif // NEWFRIENDMANAGEWIDGET_H

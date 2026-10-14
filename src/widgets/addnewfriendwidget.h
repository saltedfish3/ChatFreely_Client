#ifndef ADDNEWFRIENDWIDGET_H
#define ADDNEWFRIENDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QRegularExpressionValidator>
#include <QButtonGroup>
#include <QListView>
#include <QStandardItemModel>
#include <QScrollBar>
#include <QItemSelectionModel>
#include "frienddelegate.h"
#include "toastmanager.h"
#include "../network/tcplongconnection.h"
#include "../utils/friendmanage.h"

class AddNewFriendWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AddNewFriendWidget(QWidget *caller);

    static QWidget* findRootWidget(QWidget *caller);
signals:

private:
    enum Mode
    {
        AddFriend,
        JoinGroup,
        CreateGroup
    };
    void initWidget();
    void initStyle();
    void changeMode(Mode mode);

    QWidget* widget_card;

    QLabel* label_bigTitle;
    QLabel* label_smallTitle;
    QLabel* label_UserIDOrEmail;
    QLabel* label_verMsg;
    QLabel* label_atIcon;

    QLineEdit* edit_IDOrEmail;
    QLineEdit* edit_verMsg;
    QRegularExpressionValidator* regular_email;
    QRegularExpressionValidator* regular_password;

    QPushButton* btn_cancel;
    QPushButton* btn_add;

    QButtonGroup* group;
    QPushButton* btn_addPersonal;
    QPushButton* btn_joinGroup;
    QPushButton* btn_createGroup;

    QListView* view_inviteFriend;
    QStandardItemModel* model;
    FriendDelegate* delegate;

    Mode currentMode = AddFriend;
};

#endif // ADDNEWFRIENDWIDGET_H

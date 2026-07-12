#ifndef ADDNEWFRIENDWIDGET_H
#define ADDNEWFRIENDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QRegularExpressionValidator>
#include "toastmanager.h"
#include "../network/tcplongconnection.h"

class AddNewFriendWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AddNewFriendWidget(QWidget *caller);

    static QWidget* findRootWidget(QWidget *caller);
signals:
    void add();

private:
    void initWidget();
    void initStyle();

    QWidget* widget_card;

    QLabel* label_bigTitle;
    QLabel* label_smallTitle;
    QLabel* label_UserIDOrEmail;
    QLabel* label_atIcon;

    QLineEdit* edit_IDOrEmail;
    QRegularExpressionValidator* regular_email;
    QRegularExpressionValidator* regular_password;

    QPushButton* btn_cancel;
    QPushButton* btn_add;
};

#endif // ADDNEWFRIENDWIDGET_H

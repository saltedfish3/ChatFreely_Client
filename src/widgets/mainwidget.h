#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QButtonGroup>
#include "chatwidget.h"
#include "contactswidget.h"
#include "settingwidget.h"
#include "toastmanager.h"

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(int width, int height, QWidget *parent = nullptr);
    void changeToChat();

signals:

private slots:
    void buttonColorChange(QAbstractButton* button);
    void handleMainState(bool isSuccess, QString info);

private:
    void initSideBar();
    void initSideBarDefaultStyle();

    void initStackedChat();

    void initPage();

    void setRadius(QIcon pic, QLabel *label, int hei_wid);

    //--------------------------------
    QWidget* widget_sideBar;
    ChatWidget* widget_chat;
    ContactsWidget* widget_contacts;
    SettingWidget* widget_setting;

    QLabel* label_avatar;
    QLabel* label_chats;
    QLabel* label_contacts;
    QLabel* label_setting;

    QHash<QString, QPushButton*> sideBarButton;
    QButtonGroup* sideBarGroup;
    // QPushButton* btn_chats;
    // QPushButton* btn_contacts;
    // QPushButton* btn_setting;

    //--------------------------------
    QStackedWidget* stackedWidget_Chat;

};

#endif // MAINWIDGET_H

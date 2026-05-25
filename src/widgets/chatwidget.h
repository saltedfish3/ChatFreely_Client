#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QStackedWidget>
#include <QACtion>
#include <QPlainTextEdit>
#include <QListView>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QVBoxLayout>
#include "morewidget.h"


class ChatWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWidget(int width, int height, QWidget *parent = nullptr);

signals:

private slots:

private:
    void initSearchWidget();
    void initSearchStyle();

    void initListWidget();
    void initListStyle();

    void addConversation();
    void initConversationStyle(QWidget* conversation);
    void initMoreMenu(QWidget* conversation,QPushButton* more);

    void initStackedConversation();
    // void initStackedConversationStyle();
    //--------------------------------
    QAction* searchIcon;
    QLineEdit* edit_search;

    //--------------------------------
    QListWidget* listWidget_userList;

    QWidget* widget_search;
    QWidget* widget_header;
    QListView* listView_messages;
    QWidget* widget_input;

    QStackedWidget* stackedWidget_Conversation;
};

#endif // CHATWIDGET_H

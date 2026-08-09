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
#include <QStandardItemModel>
#include <QScrollBar>
#include <QPointer>
#include "morewidget.h"
#include "../network/tcplongconnection.h"
#include "../network/httpshortconnection.h"
#include "conversationlistdelegate.h"
#include "conversationwidget.h"
#include "../utils/friendmanage.h"
#include "../chat/conversationmanager.h"
#include "../chat/conversationitem.h"


class ChatWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWidget(int width, int height, QWidget *parent = nullptr);
    void openConversation(const QString &uid);

signals:

private slots:
    void cleanAll();

private:
    void initSearchWidget();
    void initSearchStyle();

    void initListWidget();
    void initListStyle();

    void initStackedConversation();
    void moveConversationToTop(const QString& conversationID);

    ConversationWidget* createConversation(const QString& conversationID);
    void createConversationListItem(ConversationItem* item, const DatabaseManager::ConversationInfo& info = {});

    //--------------------------------
    QAction* searchIcon;
    QLineEdit* edit_search;

    //--------------------------------
    QListView* listView_conversationList;
    QStandardItemModel* model;
    ConversationListDelegate* delegate;

    QWidget* widget_noSelect;
    QLabel* label_appIcon;

    QWidget* widget_search;

    QStackedWidget* stackedWidget_Conversation;

    QMap<QString, ConversationWidget*> map_conversation;
};

#endif // CHATWIDGET_H

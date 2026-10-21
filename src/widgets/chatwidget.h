#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QStackedWidget>
#include <QAction>
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
#include "../utils/globalinitcontroller.h"
#include "searchresultlistdelegate.h"


class ChatWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWidget(int width, int height, QWidget *parent = nullptr);
    void openConversation(const QString &conversationID);

signals:
    void switchToInfoWidget(const QString& conversationID);

private slots:
    void cleanAll();

private:
    void initSearchWidget();
    void initSearchStyle();
    void initSearchModels();

    void initListWidget();
    void initListStyle();

    void bindConversationItemSignals(ConversationItem* item);

    void initStackedConversation();
    void precreateConversations();

    void restoreSelection(const QString& conversationID);

    void loadSearchConvFirstPage(quint64 gen, const QString& keyword);
    void loadSearchConvNextPage(quint64 gen);
    void appendSearchConvItem(const DatabaseManager::SearchConvResult& result);

    void loadSearchMsgFirstPage(quint64 gen);
    void loadSearchMsgNextPage(quint64 gen);
    void appendSearchMsgItem(const Message& msg);
    void openSearchMsgView(const QString& conversationID);

    ConversationWidget* createConversation(const QString& conversationID);
    void createConversationListItem(ConversationItem* item, const DatabaseManager::ConversationInfo& info = {});

    //--------------------------------
    QAction* searchIcon;
    QLineEdit* edit_search;

    //--------------------------------
    QListView* listView_conversationList;
    QStandardItemModel* model;
    QStandardItemModel* model_searchConversation;
    QStandardItemModel* model_searchMsg;
    ConversationListDelegate* delegate;
    SearchResultListDelegate* delegate_search;

    QTimer* timer_search;
    quint64 searchGen = 0;
    QString currentSearchConversationID;

    bool isLoadingSearchConversation = false;
    bool isLoadingSearchMsg = false;

    int pos_scroll = 0;

    QWidget* widget_noSelect;
    QLabel* label_appIcon;

    QWidget* widget_search;

    QStackedWidget* stackedWidget_Conversation;

    QMap<QString, ConversationWidget*> map_conversation;

    QString currentConversation;

    //防止信号重复绑定
    QSet<QString> boundConversations;
    //强制创建列表项
    QSet<QString> forceConversations;
};

#endif // CHATWIDGET_H

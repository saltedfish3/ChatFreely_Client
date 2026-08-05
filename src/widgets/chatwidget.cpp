#include "chatwidget.h"

ChatWidget::ChatWidget(int width, int height,QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);
    this->setObjectName("this");

    initSearchWidget();
    initSearchStyle();
    initListWidget();
    initListStyle();
    initStackedConversation();
}

void ChatWidget::openConversation(const QString &uid)
{
    FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(uid);
    if(info.uid.isEmpty())
        return;

    ConversationItem *item = ConversationManager::getConversationManager().getConversationItem(uid);
    if(!item)
        return;

    if(!this->map_conversation.contains(uid))
    {
        createConversation(item);
    }

    auto it = map_conversation.find(uid);
    if(it != map_conversation.end())
    {
        ConversationWidget* widget = qobject_cast<ConversationWidget*>(this->stackedWidget_Conversation->currentWidget());
        if(widget)
            widget->setActive(false);
        this->stackedWidget_Conversation->setCurrentWidget(it.value());
        it.value()->setActive(true);

        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(uid == index.data(ConversationListDelegate::UIDRole).toString())
            {
                this->listView_conversationList->setCurrentIndex(index);
                this->listView_conversationList->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            }
        }
        return;
    }
}

void ChatWidget::cleanAll()
{
    this->model->clear();
    for(auto it = this->map_conversation.cbegin(); it != this->map_conversation.cend(); it++)
    {
        this->stackedWidget_Conversation->removeWidget(it.value());
        it.value()->deleteLater();
    }
    this->map_conversation.clear();
    this->stackedWidget_Conversation->setCurrentWidget(this->widget_noSelect);
}

void ChatWidget::initSearchWidget()
{
    this->widget_search = new QWidget(this);
    this->widget_search->setObjectName("widget_search");
    this->widget_search->setAttribute(Qt::WA_StyledBackground,true);
    this->widget_search->setGeometry(0,0,200,64);

    this->edit_search = new QLineEdit(this->widget_search);
    this->edit_search->setObjectName("edit_search");
    this->edit_search->setPlaceholderText("搜索对话...");
    this->edit_search->setMaxLength(50);
    searchIcon = new QAction(QIcon(":/default/images/search.png"),"",this->edit_search);
    this->edit_search->addAction(searchIcon,QLineEdit::LeadingPosition);
    this->edit_search->resize(this->widget_search->width() - 32,36);
    this->edit_search->move((this->widget_search->width() - this->edit_search->width()) / 2,(this->widget_search->height() - this->edit_search->height()) / 2);
}

void ChatWidget::initSearchStyle()
{
    this->setStyleSheet(R"(
                                    #widget_search
                                    {
                                        background: #ffffff;
                                        border-radius: 0;
                                        border-right: 1px solid #e0e0e0;
                                        border-bottom: 1px solid #e0e0e0;
                                    }
                                    #edit_search
                                    {
                                        background-color: #efecf8;
                                        border: 1px solid #f3f4f6;
                                        border-radius: 8px;
                                        padding-left: 0px;
                                        padding-right: 15px;
                                        color: #1b1b23;
                                        font-size: 12px;
                                        font-weight:bold;
                                        font-family: "Microsoft YaHei";
                                        margin-top: 1px;
                                    }
                                    #edit_search:focus
                                    {
                                        background-color: rgba(255, 255, 255, 255);
                                        border: 1px solid rgba(99, 102, 241, 255);
                                    }
                                    )");
}

void ChatWidget::initListWidget()
{
    this->listView_conversationList = new QListView(this);
    this->listView_conversationList->setObjectName("listView_conversationList");
    this->listView_conversationList->setGeometry(0, this->widget_search->height(), this->widget_search->width(), this->height());

    this->model = new QStandardItemModel(this);
    this->listView_conversationList->setModel(this->model);

    this->delegate = new ConversationListDelegate(this);
    this->listView_conversationList->setItemDelegate(this->delegate);

    this->listView_conversationList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->listView_conversationList->setSelectionMode(QAbstractItemView::SingleSelection);
    this->listView_conversationList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->listView_conversationList->verticalScrollBar()->setSingleStep(6);
    this->listView_conversationList->setResizeMode(QListView::Fixed);

    connect(this->delegate, &ConversationListDelegate::itemClicked, this, [this](const QModelIndex& index){
        QString uid = index.data(ConversationListDelegate::UIDRole).toString();
        openConversation(uid);
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::friendAvatarUpdate, this, [this](const QString& uid, const QPixmap& avatar){
        auto it = this->map_conversation.find(uid);
        if(it != this->map_conversation.end())
            it.value()->updateFriendAvatar(avatar);

        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(uid == index.data(ConversationListDelegate::UIDRole).toString())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                item->setData(avatar, ConversationListDelegate::AvatarRole);
            }
        }
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::friendStatusUpdate, this, [this](const QString& uid, bool isOnline){
        auto it = this->map_conversation.find(uid);
        if(it != this->map_conversation.end())
            it.value()->updateFriendStatus(isOnline);

        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(uid == index.data(ConversationListDelegate::UIDRole).toString())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                item->setData(isOnline, ConversationListDelegate::IsOnlineRole);
            }
        }
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::friendUsernameUpdate, this, [this](const QString& uid, const QString& username){
        auto it = this->map_conversation.find(uid);
        if(it != this->map_conversation.end())
            it.value()->updateFriendUsername(username);

        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(uid == index.data(ConversationListDelegate::UIDRole).toString())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                item->setData(username, ConversationListDelegate::UsernameRole);
            }
        }
    });

    connect(&ConversationManager::getConversationManager(), &ConversationManager::conversationCreated, this, [this](ConversationItem* item){
        createConversation(item);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, &ChatWidget::cleanAll);
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, &ChatWidget::cleanAll);
}

void ChatWidget::initListStyle()
{
    this->listView_conversationList->setStyleSheet(R"(
                                            #listView_conversationList
                                            {
                                                background: #ffffff;
                                                border-radius: 0;
                                                border-right: 1px solid #e0e0e0;
                                            }
                                            )");
}

void ChatWidget::initStackedConversation()
{
    this->stackedWidget_Conversation = new QStackedWidget(this);
    this->stackedWidget_Conversation->resize(this->width() - this->widget_search->width(),this->height());
    this->stackedWidget_Conversation->move(this->widget_search->width(), 0);

    this->widget_noSelect = new QWidget(this->stackedWidget_Conversation);
    this->widget_noSelect->resize(this->stackedWidget_Conversation->width(), this->stackedWidget_Conversation->height());
    this->widget_noSelect->move(0, 0);

    this->label_appIcon = new QLabel(this->widget_noSelect);
    this->label_appIcon->resize(60, 60);
    this->label_appIcon->setPixmap(QIcon(":/icon/images/blackIcon.png").pixmap(60));
    this->label_appIcon->move((this->widget_noSelect->width() - this->label_appIcon->width())/2,
                              (this->widget_noSelect->height() - this->label_appIcon->height())/2);

    this->stackedWidget_Conversation->addWidget(this->widget_noSelect);
    this->stackedWidget_Conversation->setCurrentWidget(this->widget_noSelect);
}

void ChatWidget::moveConversationToTop(const QString &conversationID)
{
    for(int i = 0; i < this->model->rowCount(); i++)
    {
        if(this->model->item(i)->data(ConversationListDelegate::UIDRole).toString() == conversationID)
        {
            if(i == 0)
                return;
            QStandardItem* item = this->model->takeItem(i);
            this->model->removeRow(i);
            this->model->insertRow(0, item);
            break;
        }
    }
    if(this->listView_conversationList->currentIndex().isValid())
    {
        QString selected = this->listView_conversationList->currentIndex().data(ConversationListDelegate::UIDRole).toString();
        if(selected == conversationID)
            this->listView_conversationList->setCurrentIndex(this->model->index(0, 0));
    }
}

ConversationWidget* ChatWidget::createConversation(ConversationItem* item)
{
    if(this->map_conversation.contains(item->getConversationID()))
        return this->map_conversation[item->getConversationID()];

    ConversationWidget* cw = new ConversationWidget(this->stackedWidget_Conversation->width(),
                                                    this->stackedWidget_Conversation->height(),
                                                    item, this->stackedWidget_Conversation);

    if(!cw)
        return nullptr;

    this->stackedWidget_Conversation->addWidget(cw);
    this->map_conversation[item->getConversationID()] = cw;

    FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(item->getConversationID());

    QStandardItem* item_standard = new QStandardItem();
    if(info.uid.isEmpty())
    {
        item_standard->setData(item->getConversationID(), ConversationListDelegate::UIDRole);
        item_standard->setData("群聊", ConversationListDelegate::UsernameRole);
        item_standard->setData(QPixmap(":/default/images/defaultAvatar.png"), ConversationListDelegate::AvatarRole);
        item_standard->setData(true, ConversationListDelegate::IsOnlineRole);
    }
    else
    {
        item_standard->setData(info.uid, ConversationListDelegate::UIDRole);
        item_standard->setData(info.username, ConversationListDelegate::UsernameRole);
        item_standard->setData(info.avatar, ConversationListDelegate::AvatarRole);
        item_standard->setData(info.isOnline, ConversationListDelegate::IsOnlineRole);
    }
    item_standard->setData(item->getLastMessage().content, ConversationListDelegate::LastMsgRole);
    item_standard->setData(item->getLastMessage().timeStamp, ConversationListDelegate::LastTimestampRole);
    item_standard->setData(item->getUnReadCount(), ConversationListDelegate::UnReadRole);
    this->model->appendRow(item_standard);

    connect(item, &ConversationItem::LastMessageChange, this, [this, item](const Message& msg){
        QString conversationID = item->getConversationID();
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            auto index = this->model->index(i, 0);
            if(conversationID == index.data(ConversationListDelegate::UIDRole).toString())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                item->setData(msg.content, ConversationListDelegate::LastMsgRole);
                item->setData(msg.timeStamp, ConversationListDelegate::LastTimestampRole);
            }
        }
        moveConversationToTop(conversationID);
    });

    connect(item, &ConversationItem::UnReadCountChange, this, [this, item](int num){
        QString conversationID = item->getConversationID();
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            auto index = this->model->index(i, 0);
            if(conversationID == index.data(ConversationListDelegate::UIDRole).toString())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                item->setData(num, ConversationListDelegate::UnReadRole);
            }
        }
        moveConversationToTop(conversationID);
    });

    return cw;
}

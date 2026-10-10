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
    initSearchModels();
}

void ChatWidget::openConversation(const QString &uid)
{
    FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(uid);
    if(info.uid.isEmpty())
        return;

    if(!this->map_conversation.contains(uid))
        createConversation(uid);

    auto it = map_conversation.find(uid);
    if(it != map_conversation.end())
    {
        ConversationWidget* widget = qobject_cast<ConversationWidget*>(this->stackedWidget_Conversation->currentWidget());

        if(widget)
            widget->setActive(false);
        this->stackedWidget_Conversation->setCurrentWidget(it.value());
        it.value()->setActive(true);

        this->currentConversation = uid;

        QModelIndex targetIndex;
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(index.data(ConversationListDelegate::IsSelectedRole).toBool())
                this->model->setData(index, false, ConversationListDelegate::IsSelectedRole);
            if(uid == index.data(ConversationListDelegate::UIDRole).toString())
                targetIndex = index;
        }
        if(targetIndex.isValid())
        {
            if(this->listView_conversationList->model() == this->model)
                this->listView_conversationList->setCurrentIndex(targetIndex);

            this->model->setData(targetIndex, true, ConversationListDelegate::IsSelectedRole);
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
    this->currentConversation.clear();
    this->stackedWidget_Conversation->setCurrentWidget(this->widget_noSelect);
}

void ChatWidget::initSearchWidget()
{
    this->widget_search = new QWidget(this);
    this->widget_search->setObjectName("widget_search");
    this->widget_search->setAttribute(Qt::WA_StyledBackground,true);
    this->widget_search->setGeometry(1,1,200,64);

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

void ChatWidget::initSearchModels()
{
    this->model_searchConversation = new QStandardItemModel(this);
    this->model_searchMsg = new QStandardItemModel(this);
    this->delegate_search = new SearchResultListDelegate(this);

    this->timer_search = new QTimer(this);
    this->timer_search->setSingleShot(true);
    this->timer_search->setInterval(100);

    connect(this->delegate_search, &SearchResultListDelegate::itemClicked, this, [this](const QModelIndex& index){
        if(index.data(SearchResultListDelegate::IsReturnBackItemRole).toBool())
        {
            this->model_searchMsg->clear();
            this->currentSearchConversationID.clear();
            this->isLoadingSearchMsg = false;

            this->listView_conversationList->setItemDelegate(this->delegate);
            this->listView_conversationList->setModel(this->model_searchConversation);
            return;
        }

        QString msgID = index.data(SearchResultListDelegate::MsgIDRole).toString();
        qint64 convSeq = index.data(SearchResultListDelegate::ConvSeqRole).toLongLong();
        QString convID = this->currentSearchConversationID;

        if(convID.isEmpty() || msgID.isEmpty())
            return;

        openConversation(convID);

        auto it = this->map_conversation.find(convID);
        if(it == this->map_conversation.end())
            return;

        ConversationWidget* widget = it.value();
        widget->jumpToMessage(msgID, convSeq);
    });

    connect(this->edit_search, &QLineEdit::textChanged, this, [this](const QString&){
        bool isInSearch = this->listView_conversationList->model() != this->model;
        const QString text = this->edit_search->text().trimmed();

        ++this->searchGen;

        this->model_searchConversation->clear();
        this->model_searchMsg->clear();
        this->currentSearchConversationID.clear();
        this->isLoadingSearchConversation = false;
        this->isLoadingSearchMsg = false;

        if(text.isEmpty())
        {
            if(isInSearch)
            {
                this->listView_conversationList->setItemDelegate(delegate);
                this->listView_conversationList->setModel(this->model);

                QTimer::singleShot(0, this, [this](){
                    this->listView_conversationList->verticalScrollBar()->setValue(this->pos_scroll);
                    if(!this->currentConversation.isEmpty())
                        restoreSelection(this->currentConversation);
                });
            }
            return;
        }

        if(!isInSearch)
            this->pos_scroll = this->listView_conversationList->verticalScrollBar()->value();

        this->listView_conversationList->setItemDelegate(delegate);
        this->listView_conversationList->setModel(this->model_searchConversation);

        if(this->timer_search->isActive())
            this->timer_search->stop();
        this->timer_search->start();
    });

    connect(this->timer_search, &QTimer::timeout, this, [this](){
        const QString text = this->edit_search->text().trimmed();
        if(text.isEmpty())
            return;

        loadSearchConvFirstPage(this->searchGen, text);
    });

    connect(this->listView_conversationList->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value){
        auto* bar = this->listView_conversationList->verticalScrollBar();
        if(bar->maximum() == 0 || bar->maximum() - value > 30)
            return;

        quint64 gen = this->searchGen;
        QAbstractItemModel* currentModel = this->listView_conversationList->model();

        if(currentModel == this->model_searchConversation)
        {
            if(this->isLoadingSearchConversation)
                return;
            loadSearchConvNextPage(gen);
        }
        else if(currentModel == this->model_searchMsg)
        {
            if(this->isLoadingSearchMsg)
                return;
            loadSearchMsgNextPage(gen);
        }
    });
}

void ChatWidget::initListWidget()
{
    this->listView_conversationList = new QListView(this);
    this->listView_conversationList->setObjectName("listView_conversationList");
    this->listView_conversationList->setGeometry(1, this->widget_search->height() + 1, this->widget_search->width(), this->height() - this->widget_search->height() - 3);

    this->model = new QStandardItemModel(this);
    this->model->setSortRole(ConversationListDelegate::LastTimestampRole);
    this->model->sort(0, Qt::DescendingOrder);
    this->listView_conversationList->setModel(this->model);

    this->delegate = new ConversationListDelegate(this);
    this->listView_conversationList->setItemDelegate(this->delegate);

    this->listView_conversationList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->listView_conversationList->setSelectionMode(QAbstractItemView::SingleSelection);
    this->listView_conversationList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->listView_conversationList->verticalScrollBar()->setSingleStep(6);
    this->listView_conversationList->setResizeMode(QListView::Fixed);

    connect(this->delegate, &ConversationListDelegate::itemClicked, this, [this](const QModelIndex& index){
        if(this->listView_conversationList->model() == this->model_searchConversation)
        {
            QString conversationID = index.data(ConversationListDelegate::UIDRole).toString();
            openSearchMsgView(conversationID);
            return;
        }

        if(this->currentConversation == index.data(ConversationListDelegate::UIDRole).toString() && !this->currentConversation.isEmpty())
        {
            this->model->setData(index, false, ConversationListDelegate::IsSelectedRole);
            this->listView_conversationList->viewport()->update();

            this->stackedWidget_Conversation->setCurrentWidget(this->widget_noSelect);
            this->currentConversation.clear();
            return;
        }

        QString uid = index.data(ConversationListDelegate::UIDRole).toString();
        openConversation(uid);
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::friendAvatarUpdate, this, [this](const QString& uid, const QPixmap& avatar, const QSize& size){
        if(size != QSize(36, 36) && !size.isEmpty())
            return;
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(uid == index.data(ConversationListDelegate::UIDRole).toString())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                if(!size.isEmpty())
                    item->setData(avatar, ConversationListDelegate::AvatarRole);
                else
                {
                    QPixmap newAvatar = FriendManage::getFriendManage().getFriendAvatar(uid, QSize(36, 36));
                    if(!newAvatar.isNull())
                        item->setData(newAvatar, ConversationListDelegate::AvatarRole);
                }
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
        createConversationListItem(item);

        const QString uid = item->getConversationID();
        if(this->map_conversation.contains(uid))
            return;
        createConversation(uid);
    });

    connect(&GlobalInitController::getController(), &GlobalInitController::allConversationsLoaded, this, [this]
            (const QList<QPair<ConversationItem*, DatabaseManager::ConversationInfo>>& list){
        for(const auto& pair : list)
        {
            ConversationItem* item = pair.first;
            if(!item)
                continue;
            createConversationListItem(item, pair.second);
        }
        this->model->setSortRole(ConversationListDelegate::LastTimestampRole);
        this->model->sort(0, Qt::DescendingOrder);
        ConversationManager::getConversationManager().startSyncMessage();

        precreateConversations();
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::allFriendList, this, [this](){
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            QString uid = index.data(ConversationListDelegate::UIDRole).toString();
            FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(uid);
            if(!info.uid.isEmpty())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                item->setData(info.username, ConversationListDelegate::UsernameRole);
                item->setData(FriendManage::getFriendManage().getFriendAvatar(info.uid, QSize(36, 36)), ConversationListDelegate::AvatarRole);
                item->setData(info.isOnline, ConversationListDelegate::IsOnlineRole);
            }
        }
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, &ChatWidget::cleanAll);
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, &ChatWidget::cleanAll);
    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::refreshExpiredExit, this, &ChatWidget::cleanAll);


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
    this->stackedWidget_Conversation->resize(this->width() - this->widget_search->width() - 3,this->height() - 2);
    this->stackedWidget_Conversation->move(this->widget_search->width() + 1, 1);

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

void ChatWidget::precreateConversations()
{
    QList<QString> uids;
    int count = qMin(30, this->model->rowCount());
    for(int i = 0; i < count; i++)
    {
        QString uid = this->model->index(i, 0).data(ConversationListDelegate::UIDRole).toString();
        if(!uid.isEmpty())
            uids.append(uid);
    }

    for(int i = 0; i < uids.size(); i++)
    {
        const QString uid = uids[i];
        QTimer::singleShot(i * 100, this, [this, uid](){
            if(this->map_conversation.contains(uid))
                return;

            createConversation(uid);
        });
    }
}

void ChatWidget::restoreSelection(const QString &conversationID)
{
    if(this->listView_conversationList->model() != this->model)
        return;

    for(int i = 0; i < this->model->rowCount(); i++)
    {
        if(this->model->index(i, 0).data(ConversationListDelegate::UIDRole).toString() == conversationID)
        {
            this->listView_conversationList->setCurrentIndex(this->model->index(i, 0));
            break;
        }
    }
}

void ChatWidget::loadSearchConvFirstPage(quint64 gen, const QString &keyword)
{
    this->isLoadingSearchConversation = true;
    DatabaseManager::getDatabaseManager().searchConversations(keyword, 0, "", 30, [this, gen](const QList<DatabaseManager::SearchConvResult>& list){
        if(gen != this->searchGen)
            return;

        this->isLoadingSearchConversation = false;

        for(const auto& result : list)
            appendSearchConvItem(result);
    });
}

void ChatWidget::loadSearchConvNextPage(quint64 gen)
{
    if(this->model_searchConversation->rowCount() == 0)
        return;

    QStandardItem* lastItem = this->model_searchConversation->item(this->model_searchConversation->rowCount() - 1);
    qint64 cursorTimestamp = lastItem->data(ConversationListDelegate::LastTimestampRole).toLongLong();
    QString cursorConversationID = lastItem->data(ConversationListDelegate::UIDRole).toString();

    this->isLoadingSearchConversation = true;
    DatabaseManager::getDatabaseManager().searchConversations(this->edit_search->text().trimmed(), cursorTimestamp, cursorConversationID, 30, [this, gen](const QList<DatabaseManager::SearchConvResult>& list){
        if(gen != this->searchGen)
            return;
        this->isLoadingSearchConversation = false;

        for(const auto& result : list)
            appendSearchConvItem(result);
    });
}

void ChatWidget::appendSearchConvItem(const DatabaseManager::SearchConvResult &result)
{
    FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(result.conversationID);
    QStandardItem* item = new QStandardItem;

    item->setData(result.conversationID, ConversationListDelegate::UIDRole);
    item->setData(info.username, ConversationListDelegate::UsernameRole);

    QPixmap avatar = info.uid.isEmpty() ? QPixmap(":/default/images/defaultAvatar.png") : FriendManage::getFriendManage().getFriendAvatar(info.uid, QSize(36, 36));
    avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
    item->setData(avatar, ConversationListDelegate::AvatarRole);

    item->setData(QString("共 %1 条匹配的聊天记录").arg(result.matchCount), ConversationListDelegate::LastMsgRole);
    item->setData(result.lastTimestamp, ConversationListDelegate::LastTimestampRole);
    item->setData(0, ConversationListDelegate::UnReadRole);
    item->setData(false, ConversationListDelegate::IsOnlineRole);
    item->setData(false, ConversationListDelegate::IsSelectedRole);
    item->setData(true, ConversationListDelegate::IsSearchModeRole);

    this->model_searchConversation->appendRow(item);
}

void ChatWidget::loadSearchMsgFirstPage(quint64 gen)
{
    this->isLoadingSearchMsg = true;

    DatabaseManager::getDatabaseManager().searchMessagesInConv(this->currentSearchConversationID, this->edit_search->text().trimmed(), 0, 50, [this, gen](const QList<Message>& list){
        if(gen != this->searchGen)
            return;
        this->isLoadingSearchMsg = false;

        for(const auto& msg : list)
            appendSearchMsgItem(msg);
    });
}

void ChatWidget::loadSearchMsgNextPage(quint64 gen)
{
    if(this->model_searchMsg->rowCount() <= 1)
        return;

    QStandardItem* lastItem = this->model_searchMsg->item(this->model_searchMsg->rowCount() - 1);
    qint64 cursorConvSeq = lastItem->data(SearchResultListDelegate::ConvSeqRole).toLongLong();
    this->isLoadingSearchMsg = true;

    DatabaseManager::getDatabaseManager().searchMessagesInConv(this->currentSearchConversationID, this->edit_search->text().trimmed(), cursorConvSeq, 50, [this, gen](const QList<Message>& list){
        if(gen != this->searchGen)
            return;
        this->isLoadingSearchMsg = false;

        for(const auto& msg : list)
            appendSearchMsgItem(msg);
    });
}

void ChatWidget::appendSearchMsgItem(const Message &msg)
{
    bool isMyself = msg.senderUID == UserInfo::getUserInfo().getUID();
    QPixmap avatar = isMyself ? UserInfo::getUserInfo().getAvatar(QSize(36, 36)) : FriendManage::getFriendManage().getFriendAvatar(msg.senderUID, QSize(36, 36));
    avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());

    QString name = isMyself ? UserInfo::getUserInfo().getUsername() : FriendManage::getFriendManage().getFriendInfo(msg.senderUID).username;
    QString msgID = msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID;

    QStandardItem* item = new QStandardItem;
    item->setData(avatar, SearchResultListDelegate::AvatarRole);
    item->setData(name, SearchResultListDelegate::UsernameRole);
    item->setData(msg.content, SearchResultListDelegate::ContentRole);
    item->setData(msg.timeStamp, SearchResultListDelegate::TimestampRole);
    item->setData(msgID, SearchResultListDelegate::MsgIDRole);
    item->setData(msg.convSeq, SearchResultListDelegate::ConvSeqRole);
    item->setData(false, SearchResultListDelegate::IsReturnBackItemRole);
    item->setData(this->edit_search->text().trimmed(), SearchResultListDelegate::HighLightKeywordRole);

    this->model_searchMsg->appendRow(item);
}

void ChatWidget::openSearchMsgView(const QString &conversationID)
{
    this->currentSearchConversationID = conversationID;
    this->model_searchMsg->clear();
    this->isLoadingSearchMsg = false;

    QStandardItem* returnItem = new QStandardItem;
    returnItem->setData(true, SearchResultListDelegate::IsReturnBackItemRole);
    this->model_searchMsg->appendRow(returnItem);

    this->listView_conversationList->setItemDelegate(this->delegate_search);
    this->listView_conversationList->setModel(this->model_searchMsg);

    this->listView_conversationList->verticalScrollBar()->setValue(0);

    quint64 gen = this->searchGen;
    loadSearchMsgFirstPage(gen);
}

ConversationWidget* ChatWidget::createConversation(const QString& conversationID)
{
    ConversationItem* item = ConversationManager::getConversationManager().getOrCreateConversationItem(conversationID);
    if(this->map_conversation.contains(item->getConversationID()))
        return this->map_conversation[item->getConversationID()];

    ConversationWidget* cw = new ConversationWidget(this->stackedWidget_Conversation->width(),
                                                    this->stackedWidget_Conversation->height(),
                                                    item, this->stackedWidget_Conversation);

    if(!cw)
        return nullptr;

    this->stackedWidget_Conversation->addWidget(cw);
    this->map_conversation[item->getConversationID()] = cw;

    return cw;
}

void ChatWidget::createConversationListItem(ConversationItem *item, const DatabaseManager::ConversationInfo& info)
{
    const QString conversationID = item->getConversationID();
    for(int i = 0; i < this->model->rowCount(); i++)
    {
        if(this->model->item(i)->data(ConversationListDelegate::UIDRole).toString() == conversationID)
        {
            if(!info.conversationID.isEmpty())
            {
                QStandardItem* temp_item = this->model->item(i);
                temp_item->setData(info.lastMsg, ConversationListDelegate::LastMsgRole);
                temp_item->setData(info.lastTimestamp, ConversationListDelegate::LastTimestampRole);
                temp_item->setData(info.unReadCount, ConversationListDelegate::UnReadRole);
                item->setUnReadCount(info.unReadCount);
            }
            return;
        }
    }

    FriendManage::FriendInfo friend_info = FriendManage::getFriendManage().getFriendInfo(conversationID);
    QStandardItem* item_standard = new QStandardItem();
    if(friend_info.uid.isEmpty())
    {
        item_standard->setData(item->getConversationID(), ConversationListDelegate::UIDRole);
        item_standard->setData("加载中...", ConversationListDelegate::UsernameRole);
        item_standard->setData(QPixmap(":/default/images/defaultAvatar.png"), ConversationListDelegate::AvatarRole);
        item_standard->setData(true, ConversationListDelegate::IsOnlineRole);
    }
    else
    {
        item_standard->setData(friend_info.uid, ConversationListDelegate::UIDRole);
        item_standard->setData(friend_info.username, ConversationListDelegate::UsernameRole);
        item_standard->setData(FriendManage::getFriendManage().getFriendAvatar(friend_info.uid, QSize(36, 36)), ConversationListDelegate::AvatarRole);
        item_standard->setData(friend_info.isOnline, ConversationListDelegate::IsOnlineRole);
    }

    if(!info.conversationID.isEmpty())
    {
        item_standard->setData(info.lastMsg, ConversationListDelegate::LastMsgRole);
        item_standard->setData(info.lastTimestamp, ConversationListDelegate::LastTimestampRole);
        item_standard->setData(info.unReadCount, ConversationListDelegate::UnReadRole);
        item->setUnReadCount(info.unReadCount);
    }
    else
    {
        item_standard->setData(item->getLastMessage().content, ConversationListDelegate::LastMsgRole);
        item_standard->setData(item->getLastMessage().timeStamp, ConversationListDelegate::LastTimestampRole);
        item_standard->setData(item->getUnReadCount(), ConversationListDelegate::UnReadRole);
    }
    this->model->appendRow(item_standard);

    connect(item, &ConversationItem::LastMessageChange, this, [this, item](const Message& msg){
        QString selectedUID;
        QModelIndex currentIndex = this->listView_conversationList->currentIndex();
        if(currentIndex.isValid())
            selectedUID = currentIndex.data(ConversationListDelegate::UIDRole).toString();

        QString conversationID = item->getConversationID();
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            auto index = this->model->index(i, 0);
            if(conversationID == index.data(ConversationListDelegate::UIDRole).toString())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                if(msg.contentType == ContentType::Text)
                    item->setData(msg.content, ConversationListDelegate::LastMsgRole);
                else if(msg.contentType == ContentType::Image)
                    item->setData("[图片]", ConversationListDelegate::LastMsgRole);
                else if(msg.contentType == ContentType::Video)
                    item->setData("[视频]", ConversationListDelegate::LastMsgRole);
                else if(msg.contentType == ContentType::File)
                    item->setData("[文件]", ConversationListDelegate::LastMsgRole);
                item->setData(msg.timeStamp, ConversationListDelegate::LastTimestampRole);
            }
        }

        this->model->sort(0, Qt::DescendingOrder);

        if(this->listView_conversationList->model() != this->model)
            return;

        if(!selectedUID.isEmpty())
            restoreSelection(selectedUID);
        else
        {
            this->listView_conversationList->setCurrentIndex(QModelIndex());
            this->listView_conversationList->clearSelection();
        }
    });

    connect(item, &ConversationItem::UnReadCountChange, this, [this, item](int num){
        QString selectedUID;
        QModelIndex currentIndex = this->listView_conversationList->currentIndex();
        if(currentIndex.isValid())
            selectedUID = currentIndex.data(ConversationListDelegate::UIDRole).toString();

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

        this->model->sort(0, Qt::DescendingOrder);

        if(this->listView_conversationList->model() != this->model)
            return;

        if(!selectedUID.isEmpty())
            restoreSelection(selectedUID);
        else
        {
            this->listView_conversationList->setCurrentIndex(QModelIndex());
            this->listView_conversationList->clearSelection();
        }
    });
}

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

void ChatWidget::openConversation(const QString &conversationID)
{
    if(conversationID.isEmpty())
        return;

    this->forceConversations.insert(conversationID);

    if(!this->map_conversation.contains(conversationID))
    {
        if(!createConversation(conversationID))
        {
            this->forceConversations.remove(conversationID);
            return;
        }
    }

    bool isInList = false;
    for(int i = 0; i < this->model->rowCount(); i++)
    {

        if(this->model->index(i, 0).data(ConversationListDelegate::ConversationIDRole).toString() == conversationID)
        {
            isInList = true;
            break;
        }
    }

    if(!isInList)
    {
        auto it = this->map_conversation.find(conversationID);
        if(it != this->map_conversation.end())
        {
            ConversationItem* item = ConversationManager::getConversationManager().getOrCreateConversationItem(conversationID,
                                                                                                               GroupManager::getGroupManage().getGroupInfo(conversationID).conversationID.isEmpty() ?
                                                                                                                   ConversationItem::ConversationType::Personal : ConversationItem::ConversationType::Group);

            if(item)
                createConversationListItem(item);
        }
    }

    this->forceConversations.remove(conversationID);

    auto it = map_conversation.find(conversationID);
    if(it != map_conversation.end())
    {
        ConversationWidget* widget = qobject_cast<ConversationWidget*>(this->stackedWidget_Conversation->currentWidget());

        if(widget)
            widget->setActive(false);
        this->stackedWidget_Conversation->setCurrentWidget(it.value());
        it.value()->setActive(true);

        this->currentConversation = conversationID;

        QModelIndex targetIndex;
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(index.data(ConversationListDelegate::IsSelectedRole).toBool())
                this->model->setData(index, false, ConversationListDelegate::IsSelectedRole);
            if(conversationID == index.data(ConversationListDelegate::ConversationIDRole).toString())
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
    this->boundConversations.clear();
    this->forceConversations.clear();
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
            QString conversationID = index.data(ConversationListDelegate::ConversationIDRole).toString();
            openSearchMsgView(conversationID);
            return;
        }

        QString conversationID = index.data(ConversationListDelegate::ConversationIDRole).toString();
        if(conversationID.isEmpty())
            return;

        if(this->currentConversation == index.data(ConversationListDelegate::ConversationIDRole).toString() && !this->currentConversation.isEmpty())
        {
            this->model->setData(index, false, ConversationListDelegate::IsSelectedRole);
            this->listView_conversationList->viewport()->update();

            this->stackedWidget_Conversation->setCurrentWidget(this->widget_noSelect);
            this->currentConversation.clear();
            return;
        }

        openConversation(conversationID);
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::friendAvatarUpdate, this, [this](const QString& uid, const QPixmap& avatar, const QSize& size){
        if(size != QSize(36, 36) && !size.isEmpty())
            return;

        QPixmap avatar_copy = avatar;
        avatar_copy.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());

        const QString conversationID = FriendManage::getFriendManage().getFriendInfo(uid).conversationID;
        if(conversationID.isEmpty())
            return;

        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(conversationID == index.data(ConversationListDelegate::ConversationIDRole).toString())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                if(!size.isEmpty())
                    item->setData(avatar_copy, ConversationListDelegate::AvatarRole);
                else
                {
                    QPixmap newAvatar = FriendManage::getFriendManage().getFriendAvatar(uid, QSize(36, 36));
                    newAvatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                    if(!newAvatar.isNull())
                        item->setData(newAvatar, ConversationListDelegate::AvatarRole);
                }
            }
        }

        for(int i = 0; i < this->model_searchConversation->rowCount(); i++)
        {
            QModelIndex index = this->model_searchConversation->index(i, 0);
            if(conversationID == index.data(ConversationListDelegate::ConversationIDRole).toString())
            {
                QStandardItem* item = this->model_searchConversation->itemFromIndex(index);
                if(!size.isEmpty())
                    item->setData(avatar_copy, ConversationListDelegate::AvatarRole);
                else
                {
                    QPixmap newAvatar = FriendManage::getFriendManage().getFriendAvatar(uid, QSize(36, 36));
                    newAvatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                    if(!newAvatar.isNull())
                        item->setData(newAvatar, ConversationListDelegate::AvatarRole);
                }
            }
        }

        for(int i = 0; i < this->model_searchMsg->rowCount(); i++)
        {
            QModelIndex index = this->model_searchMsg->index(i, 0);
            if(uid == index.data(SearchResultListDelegate::SenderUIDRole).toString())
            {
                QStandardItem* item = this->model_searchMsg->itemFromIndex(index);
                if(!size.isEmpty())
                    item->setData(avatar_copy, SearchResultListDelegate::AvatarRole);
                else
                {
                    QPixmap newAvatar = FriendManage::getFriendManage().getFriendAvatar(uid, QSize(36, 36));
                    newAvatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                    if(!newAvatar.isNull())
                        item->setData(newAvatar, SearchResultListDelegate::AvatarRole);
                }
            }
        }
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::friendStatusUpdate, this, [this](const QString& uid, bool isOnline){
        const QString conversationID = FriendManage::getFriendManage().getFriendInfo(uid).conversationID;
        if(conversationID.isEmpty())
            return;

        auto it = this->map_conversation.find(conversationID);
        if(it != this->map_conversation.end())
            it.value()->updateFriendStatus(isOnline);

        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(conversationID == index.data(ConversationListDelegate::ConversationIDRole).toString())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                item->setData(isOnline, ConversationListDelegate::IsOnlineRole);
            }
        }
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::friendUsernameUpdate, this, [this](const QString& uid, const QString& username){
        const QString conversationID = FriendManage::getFriendManage().getFriendInfo(uid).conversationID;
        if(conversationID.isEmpty())
            return;

        auto it = this->map_conversation.find(conversationID);
        if(it != this->map_conversation.end())
            it.value()->updateFriendUsername(username);

        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(conversationID == index.data(ConversationListDelegate::ConversationIDRole).toString())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                item->setData(username, ConversationListDelegate::UsernameRole);
            }
        }

        for(int i = 0; i < this->model_searchConversation->rowCount(); i++)
        {
            QModelIndex index = this->model_searchConversation->index(i, 0);
            if(conversationID == index.data(ConversationListDelegate::ConversationIDRole).toString())
            {
                QStandardItem* item = this->model_searchConversation->itemFromIndex(index);
                item->setData(username, ConversationListDelegate::UsernameRole);
            }
        }

        for(int i = 0; i < this->model_searchMsg->rowCount(); i++)
        {
            QModelIndex index = this->model_searchMsg->index(i, 0);
            if(uid == index.data(SearchResultListDelegate::SenderUIDRole).toString())
            {
                QStandardItem* item = this->model_searchMsg->itemFromIndex(index);
                item->setData(username, SearchResultListDelegate::UsernameRole);
            }
        }
    });

    connect(&GroupManager::getGroupManage(), &GroupManager::groupAvatarUpdate, this, [this](const QString& conversationID, const QPixmap& avatar, const QSize& size){
        if(size != QSize(36, 36) && !size.isEmpty())
            return;

        QPixmap avatar_copy = avatar;
        avatar_copy.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());

        if(conversationID.isEmpty())
            return;

        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(conversationID == index.data(ConversationListDelegate::ConversationIDRole).toString())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                if(!size.isEmpty())
                    item->setData(avatar_copy, ConversationListDelegate::AvatarRole);
                else
                {
                    QPixmap newAvatar = GroupManager::getGroupManage().getGroupAvatar(conversationID, QSize(36, 36));
                    newAvatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                    if(!newAvatar.isNull())
                        item->setData(newAvatar, ConversationListDelegate::AvatarRole);
                }
            }
        }

        for(int i = 0; i < this->model_searchConversation->rowCount(); i++)
        {
            QModelIndex index = this->model_searchConversation->index(i, 0);
            if(conversationID == index.data(ConversationListDelegate::ConversationIDRole).toString())
            {
                QStandardItem* item = this->model_searchConversation->itemFromIndex(index);
                if(!size.isEmpty())
                    item->setData(avatar_copy, ConversationListDelegate::AvatarRole);
                else
                {
                    QPixmap newAvatar = GroupManager::getGroupManage().getGroupAvatar(conversationID, QSize(36, 36));
                    newAvatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                    if(!newAvatar.isNull())
                        item->setData(newAvatar, ConversationListDelegate::AvatarRole);
                }
            }
        }
    });

    connect(&ConversationManager::getConversationManager(), &ConversationManager::conversationCreated, this, [this](ConversationItem* item){
        createConversationListItem(item);

        const QString conversationID = item->getConversationID();
        if(this->map_conversation.contains(conversationID))
            return;
        createConversation(conversationID);
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
            QString conversationID = index.data(ConversationListDelegate::ConversationIDRole).toString();
            QString uid = FriendManage::getFriendManage().getFriendUIDByConversationID(conversationID);
            FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(uid);

            if(!info.uid.isEmpty())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                item->setData(info.username, ConversationListDelegate::UsernameRole);

                QPixmap avatar = FriendManage::getFriendManage().getFriendAvatar(info.uid, QSize(36, 36));
                avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                item->setData(avatar, ConversationListDelegate::AvatarRole);

                item->setData(info.isOnline, ConversationListDelegate::IsOnlineRole);
            }
        }

        for(int i = 0; i < this->model_searchConversation->rowCount(); i++)
        {
            QModelIndex index = this->model_searchConversation->index(i, 0);
            QString conversationID = index.data(ConversationListDelegate::ConversationIDRole).toString();
            QString uid = FriendManage::getFriendManage().getFriendUIDByConversationID(conversationID);
            FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(uid);

            if(!info.uid.isEmpty())
            {
                QStandardItem* item = this->model_searchConversation->itemFromIndex(index);
                item->setData(info.username, ConversationListDelegate::UsernameRole);

                QPixmap avatar = FriendManage::getFriendManage().getFriendAvatar(info.uid, QSize(36, 36));
                avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                item->setData(avatar, ConversationListDelegate::AvatarRole);
            }
        }

        for(int i = 0; i < this->model_searchMsg->rowCount(); i++)
        {
            QModelIndex index = this->model_searchMsg->index(i, 0);
            QString uid = index.data(SearchResultListDelegate::SenderUIDRole).toString();
            FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(uid);
            if(!info.uid.isEmpty())
            {
                QStandardItem* item = this->model_searchMsg->itemFromIndex(index);
                item->setData(info.username, SearchResultListDelegate::UsernameRole);

                QPixmap avatar = FriendManage::getFriendManage().getFriendAvatar(info.uid, QSize(36, 36));
                avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                item->setData(avatar, SearchResultListDelegate::AvatarRole);
            }
        }
    });

    connect(&GroupManager::getGroupManage(), &GroupManager::allGroupList, this, [this](){
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            QString conversationID = index.data(ConversationListDelegate::ConversationIDRole).toString();

            GroupManager::GroupInfo info = GroupManager::getGroupManage().getGroupInfo(conversationID);

            if(!info.conversationID.isEmpty())
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                item->setData(info.groupName, ConversationListDelegate::UsernameRole);

                QPixmap avatar = GroupManager::getGroupManage().getGroupAvatar(info.conversationID, QSize(36, 36));
                avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                item->setData(avatar, ConversationListDelegate::AvatarRole);
            }
        }

        for(int i = 0; i < this->model_searchConversation->rowCount(); i++)
        {
            QModelIndex index = this->model_searchConversation->index(i, 0);
            QString conversationID = index.data(ConversationListDelegate::ConversationIDRole).toString();
            GroupManager::GroupInfo info = GroupManager::getGroupManage().getGroupInfo(conversationID);

            if(!info.conversationID.isEmpty())
            {
                QStandardItem* item = this->model_searchConversation->itemFromIndex(index);
                item->setData(info.groupName, ConversationListDelegate::UsernameRole);

                QPixmap avatar = GroupManager::getGroupManage().getGroupAvatar(info.conversationID, QSize(36, 36));
                avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                item->setData(avatar, ConversationListDelegate::AvatarRole);
            }
        }
    });

    connect(&GroupManager::getGroupManage(), &GroupManager::groupInfoUpdate, this, [this](const QString& conversationID){
        GroupManager::GroupInfo info = GroupManager::getGroupManage().getGroupInfo(conversationID);
        if(info.conversationID.isEmpty())
            return;

        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(index.data(ConversationListDelegate::ConversationIDRole).toString() == conversationID)
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                item->setData(info.memberInfos.size(), ConversationListDelegate::GroupMemberCountRole);
                break;
            }
        }

        for(int i = 0; i < this->model_searchConversation->rowCount(); i++)
        {
            QModelIndex index = this->model_searchConversation->index(i, 0);
            if(index.data(ConversationListDelegate::ConversationIDRole).toString() == conversationID)
            {
                QStandardItem* item = this->model_searchConversation->itemFromIndex(index);
                item->setData(info.memberInfos.size(), ConversationListDelegate::GroupMemberCountRole);
                break;
            }
        }

        auto it = this->map_conversation.find(conversationID);
        if(it != this->map_conversation.end())
            it.value()->updateGroupInfo();
    });

    connect(&UserInfo::getUserInfo(), &UserInfo::updateInfo, this, [this](QString username, QString email, QString sid){

        for(int i = 0; i < this->model_searchMsg->rowCount(); i++)
        {
            QModelIndex index = this->model_searchMsg->index(i, 0);
            QString uid = index.data(SearchResultListDelegate::SenderUIDRole).toString();
            if(uid == UserInfo::getUserInfo().getUID())
            {
                QStandardItem* item = this->model_searchMsg->itemFromIndex(index);
                item->setData(username, SearchResultListDelegate::UsernameRole);

                QPixmap avatar = UserInfo::getUserInfo().getAvatar(QSize(36, 36));
                avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                item->setData(avatar, ConversationListDelegate::AvatarRole);
            }
        }
    });

    connect(&UserInfo::getUserInfo(), &UserInfo::updateAvatar, this, [this](const QPixmap& avatar, const QSize& size){
        QPixmap avatar_copy = avatar;
        avatar_copy.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());

        for(int i = 0; i < this->model_searchMsg->rowCount(); i++)
        {
            QModelIndex index = this->model_searchMsg->index(i, 0);
            QString uid = index.data(SearchResultListDelegate::SenderUIDRole).toString();
            if(uid == UserInfo::getUserInfo().getUID())
            {
                QStandardItem* item = this->model_searchMsg->itemFromIndex(index);
                item->setData(size == QSize(36, 36) ? avatar_copy : UserInfo::getUserInfo().getAvatar(QSize(36, 36)), SearchResultListDelegate::AvatarRole);
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

void ChatWidget::bindConversationItemSignals(ConversationItem *item)
{
    const QString conversationID = item->getConversationID();
    if(this->boundConversations.contains(conversationID))
        return;
    this->boundConversations.insert(conversationID);

    connect(item, &ConversationItem::LastMessageChange, this, [this, item](const Message& msg){
        QString selectedUID;
        QModelIndex currentIndex = this->listView_conversationList->currentIndex();
        if(currentIndex.isValid())
            selectedUID = currentIndex.data(ConversationListDelegate::ConversationIDRole).toString();

        bool isFound = false;
        QString conversationID = item->getConversationID();
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            auto index = this->model->index(i, 0);
            if(conversationID == index.data(ConversationListDelegate::ConversationIDRole).toString())
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
                break;
            }
        }

        if(!isFound)
            createConversationListItem(item);

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
            selectedUID = currentIndex.data(ConversationListDelegate::ConversationIDRole).toString();

        QString conversationID = item->getConversationID();
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            auto index = this->model->index(i, 0);
            if(conversationID == index.data(ConversationListDelegate::ConversationIDRole).toString())
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
    QList<QString> conversationIDs;
    int count = qMin(30, this->model->rowCount());
    for(int i = 0; i < count; i++)
    {
        QString conversationID = this->model->index(i, 0).data(ConversationListDelegate::ConversationIDRole).toString();
        if(!conversationID.isEmpty())
            conversationIDs.append(conversationID);
    }

    for(int i = 0; i < conversationIDs.size(); i++)
    {
        const QString conversationID = conversationIDs[i];
        QTimer::singleShot(i * 100, this, [this, conversationID](){
            if(this->map_conversation.contains(conversationID))
                return;

            createConversation(conversationID);
        });
    }
}

void ChatWidget::restoreSelection(const QString &conversationID)
{
    if(this->listView_conversationList->model() != this->model)
        return;

    for(int i = 0; i < this->model->rowCount(); i++)
    {
        if(this->model->index(i, 0).data(ConversationListDelegate::ConversationIDRole).toString() == conversationID)
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
    QString cursorConversationID = lastItem->data(ConversationListDelegate::ConversationIDRole).toString();

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
    bool isGroup = !GroupManager::getGroupManage().getGroupInfo(result.conversationID).conversationID.isEmpty();

    QString username;
    QPixmap avatar;
    if(isGroup)
    {
        username = GroupManager::getGroupManage().getGroupInfo(result.conversationID).groupName;
        avatar = GroupManager::getGroupManage().getGroupAvatar(result.conversationID, QSize(36, 36));
    }
    else
    {
        QString uid = FriendManage::getFriendManage().getFriendUIDByConversationID(result.conversationID);
        FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(uid);
        username = info.username;
        avatar = info.uid.isEmpty() ? QPixmap(":/default/images/defaultAvatar.png") : FriendManage::getFriendManage().getFriendAvatar(info.uid, QSize(36, 36));
    }

    QStandardItem* item = new QStandardItem;

    item->setData(result.conversationID, ConversationListDelegate::ConversationIDRole);
    item->setData(username, ConversationListDelegate::UsernameRole);


    avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
    item->setData(avatar, ConversationListDelegate::AvatarRole);

    item->setData(QString("共 %1 条匹配的聊天记录").arg(result.matchCount), ConversationListDelegate::LastMsgRole);
    item->setData(result.lastTimestamp, ConversationListDelegate::LastTimestampRole);
    item->setData(0, ConversationListDelegate::UnReadRole);
    item->setData(false, ConversationListDelegate::IsOnlineRole);
    item->setData(false, ConversationListDelegate::IsSelectedRole);
    item->setData(true, ConversationListDelegate::IsSearchModeRole);
    if(isGroup)
        item->setData(GroupManager::getGroupManage().getGroupInfo(result.conversationID).memberInfos.size(), ConversationListDelegate::GroupMemberCountRole);

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
    GroupManager::GroupInfo info = GroupManager::getGroupManage().getGroupInfo(this->currentSearchConversationID);
    bool isGroup = !info.conversationID.isEmpty();

    QString name;
    QPixmap avatar;

    if(isMyself)
    {
        name = UserInfo::getUserInfo().getUsername();
        avatar = UserInfo::getUserInfo().getAvatar(QSize(36, 36));
    }
    else if(isGroup)
    {
        if(info.memberInfos.contains(msg.senderUID))
        {
            GroupManager::GroupMemberInfo memberInfo = info.memberInfos.value(msg.senderUID);
            name = memberInfo.username;
            QString avatarUrl = memberInfo.avatarUrl.isEmpty() ? ":/default/images/defaultAvatar.png" : memberInfo.avatarUrl;
            avatar = ImageCacheManager::getManager().fastLoadImage(avatarUrl, 0, 0, QSize(36, 36));
            if(avatar.isNull())
            {
                avatar = QPixmap(":/default/images/defaultAvatar.png");
                QString currentReqSearchConversationID = this->currentSearchConversationID;
                QString currentSenderUID = memberInfo.uid;
                ImageCacheManager::getManager().loadImage(avatarUrl, [this, currentReqSearchConversationID, currentSenderUID](const QPixmap& pix){
                    if(currentReqSearchConversationID != this->currentSearchConversationID)
                        return;

                    for(int i = 0; i < this->model_searchMsg->rowCount(); i++)
                    {
                        QModelIndex index = this->model_searchMsg->index(i, 0);
                        if(index.data(SearchResultListDelegate::SenderUIDRole).toString() == currentSenderUID)
                        {
                            QStandardItem* item = this->model_searchMsg->itemFromIndex(index);
                            QPixmap avatar = pix;
                            avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                            item->setData(avatar, SearchResultListDelegate::AvatarRole);
                        }
                    }
                }, false, 0, 0, QSize(36, 36));
            }
        }
        else
        {
            name = "未知用户";
            avatar = QPixmap(":/default/images/defaultAvatar.png");
        }
    }
    else
    {
        FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(msg.senderUID);
        if (!info.uid.isEmpty())
        {
            name = info.username;
            avatar = FriendManage::getFriendManage().getFriendAvatar(info.uid, QSize(36, 36));
        }
        else
        {
            name = "未知用户";
            avatar = QPixmap(":/default/images/defaultAvatar.png");
        }
    }

    if(avatar.isNull())
        avatar = QPixmap(":/default/images/defaultAvatar.png");

    avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());

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
    item->setData(msg.senderUID, SearchResultListDelegate::SenderUIDRole);

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
    ConversationItem* item = ConversationManager::getConversationManager().getOrCreateConversationItem(conversationID,
                                                                                                       GroupManager::getGroupManage().getGroupInfo(conversationID).conversationID.isEmpty() ?
                                                                                                           ConversationItem::ConversationType::Personal : ConversationItem::ConversationType::Group);
    if(!item)
        return nullptr;

    if(this->map_conversation.contains(item->getConversationID()))
        return this->map_conversation[item->getConversationID()];

    ConversationWidget* cw = new ConversationWidget(this->stackedWidget_Conversation->width(),
                                                    this->stackedWidget_Conversation->height(),
                                                    item, this->stackedWidget_Conversation);

    if(!cw)
        return nullptr;

    connect(cw, &ConversationWidget::switchToInfoWidget, this, [this, item](){
        if(!item)
            return;

        ConversationItem::ConversationType type = item->getConversationType();
        emit switchToInfoWidget(item->getConversationID());
    });

    this->stackedWidget_Conversation->addWidget(cw);
    this->map_conversation[item->getConversationID()] = cw;

    return cw;
}

void ChatWidget::createConversationListItem(ConversationItem *item, const DatabaseManager::ConversationInfo& info)
{
    const QString conversationID = item->getConversationID();
    if(conversationID.isEmpty())
        return;

    for(int i = 0; i < this->model->rowCount(); i++)
    {
        if(this->model->item(i)->data(ConversationListDelegate::ConversationIDRole).toString() == conversationID)
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

    Message lastMsg = item->getLastMessage();
    if(!this->forceConversations.contains(conversationID) && info.conversationID.isEmpty() && lastMsg.tempMsgID.isEmpty() && lastMsg.serverMsgID.isEmpty())
    {
        bindConversationItemSignals(item);
        return;
    }

    QString name;
    QPixmap avatar;
    bool isOnline = false;

    ConversationItem::ConversationType type = item->getConversationType();
    if(type == ConversationItem::ConversationType::Personal)
    {
        QString uid = FriendManage::getFriendManage().getFriendUIDByConversationID(conversationID);
        FriendManage::FriendInfo friend_info = FriendManage::getFriendManage().getFriendInfo(uid);
        if(friend_info.conversationID.isEmpty() || friend_info.uid.isEmpty())
            return;
        name = friend_info.username;
        avatar = FriendManage::getFriendManage().getFriendAvatar(friend_info.uid, QSize(36, 36));
        avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
        isOnline = friend_info.isOnline;
    }
    else if(type == ConversationItem::ConversationType::Group)
    {
        GroupManager::GroupInfo group_info = GroupManager::getGroupManage().getGroupInfo(conversationID);
        if(group_info.conversationID.isEmpty())
            return;

        name = group_info.groupName;
        avatar = GroupManager::getGroupManage().getGroupAvatar(conversationID, QSize(36, 36));
        avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
    }

    QStandardItem* item_standard = new QStandardItem();

    item_standard->setData(conversationID, ConversationListDelegate::ConversationIDRole);
    item_standard->setData(avatar, ConversationListDelegate::AvatarRole);
    item_standard->setData(name, ConversationListDelegate::UsernameRole);
    if(type == ConversationItem::ConversationType::Personal)
    {
        item_standard->setData(isOnline, ConversationListDelegate::IsOnlineRole);
        item_standard->setData(false, ConversationListDelegate::IsGroupRole);
    }
    else
    {
        item_standard->setData(true, ConversationListDelegate::IsGroupRole);
        GroupManager::GroupInfo group_info = GroupManager::getGroupManage().getGroupInfo(conversationID);
        item_standard->setData(group_info.memberInfos.size(), ConversationListDelegate::GroupMemberCountRole);
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

    bindConversationItemSignals(item);
}

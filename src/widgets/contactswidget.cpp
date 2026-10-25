#include "contactswidget.h"

ContactsWidget::ContactsWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);
    this->setObjectName("this");

    initSideBar();
    initSideBarStyle();

    initSideUn();
    initSideUnStyle();

    initStacked();
}

void ContactsWidget::addFriendItem(QString uid, QString sid, QString username, QPixmap avatar, QString email, bool isOnline)
{
    //添加项目
    QStandardItem* item = new QStandardItem();
    item->setData(uid, FriendDelegate::UIDRole);
    item->setData(sid, FriendDelegate::SIDRole);
    item->setData(username, FriendDelegate::UsernameRole);
    item->setData(avatar, FriendDelegate::AvatarRole);
    item->setData(email, FriendDelegate::EmailRole);
    item->setData(isOnline, FriendDelegate::IsOnlineRole);
    this->model->appendRow(item);
}

void ContactsWidget::addGroupItem(const QString &conversationID, const QString &groupID, const QString &groupName, const QString &ownerUID, const QHash<QString, GroupManager::GroupMemberInfo> &memberInfos, const QPixmap &avatar)
{

    QStandardItem* item = new QStandardItem();
    item->setData(conversationID, GroupDelegate::ConversationIDRole);
    item->setData(groupID, GroupDelegate::GroupIDRole);
    item->setData(groupName, GroupDelegate::NameRole);
    item->setData(avatar, GroupDelegate::AvatarRole);
    item->setData(ownerUID, GroupDelegate::OwnerUIDRole);
    item->setData(memberInfos.size(), GroupDelegate::MemberCountRole);
    this->model_group->appendRow(item);
}

void ContactsWidget::switchToInfoWidget(const QString &conversaionID)
{
    bool isFriend = !FriendManage::getFriendManage().getFriendInfo(conversaionID).uid.isEmpty();
    if(isFriend)
    {
        this->btn_myFriend->click();
        FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(conversaionID);
        QString uid = info.uid;
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(uid == index.data(FriendDelegate::UIDRole))
            {
                this->listView_myFriend->setCurrentIndex(index);
                this->listView_myFriend->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);

                QPixmap avatar = FriendManage::getFriendManage().getFriendAvatar(conversaionID, QSize(128, 128));
                avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                this->widget_contactsInfo->changeInfo(avatar,
                                                      info.username,
                                                      conversaionID,
                                                      info.sid,
                                                      info.email,
                                                      info.isOnline);

                this->stackedWidget->setCurrentWidget(this->widget_contactsInfo);
                return;
            }
        }
    }
    else
    {
        this->btn_myGroups->click();
        GroupManager::GroupInfo info = GroupManager::getGroupManage().getGroupInfo(conversaionID);
        for(int i = 0; i < this->model_group->rowCount(); i++)
        {
            QModelIndex index = this->model_group->index(i, 0);
            if(conversaionID == index.data(GroupDelegate::ConversationIDRole))
            {
                this->listView_myFriend->setCurrentIndex(index);
                this->listView_myFriend->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);

                QPixmap avatar = GroupManager::getGroupManage().getGroupAvatar(conversaionID, QSize(128, 128));
                avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());

                this->widget_groupInfo->changeInfo(conversaionID,
                                                      info.groupID,
                                                      info.groupName,
                                                      info.ownerUID,
                                                      info.memberInfos,
                                                      avatar);

                this->stackedWidget->setCurrentWidget(this->widget_groupInfo);
                return;
            }
        }
    }
}

void ContactsWidget::initSideBar()
{
    this->widget_sideBar = new QWidget(this);
    this->widget_sideBar->setObjectName("widget_sideBar");
    this->widget_sideBar->resize(200,this->height() - 3);
    this->widget_sideBar->move(1,1);

    this->btn_addFriend = new QPushButton("添加联系人",this->widget_sideBar);
    this->btn_addFriend->setObjectName("btn_addFriend");
    this->btn_addFriend->resize(this->widget_sideBar->width() - 32,35);
    this->btn_addFriend->move((this->widget_sideBar->width() - this->btn_addFriend->width())/2 , (this->widget_sideBar->width() - this->btn_addFriend->width())/2);

    connect(this->btn_addFriend, &QPushButton::clicked, this, [this](){
        AddNewFriendWidget* anfw = new AddNewFriendWidget(this);
        anfw->show();
    });

    this->edit_searchFriend = new QLineEdit(this->widget_sideBar);
    this->edit_searchFriend->setObjectName("edit_searchFriend");
    this->edit_searchFriend->setPlaceholderText("搜索联系人");
    QAction* searchIcon = new QAction(QIcon(":/default/images/search.png"),"",this->edit_searchFriend);
    this->edit_searchFriend->addAction(searchIcon,QLineEdit::LeadingPosition);
    this->edit_searchFriend->resize(this->widget_sideBar->width() - 32,36);
    this->edit_searchFriend->move((this->widget_sideBar->width() - this->edit_searchFriend->width())/2 ,
                                  this->btn_addFriend->pos().y() + this->btn_addFriend->height() + (this->widget_sideBar->width() - this->btn_addFriend->width())/2);

}

void ContactsWidget::initSideBarStyle()
{
    this->widget_sideBar->setStyleSheet(R"(
                                #widget_sideBar
                                {
                                    background:#ffffff;
                                    border-right:1px solid #e0e0e0;
                                }
                                #btn_addFriend
                                {
                                    color:#ffffff;
                                    background: #4648d4;
                                    border:none;
                                    border-radius: 8px;
                                    font-size: 13px;
                                    font-weight: bold;
                                }
                                #btn_addFriend:hover
                                {
                                    background: #6063ee;
                                }
                                #btn_addFriend:pressed
                                {
                                    background: #2f2ebe;
                                }
                                #edit_searchFriend
                                {
                                    background: #efecf8;
                                    border: 1px solid #f3f4f6;
                                    border-radius: 8px;
                                    padding: 8px 12px 8px 0px;
                                    font-size: 12px;
                                    font-weight:bold;
                                    color: #1b1b23;
                                }
                                #edit_searchFriend:focus
                                {
                                    background: #ffffff;
                                    border: 1px solid #4648d4;
                                    margin-top: 1px;
                                }
                                        )");
}

void ContactsWidget::initSideUn()
{
    this->widget_sideUn = new QWidget(this->widget_sideBar);
    this->widget_sideUn->setObjectName("widget_sideUn");
    int upPos = this->edit_searchFriend->pos().y() + this->edit_searchFriend->height() + (this->widget_sideBar->width() - this->edit_searchFriend->width()) / 2;
    this->widget_sideUn->resize(this->widget_sideBar->width() - 1,this->widget_sideBar->height() - upPos);
    this->widget_sideUn->move(0,upPos);

    this->btn_newFriend = new NewFriendButton(this->widget_sideUn->width(),55,this->widget_sideUn);
    this->btn_newFriend->setObjectName("btn_newFriend");
    this->btn_newFriend->move(0, 10);
    this->btn_newFriend->setCheckable(true);
    this->btn_newFriend->setChecked(false);
    connect(this->btn_newFriend,&QPushButton::clicked,this,[=](){
        this->listView_myFriend->clearSelection();
        this->listView_myFriend->setCurrentIndex(QModelIndex());

        this->selectedUID.clear();

        this->stackedWidget->setCurrentWidget(this->widget_newFriend);
    });

    this->group = new QButtonGroup(this);

    this->btn_myFriend = new QPushButton("我的好友",this->widget_sideUn);
    this->btn_myFriend->setObjectName("btn_myFriend");
    this->btn_myFriend->resize(this->widget_sideUn->width() / 2, 35);
    this->btn_myFriend->move(0, this->btn_newFriend->pos().y()+this->btn_newFriend->height()+8);
    this->btn_myFriend->setCheckable(true);
    this->group->addButton(this->btn_myFriend);
    this->btn_myFriend->setChecked(true);

    connect(this->btn_myFriend, &QPushButton::clicked, [this](){
        this->listView_myFriend->setModel(this->model);
        this->listView_myFriend->setItemDelegate(this->delegate);
        this->edit_searchFriend->setPlaceholderText("搜索联系人");
        this->label_empty->setText("暂无好友");

        if(this->model->rowCount() == 0)
            changeStatus(Empty);
        else
            changeStatus(Hide);
        restoreSelect();
    });

    this->btn_myGroups = new QPushButton("我的群聊",this->widget_sideUn);
    this->btn_myGroups->setObjectName("btn_myGroups");
    this->btn_myGroups->resize(this->widget_sideUn->width() - this->btn_myFriend->width(), 35);
    this->btn_myGroups->move(this->btn_myFriend->pos().x() + this->btn_myFriend->width(),
                             this->btn_myFriend->pos().y());
    this->btn_myGroups->setCheckable(true);
    this->group->addButton(this->btn_myGroups);

    connect(this->btn_myGroups, &QPushButton::clicked, [this](){
        this->listView_myFriend->setModel(this->model_group);
        this->listView_myFriend->setItemDelegate(this->delegate_group);
        this->edit_searchFriend->setPlaceholderText("搜索群聊");
        this->label_empty->setText("暂无群聊");

        if(this->model_group->rowCount() == 0)
            changeStatus(Empty);
        else
            changeStatus(Hide);
        restoreSelect();
    });

    this->listView_myFriend = new QListView(this->widget_sideUn);
    this->listView_myFriend->setObjectName("listView_myFriend");
    this->listView_myFriend->resize(this->widget_sideUn->width(),
                                    this->widget_sideUn->height() - this->btn_myFriend->pos().y() - this->btn_myFriend->height());
    this->listView_myFriend->move(0,this->btn_myFriend->pos().y() + this->btn_myFriend->height());

    this->model = new QStandardItemModel(this);
    this->listView_myFriend->setModel(this->model);

    this->delegate = new FriendDelegate(this);
    this->listView_myFriend->setItemDelegate(this->delegate);

    this->model_group = new QStandardItemModel(this);
    this->delegate_group = new GroupDelegate(this);

    this->listView_myFriend->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->listView_myFriend->setSelectionMode(QAbstractItemView::SingleSelection);
    this->listView_myFriend->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->listView_myFriend->verticalScrollBar()->setSingleStep(6);
    this->listView_myFriend->setResizeMode(QListView::Fixed);

    this->widget_status = new QWidget(this->listView_myFriend);
    this->widget_status->setObjectName("widget_status");
    this->widget_status->resize(this->listView_myFriend->width(), this->listView_myFriend->height());
    this->widget_status->move(0, 0);
    this->widget_status->hide();

    this->label_empty = new QLabel("暂无好友", this->widget_status);
    this->label_empty->setObjectName("label_empty");
    this->label_empty->setAlignment(Qt::AlignCenter);
    this->label_empty->resize(this->widget_status->width(), 40);
    this->label_empty->move(0, (this->widget_status->height() - this->label_empty->height())/2);
    this->label_empty->hide();

    this->label_loading = new QLabel("加载中...", this->widget_status);
    this->label_loading->setObjectName("label_loading");
    this->label_loading->setAlignment(Qt::AlignCenter);
    this->label_loading->resize(this->widget_status->width(), 40);
    this->label_loading->move(0, (this->widget_status->height() - this->label_loading->height())/2);
    this->label_loading->hide();

    this->label_loadingIcon = new LoadingLabel(this->widget_status);
    this->label_loadingIcon->setObjectName("label_loadingIcon");
    this->label_loadingIcon->setPixmap(QIcon(":/default/images/loading.png").pixmap(24));
    this->label_loadingIcon->resize(30, 30);
    this->label_loadingIcon->move((this->widget_status->width() - this->label_loadingIcon->width())/2,
                                  (this->widget_status->height() - this->label_loadingIcon->height())/2 - this->label_loadingIcon->height());
    this->label_loadingIcon->hide();

    this->animation_loading = new QPropertyAnimation(this->label_loadingIcon, "angle", this);
    this->animation_loading->setStartValue(0);
    this->animation_loading->setEndValue(360);
    this->animation_loading->setDuration(2000);
    this->animation_loading->setLoopCount(-1);

    connect(this->delegate, &FriendDelegate::itemClicked, this, [this](const QModelIndex& item){
        this->btn_newFriend->setChecked(false);

        this->selectedUID = item.data(FriendDelegate::UIDRole).toString();
        QString conversationID = FriendManage::getFriendManage().getFriendConversationIDByUID(this->selectedUID);
        FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(conversationID);

        this->widget_contactsInfo->changeInfo(FriendManage::getFriendManage().getFriendAvatar(conversationID, QSize(128, 128)),
                                                  item.data(FriendDelegate::UsernameRole).toString(),
                                                  info.conversationID,
                                                  item.data(FriendDelegate::SIDRole).toString(),
                                                  item.data(FriendDelegate::EmailRole).toString(),
                                                  item.data(FriendDelegate::IsOnlineRole).toBool());
        this->stackedWidget->setCurrentWidget(this->widget_contactsInfo);
    });

    connect(this->delegate_group, &GroupDelegate::itemClicked, this, [this](const QModelIndex& item){
        this->btn_newFriend->setChecked(false);

        this->selectedUID = item.data(GroupDelegate::ConversationIDRole).toString();
        GroupManager::GroupInfo info = GroupManager::getGroupManage().getGroupInfo(this->selectedUID);
        this->widget_groupInfo->changeInfo(info.conversationID, info.groupID, info.groupName, info.ownerUID, info.memberInfos,
                                               GroupManager::getGroupManage().getGroupAvatar(info.conversationID, QSize(128, 128)));
        this->stackedWidget->setCurrentWidget(this->widget_groupInfo);

        if(!GroupManager::getGroupManage().getGroupInfo(info.conversationID).conversationID.isEmpty())
            TcpLongConnection::getTcpClient().getGroupInfo(info.conversationID);
    });

    connect(this->edit_searchFriend, &QLineEdit::textChanged, this, [this](){
        bool isFriend = this->btn_myFriend->isChecked();
        changeStatus(Loading);
        QString text = this->edit_searchFriend->text().trimmed();

        if(isFriend)
        {
            this->model->clear();
            if(text.isEmpty())
                sortedFriendList();
            else
                searchFriend(text);
            if(this->model->rowCount() == 0)
                changeStatus(Empty);
            else
                changeStatus(Hide);
        }
        else
        {
            this->model_group->clear();
            if(text.isEmpty())
                sortedGroupList();
            else
                searchGroup(text);
            if(this->model_group->rowCount() == 0)
                changeStatus(Empty);
            else
                changeStatus(Hide);
        }
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::allFriendList, this, [this](){
        QString text = this->edit_searchFriend->text().trimmed();
        this->model->clear();

        if(text.isEmpty())
        {
            sortedFriendList();
        }
        else
        {
            changeStatus(Loading);
            searchFriend(text);
        }

        if(!this->btn_myFriend->isChecked())
            return;

        if(this->model->rowCount() == 0)
            changeStatus(Empty);
        else
            changeStatus(Hide);
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::friendAvatarUpdate, this, [this](const QString& uid, const QPixmap& avatar, const QSize& size){
        if(size == QSize(38, 38) || size.isEmpty())
        {
            for(int i = 0; i < this->model->rowCount(); i++)
            {
                QModelIndex index = this->model->index(i, 0);
                if(index.data(FriendDelegate::UIDRole).toString() == uid)
                {
                    QStandardItem* item = this->model->itemFromIndex(index);
                    if(!size.isEmpty())
                    {
                        QPixmap avatar_copy = avatar;
                        avatar_copy.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                        item->setData(avatar_copy, FriendDelegate::AvatarRole);
                    }
                    else
                    {
                        QPixmap newAvatar = FriendManage::getFriendManage().getFriendAvatar(FriendManage::getFriendManage().getFriendConversationIDByUID(uid), QSize(38, 38));
                        if(!newAvatar.isNull())
                        {
                            newAvatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                            item->setData(newAvatar, FriendDelegate::AvatarRole);
                        }
                    }
                    break;
                }
            }
        }
        if(size == QSize(128, 128))
        {
            if(uid == this->selectedUID)
                this->widget_contactsInfo->changeSelectedAvatar(avatar);
        }
        else if(size.isEmpty())
        {
            if(uid == this->selectedUID)
            {
                QPixmap bigAvatar = FriendManage::getFriendManage().getFriendAvatar(FriendManage::getFriendManage().getFriendConversationIDByUID(uid), QSize(128, 128));
                if(!bigAvatar.isNull())
                    this->widget_contactsInfo->changeSelectedAvatar(bigAvatar);
            }
        }
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::friendUsernameUpdate, this, [this](const QString& uid, const QString& username){
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(index.data(FriendDelegate::UIDRole).toString() == uid)
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                item->setData(username, FriendDelegate::UsernameRole);
                break;
            }
        }
        if(uid == this->selectedUID)
            this->widget_contactsInfo->changeSelectedUsername(username);
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::friendStatusUpdate, this, [this](const QString& uid, bool isOnline){
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            if(index.data(FriendDelegate::UIDRole).toString() == uid)
            {
                QStandardItem* item = this->model->itemFromIndex(index);
                item->setData(isOnline, FriendDelegate::IsOnlineRole);
                break;
            }
        }
        if(uid == this->selectedUID)
            this->widget_contactsInfo->changeOnlineStatus(isOnline);
    });

    connect(&GroupManager::getGroupManage(), &GroupManager::allGroupList, this, [this](){
        QString text = this->edit_searchFriend->text().trimmed();
        this->model_group->clear();

        if(text.isEmpty())
        {
            sortedGroupList();
        }
        else
        {
            changeStatus(Loading);
            searchGroup(text);
        }

        if(!this->btn_myGroups->isChecked())
            return;

        if(this->model_group->rowCount() == 0)
            changeStatus(Empty);
        else
            changeStatus(Hide);
    });

    connect(&GroupManager::getGroupManage(), &GroupManager::groupAvatarUpdate, this, [this](const QString& conversationID, const QPixmap& avatar, const QSize& size){
        if(size == QSize(38, 38) || size.isEmpty())
        {
            for(int i = 0; i < this->model_group->rowCount(); i++)
            {
                QModelIndex index = this->model_group->index(i, 0);
                if(index.data(GroupDelegate::ConversationIDRole).toString() == conversationID)
                {
                    QStandardItem* item = this->model_group->itemFromIndex(index);
                    if(!size.isEmpty())
                    {
                        QPixmap avatar_copy = avatar;
                        avatar_copy.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                        item->setData(avatar_copy, GroupDelegate::AvatarRole);
                    }
                    else
                    {
                        QPixmap newAvatar = GroupManager::getGroupManage().getGroupAvatar(conversationID, QSize(38, 38));
                        if(!newAvatar.isNull())
                        {
                            newAvatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                            item->setData(newAvatar, GroupDelegate::AvatarRole);
                        }
                    }
                    break;
                }
            }
        }
        if(size == QSize(128, 128))
        {
            if(conversationID == this->selectedUID)
                this->widget_groupInfo->changeSelectedAvatar(avatar);
        }
        else if(size.isEmpty())
        {
            if(conversationID == this->selectedUID)
            {
                QPixmap bigAvatar = GroupManager::getGroupManage().getGroupAvatar(conversationID, QSize(128, 128));
                if(!bigAvatar.isNull())
                    this->widget_groupInfo->changeSelectedAvatar(bigAvatar);
            }
        }
    });

    connect(&GroupManager::getGroupManage(), &GroupManager::groupInfoUpdate, this, [this](const QString& conversationID){
        GroupManager::GroupInfo info = GroupManager::getGroupManage().getGroupInfo(conversationID);
        if(info.conversationID.isEmpty())
            return;

        bool isFound = false;
        for(int i = 0; i < this->model_group->rowCount(); i++) {
            QModelIndex index = this->model_group->index(i, 0);

            QVariant vi = index.data(GroupDelegate::GroupRole);
            if(vi.isValid() && vi.toBool())
                continue;

            if(index.data(GroupDelegate::ConversationIDRole).toString() == conversationID)
            {
                QStandardItem* item = this->model_group->itemFromIndex(index);
                item->setData(info.groupName, GroupDelegate::NameRole);
                item->setData(info.memberInfos.size(), GroupDelegate::MemberCountRole);

                QPixmap avatar = GroupManager::getGroupManage().getGroupAvatar(conversationID, QSize(38, 38));
                avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                item->setData(avatar, GroupDelegate::AvatarRole);

                isFound = true;
                break;
            }
        }

        if(!isFound)
        {
            QPixmap avatar = GroupManager::getGroupManage().getGroupAvatar(conversationID, QSize(38, 38));
            avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());

            addGroupItem(info.conversationID, info.groupID, info.groupName, info.ownerUID, info.memberInfos, avatar);

            if(this->btn_myGroups->isChecked())
            {
                if(this->model_group->rowCount() == 0)
                    changeStatus(Empty);
                else
                    changeStatus(Hide);
            }
        }
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, &ContactsWidget::cleanAll);
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, &ContactsWidget::cleanAll);
    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::refreshExpiredExit, this, &ContactsWidget::cleanAll);

    changeStatus(Empty);
}

void ContactsWidget::initSideUnStyle()
{
    this->widget_sideUn->setStyleSheet(R"(
                                #widget_sideUn
                                {
                                    border-top:1px solid #e0e0e0;
                                    background:#ffffff;
                                }
                                #btn_myFriend
                                {
                                    color:#1f2937;
                                    font-size:12px;
                                    font-weight:bold;
                                    border-right: 1px solid #e0e0e0;
                                    border-radius: 0;
                                    padding-bottom: 2px;
                                }
                                #btn_myGroups
                                {
                                    color:#1f2937;
                                    font-size:12px;
                                    font-weight:bold;
                                    border-radius: 0;
                                    padding-bottom: 2px;
                                }
                                #btn_myFriend:hover,
                                #btn_myGroups:hover
                                {
                                    border-bottom: 2px solid rgba(99, 102, 241, 150);
                                    padding-bottom: 0px;
                                }
                                #btn_myFriend:checked,
                                #btn_myGroups:checked
                                {
                                    border-bottom: 2px solid rgba(99, 102, 241, 255);
                                    padding-bottom: 0px;
                                }
                                #listView_myFriend
                                {
                                    background: transparent;
                                    border-radius:0px;
                                    margin-bottom: 10px;
                                }
                                #listView_myFriend QScrollBar:vertical
                                {
                                    width: 5px;
                                    background: transparent;
                                    margin: 0px;
                                    margin-top: 2px;
                                }
                                #listView_myFriend QScrollBar::handle:vertical
                                {
                                    background: rgba(0, 0, 0, 0.2);
                                    border-radius: 2px;
                                    min-height: 30px;
                                    margin-right: 1px;
                                }
                                #listView_myFriend QScrollBar::handle:vertical:hover
                                {
                                    background: rgba(0, 0, 0, 0.35);
                                }
                                #listView_myFriend QScrollBar::add-line:vertical,
                                #listView_myFriend QScrollBar::sub-line:vertical
                                {
                                    height: 0px;
                                }
                                #listView_myFriend QScrollBar::add-page:vertical,
                                #listView_myFriend QScrollBar::sub-page:vertical
                                {
                                    background: none;
                                }
                                #widget_status[status = "loading"]
                                {
                                    background: rgba(0, 0 ,0, 80);
                                }
                                #widget_status[status = "empty"]
                                {
                                    background: transparent;
                                }
                                #label_loading
                                {
                                    color: #374151;
                                    font-size:14px;
                                    font-weight:600;
                                    padding-left: 6px;
                                }
                                #label_empty
                                {
                                    color:#6B7280;
                                    font-size:15px;
                                    font-weight:600;
                                }
                            )");
}

void ContactsWidget::initStacked()
{
    this->stackedWidget = new QStackedWidget(this);
    this->stackedWidget->setObjectName("stackedWidget");
    this->stackedWidget->resize(this->width() - this->widget_sideBar->width(),this->height());
    this->stackedWidget->move(this->widget_sideBar->width(),0);

    this->widget_noSelect = new QWidget(this->stackedWidget);
    this->widget_noSelect->resize(this->stackedWidget->width(),this->stackedWidget->height());

    this->label_noSelect = new QLabel(this->widget_noSelect);
    this->label_noSelect->setObjectName("label_noSelect");
    this->label_noSelect->resize(60, 60);
    this->label_noSelect->setPixmap(QIcon(":/icon/images/blackIcon.png").pixmap(60));
    this->label_noSelect->move((this->widget_noSelect->width() - this->label_noSelect->width())/2,
                              (this->widget_noSelect->height() - this->label_noSelect->height())/2);

    this->widget_newFriend = new NewFriendManageWidget(this->stackedWidget->width(),this->stackedWidget->height());

    connect(this->widget_newFriend, &NewFriendManageWidget::RequestsNumberChange, this, [this](size_t num){
        this->btn_newFriend->setUnProcessedRequests(num);
    });

    this->widget_contactsInfo = new ContactsInfoWidget(this->stackedWidget->width(),this->stackedWidget->height());
    this->widget_groupInfo = new GroupInfoWidget(this->stackedWidget->width(),this->stackedWidget->height());

    this->stackedWidget->addWidget(this->widget_noSelect);
    this->stackedWidget->addWidget(this->widget_newFriend);
    this->stackedWidget->addWidget(this->widget_contactsInfo);
    this->stackedWidget->addWidget(this->widget_groupInfo);

    this->stackedWidget->setCurrentWidget(this->widget_noSelect);

    connect(this->widget_contactsInfo, &ContactsInfoWidget::openConversation, this, [this](const QString &conversationID){
        //转发信号
        emit openConversation(conversationID);
    });

    connect(this->widget_groupInfo, &GroupInfoWidget::openConversation, this, [this](const QString &conversationID){
        //转发信号
        emit openConversation(conversationID);
    });
}

void ContactsWidget::cleanAll()
{
    this->selectedUID.clear();
    this->model->clear();
    this->model_group->clear();

    this->btn_newFriend->setChecked(false);
    this->btn_myFriend->setChecked(true);
    this->listView_myFriend->setModel(this->model);
    this->listView_myFriend->setItemDelegate(this->delegate);

    this->stackedWidget->setCurrentWidget(this->widget_noSelect);
    changeStatus(Empty);
}

void ContactsWidget::changeStatus(Status status)
{
    this->label_empty->hide();
    this->label_loading->hide();
    this->label_loadingIcon->hide();
    this->animation_loading->stop();
    if(status == Empty)
    {
        this->widget_status->setProperty("status", "empty");
        this->label_empty->show();
        this->widget_status->show();
    }
    else if(status == Loading)
    {
        this->widget_status->setProperty("status", "loading");
        this->label_loading->show();
        this->label_loadingIcon->show();
        this->animation_loading->start();
        this->widget_status->show();
    }
    else if(status == Hide)
    {
        this->widget_status->hide();
    }
    this->widget_status->style()->unpolish(this->widget_status);
    this->widget_status->style()->polish(this->widget_status);
}

void ContactsWidget::sortedFriendList()
{
    QList<FriendManage::FriendInfo> sortedList = FriendManage::getFriendManage().getAllFriend();
    QCollator collator;
    std::sort(sortedList.begin(), sortedList.end(), [&collator, this](const FriendManage::FriendInfo& a, const FriendManage::FriendInfo& b){
        QString letterA = getFirstLetter(a.username);
        QString letterB = getFirstLetter(b.username);
        //让#排最后面
        bool aIsSpecial = (letterA == "#");
        bool bIsSpecial = (letterB == "#");

        if (aIsSpecial && !bIsSpecial)
            return false;
        if (!aIsSpecial && bIsSpecial)
            return true;
        if (aIsSpecial && bIsSpecial)
            return collator.compare(a.username, b.username) < 0;
        return collator.compare(a.username, b.username) < 0;
    });

    QString preGroup;
    for(const FriendManage::FriendInfo& fi : std::as_const(sortedList))
    {
        QString thisGroup = getFirstLetter(fi.username);
        if(thisGroup != preGroup)
        {
            //添加组头
            QStandardItem* header = new QStandardItem(thisGroup);
            header->setData(true, FriendDelegate::GroupRole);
            header->setFlags(header->flags() & ~Qt::ItemIsSelectable);
            this->model->appendRow(header);
            preGroup = thisGroup;
        }

        QPixmap avatar = FriendManage::getFriendManage().getFriendAvatar(fi.conversationID, QSize(38, 38));
        avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
        addFriendItem(fi.uid, fi.sid, fi.username, avatar, fi.email, fi.isOnline);
    }
    restoreSelect();
}

void ContactsWidget::sortedGroupList()
{
    QList<GroupManager::GroupInfo> sortedList = GroupManager::getGroupManage().getAllGroups();
    QCollator collator;
    std::sort(sortedList.begin(), sortedList.end(), [&collator, this](const GroupManager::GroupInfo& a, const GroupManager::GroupInfo& b){
        QString letterA = getFirstLetter(a.groupName);
        QString letterB = getFirstLetter(b.groupName);
        //让#排最后面
        bool aIsSpecial = (letterA == "#");
        bool bIsSpecial = (letterB == "#");

        if (aIsSpecial && !bIsSpecial)
            return false;
        if (!aIsSpecial && bIsSpecial)
            return true;
        if (aIsSpecial && bIsSpecial)
            return collator.compare(a.groupName, b.groupName) < 0;
        return collator.compare(a.groupName, b.groupName) < 0;
    });

    QString preGroup;
    for(const GroupManager::GroupInfo& gi : std::as_const(sortedList))
    {
        QString thisGroup = getFirstLetter(gi.groupName);
        if(thisGroup != preGroup)
        {
            //添加组头
            QStandardItem* header = new QStandardItem(thisGroup);
            header->setData(true, GroupDelegate::GroupRole);
            header->setFlags(header->flags() & ~Qt::ItemIsSelectable);
            this->model_group->appendRow(header);
            preGroup = thisGroup;
        }

        QPixmap avatar = GroupManager::getGroupManage().getGroupAvatar(gi.conversationID, QSize(38, 38));
        avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
        addGroupItem(gi.conversationID, gi.groupID, gi.groupName, gi.ownerUID, gi.memberInfos, avatar);
    }
    restoreSelect();
}

void ContactsWidget::searchFriend(const QString& text)
{
    QList<FriendManage::FriendInfo> list = FriendManage::getFriendManage().getAllFriend();
    for(const FriendManage::FriendInfo& fi : std::as_const(list))
    {
        if(fi.username.contains(text, Qt::CaseInsensitive))
        {
            QPixmap avatar = FriendManage::getFriendManage().getFriendAvatar(fi.conversationID, QSize(38, 38));
            avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
            addFriendItem(fi.uid, fi.sid, fi.username, avatar, fi.email, fi.isOnline);
        }
    }
    restoreSelect();
}

void ContactsWidget::searchGroup(const QString &text)
{
    QList<GroupManager::GroupInfo> list = GroupManager::getGroupManage().getAllGroups();
    for(const GroupManager::GroupInfo& gi : std::as_const(list))
    {
        if(gi.groupName.contains(text, Qt::CaseInsensitive))
        {
            QPixmap avatar = GroupManager::getGroupManage().getGroupAvatar(gi.conversationID, QSize(38, 38));
            avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
            addGroupItem(gi.conversationID, gi.groupID, gi.groupName, gi.ownerUID, gi.memberInfos, avatar);
        }
    }
    restoreSelect();
}

void ContactsWidget::restoreSelect()
{
    bool isFriend = this->btn_myFriend->isChecked();
    if(this->selectedUID.isEmpty())
        return;

    if(isFriend)
    {
        if(FriendManage::getFriendManage().getFriendCount() == 0)
            return;
    }
    else
    {
        if(GroupManager::getGroupManage().getGroupsCount() == 0)
            return;
    }

    if(isFriend)
    {
        for(int i = 0; i < this->model->rowCount(); i++)
        {
            QModelIndex index = this->model->index(i, 0);
            QVariant vi = index.data(FriendDelegate::GroupRole);
            bool isGroup = vi.isValid() ? vi.toBool() : false;
            if(isGroup)
                continue;
            if(index.data(FriendDelegate::UIDRole).toString() == this->selectedUID)
            {
                this->listView_myFriend->setCurrentIndex(index);
                this->listView_myFriend->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
                break;
            }
        }
    }
    else
    {
        for(int i = 0; i < this->model_group->rowCount(); i++)
        {
            QModelIndex index = this->model_group->index(i, 0);
            QVariant vi = index.data(GroupDelegate::GroupRole);
            bool isGroup = vi.isValid() ? vi.toBool() : false;
            if(isGroup)
                continue;

            //此处复用uid 当为群聊的时候作为会话ID
            if(index.data(GroupDelegate::ConversationIDRole).toString() == this->selectedUID)
            {
                this->listView_myFriend->setCurrentIndex(index);
                this->listView_myFriend->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
                break;
            }
        }
    }

    if(!this->selectedUID.isEmpty())
    {
        if(isFriend)
        {
            QString conversationID = FriendManage::getFriendManage().getFriendConversationIDByUID(this->selectedUID);
            FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(conversationID);

            this->widget_contactsInfo->changeInfo(FriendManage::getFriendManage().getFriendAvatar(info.conversationID, QSize(128, 128)), info.username, info.conversationID, info.sid,
                                                  info.email, info.isOnline);
        }
        else
        {
            //设置群聊info界面
        }
    }
}

QString ContactsWidget::getFirstLetter(const QString &str)
{
    static QString letterTable = "YDYQSXMWZSSXJBYMGCCZQPSSQBYCDSCDQLDYLYBSSJGYZZJJFKCCLZDHWDWZJLJPFYYNWJJTMYHZWZHFLZPPQHGSCYYYNJQYXXGJ"
    "HHSDSJNKKTMOMLCRXYPSNQSECCQZGGLLYJLMYZZSECYKYYHQWJSSGGYXYZYJWWKDJHYCHMYXJTLXJYQBYXZLDWRDJRWYSRLDZJPC"
    "BZJJBRCFTLECZSTZFXXZHTRQHYBDLYCZSSYMMRFMYQZPWWJJYFCRWFDFZQPYDDWYXKYJAWJFFXYPSFTZYHHYZYSWCJYXSCLCXXWZ"
    "ZXNBGNNXBXLZSZSBSGPYSYZDHMDZBQBZCWDZZYYTZHBTSYYBZGNTNXQYWQSKBPHHLXGYBFMJEBJHHGQTJCYSXSTKZHLYCKGLYSMZ"
    "XYALMELDCCXGZYRJXSDLTYZCQKCNNJWHJTZZCQLJSTSTBNXBTYXCEQXGKWJYFLZQLYHYXSPSFXLMPBYSXXXYDJCZYLLLSJXFHJXP"
    "JBTFFYABYXBHZZBJYZLWLCZGGBTSSMDTJZXPTHYQTGLJSCQFZKJZJQNLZWLSLHDZBWJNCJZYZSQQYCQYRZCJJWYBRTWPYFTWEXCS"
    "KDZCTBZHYZZYYJXZCFFZZMJYXXSDZZOTTBZLQWFCKSZSXFYRLNYJMBDTHJXSQQCCSBXYYTSYFBXDZTGBCNSLCYZZPSAZYZZSCJCS"
    "HZQYDXLBPJLLMQXTYDZXSQJTZPXLCGLQTZWJBHCTSYJSFXYEJJTLBGXSXJMYJQQPFZASYJNTYDJXKJCDJSZCBARTDCLYJQMWNQNC"
    "LLLKBYBZZSYHQQLTWLCCXTXLLZNTYLNEWYZYXCZXXGRKRMTCNDNJTSYYSSDQDGHSDBJGHRWRQLYBGLXHLGTGXBQJDZPYJSJYJCTM"
    "RNYMGRZJCZGJMZMGXMPRYXKJNYMSGMZJYMKMFXMLDTGFBHCJHKYLPFMDXLQJJSMTQGZSJLQDLDGJYCALCMZCSDJLLNXDJFFFFJCZ"
    "FMZFFPFKHKGDPSXKTACJDHHZDDCRRCFQYJKQCCWJDXHWJLYLLZGCFCQDSMLZPBJJPLSBCJGGDCKKDEZSQCCKJGCGKDJTJDLZYCXK"
    "LQSCGJCLTFPCQCZGWPJDQYZJJBYJHSJDZWGFSJGZKQCCZLLPSPKJGQJHZZLJPLGJGJJTHJJYJZCZMLZLYQBGJWMLJKXZDZNJQSYZ"
    "MLJLLJKYWXMKJLHSKJGBMCLYYMKXJQLBMLLKMDXXKWYXYSLMLPSJQQJQXYXFJTJDXMXXLLCXQBSYJBGWYMBGGBCYXPJYGPEPFGDJ"
    "GBHBNSQJYZJKJKHXQFGQZKFHYGKHDKLLSDJQXPQYKYBNQSXQNSZSWHBSXWHXWBZZXDMNSJBSBKBBZKLYLXGWXDRWYQZMYWSJQLCJ"
    "XXJXKJEQXSCYETLZHLYYYSDZPAQYZCMTLSHTZCFYZYXYLJSDCJQAGYSLCQLYYYSHMRQQKLDXZSCSSSYDYCJYSFSJBFRSSZQSBXXP"
    "XJYSDRCKGJLGDKZJZBDKTCSYQPYHSTCLDJDHMXMCGXYZHJDDTMHLTXZXYLYMOHYJCLTYFBQQXPFBDFHHTKSQHZYYWCNXXCRWHOWG"
    "YJLEGWDQCWGFJYCSNTMYTOLBYGWQWESJPWNMLRYDZSZTXYQPZGCWXHNGPYXSHMYQJXZTDPPBFYHZHTJYFDZWKGKZBLDNTSXHQEEG"
    "ZZYLZMMZYJZGXZXKHKSTXNXXWYLYAPSTHXDWHZYMPXAGKYDXBHNHXKDPJNMYHYLPMGOCSLNZHKXXLPZZLBMLSFBHHGYGYYGGBHSC"
    "YAQTYWLXTZQCEZYDQDQMMHTKLLSZHLSJZWFYHQSWSCWLQAZYNYTLSXTHAZNKZZSZZLAXXZWWCTGQQTDDYZTCCHYQZFLXPSLZYGPZ"
    "SZNGLNDQTBDLXGTCTAJDKYWNSYZLJHHZZCWNYYZYWMHYCHHYXHJKZWSXHZYXLYSKQYSPSLYZWMYPPKBYGLKZHTYXAXQSYSHXASMC"
    "HKDSCRSWJPWXSGZJLWWSCHSJHSQNHCSEGNDAQTBAALZZMSSTDQJCJKTSCJAXPLGGXHHGXXZCXPDMMHLDGTYBYSJMXHMRCPXXJZCK"
    "ZXSHMLQXXTTHXWZFKHCCZDYTCJYXQHLXDHYPJQXYLSYYDZOZJNYXQEZYSQYAYXWYPDGXDDXSPPYZNDLTWRHXYDXZZJHTCXMCZLHP"
    "YYYYMHZLLHNXMYLLLMDCPPXHMXDKYCYRDLTXJCHHZZXZLCCLYLNZSHZJZZLNNRLWHYQSNJHXYNTTTKYJPYCHHYEGKCTTWLGQRLGG"
    "TGTYGYHPYHYLQYQGCWYQKPYYYTTTTLHYHLLTYTTSPLKYZXGZWGPYDSSZZDQXSKCQNMJJZZBXYQMJRTFFBTKHZKBXLJJKDXJTLBWF"
    "ZPPTKQTZTGPDGNTPJYFALQMKGXBDCLZFHZCLLLLADPMXDJHLCCLGYHDZFGYDDGCYYFGYDXKSSEBDHYKDKDKHNAXXYBPBYYHXZQGA"
    "FFQYJXDMLJCSQZLLPCHBSXGJYNDYBYQSPZWJLZKSDDTACTBXZDYZYPJZQSJNKKTKNJDJGYYPGTLFYQKASDNTCYHBLWDZHBBYDWJR"
    "YGKZYHEYYFJMSDTYFZJJHGCXPLXHLDWXXJKYTCYKSSSMTWCTTQZLPBSZDZWZXGZAGYKTYWXLHLSPBCLLOQMMZSSLCMBJCSZZKYDC"
    "ZJGQQDSMCYTZQQLWZQZXSSFPTTFQMDDZDSHDTDWFHTDYZJYQJQKYPBDJYYXTLJHDRQXXXHAYDHRJLKLYTWHLLRLLRCXYLBWSRSZZ"
    "SYMKZZHHKYHXKSMDSYDYCJPBZBSQLFCXXXNXKXWYWSDZYQOGGQMMYHCDZTTFJYYBGSTTTYBYKJDHKYXBELHTYPJQNFXFDYKZHQKZ"
    "BYJTZBXHFDXKDASWTAWAJLDYJSFHBLDNNTNQJTJNCHXFJSRFWHZFMDRYJYJWZPDJKZYJYMPCYZNYNXFBYTFYFWYGDBNZZZDNYTXZ"
    "EMMQBSQEHXFZMBMFLZZSRXYMJGSXWZJSPRYDJSJGXHJJGLJJYNZZJXHGXKYMLPYYYCXYTWQZSWHWLYRJLPXSLSXMFSWWKLCTNXNY"
    "NPSJSZHDZEPTXMYYWXYYSYWLXJQZQXZDCLEEELMCPJPCLWBXSQHFWWTFFJTNQJHJQDXHWLBYZNFJLALKYYJLDXHHYCSTYYWNRJYX"
    "YWTRMDRQHWQCMFJDYZMHMYYXJWMYZQZXTLMRSPWWCHAQBXYGZYPXYYRRCLMPYMGKSJSZYSRMYJSNXTPLNBAPPYPYLXYYZKYNLDZY"
    "JZCZNNLMZHHARQMPGWQTZMXXMLLHGDZXYHXKYXYCJMFFYYHJFSBSSQLXXNDYCANNMTCJCYPRRNYTYQNYYMBMSXNDLYLYSLJRLXYS"
    "XQMLLYZLZJJJKYZZCSFBZXXMSTBJGNXYZHLXNMCWSCYZYFZLXBRNNNYLBNRTGZQYSATSWRYHYJZMZDHZGZDWYBSSCSKXSYHYTXXG"
    "CQGXZZSHYXJSCRHMKKBXCZJYJYMKQHZJFNBHMQHYSNJNZYBKNQMCLGQHWLZNZSWXKHLJHYYBQLBFCDSXDLDSPFZPSKJYZWZXZDDX"
    "JSMMEGJSCSSMGCLXXKYYYLNYPWWWGYDKZJGGGZGGSYCKNJWNJPCXBJJTQTJWDSSPJXZXNZXUMELPXFSXTLLXCLJXJJLJZXCTPSWX"
    "LYDHLYQRWHSYCSQYYBYAYWJJJQFWQCQQCJQGXALDBZZYJGKGXPLTZYFXJLTPADKYQHPMATLCPDCKBMTXYBHKLENXDLEEGQDYMSAW"
    "HZMLJTWYGXLYQZLJEEYYBQQFFNLYXRDSCTGJGXYYNKLLYQKCCTLHJLQMKKZGCYYGLLLJDZGYDHZWXPYSJBZKDZGYZZHYWYFQYTYZ"
    "SZYEZZLYMHJJHTSMQWYZLKYYWZCSRKQYTLTDXWCTYJKLWSQZWBDCQYNCJSRSZJLKCDCDTLZZZACQQZZDDXYPLXZBQJYLZLLLQDDZ"
    "QJYJYJZYXNYYYNYJXKXDAZWYRDLJYYYRJLXLLDYXJCYWYWNQCCLDDNYYYNYCKCZHXXCCLGZQJGKWPPCQQJYSBZZXYJSQPXJPZBSB"
    "DSFNSFPZXHDWZTDWPPTFLZZBZDMYYPQJRSDZSQZSQXBDGCPZSWDWCSQZGMDHZXMWWFYBPDGPHTMJTHZSMMBGZMBZJCFZWFZBBZMQ"
    "CFMBDMCJXLGPNJBBXGYHYYJGPTZGZMQBQTCGYXJXLWZKYDPDYMGCFTPFXYZTZXDZXTGKMTYBBCLBJASKYTSSQYYMSZXFJEWLXLLS"
    "ZBQJJJAKLYLXLYCCTSXMCWFKKKBSXLLLLJYXTYLTJYYTDPJHNHNNKBYQNFQYYZBYYESSESSGDYHFHWTCJBSDZZTFDMXHCNJZYMQW"
    "SRYJDZJQPDQBBSTJGGFBKJBXTGQHNGWJXJGDLLTHZHHYYYYYYSXWTYYYCCBDBPYPZYCCZYJPZYWCBDLFWZCWJDXXHYHLHWZZXJTC"
    "ZLCDPXUJCZZZLYXJJTXPHFXWPYWXZPTDZZBDZCYHJHMLXBQXSBYLRDTGJRRCTTTHYTCZWMXFYTWWZCWJWXJYWCSKYBZSCCTZQNHX"
    "NWXXKHKFHTSWOCCJYBCMPZZYKBNNZPBZHHZDLSYDDYTYFJPXYNGFXBYQXCBHXCPSXTYZDMKYSNXSXLHKMZXLYHDHKWHXXSSKQYHH"
    "CJYXGLHZXCSNHEKDTGZXQYPKDHEXTYKCNYMYYYPKQYYYKXZLTHJQTBYQHXBMYHSQCKWWYLLHCYYLNNEQXQWMCFBDCCMLJGGXDQKT"
    "LXKGNQCDGZJWYJJLYHHQTTTNWCHMXCXWHWSZJYDJCCDBQCDGDNYXZTHCQRXCBHZTQCBXWGQWYYBXHMBYMYQTYEXMQKYAQYRGYZSL"
    "FYKKQHYSSQYSHJGJCNXKZYCXSBXYXHYYLSTYCXQTHYSMGSCPMMGCCCCCMTZTASMGQZJHKLOSQYLSWTMXSYQKDZLJQQYPLSYCZTCQ"
    "QPBBQJZCLPKHQZYYXXDTDDTSJCXFFLLCHQXMJLWCJCXTSPYCXNDTJSHJWXDQQJSKXYAMYLSJHMLALYKXCYYDMNMDQMXMCZNNCYBZ"
    "KKYFLMCHCMLHXRCJJHSYLNMTJZGZGYWJXSRXCWJGJQHQZDQJDCJJZKJKGDZQGJJYJYLXZXXCDQHHHEYTMHLFSBDJSYYSHFYSTCZQ"
    "LPBDRFRZTZYKYWHSZYQKWDQZRKMSYNBCRXQBJYFAZPZZEDZCJYWBCJWHYJBQSZYWRYSZPTDKZPFPBNZTKLQYHBBZPNPPTYZZYBQN"
    "YDCPJMMCYCQMCYFZZDCMNLFPBPLNGQJTBTTNJZPZBBZNJKLJQYLNBZQHKSJZNGGQSZZKYXSHPZSNBCGZKDDZQANZHJKDRTLZLSWJ"
    "LJZLYWTJNDJZJHXYAYNCBGTZCSSQMNJPJYTYSWXZFKWJQTKHTZPLBHSNJZSYZBWZZZZLSYLSBJHDWWQPSLMMFBJDWAQYZTCJTBNN"
    "WZXQXCDSLQGDSDPDZHJTQQPSWLYYJZLGYXYZLCTCBJTKTYCZJTQKBSJLGMGZDMCSGPYNJZYQYYKNXRPWSZXMTNCSZZYXYBYHYZAX"
    "YWQCJTLLCKJJTJHGDXDXYQYZZBYWDLWQCGLZGJGQRQZCZSSBCRPCSKYDZNXJSQGXSSJMYDNSTZTPBDLTKZWXQWQTZEXNQCZGWEZK"
    "SSBYBRTSSSLCCGBPSZQSZLCCGLLLZXHZQTHCZMQGYZQZNMCOCSZJMMZSQPJYGQLJYJPPLDXRGZYXCCSXHSHGTZNLZWZKJCXTCFCJ"
    "XLBMQBCZZWPQDNHXLJCTHYZLGYLNLSZZPCXDSCQQHJQKSXZPBAJYEMSMJTZDXLCJYRYYNWJBNGZZTMJXLTBSLYRZPYLSSCNXPHLL"
    "HYLLQQZQLXYMRSYCXZLMMCZLTZSDWTJJLLNZGGQXPFSKYGYGHBFZPDKMWGHCXMSGDXJMCJZDYCABXJDLNBCDQYGSKYDQTXDJJYXM"
    "SZQAZDZFSLQXYJSJZYLBTXXWXQQZBJZUFBBLYLWDSLJHXJYZJWTDJCZFQZQZZDZSXZZQLZCDZFJHYSPYMPQZMLPPLFFXJJNZZYLS"
    "JEYQZFPFZKSYWJJJHRDJZZXTXXGLGHYDXCSKYSWMMZCWYBAZBJKSHFHJCXMHFQHYXXYZFTSJYZFXYXPZLCHMZMBXHZZSXYFYMNCW"
    "DABAZLXKTCSHHXKXJJZJSTHYGXSXYYHHHJWXKZXSSBZZWHHHCWTZZZPJXSNXQQJGZYZYWLLCWXZFXXYXYHXMKYYSWSQMNLNAYCYS"
    "PMJKHWCQHYLAJJMZXHMMCNZHBHXCLXTJPLTXYJHDYYLTTXFSZHYXXSJBJYAYRSMXYPLCKDUYHLXRLNLLSTYZYYQYGYHHSCCSMZCT"
    "ZQXKYQFPYYRPFFLKQUNTSZLLZMWWTCQQYZWTLLMLMPWMBZSSTZRBPDDTLQJJBXZCSRZQQYGWCSXFWZLXCCRSZDZMCYGGDZQSGTJS"
    "WLJMYMMZYHFBJDGYXCCPSHXNZCSBSJYJGJMPPWAFFYFNXHYZXZYLREMZGZCYZSSZDLLJCSQFNXZKPTXZGXJJGFMYYYSNBTYLBNLH"
    "PFZDCYFBMGQRRSSSZXYSGTZRNYDZZCDGPJAFJFZKNZBLCZSZPSGCYCJSZLMLRSZBZZLDLSLLYSXSQZQLYXZLSKKBRXBRBZCYCXZZ"
    "ZEEYFGKLZLYYHGZSGZLFJHGTGWKRAAJYZKZQTSSHJJXDCYZUYJLZYRZDQQHGJZXSSZBYKJPBFRTJXLLFQWJHYLQTYMBLPZDXTZYG"
    "BDHZZRBGXHWNJTJXLKSCFSMWLSDQYSJTXKZSCFWJLBXFTZLLJZLLQBLSQMQQCGCZFPBPHZCZJLPYYGGDTGWDCFCZQYYYQYSSCLXZ"
    "SKLZZZGFFCQNWGLHQYZJJCZLQZZYJPJZZBPDCCMHJGXDQDGDLZQMFGPSYTSDYFWWDJZJYSXYYCZCYHZWPBYKXRYLYBHKJKSFXTZJ"
    "MMCKHLLTNYYMSYXYZPYJQYCSYCWMTJJKQYRHLLQXPSGTLYYCLJSCPXJYZFNMLRGJJTYZBXYZMSJYJHHFZQMSYXRSZCWTLRTQZSST"
    "KXGQKGSPTGCZNJSJCQCXHMXGGZTQYDJKZDLBZSXJLHYQGGGTHQSZPYHJHHGYYGKGGCWJZZYLCZLXQSFTGZSLLLMLJSKCTBLLZZSZ"
    "MMNYTPZSXQHJCJYQXYZXZQZCPSHKZZYSXCDFGMWQRLLQXRFZTLYSTCTMJCXJJXHJNXTNRZTZFQYHQGLLGCXSZSJDJLJCYDSJTLNY"
    "XHSZXCGJZYQPYLFHDJSBPCCZHJJJQZJQDYBSSLLCMYTTMQTBHJQNNYGKYRQYQMZGCJKPDCGMYZHQLLSLLCLMHOLZGDYYFZSLJCQZ"
    "LYLZQJESHNYLLJXGJXLYSYYYXNBZLJSSZCQQCJYLLZLTJYLLZLLBNYLGQCHXYYXOXCXQKYJXXXYKLXSXXYQXCYKQXQCSGYXXYQXY"
    "GYTQOHXHXPYXXXULCYEYCHZZCBWQBBWJQZSCSZSSLZYLKDESJZWMYMCYTSDSXXSCJPQQSQYLYYZYCMDJDZYWCBTJSYDJKCYDDJLB"
    "DJJSODZYSYXQQYXDHHGQQYQHDYXWGMMMAJDYBBBPPBCMUUPLJZSMTXERXJMHQNUTPJDCBSSMSSSTKJTSSMMTRCPLZSZMLQDSDMJM"
    "QPNQDXCFYNBFSDQXYXHYAYKQYDDLQYYYSSZBYDSLNTFQTZQPZMCHDHCZCWFDXTMYQSPHQYYXSRGJCWTJTZZQMGWJJTJHTQJBBHWZ"
    "PXXHYQFXXQYWYYHYSCDYDHHQMNMTMWCPBSZPPZZGLMZFOLLCFWHMMSJZTTDHZZYFFYTZZGZYSKYJXQYJZQBHMBZZLYGHGFMSHPZF"
    "ZSNCLPBQSNJXZSLXXFPMTYJYGBXLLDLXPZJYZJYHHZCYWHJYLSJEXFSZZYWXKZJLUYDTMLYMQJPWXYHXSKTQJEZRPXXZHHMHWQPW"
    "QLYJJQJJZSZCPHJLCHHNXJLQWZJHBMZYXBDHHYPZLHLHLGFWLCHYYTLHJXCJMSCPXSTKPNHQXSRTYXXTESYJCTLSSLSTDLLLWWYH"
    "DHRJZSFGXTSYCZYNYHTDHWJSLHTZDQDJZXXQHGYLTZPHCSQFCLNJTCLZPFSTPDYNYLGMJLLYCQHYSSHCHYLHQYQTMZYPBYWRFQYK"
    "QSYSLZDQJMPXYYSSRHZJNYWTQDFZBWWTWWRXCWHGYHXMKMYYYQMSMZHNGCEPMLQQMTCWCTMMPXJPJJHFXYYZSXZHTYBMSTSYJTTQ"
    "QQYYLHYNPYQZLCYZHZWSMYLKFJXLWGXYPJYTYSYXYMZCKTTWLKSMZSYLMPWLZWXWQZSSAQSYXYRHSSNTSRAPXCPWCMGDXHXZDZYF"
    "JHGZTTSBJHGYZSZYSMYCLLLXBTYXHBBZJKSSDMALXHYCFYGMQYPJYCQXJLLLJGSLZGQLYCJCCZOTYXMTMTTLLWTGPXYMZMKLPSZZ"
    "ZXHKQYSXCTYJZYHXSHYXZKXLZWPSQPYHJWPJPWXQQYLXSDHMRSLZZYZWTTCYXYSZZSHBSCCSTPLWSSCJCHNLCGCHSSPHYLHFHHXJ"
    "SXYLLNYLSZDHZXYLSXLWZYKCLDYAXZCMDDYSPJTQJZLNWQPSSSWCTSTSZLBLNXSMNYYMJQBQHRZWTYYDCHQLXKPZWBGQYBKFCMZW"
    "PZLLYYLSZYDWHXPSBCMLJBSCGBHXLQHYRLJXYSWXWXZSLDFHLSLYNJLZYFLYJYCDRJLFSYZFSLLCQYQFGJYHYXZLYLMSTDJCYHBZ"
    "LLNWLXXYGYYHSMGDHXXHHLZZJZXCZZZCYQZFNGWPYLCPKPYYPMCLQKDGXZGGWQBDXZZKZFBXXLZXJTPJPTTBYTSZZDWSLCHZHSLT"
    "YXHQLHYXXXYYZYSWTXZKHLXZXZPYHGCHKCFSYHUTJRLXFJXPTZTWHPLYXFCRHXSHXKYXXYHZQDXQWULHYHMJTBFLKHTXCWHJFWJC"
    "FPQRYQXCYYYQYGRPYWSGSUNGWCHKZDXYFLXXHJJBYZWTSXXNCYJJYMSWZJQRMHXZWFQSYLZJZGBHYNSLBGTTCSYBYXXWXYHXYYXN"
    "SQYXMQYWRGYQLXBBZLJSYLPSYTJZYHYZAWLRORJMKSCZJXXXYXCHDYXRYXXJDTSQFXLYLTSFFYXLMTYJMJUYYYXLTZCSXQZQHZXL"
    "YYXZHDNBRXXXJCTYHLBRLMBRLLAXKYLLLJLYXXLYCRYLCJTGJCMTLZLLCYZZPZPCYAWHJJFYBDYYZSMPCKZDQYQPBPCJPDCYZMDP"
    "BCYYDYCNNPLMTMLRMFMMGWYZBSJGYGSMZQQQZTXMKQWGXLLPJGZBQCDJJJFPKJKCXBLJMSWMDTQJXLDLPPBXCWRCQFBFQJCZAHZG"
    "MYKPHYYHZYKNDKZMBPJYXPXYHLFPNYYGXJDBKXNXHJMZJXSTRSTLDXSKZYSYBZXJLXYSLBZYSLHXJPFXPQNBYLLJQKYGZMCYZZYM"
    "CCSLCLHZFWFWYXZMWSXTYNXJHPYYMCYSPMHYSMYDYSHQYZCHMJJMZCAAGCFJBBHPLYZYLXXSDJGXDHKXXTXXNBHRMLYJSLTXMRHN"
    "LXQJXYZLLYSWQGDLBJHDCGJYQYCMHWFMJYBMBYJYJWYMDPWHXQLDYGPDFXXBCGJSPCKRSSYZJMSLBZZJFLJJJLGXZGYXYXLSZQYX"
    "BEXYXHGCXBPLDYHWETTWWCJMBTXCHXYQXLLXFLYXLLJLSSFWDPZSMYJCLMWYTCZPCHQEKCQBWLCQYDPLQPPQZQFJQDJHYMMCXTXD"
    "RMJWRHXCJZYLQXDYYNHYYHRSLSRSYWWZJYMTLTLLGTQCJZYABTCKZCJYCCQLJZQXALMZYHYWLWDXZXQDLLQSHGPJFJLJHJABCQZD"
    "JGTKHSSTCYJLPSWZLXZXRWGLDLZRLZXTGSLLLLZLYXXWGDZYGBDPHZPBRLWSXQBPFDWOFMWHLYPCBJCCLDMBZPBZZLCYQXLDOMZB"
    "LZWPDWYYGDSTTHCSQSCCRSSSYSLFYBFNTYJSZDFNDPDHDZZMBBLSLCMYFFGTJJQWFTMTPJWFNLBZCMMJTGBDZLQLPYFHYYMJYLSD"
    "CHDZJWJCCTLJCLDTLJJCPDDSQDSSZYBNDBJLGGJZXSXNLYCYBJXQYCBYLZCFZPPGKCXZDZFZTJJFJSJXZBNZYJQTTYJYHTYCZHYM"
    "DJXTTMPXSPLZCDWSLSHXYPZGTFMLCJTYCBPMGDKWYCYZCDSZZYHFLYCTYGWHKJYYLSJCXGYWJCBLLCSNDDBTZBSCLYZCZZSSQDLL"
    "MQYYHFSLQLLXFTYHABXGWNYWYYPLLSDLDLLBJCYXJZMLHLJDXYYQYTDLLLBUGBFDFBBQJZZMDPJHGCLGMJJPGAEHHBWCQXAXHHHZ"
    "CHXYPHJAXHLPHJPGPZJQCQZGJJZZUZDMQYYBZZPHYHYBWHAZYJHYKFGDPFQSDLZMLJXKXGALXZDAGLMDGXMWZQYXXDXXPFDMMSSY"
    "MPFMDMMKXKSYZYSHDZKXSYSMMZZZMSYDNZZCZXFPLSTMZDNMXCKJMZTYYMZMZZMSXHHDCZJEMXXKLJSTLWLSQLYJZLLZJSSDPPMH"
    "NLZJCZYHMXXHGZCJMDHXTKGRMXFWMCGMWKDTKSXQMMMFZZYDKMSCLCMPCGMHSPXQPZDSSLCXKYXTWLWJYAHZJGZQMCSNXYYMMPML"
    "KJXMHLMLQMXCTKZMJQYSZJSYSZHSYJZJCDAJZYBSDQJZGWZQQXFKDMSDJLFWEHKZQKJPEYPZYSZCDWYJFFMZZYLTTDZZEFMZLBNP"
    "PLPLPEPSZALLTYLKCKQZKGENQLWAGYXYDPXLHSXQQWQCQXQCLHYXXMLYCCWLYMQYSKGCHLCJNSZKPYZKCQZQLJPDMDZHLASXLBYD"
    "WQLWDNBQCRYDDZTJYBKBWSZDXDTNPJDTCTQDFXQQMGNXECLTTBKPWSLCTYQLPWYZZKLPYGZCQQPLLKCCYLPQMZCZQCLJSLQZDJXL"
    "DDHPZQDLJJXZQDXYZQKZLJCYQDYJPPYPQYKJYRMPCBYMCXKLLZLLFQPYLLLMBSGLCYSSLRSYSQTMXYXZQZFDZUYSYZTFFMZZSMZQ"
    "HZSSCCMLYXWTPZGXZJGZGSJSGKDDHTQGGZLLBJDZLCBCHYXYZHZFYWXYZYMSDBZZYJGTSMTFXQYXQSTDGSLNXDLRYZZLRYYLXQHT"
    "XSRTZNGZXBNQQZFMYKMZJBZYMKBPNLYZPBLMCNQYZZZSJZHJCTZKHYZZJRDYZHNPXGLFZTLKGJTCTSSYLLGZRZBBQZZKLPKLCZYS"
    "SUYXBJFPNJZZXCDWXZYJXZZDJJKGGRSRJKMSMZJLSJYWQSKYHQJSXPJZZZLSNSHRNYPZTWCHKLPSRZLZXYJQXQKYSJYCZTLQZYBB"
    "YBWZPQDWWYZCYTJCJXCKCWDKKZXSGKDZXWWYYJQYYTCYTDLLXWKCZKKLCCLZCQQDZLQLCSFQCHQHSFSMQZZLNBJJZBSJHTSZDYSJ"
    "QJPDLZCDCWJKJZZLPYCGMZWDJJBSJQZSYZYHHXJPBJYDSSXDZNCGLQMBTSFSBPDZDLZNFGFJGFSMPXJQLMBLGQCYYXBQKDJJQYRF"
    "KZTJDHCZKLBSDZCFJTPLLJGXHYXZCSSZZXSTJYGKGCKGYOQXJPLZPBPGTGYJZGHZQZZLBJLSQFZGKQQJZGYCZBZQTLDXRJXBSXXP"
    "ZXHYZYCLWDXJJHXMFDZPFZHQHQMQGKSLYHTYCGFRZGNQXCLPDLBZCSCZQLLJBLHBZCYPZZPPDYMZZSGYHCKCPZJGSLJLNSCDSLDL"
    "XBMSTLDDFJMKDJDHZLZXLSZQPQPGJLLYBDSZGQLBZLSLKYYHZTTNTJYQTZZPSZQZTLLJTYYLLQLLQYZQLBDZLSLYYZYMDFSZSNHL"
    "XZNCZQZPBWSKRFBSYZMTHBLGJPMCZZLSTLXSHTCSYZLZBLFEQHLXFLCJLYLJQCBZLZJHHSSTBRMHXZHJZCLXFNBGXGTQJCZTMSFZ"
    "KJMSSNXLJKBHSJXNTNLZDNTLMSJXGZJYJCZXYJYJWRWWQNZTNFJSZPZSHZJFYRDJSFSZJZBJFZQZZHZLXFYSBZQLZSGYFTZDCSZX"
    "ZJBQMSZKJRHYJZCKMJKHCHGTXKXQGLXPXFXTRTYLXJXHDTSJXHJZJXZWZLCQSBTXWXGXTXXHXFTSDKFJHZYJFJXRZSDLLLTQSQQZ"
    "QWZXSYQTWGWBZCGZLLYZBCLMQQTZHZXZXLJFRMYZFLXYSQXXJKXRMQDZDMMYYBSQBHGZMWFWXGMXLZPYYTGZYCCDXYZXYWGSYJYZ"
    "NBHPZJSQSYXSXRTFYZGRHZTXSZZTHCBFCLSYXZLZQMZLMPLMXZJXSFLBYZMYQHXJSXRXSQZZZSSLYFRCZJRCRXHHZXQYDYHXSJJH"
    "ZCXZBTYNSYSXJBQLPXZQPYMLXZKYXLXCJLCYSXXZZLXDLLLJJYHZXGYJWKJRWYHCPSGNRZLFZWFZZNSXGXFLZSXZZZBFCSYJDBRJ"
    "KRDHHGXJLJJTGXJXXSTJTJXLYXQFCSGSWMSBCTLQZZWLZZKXJMLTMJYHSDDBXGZHDLBMYJFRZFSGCLYJBPMLYSMSXLSZJQQHJZFX"
    "GFQFQBPXZGYYQXGZTCQWYLTLGWSGWHRLFSFGZJMGMGBGTJFSYZZGZYZAFLSSPMLPFLCWBJZCLJJMZLPJJLYMQDMYYYFBGYGYZMLY"
    "ZDXQYXRQQQHSYYYQXYLJTYXFSFSLLGNQCYHYCWFHCCCFXPYLYPLLZYXXXXXKQHHXSHJZCFZSCZJXCPZWHHHHHAPYLQALPQAFYHXD"
    "YLUKMZQGGGDDESRNNZLTZGCHYPPYSQJJHCLLJTOLNJPZLJLHYMHEYDYDSQYCDDHGZUNDZCLZYZLLZNTNYZGSLHSLPJJBDGWXPCDU"
    "TJCKLKCLWKLLCASSTKZZDNQNTTLYYZSSYSSZZRYLJQKCQDHHCRXRZYDGRGCWCGZQFFFPPJFZYNAKRGYWYQPQXXFKJTSZZXSWZDDF"
    "BBXTBGTZKZNPZZPZXZPJSZBMQHKCYXYLDKLJNYPKYGHGDZJXXEAHPNZKZTZCMXCXMMJXNKSZQNMNLWBWWXJKYHCPSTMCSQTZJYXT"
    "PCTPDTNNPGLLLZSJLSPBLPLQHDTNJNLYYRSZFFJFQWDPHZDWMRZCCLODAXNSSNYZRESTYJWJYJDBCFXNMWTTBYLWSTSZGYBLJPXG"
    "LBOCLHPCBJLTMXZLJYLZXCLTPNCLCKXTPZJSWCYXSFYSZDKNTLBYJCYJLLSTGQCBXRYZXBXKLYLHZLQZLNZCXWJZLJZJNCJHXMNZ"
    "ZGJZZXTZJXYCYYCXXJYYXJJXSSSJSTSSTTPPGQTCSXWZDCSYFPTFBFHFBBLZJCLZZDBXGCXLQPXKFZFLSYLTUWBMQJHSZBMDDBCY"
    "SCCLDXYCDDQLYJJWMQLLCSGLJJSYFPYYCCYLTJANTJJPWYCMMGQYYSXDXQMZHSZXPFTWWZQSWQRFKJLZJQQYFBRXJHHFWJJZYQAZ"
    "MYFRHCYYBYQWLPEXCCZSTYRLTTDMQLYKMBBGMYYJPRKZNPBSXYXBHYZDJDNGHPMFSGMWFZMFQMMBCMZZCJJLCNUXYQLMLRYGQZCY"
    "XZLWJGCJCGGMCJNFYZZJHYCPRRCMTZQZXHFQGTJXCCJEAQCRJYHPLQLSZDJRBCQHQDYRHYLYXJSYMHZYDWLDFRYHBPYDTSSCNWBX"
    "GLPZMLZZTQSSCPJMXXYCSJYTYCGHYCJWYRXXLFEMWJNMKLLSWTXHYYYNCMMCWJDQDJZGLLJWJRKHPZGGFLCCSCZMCBLTBHBQJXQD"
    "SPDJZZGKGLFQYWBZYZJLTSTDHQHCTCBCHFLQMPWDSHYYTQWCNZZJTLBYMBPDYYYXSQKXWYYFLXXNCWCXYPMAELYKKJMZZZBRXYYQ"
    "JFLJPFHHHYTZZXSGQQMHSPGDZQWBWPJHZJDYSCQWZKTXXSQLZYYMYSDZGRXCKKUJLWPYSYSCSYZLRMLQSYLJXBCXTLWDQZPCYCYK"
    "PPPNSXFYZJJRCEMHSZMSXLXGLRWGCSTLRSXBZGBZGZTCPLUJLSLYLYMTXMTZPALZXPXJTJWTCYYZLBLXBZLQMYLXPGHDSLSSDMXM"
    "BDZZSXWHAMLCZCPJMCNHJYSNSYGCHSKQMZZQDLLKABLWJXSFMOCDXJRRLYQZKJMYBYQLYHETFJZFRFKSRYXFJTWDSXXSYSQJYSLY"
    "XWJHSNLXYYXHBHAWHHJZXWMYLJCSSLKYDZTXBZSYFDXGXZJKHSXXYBSSXDPYNZWRPTQZCZENYGCXQFJYKJBZMLJCMQQXUOXSLYXX"
    "LYLLJDZBTYMHPFSTTQQWLHOKYBLZZALZXQLHZWRRQHLSTMYPYXJJXMQSJFNBXYXYJXXYQYLTHYLQYFMLKLJTMLLHSZWKZHLJMLHL"
    "JKLJSTLQXYLMBHHLNLZXQJHXCFXXLHYHJJGBYZZKBXSCQDJQDSUJZYYHZHHMGSXCSYMXFEBCQWWRBPYYJQTYZCYQYQQZYHMWFFHG"
    "ZFRJFCDPXNTQYZPDYKHJLFRZXPPXZDBBGZQSTLGDGYLCQMLCHHMFYWLZYXKJLYPQHSYWMQQGQZMLZJNSQXJQSYJYCBEHSXFSZPXZ"
    "WFLLBCYYJDYTDTHWZSFJMQQYJLMQXXLLDTTKHHYBFPWTYYSQQWNQWLGWDEBZWCMYGCULKJXTMXMYJSXHYBRWFYMWFRXYQMXYSZTZ"
    "ZTFYKMLDHQDXWYYNLCRYJBLPSXCXYWLSPRRJWXHQYPHTYDNXHHMMYWYTZCSQMTSSCCDALWZTCPQPYJLLQZYJSWXMZZMMYLMXCLMX"
    "CZMXMZSQTZPPQQBLPGXQZHFLJJHYTJSRXWZXSCCDLXTYJDCQJXSLQYCLZXLZZXMXQRJMHRHZJBHMFLJLMLCLQNLDXZLLLPYPSYJY"
    "SXCQQDCMQJZZXHNPNXZMEKMXXYKYQLXSXTXJYYHWDCWDZHQYYBGYBCYSCFGPSJNZDYZZJZXRZRQJJYMCANYRJTLDPPYZBSTJKXXZ"
    "YPFDWFGZZRPYMTNGXZQBYXNBUFNQKRJQZMJEGRZGYCLKXZDSKKNSXKCLJSPJYYZLQQJYBZSSQLLLKJXTBKTYLCCDDBLSPPFYLGYD"
    "TZJYQGGKQTTFZXBDKTYYHYBBFYTYYBCLPDYTGDHRYRNJSPTCSNYJQHKLLLZSLYDXXWBCJQSPXBPJZJCJDZFFXXBRMLAZHCSNDLBJ"
    "DSZBLPRZTSWSBXBCLLXXLZDJZSJPYLYXXYFTFFFBHJJXGBYXJPMMMPSSJZJMTLYZJXSWXTYLEDQPJMYGQZJGDJLQJWJQLLSJGJGY"
    "GMSCLJJXDTYGJQJQJCJZCJGDZZSXQGSJGGCXHQXSNQLZZBXHSGZXCXYLJXYXYYDFQQJHJFXDHCTXJYRXYSQTJXYEFYYSSYYJXNCY"
    "ZXFXMSYSZXYYSCHSHXZZZGZZZGFJDLTYLNPZGYJYZYYQZPBXQBDZTZCZYXXYHHSQXSHDHGQHJHGYWSZTMZMLHYXGEBTYLZKQWYTJ"
    "ZRCLEKYSTDBCYKQQSAYXCJXWWGSBHJYZYDHCSJKQCXSWXFLTYNYZPZCCZJQTZWJQDZZZQZLJJXLSBHPYXXPSXSHHEZTXFPTLQYZZ"
    "XHYTXNCFZYYHXGNXMYWXTZSJPTHHGYMXMXQZXTSBCZYJYXXTYYZYPCQLMMSZMJZZLLZXGXZAAJZYXJMZXWDXZSXZDZXLEYJJZQBH"
    "ZWZZZQTZPSXZTDSXJJJZNYAZPHXYYSRNQDTHZHYYKYJHDZXZLSWCLYBZYECWCYCRYLCXNHZYDZYDYJDFRJJHTRSQTXYXJRJHOJYN"
    "XELXSFSFJZGHPZSXZSZDZCQZBYYKLSGSJHCZSHDGQGXYZGXCHXZJWYQWGYHKSSEQZZNDZFKWYSSTCLZSTSYMCDHJXXYWEYXCZAYD"
    "MPXMDSXYBSQMJMZJMTZQLPJYQZCGQHXJHHLXXHLHDLDJQCLDWBSXFZZYYSCHTYTYYBHECXHYKGJPXHHYZJFXHWHBDZFYZBCAPNPG"
    "NYDMSXHMMMMAMYNBYJTMPXYYMCTHJBZYFCGTYHWPHFTWZZEZSBZEGPFMTSKFTYCMHFLLHGPZJXZJGZJYXZSBBQSCZZLZCCSTPGXM"
    "JSFTCCZJZDJXCYBZLFCJSYZFGSZLYBCWZZBYZDZYPSWYJZXZBDSYUXLZZBZFYGCZXBZHZFTPBGZGEJBSTGKDMFHYZZJHZLLZZGJQ"
    "ZLSFDJSSCBZGPDLFZFZSZYZYZSYGCXSNXXCHCZXTZZLJFZGQSQYXZJQDCCZTQCDXZJYQJQCHXZTDLGSCXZSYQJQTZWLQDQZTQCHQ"
    "QJZYEZZZPBWKDJFCJPZTYPQYQTTYNLMBDKTJZPQZQZZFPZSBNJLGYJDXJDZZKZGQKXDLPZJTCJDQBXDJQJSTCKNXBXZMSLYJCQMT"
    "JQWWCJQNJNLLLHJCWQTBZQYDZCZPZZDZYDDCYZZZCCJTTJFZDPRRTZTJDCQTQZDTJNPLZBCLLCTZSXKJZQZPZLBZRBTJDCXFCZDB"
    "CCJJLTQQPLDCGZDBBZJCQDCJWYNLLZYZCCDWLLXWZLXRXNTQQCZXKQLSGDFQTDDGLRLAJJTKUYMKQLLTZYTDYYCZGJWYXDXFRSKS"
    "TQTENQMRKQZHHQKDLDAZFKYPBGGPZREBZZYKZZSPEGJXGYKQZZZSLYSYYYZWFQZYLZZLZHWCHKYPQGNPGBLPLRRJYXCCSYYHSFZF"
    "YBZYYTGZXYLXCZWXXZJZBLFFLGSKHYJZEYJHLPLLLLCZGXDRZELRHGKLZZYHZLYQSZZJZQLJZFLNBHGWLCZCFJYSPYXZLZLXGCCP"
    "ZBLLCYBBBBUBBCBPCRNNZCZYRBFSRLDCGQYYQXYGMQZWTZYTYJXYFWTEHZZJYWLCCNTZYJJZDEDPZDZTSYQJHDYMBJNYJZLXTSST"
    "PHNDJXXBYXQTZQDDTJTDYYTGWSCSZQFLSHLGLBCZPHDLYZJYCKWTYTYLBNYTSDSYCCTYSZYYEBHEXHQDTWNYGYCLXTSZYSTQMYGZ"
    "AZCCSZZDSLZCLZRQXYYELJSBYMXSXZTEMBBLLYYLLYTDQYSHYMRQWKFKBFXNXSBYCHXBWJYHTQBPBSBWDZYLKGZSKYHXQZJXHXJX"
    "GNLJKZLYYCDXLFYFGHLJGJYBXQLYBXQPQGZTZPLNCYPXDJYQYDYMRBESJYYHKXXSTMXRCZZYWXYQYBMCLLYZHQYZWQXDBXBZWZMS"
    "LPDMYSKFMZKLZCYQYCZLQXFZZYDQZPZYGYJYZMZXDZFYFYTTQTZHGSPCZMLCCYTZXJCYTJMKSLPZHYSNZLLYTPZCTZZCKTXDHXXT"
    "QCYFKSMQCCYYAZHTJPCYLZLYJBJXTPNYLJYYNRXSYLMMNXJSMYBCSYSYLZYLXJJQYLDZLPQBFZZBLFNDXQKCZFYWHGQMRDSXYCYT"
    "XNQQJZYYPFZXDYZFPRXEJDGYQBXRCNFYYQPGHYJDYZXGRHTKYLNWDZNTSMPKLBTHBPYSZBZTJZSZZJTYYXZPHSSZZBZCZPTQFZMY"
    "FLYPYBBJQXZMXXDJMTSYSKKBJZXHJCKLPSMKYJZCXTMLJYXRZZQSLXXQPYZXMKYXXXJCLJPRMYYGADYSKQLSNDHYZKQXZYZTCGHZ"
    "TLMLWZYBWSYCTBHJHJFCWZTXWYTKZLXQSHLYJZJXTMPLPYCGLTBZZTLZJCYJGDTCLKLPLLQPJMZPAPXYZLKKTKDZCZZBNZDYDYQZ"
    "JYJGMCTXLTGXSZLMLHBGLKFWNWZHDXUHLFMKYSLGXDTWWFRJEJZTZHYDXYKSHWFZCQSHKTMQQHTZHYMJDJSKHXZJZBZZXYMPAGQM"
    "STPXLSKLZYNWRTSQLSZBPSPSGZWYHTLKSSSWHZZLYYTNXJGMJSZSUFWNLSOZTXGXLSAMMLBWLDSZYLAKQCQCTMYCFJBSLXCLZZCL"
    "XXKSBZQCLHJPSQPLSXXCKSLNHPSFQQYTXYJZLQLDXZQJZDYYDJNZPTUZDSKJFSLJHYLZSQZLBTXYDGTQFDBYAZXDZHZJNHHQBYKN"
    "XJJQCZMLLJZKSPLDYCLBBLXKLELXJLBQYCXJXGCNLCQPLZLZYJTZLJGYZDZPLTQCSXFDMNYCXGBTJDCZNBGBQYQJWGKFHTNPYQZQ"
    "GBKPBBYZMTJDYTBLSQMPSXTBNPDXKLEMYYCJYNZCTLDYKZZXDDXHQSHDGMZSJYCCTAYRZLPYLTLKXSLZCGGEXCLFXLKJRTLQJAQZ"
    "NCMBYDKKCXGLCZJZXJHPTDJJMZQYKQSECQZDSHHADMLZFMMZBGNTJNNLGBYJBRBTMLBYJDZXLCJLPLDLPCQDHLXZLYCBLCXZZJAD"
    "JLNZMMSSSMYBHBSQKBHRSXXJMXSDZNZPXLGBRHWGGFCXGMSKLLTSJYYCQLTSKYWYYHYWXBXQYWPYWYKQLSQPTNTKHQCWDQKTWPXX"
    "HCPTHTWUMSSYHBWCRWXHJMKMZNGWTMLKFGHKJYLSYYCXWHYECLQHKQHTTQKHFZLDXQWYZYYDESBPKYRZPJFYYZJCEQDZZDLATZBB"
    "FJLLCXDLMJSSXEGYGSJQXCWBXSSZPDYZCXDNYXPPZYDLYJCZPLTXLSXYZYRXCYYYDYLWWNZSAHJSYQYHGYWWAXTJZDAXYSRLTDPS"
    "SYYFNEJDXYZHLXLLLZQZSJNYQYQQXYJGHZGZCYJCHZLYCDSHWSHJZYJXCLLNXZJJYYXNFXMWFPYLCYLLABWDDHWDXJMCXZTZPMLQ"
    "ZHSFHZYNZTLLDYWLSLXHYMMYLMBWWKYXYADTXYLLDJPYBPWUXJMWMLLSAFDLLYFLBHHHBQQLTZJCQJLDJTFFKMMMBYTHYGDCQRDD"
    "WRQJXNBYSNWZDBYYTBJHPYBYTTJXAAHGQDQTMYSTQXKBTZPKJLZRBEQQSSMJJBDJOTGTBXPGBKTLHQXJJJCTHXQDWJLWRFWQGWSH"
    "CKRYSWGFTGYGBXSDWDWRFHWYTJJXXXJYZYSLPYYYPAYXHYDQKXSHXYXGSKQHYWFDDDPPLCJLQQEEWXKSYYKDYPLTJTHKJLTCYYHH"
    "JTTPLTZZCDLTHQKZXQYSTEEYWYYZYXXYYSTTJKLLPZMCYHQGXYHSRMBXPLLNQYDQHXSXXWGDQBSHYLLPJJJTHYJKYPPTHYYKTYEZ"
    "YENMDSHLCRPQFDGFXZPSFTLJXXJBSWYYSKSFLXLPPLBBBLBSFXFYZBSJSSYLPBBFFFFSSCJDSTZSXZRYYSYFFSYZYZBJTBCTSBSD"
    "HRTJJBYTCXYJEYLXCBNEBJDSYXYKGSJZBXBYTFZWGENYHHTHZHHXFWGCSTBGXKLSXYWMTMBYXJSTZSCDYQRCYTWXZFHMYMCXLZNS"
    "DJTTTXRYCFYJSBSDYERXJLJXBBDEYNJGHXGCKGSCYMBLXJMSZNSKGXFBNBPTHFJAAFXYXFPXMYPQDTZCXZZPXRSYWZDLYBBKTYQP"
    "QJPZYPZJZNJPZJLZZFYSBTTSLMPTZRTDXQSJEHBZYLZDHLJSQMLHTXTJECXSLZZSPKTLZKQQYFSYGYWPCPQFHQHYTQXZKRSGTTSQ"
    "CZLPTXCDYYZXSQZSLXLZMYCPCQBZYXHBSXLZDLTCDXTYLZJYYZPZYZLTXJSJXHLPMYTXCQRBLZSSFJZZTNJYTXMYJHLHPPLCYXQJ"
    "QQKZZSCPZKSWALQSBLCCZJSXGWWWYGYKTJBBZTDKHXHKGTGPBKQYSLPXPJCKBMLLXDZSTBKLGGQKQLSBKKTFXRMDKBFTPZFRTBBR"
    "FERQGXYJPZSSTLBZTPSZQZSJDHLJQLZBPMSMMSXLQQNHKNBLRDDNXXDHDDJCYYGYLXGZLXSYGMQQGKHBPMXYXLYTQWLWGCPBMQXC"
    "YZYDRJBHTDJYHQSHTMJSBYPLWHLZFFNYPMHXXHPLTBQPFBJWQDBYGPNZTPFZJGSDDTQSHZEAWZZYLLTYYBWJKXXGHLFKXDJTMSZS"
    "QYNZGGSWQSPHTLSSKMCLZXYSZQZXNCJDQGZDLFNYKLJCJLLZLMZZNHYDSSHTHZZLZZBBHQZWWYCRZHLYQQJBEYFXXXWHSRXWQHWP"
    "SLMSSKZTTYGYQQWRSLALHMJTQJSMXQBJJZJXZYZKXBYQXBJXSHZTSFJLXMXZXFGHKZSZGGYLCLSARJYHSLLLMZXELGLXYDJYTLFB"
    "HBPNLYZFBBHPTGJKWETZHKJJXZXXGLLJLSTGSHJJYQLQZFKCGNNDJSSZFDBCTWWSEQFHQJBSAQTGYPQLBXBMMYWXGSLZHGLZGQYF"
    "LZBYFZJFRYSFMBYZHQGFWZSYFYJJPHZBYYZFFWODGRLMFTWLBZGYCQXCDJYGZYYYYTYTYDWEGAZYHXJLZYYHLRMGRXXZCLHNELJJ"
    "TJTPWJYBJJBXJJTJTEEKHWSLJPLPSFYZPQQBDLQJJTYYQLYZKDKSQJYYQZLDQTGJQYZJSUCMRYQTHTEJMFCTYHYPKMHYZWJDQFHY"
    "YXWSHCTXRLJHQXHCCYYYJLTKTTYTMXGTCJTZAYYOCZLYLBSZYWJYTSJYHBYSHFJLYGJXXTMZYYLTXXYPZLXYJZYZYYPNHMYMDYYL"
    "BLHLSYYQQLLNJJYMSOYQBZGDLYXYLCQYXTSZEGXHZGLHWBLJHEYXTWQMAKBPQCGYSHHEGQCMWYYWLJYJHYYZLLJJYLHZYHMGSLJL"
    "JXCJJYCLYCJPCPZJZJMMYLCQLNQLJQJSXYJMLSZLJQLYCMMHCFMMFPQQMFYLQMCFFQMMMMHMZNFHHJGTTHHKHSLNCHHYQDXTMMQD"
    "CYZYXYQMYQYLTDCYYYZAZZCYMZYDLZFFFMMYCQZWZZMABTBYZTDMNZZGGDFTYPCGQYTTSSFFWFDTZQSSYSTWXJHXYTSXXYLBYQHW"
    "WKXHZXWZNNZZJZJJQJCCCHYYXBZXZCYZTLLCQXYNJYCYYCYNZZQYYYEWYCZDCJYCCHYJLBTZYYCQWMPWPYMLGKDLDLGKQQBGYCHJ"
    "XY";

    if(str.isEmpty() || letterTable.isEmpty())
        return "#";
    const QChar firstChar = str.at(0);
    const unsigned short unicode = firstChar.unicode();

    if((unicode >= 'a' && unicode <= 'z') || (unicode >= 'A' && unicode <= 'Z'))
        return QString(firstChar.toUpper());
    if(unicode >= 0x4E00 && unicode <= 0x9FA5)
    {
        //计算偏移量精准定位
        int index = unicode - 0x4E00;
        if(index < letterTable.size())
            return QString(letterTable.at(index));
    }
    return "#";

}

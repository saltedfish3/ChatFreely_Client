#include "groupinfowidget.h"

GroupInfoWidget::GroupInfoWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);

    this->area = new QScrollArea(this);
    this->area->setObjectName("area");
    this->area->setGeometry(0, 1, this->width() - 1,this->height() - 10);

    init();
    initStyle();
    this->setObjectName("this");
}

void GroupInfoWidget::changeInfo(const QString &conversationID, const QString &groupID, const QString &groupName, const QString &ownerUID, const QHash<QString, GroupManager::GroupMemberInfo>& memberInfos, const QPixmap &pix)
{
    this->conversationID = conversationID;
    this->currentGroupName = groupName;
    this->currentMemberCount = memberInfos.size();
    this->label_groupID->setText("ID:" + groupID);

    if(UserInfo::getUserInfo().getUID() == ownerUID)
    {
        this->btn_doneOrEdit->show();
        this->label_camera_icon->show();
        this->view_avatar->setAnimation(true);
    }
    else
    {
        this->btn_doneOrEdit->hide();
        this->label_camera_icon->hide();
        this->view_avatar->setAnimation(false);
    }

    this->model->removeRows(0, this->model->rowCount());

    QPixmap avatar = pix;
    setRadius(avatar, this->view_avatar, 64);

    QFontMetrics fm(this->edit_groupName->font());

    QString memberCount = "(" + QString::number(this->currentMemberCount) + ")";
    int width_memberCount = fm.horizontalAdvance(memberCount);

    int width_groupName = fm.horizontalAdvance(this->currentGroupName);
    if(width_groupName >= this->widget_holy->width() - width_memberCount - 120)
        width_groupName = this->widget_holy->width() - width_memberCount - 120;

    QString showGroupName = fm.elidedText(currentGroupName, Qt::ElideRight, width_groupName);
    showGroupName += memberCount;
    this->edit_groupName->setText(showGroupName);

    updateEditPos();

    QPixmap defaultAvatar = ImageCacheManager::getManager().fastLoadImage(":/default/images/defaultAvatar.png", 0, 0, QSize(38, 38));
    if(defaultAvatar.isNull())
        defaultAvatar = QPixmap(":/default/images/defaultAvatar.png");
    defaultAvatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());

    for(const auto& it : memberInfos)
    {
        if(it.uid == ownerUID)
        {
            this->label_ownerUsername->setText(it.username);
            this->label_ownerSID->setText("ID:" + it.sid);
            setRadiusOnLabel(defaultAvatar, this->label_ownerAvatar, 40);

            if(ownerUID == UserInfo::getUserInfo().getUID())
                this->btn_addFriend->hide();
            else
            {
                FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(FriendManage::getFriendManage().getFriendConversationIDByUID(it.uid));
                if(info.uid.isEmpty() || info.isDeleted)
                    this->btn_addFriend->show();
                else
                    this->btn_addFriend->hide();
            }

            continue;
        }
        addGroupMemberItem(it.username, it.sid, it.uid, defaultAvatar);
    }

    for(const auto& it : memberInfos)
    {
        if(it.avatarUrl.isEmpty())
            continue;

        QString currentUID = it.uid;

        QString reqConversationID = this->conversationID;

        ImageCacheManager::getManager().loadImage(it.avatarUrl, [this, currentUID, reqConversationID, ownerUID](const QPixmap& pix){
            if(pix.isNull())
                return;

            if(this->conversationID != reqConversationID)
                return;

            QPixmap avatar = pix;
            avatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());

            if(ownerUID == currentUID)
            {
                setRadiusOnLabel(avatar, this->label_ownerAvatar, 40);
                return;
            }
            changeMemberAvatar(currentUID, avatar);
        }, false, 0, 0, QSize(38, 38));
    }
}

void GroupInfoWidget::changeSelectedGroupName(const QString& groupName)
{
    this->currentGroupName = groupName;
    QFontMetrics fm(this->edit_groupName->font());

    QString memberCount = "(" + QString::number(this->currentMemberCount) + ")";
    int width_memberCount = fm.horizontalAdvance(memberCount);

    int width_groupName = fm.horizontalAdvance(this->currentGroupName);
    if(width_groupName >= this->widget_holy->width() - width_memberCount - 120)
        width_groupName = this->widget_holy->width() - width_memberCount - 120;

    QString showGroupName = fm.elidedText(currentGroupName, Qt::ElideRight, width_groupName);
    showGroupName += memberCount;
    this->edit_groupName->setText(showGroupName);

    updateEditPos();
}

void GroupInfoWidget::changeSelectedAvatar(const QPixmap& avatar)
{
    QPixmap avatar_ = avatar;
    setRadius(avatar_, this->view_avatar, 64);
}

void GroupInfoWidget::changeMemberAvatar(const QString &uid, const QPixmap &avatar)
{
    for(int i = 0; i < this->model->rowCount(); i++)
    {
        QModelIndex index = this->model->index(i, 0);
        if(index.data(GroupMemberDelegate::UIDRole).toString() == uid)
        {
            QStandardItem* item = this->model->itemFromIndex(index);
            item->setData(avatar, GroupMemberDelegate::AvatarRole);
        }
    }
}

void GroupInfoWidget::init()
{
    this->widget_holy = new QWidget();
    this->widget_holy->setObjectName("widget_holy");
    this->widget_holy->resize(this->area->width(), this->area->height());

    this->area->setWidget(this->widget_holy);

    this->view_avatar = new AvatarView(0.95, this->widget_holy);
    this->view_avatar->setObjectName("view_avatar");
    this->view_avatar->resize(136, 136);
    this->view_avatar->move((this->widget_holy->width() - this->view_avatar->width())/2,64);

    QPixmap defaultAvatar = ImageCacheManager::getManager().fastLoadImage(":/default/images/defaultAvatar.png", 0, 0, QSize(128, 128));
    if(defaultAvatar.isNull())
    {
        ImageCacheManager::getManager().loadImage(":/default/images/defaultAvatar.png", [this](const QPixmap& pix){
            QPixmap p_ = pix;
            setRadius(p_, this->view_avatar, 64);
        }, false, 0, 0, QSize(128, 128));
    }
    else
        setRadius(defaultAvatar, this->view_avatar, 64);

    connect(this->view_avatar, &AvatarView::itemClicked, this, [this](){
        if(UserInfo::getUserInfo().getUID() != GroupManager::getGroupManage().getGroupInfo(this->conversationID).ownerUID)
            return;

        QString filePath = QFileDialog::getOpenFileName(this,"选择图片","","图片文件 (*.png *.jpg *.jpeg)");
        if(filePath.isEmpty())
            return;
        HttpShortConnection::getHttpClient().uploadGroupAvatar(this->conversationID, filePath);
    });

    this->label_camera_icon = new QLabel(this->view_avatar);
    this->label_camera_icon->setObjectName("label_camera_icon");
    this->label_camera_icon->resize(24,24);
    this->label_camera_icon->setAlignment(Qt::AlignCenter);
    this->label_camera_icon->setPixmap(QIcon(":/default/images/camera.png").pixmap(16));
    this->label_camera_icon->move(this->view_avatar->width() - this->label_camera_icon->width(),
                                  this->view_avatar->height()-this->label_camera_icon->height());

    connect(this->view_avatar, &AvatarView::valueChanged, this, [this](const QRect& rect){
        this->label_camera_icon->move(rect.right() - this->label_camera_icon->width(),
                                      rect.bottom() - this->label_camera_icon->height());
    });

    this->edit_groupName = new QLineEdit(this->widget_holy);
    this->edit_groupName->setText("群聊");
    this->edit_groupName->resize(this->width() - 120, 32);
    this->edit_groupName->setAlignment(Qt::AlignCenter);
    this->edit_groupName->setObjectName("edit_groupName");
    this->edit_groupName->move((this->widget_holy->width() - this->edit_groupName->width()) / 2,
                               this->view_avatar->pos().y() + this->view_avatar->height() + 10);
    this->edit_groupName->setProperty("editing", false);
    this->edit_groupName->setDisabled(true);

    connect(this->edit_groupName, &QLineEdit::textChanged, this, [this](const QString& text){
        updateEditPos();
    });

    this->btn_doneOrEdit = new QPushButton(this->widget_holy);
    this->btn_doneOrEdit->setObjectName("btn_doneOrEdit");
    this->btn_doneOrEdit->resize(this->edit_groupName->height(), this->edit_groupName->height());
    this->btn_doneOrEdit->move(this->edit_groupName->pos().x() + this->edit_groupName->width() + 5, this->edit_groupName->pos().y());
    this->btn_doneOrEdit->setIcon(QIcon(":/default/images/edit.png"));
    this->btn_doneOrEdit->setIconSize(QSize(24, 24));
    this->btn_doneOrEdit->setCheckable(true);

    connect(this->btn_doneOrEdit, &QPushButton::clicked, this, [this](bool checked){
        if(checked)
        {
            this->btn_doneOrEdit->setIcon(QIcon(":/default/images/done.png"));
            this->edit_groupName->setProperty("editing", true);
            this->edit_groupName->style()->unpolish(this->edit_groupName);
            this->edit_groupName->style()->polish(this->edit_groupName);
            this->edit_groupName->setDisabled(false);
            this->edit_groupName->setText(this->currentGroupName);
            this->edit_groupName->setFocus();
            this->edit_groupName->setCursorPosition(this->edit_groupName->text().length());
        }
        else
        {
            this->btn_doneOrEdit->setIcon(QIcon(":/default/images/edit.png"));
            this->edit_groupName->setProperty("editing", false);
            this->edit_groupName->style()->unpolish(this->edit_groupName);
            this->edit_groupName->style()->polish(this->edit_groupName);
            this->edit_groupName->setDisabled(true);

            QString newGroupName = this->edit_groupName->text().trimmed();
            if(newGroupName == this->currentGroupName || newGroupName.size() > 64)
            {
                QFontMetrics fm(this->edit_groupName->font());

                QString memberCount = "(" + QString::number(this->currentMemberCount) + ")";
                int width_memberCount = fm.horizontalAdvance(memberCount);

                int width_groupName = fm.horizontalAdvance(this->currentGroupName);
                if(width_groupName >= this->widget_holy->width() - width_memberCount - 120)
                    width_groupName = this->widget_holy->width() - width_memberCount - 120;

                QString showGroupName = fm.elidedText(currentGroupName, Qt::ElideRight, width_groupName);
                showGroupName += memberCount;
                this->edit_groupName->setText(showGroupName);
                return;
            }

            TcpLongConnection::getTcpClient().sendUpdateGroupName(this->conversationID, newGroupName);

            QFontMetrics fm(this->edit_groupName->font());

            QString memberCount = "(" + QString::number(this->currentMemberCount) + ")";
            int width_memberCount = fm.horizontalAdvance(memberCount);

            int width_groupName = fm.horizontalAdvance(newGroupName);
            if(width_groupName >= this->widget_holy->width() - width_memberCount - 120)
                width_groupName = this->widget_holy->width() - width_memberCount - 120;

            QString showGroupName = fm.elidedText(newGroupName, Qt::ElideRight, width_groupName);
            showGroupName += memberCount;
            this->edit_groupName->setText(showGroupName);
            return;
        }
    });

    this->label_groupID = new QLabel("ID:124216457457", this->widget_holy);
    this->label_groupID->resize(this->width(),16);
    this->label_groupID->setAlignment(Qt::AlignCenter);
    this->label_groupID->setObjectName("label_groupID");
    this->label_groupID->move(0,this->edit_groupName->pos().y() + this->edit_groupName->height() + 4);

    int pos_middle = this->width() / 2;

    this->btn_sendMsg = new QPushButton("发送消息", this->widget_holy);
    this->btn_sendMsg->setObjectName("btn_sendMsg");
    this->btn_sendMsg->resize(150,48);
    this->btn_sendMsg->move(pos_middle - this->btn_sendMsg->width() - 10,
                            this->label_groupID->pos().y() + this->label_groupID->height() + 32);

    connect(this->btn_sendMsg, &QPushButton::clicked, this, [this](){
        emit openConversation(this->conversationID);
    });

    this->label_sendMsg_icon = new QLabel(this->btn_sendMsg);
    this->label_sendMsg_icon->setObjectName("label_sendMsg_icon");
    this->label_sendMsg_icon->resize(20,20);
    this->label_sendMsg_icon->setPixmap(QIcon(":/default/images/sendMsg.png").pixmap(20));
    this->label_sendMsg_icon->move(30,(this->btn_sendMsg->height() - this->label_sendMsg_icon->height())/2);

    this->btn_exitGroup = new QPushButton("退出群聊", this->widget_holy);
    this->btn_exitGroup->setObjectName("btn_exitGroup");
    this->btn_exitGroup->resize(150,48);
    this->btn_exitGroup->move(pos_middle + 10,
                                 this->label_groupID->pos().y() + this->label_groupID->height() + 32);

    this->label_exitGroup = new QLabel(this->btn_exitGroup);
    this->label_exitGroup->setObjectName("label_exitGroup");
    this->label_exitGroup->resize(20,20);
    this->label_exitGroup->setPixmap(QIcon(":/default/images/deleteUser.png").pixmap(20));
    this->label_exitGroup->move(30,(this->btn_exitGroup->height() - this->label_exitGroup->height())/2);

    this->widget_memberRegion = new QWidget(this->widget_holy);
    this->widget_memberRegion->setObjectName("widget_memberRegion");
    this->widget_memberRegion->resize(420, 400);
    this->widget_memberRegion->setAttribute(Qt::WA_StyledBackground);
    this->widget_memberRegion->move((this->widget_holy->width() - this->widget_memberRegion->width())/2,
                                   this->btn_exitGroup->pos().y() + this->btn_exitGroup->height() + 48);

    this->label_ownerName = new QLabel("群主", this->widget_memberRegion);
    this->label_ownerName->setObjectName("label_ownerName");
    this->label_ownerName->resize(this->widget_memberRegion->width() - 60, 30);
    this->label_ownerName->move((this->widget_memberRegion->width() - this->label_ownerName->width()) / 2, 20);

    this->label_ownerAvatar = new QLabel(this->widget_memberRegion);
    this->label_ownerAvatar->resize(40, 40);
    QPixmap defAvatar(":/default/images/defaultAvatar.png");
    setRadiusOnLabel(defAvatar, this->label_ownerAvatar, this->label_ownerAvatar->width() / 2);
    this->label_ownerAvatar->move(this->label_ownerName->x() + 10, this->label_ownerName->y() + this->label_ownerName->height() + 10);

    this->label_ownerUsername = new QLabel("群主名字", this->widget_memberRegion);
    this->label_ownerUsername->setObjectName("label_ownerUsername");
    this->label_ownerUsername->resize(this->widget_memberRegion->width() - this->label_ownerAvatar->pos().x() - this->label_ownerAvatar->width() - this->label_ownerName->pos().x(), 20);
    this->label_ownerUsername->move(this->label_ownerAvatar->pos().x() + this->label_ownerAvatar->width() + 6, this->label_ownerAvatar->pos().y() + 2);

    this->btn_addFriend = new QPushButton("添加", this->widget_memberRegion);
    this->btn_addFriend->setObjectName("btn_addFriend");
    this->btn_addFriend->resize(44, 28);
    this->btn_addFriend->move(this->label_ownerName->pos().x() + this->label_ownerName->width() - 10 - this->btn_addFriend->width(),
                              this->label_ownerAvatar->pos().y() + (this->label_ownerAvatar->height() - this->btn_addFriend->height()) / 2);

    this->label_ownerSID = new QLabel("ID:1234567892", this->widget_memberRegion);
    this->label_ownerSID->setObjectName("label_ownerSID");
    this->label_ownerSID->resize(this->label_ownerUsername->width(), 15);
    this->label_ownerSID->move(this->label_ownerUsername->pos().x(), this->label_ownerUsername->pos().y() + this->label_ownerUsername->height());

    this->label_memberName = new QLabel("成员", this->widget_memberRegion);
    this->label_memberName->setObjectName("label_memberName");
    this->label_memberName->resize(this->label_ownerName->width(), this->label_ownerName->height());
    this->label_memberName->move(this->label_ownerName->pos().x(), this->label_ownerAvatar->pos().y() + this->label_ownerAvatar->height() + 10);

    this->view = new QListView(this->widget_memberRegion);
    this->view->setObjectName("view");
    this->view->resize(this->widget_memberRegion->width() - this->label_memberName->pos().x() * 2,
                       this->widget_memberRegion->height() - this->label_memberName->pos().y() - this->label_memberName->height() - 6);
    this->view->move(this->label_memberName->pos().x(), this->label_memberName->pos().y() + this->label_memberName->height() + 3);

    this->model = new QStandardItemModel(this);
    this->view->setModel(this->model);

    this->delegate = new GroupMemberDelegate(this);
    this->view->setItemDelegate(this->delegate);

    this->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->view->setSelectionMode(QAbstractItemView::SingleSelection);
    this->view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->view->verticalScrollBar()->setSingleStep(6);
    this->view->setResizeMode(QListView::Fixed);

    this->view->setMouseTracking(true);

    this->widget_holy->setMinimumHeight(this->widget_memberRegion->pos().y() + this->widget_memberRegion->height() + 20);
    this->area->setWidgetResizable(true);

    connect(&GroupManager::getGroupManage(), &GroupManager::groupInfoUpdate, this, [this](const QString& conversationID){
        if(this->conversationID != conversationID)
            return;

        GroupManager::GroupInfo info = GroupManager::getGroupManage().getGroupInfo(conversationID);
        QPixmap pix = GroupManager::getGroupManage().getGroupAvatar(conversationID, QSize(128, 128));
        changeInfo(info.conversationID, info.groupID, info.groupName, info.ownerUID, info.memberInfos, pix);
    });

    connect(this->delegate, &GroupMemberDelegate::addFriendClicked, this, [this](const QString& SID, const QModelIndex& index){
        TcpLongConnection::getTcpClient().sendAddNewFriendRequest(SID);
    });

    connect(this->btn_addFriend, &QPushButton::clicked, this, [this](){
        QString sid = this->label_ownerSID->text().mid(3);
        if(sid.isEmpty())
            return;
        TcpLongConnection::getTcpClient().sendAddNewFriendRequest(sid);
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::allFriendList, this, [this](){
        updateIsFriend();
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::updateGroupNameFailed, this, [this](const QString& conversationID){
        if(this->conversationID != conversationID)
            return;

        QFontMetrics fm(this->edit_groupName->font());

        QString memberCount = "(" + QString::number(this->currentMemberCount) + ")";
        int width_memberCount = fm.horizontalAdvance(memberCount);

        int width_groupName = fm.horizontalAdvance(this->currentGroupName);
        if(width_groupName >= this->widget_holy->width() - width_memberCount - 120)
            width_groupName = this->widget_holy->width() - width_memberCount - 120;

        QString showGroupName = fm.elidedText(currentGroupName, Qt::ElideRight, width_groupName);
        showGroupName += memberCount;
        this->edit_groupName->setText(showGroupName);
    });
}

void GroupInfoWidget::initStyle()
{
    this->setStyleSheet(QString(R"(
                    #area,
                    #widget_holy
                    {
                        background:transparent;
                        border:none;
                    }
                    #view_avatar
                    {
                        background-color: transparent;
                    }
                    #label_camera_icon
                    {
                        background-color: rgba(99, 102, 241, 255);
                        border: 2px solid rgba(255, 255, 255, 255);
                        border-radius: 12px;
                    }
                    #edit_groupName
                    {
                        font-size: 24px;
                        font-weight:bold;
                        color:rgba(17, 24, 39, 255);
                        border: none;
                        border-radius: 0;
                        background: transparent;
                    }
                    #edit_groupName[editing="true"]
                    {
                        border-bottom: 1px solid black;
                        padding-bottom: 2px;
                    }
                    #edit_groupName[editing="false"]
                    {
                        border: none;
                        padding: 0;
                    }
                    #btn_doneOrEdit
                    {
                        background: transparent;
                        border: none;
                    }
                    #label_groupID
                    {
                        font-size:14px;
                        color:rgba(107, 114, 128, 255);
                    }
                    #btn_sendMsg
                    {
                        background-color: rgba(99, 102, 241, 255);
                        color: rgba(255, 255, 255, 255);
                        border-radius: 8px;
                        font-size: 14px;
                        font-weight: bold;
                        padding-left:30px;
                    }
                    #btn_sendMsg:hover
                    {
                        background-color: rgba(79, 70, 229, 255);
                    }
                    #btn_sendMsg:pressed
                    {
                        background-color: rgba(67, 56, 202, 255);
                    }
                    #btn_exitGroup
                    {
                        background-color: rgba(254, 226, 226, 255);
                        color: rgba(185, 28, 28, 255);
                        border-radius: 8px;
                        font-size: 14px;
                        padding-left:30px;
                    }
                    #btn_exitGroup:hover
                    {
                        background-color: rgba(255, 201, 206, 255);
                    }
                    #btn_exitGroup:pressed
                    {
                        background-color: rgba(255, 179, 186, 255);
                        color: rgba(190, 18, 60, 255);
                    }
                    #widget_memberRegion
                    {
                        background-color: rgba(255, 255, 255, 255);
                        border: 1px solid rgba(0, 0, 0, 25);
                        border-radius: 12px;
                        margin: 1px;
                    }
                    QScrollBar:vertical
                    {
                        width: 6px;
                        background: transparent;
                        margin: 0px;
                    }
                    QScrollBar::handle:vertical
                    {
                        background: rgba(0, 0, 0, 0.2);
                        border-radius: 2px;
                        min-height: 30px;
                        margin-right: 2px;
                    }
                    QScrollBar::handle:vertical:hover
                    {
                        background: rgba(0, 0, 0, 0.35);
                    }
                    QScrollBar::add-line:vertical,
                    QScrollBar::sub-line:vertical
                    {
                        height: 0px;
                    }
                    QScrollBar::add-page:vertical,
                    QScrollBar::sub-page:vertical
                    {
                        background: none;
                    }
                    #label_ownerName,
                    #label_memberName
                    {
                        font-size: 12px;
                        color: rgba(107, 114, 128, 255);
                        border: none;
                        border-bottom: 1px solid rgba(0, 0, 0, 25);
                    }
                    #label_ownerUsername
                    {
                        font-size: 13px;
                        font-weight:bold;
                        color: rgba(17, 24, 39, 255);
                    }
                    #label_ownerSID
                    {
                        font-size: 10px;
                        color: rgba(107, 114, 128, 255);
                    }
                    #view
                    {
                        background: transparent;
                        border-radius: 0;
                        border: none;
                    }
                    #btn_addFriend
                    {
                        margin: 1px;
                        background-color: transparent;
                        border: 1px solid rgb(99, 102, 241);
                        border-radius: 4px;
                        color: rgb(99, 102, 241);
                        font-size: 9pt;
                        font-weight: normal;
                    }
                    #btn_addFriend:hover
                    {
                        background-color: rgba(99, 102, 241, 20);
                    }
                    #btn_addFriend:pressed
                    {
                        background-color: rgba(99, 102, 241, 50);
                    }
                        )"));
}

void GroupInfoWidget::setRadius(QPixmap& pix, AvatarView *view, int radius)
{
    if(pix.isNull())
        return;

    qreal dpr = view->devicePixelRatioF();
    QPixmap rounded(view->size() * dpr);
    rounded.fill(Qt::transparent);
    rounded.setDevicePixelRatio(dpr);

    QPainter painter(&rounded);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addRoundedRect(view->rect(), radius, radius);
    painter.setClipPath(path);
    painter.drawPixmap(view->rect(), pix);
    painter.end();

    view->setAvatar(rounded);
}

void GroupInfoWidget::setRadiusOnLabel(QPixmap &pix, QLabel *label, int radius)
{
    if(pix.isNull())
        return;

    qreal dpr = label->devicePixelRatioF();
    QPixmap rounded(label->size() * dpr);
    rounded.fill(Qt::transparent);
    rounded.setDevicePixelRatio(dpr);

    QPainter painter(&rounded);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addRoundedRect(label->rect(), radius, radius);
    painter.setClipPath(path);
    painter.drawPixmap(label->rect(), pix);
    painter.end();

    label->setPixmap(rounded);
}

void GroupInfoWidget::addGroupMemberItem(const QString &username, const QString &sid, const QString &uid, const QPixmap &avatar)
{
    QStandardItem* item = new QStandardItem;
    item->setData(username, GroupMemberDelegate::UsernameRole);
    item->setData(sid, GroupMemberDelegate::SIDRole);
    item->setData(uid, GroupMemberDelegate::UIDRole);
    item->setData(avatar, GroupMemberDelegate::AvatarRole);

    bool canAdd = true;
    if(uid != UserInfo::getUserInfo().getUID())
    {
        FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(FriendManage::getFriendManage().getFriendConversationIDByUID(uid));
        canAdd = !info.uid.isEmpty() && !info.isDeleted;
    }

    item->setData(canAdd, GroupMemberDelegate::IsFriendRole);

    this->model->appendRow(item);
}

void GroupInfoWidget::updateIsFriend()
{
    for(int i = 0; i < this->model->rowCount(); i++)
    {
        QModelIndex index = this->model->index(i, 0);
        QString conversationID = FriendManage::getFriendManage().getFriendConversationIDByUID(index.data(GroupMemberDelegate::UIDRole).toString());
        FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(conversationID);

        bool isFriend = !info.uid.isEmpty() && !info.isDeleted;
        QStandardItem* item = this->model->itemFromIndex(index);
        item->setData(isFriend, GroupMemberDelegate::IsFriendRole);
    }

    QString sid = this->label_ownerSID->text().mid(3);
    if(sid.isEmpty() || sid == UserInfo::getUserInfo().getSID())
        return;

    QString uid = FriendManage::getFriendManage().getFriendUIDBySID(sid);
    bool isFriend = false;

    if(!uid.isEmpty())
    {
        FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(FriendManage::getFriendManage().getFriendConversationIDByUID(uid));
        isFriend = !info.uid.isEmpty() && !info.isDeleted;
    }

    if(!isFriend)
        this->btn_addFriend->show();
    else
        this->btn_addFriend->hide();
}

void GroupInfoWidget::updateEditPos()
{
    QFontMetrics fm(this->edit_groupName->fontMetrics());
    int textWidth = fm.horizontalAdvance(this->edit_groupName->text());

    if(textWidth > this->widget_holy->width() - 120)
        textWidth = this->widget_holy->width() - 120;

    textWidth += 10;

    this->edit_groupName->resize(textWidth, this->edit_groupName->height());
    this->edit_groupName->move((this->widget_holy->width() - textWidth) / 2,
                               this->view_avatar->pos().y() + this->view_avatar->height() + 10);
    this->btn_doneOrEdit->move(this->edit_groupName->pos().x() + this->edit_groupName->width() + 5, this->edit_groupName->pos().y());
}

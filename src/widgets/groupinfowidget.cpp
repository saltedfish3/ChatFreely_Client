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

    this->model->removeRows(0, this->model->rowCount());

    QPixmap avatar = pix;
    setRadius(avatar, this->label_avatar, 64);
    QFontMetrics fm(this->label_groupName->font());

    QString memberCount = "(" + QString::number(memberInfos.size()) + ")";
    int width_memberCount = fm.horizontalAdvance(memberCount);

    QString showGroupName = fm.elidedText(groupName, Qt::ElideRight, this->label_groupName->width() - 20 - width_memberCount);
    showGroupName += memberCount;
    this->label_groupName->setText(showGroupName);
    this->label_groupID->setText("ID:" + groupID);

    QPixmap defaultAvatar = ImageCacheManager::getManager().fastLoadImage(":/default/images/defaultAvatar.png", 0, 0, QSize(48, 48));
    if(defaultAvatar.isNull())
        defaultAvatar = QPixmap(":/default/images/defaultAvatar.png");
    defaultAvatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());

    for(const auto& it : memberInfos)
    {
        if(it.uid == ownerUID)
        {
            this->label_ownerUsername->setText(it.username);
            this->label_ownerSID->setText("ID:" + it.sid);
            setRadius(defaultAvatar, this->label_ownerAvatar, 40);
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
                setRadius(avatar, this->label_ownerAvatar, 40);
                return;
            }
            changeMemberAvatar(currentUID, avatar);
        });
    }
}

void GroupInfoWidget::changeSelectedGroupName(const QString& groupName)
{
    QFontMetrics fm(font());
    QString showUsername = fm.elidedText(groupName, Qt::ElideRight, this->label_groupName->width() - 20);
    this->label_groupName->setText(showUsername);
}

void GroupInfoWidget::changeSelectedAvatar(const QPixmap& avatar)
{
    QPixmap avatar_ = avatar;
    setRadius(avatar_, this->label_avatar, 64);
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

    this->label_avatar = new QLabel(this->widget_holy);
    this->label_avatar->resize(136,136);
    this->label_avatar->setObjectName("label_avatar");
    this->label_avatar->move((this->widget_holy->width() - this->label_avatar->width())/2,64);
    QPixmap defaultAvatar = ImageCacheManager::getManager().fastLoadImage(":/default/images/defaultAvatar.png", 0, 0, QSize(128, 128));
    if(defaultAvatar.isNull())
    {
        ImageCacheManager::getManager().loadImage(":/default/images/defaultAvatar.png", [this](const QPixmap& pix){
            QPixmap p_ = pix;
            setRadius(p_, this->label_avatar, 64);
        }, false, 0, 0, QSize(128, 128));
    }
    else
        setRadius(defaultAvatar, this->label_avatar, 64);

    this->label_groupName = new QLabel("群聊", this->widget_holy);
    this->label_groupName->resize(this->width(),32);
    this->label_groupName->setAlignment(Qt::AlignCenter);
    this->label_groupName->setObjectName("label_groupName");
    this->label_groupName->move(0,this->label_avatar->pos().y() + this->label_avatar->height() + 10);

    this->label_groupID = new QLabel("ID:124216457457", this->widget_holy);
    this->label_groupID->resize(this->width(),16);
    this->label_groupID->setAlignment(Qt::AlignCenter);
    this->label_groupID->setObjectName("label_groupID");
    this->label_groupID->move(0,this->label_groupName->pos().y() + this->label_groupName->height() + 4);

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
    setRadius(defAvatar, this->label_ownerAvatar, this->label_ownerAvatar->width() / 2);
    this->label_ownerAvatar->move(this->label_ownerName->x() + 10, this->label_ownerName->y() + this->label_ownerName->height() + 10);

    this->label_ownerUsername = new QLabel("群主名字", this->widget_memberRegion);
    this->label_ownerUsername->setObjectName("label_ownerUsername");
    this->label_ownerUsername->resize(this->widget_memberRegion->width() - this->label_ownerAvatar->pos().x() - this->label_ownerAvatar->width() - this->label_ownerName->pos().x(), 20);
    this->label_ownerUsername->move(this->label_ownerAvatar->pos().x() + this->label_ownerAvatar->width() + 6, this->label_ownerAvatar->pos().y() + 2);

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

    this->widget_holy->setMinimumHeight(this->widget_memberRegion->pos().y() + this->widget_memberRegion->height() + 20);
    this->area->setWidgetResizable(true);

    connect(&GroupManager::getGroupManage(), &GroupManager::groupInfoUpdate, this, [this](const QString& conversationID){
        if(this->conversationID != conversationID)
            return;

        GroupManager::GroupInfo info = GroupManager::getGroupManage().getGroupInfo(conversationID);
        QPixmap pix = GroupManager::getGroupManage().getGroupAvatar(conversationID, QSize(128, 128));
        changeInfo(info.conversationID, info.groupID, info.groupName, info.ownerUID, info.memberInfos, pix);
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
                    #label_groupName
                    {
                        font-size: 24px;
                        font-weight:bold;
                        color:rgba(17, 24, 39, 255);
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
                        )"));
}

void GroupInfoWidget::setRadius(QPixmap& pix, QLabel *label, int radius)
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

    this->model->appendRow(item);
}

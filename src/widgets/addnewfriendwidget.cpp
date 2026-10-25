#include "addnewfriendwidget.h"

QWidget *AddNewFriendWidget::findRootWidget(QWidget *caller)
{
    if(caller == nullptr)
        return nullptr;
    QWidget* top = caller;
    while(top->parentWidget() != nullptr)
        top = top->parentWidget();

    return top;
}

AddNewFriendWidget::AddNewFriendWidget(QWidget *caller)
    : QWidget{findRootWidget(caller)}
{
    //调试时防止传错组件
    Q_ASSERT(this->parentWidget() != nullptr);
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setObjectName("widget_addNewFriend");
    this->resize(this->parentWidget()->size());
    this->move(0,0);
    initWidget();
    initStyle();
}

void AddNewFriendWidget::initWidget()
{
    this->widget_card = new QWidget(this);
    this->widget_card->setObjectName("widget_card");
    this->widget_card->resize(360, 340);//290
    this->widget_card->move((this->width() - this->widget_card->width()) / 2,
                            (this->height() - this->widget_card->height()) / 2);

    this->label_bigTitle = new QLabel("添加好友", this->widget_card);
    this->label_bigTitle->setObjectName("label_bigTitle");
    this->label_bigTitle->move(24, 22);
    this->label_bigTitle->resize(200, 24);

    this->label_smallTitle = new QLabel("输入对方信息即可添加好友", this->widget_card);
    this->label_smallTitle->setObjectName("label_smallTitle");
    this->label_smallTitle->move(24, this->label_bigTitle->pos().y() + this->label_bigTitle->height() + 4);//50
    this->label_smallTitle->resize(280, 18);

    this->label_UserIDOrEmail = new QLabel("用户ID 或 邮箱", this->widget_card);
    this->label_UserIDOrEmail->setObjectName("label_UserIDOrEmail");
    this->label_UserIDOrEmail->move(24, this->label_smallTitle->pos().y() + this->label_smallTitle->height() + 20);//88
    this->label_UserIDOrEmail->resize(200, 16);

    this->edit_IDOrEmail = new QLineEdit(this->widget_card);
    this->edit_IDOrEmail->setObjectName("edit_IDOrEmail");
    this->edit_IDOrEmail->setPlaceholderText("name@example.com 或 123456789");
    this->edit_IDOrEmail->setMaxLength(254);
    this->edit_IDOrEmail->move(24, this->label_UserIDOrEmail->pos().y() + this->label_UserIDOrEmail->height() + 6);//110
    this->edit_IDOrEmail->resize(this->widget_card->width() - 48, 38);

    this->label_atIcon = new QLabel("@", this->edit_IDOrEmail);
    this->label_atIcon->setObjectName("label_atIcon");
    this->label_atIcon->resize(16, 16);
    this->label_atIcon->move(10, (this->edit_IDOrEmail->height() - this->label_atIcon->height()) / 2);

    this->label_verMsg = new QLabel("验证消息(选填)", this->widget_card);
    this->label_verMsg->setObjectName("label_varMsg");
    this->label_verMsg->move(24, this->edit_IDOrEmail->pos().y() + this->edit_IDOrEmail->height() + 8);//156
    this->label_verMsg->resize(200, 16);

    this->edit_verMsg = new QLineEdit(this->widget_card);
    this->edit_verMsg->setObjectName("edit_varMsg");
    this->edit_verMsg->setPlaceholderText("打个招呼...");
    this->edit_verMsg->setMaxLength(25);
    this->edit_verMsg->move(24, this->label_verMsg->pos().y() + this->label_verMsg->height() + 6);//176
    this->edit_verMsg->resize(this->widget_card->width() - 48, 38);

    this->group = new QButtonGroup(this);

    this->btn_addPersonal = new QPushButton("添加朋友", this->widget_card);
    this->btn_addPersonal->setObjectName("btn_addPersonal");
    this->btn_addPersonal->setIcon(QIcon(":/default/images/userInfo.png"));
    this->btn_addPersonal->setIconSize(QSize(18, 18));
    this->btn_addPersonal->resize(90, 36);
    this->btn_addPersonal->move(30, this->edit_verMsg->pos().y() + this->edit_verMsg->height() + 16);
    this->btn_addPersonal->setCheckable(true);

    this->btn_joinGroup = new QPushButton("加入群聊", this->widget_card);
    this->btn_joinGroup->setObjectName("btn_joinGroup");
    this->btn_joinGroup->setIcon(QIcon(":/default/images/group.png"));
    this->btn_joinGroup->setIconSize(QSize(24, 24));
    this->btn_joinGroup->resize(90, 36);
    this->btn_joinGroup->move(this->btn_addPersonal->pos().x() + this->btn_addPersonal->width() + 15,
                              this->btn_addPersonal->pos().y());
    this->btn_joinGroup->setCheckable(true);

    this->btn_createGroup = new QPushButton("创建群聊", this->widget_card);
    this->btn_createGroup->setObjectName("btn_createGroup");
    this->btn_createGroup->setIcon(QIcon(":/default/images/group.png"));
    this->btn_createGroup->setIconSize(QSize(24, 24));
    this->btn_createGroup->resize(90, 36);
    this->btn_createGroup->move(this->btn_joinGroup->pos().x() + this->btn_joinGroup->width() + 15,
                              this->btn_addPersonal->pos().y());
    this->btn_createGroup->setCheckable(true);

    this->group->addButton(this->btn_createGroup);
    this->group->addButton(this->btn_joinGroup);
    this->group->addButton(this->btn_addPersonal);
    this->btn_addPersonal->setChecked(true);

    this->view_inviteFriend = new QListView(this->widget_card);
    this->view_inviteFriend->setObjectName("view_inviteFriend");
    this->view_inviteFriend->resize(this->widget_card->width() - 40, 180);
    this->view_inviteFriend->move(20, this->btn_addPersonal->pos().y() + this->btn_addPersonal->height() + 18);
    this->view_inviteFriend->hide();

    this->model = new QStandardItemModel(this);
    this->delegate = new FriendDelegate(this);
    this->view_inviteFriend->setItemDelegate(this->delegate);
    this->view_inviteFriend->setModel(this->model);

    this->view_inviteFriend->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->view_inviteFriend->setSelectionMode(QAbstractItemView::MultiSelection);
    this->view_inviteFriend->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->view_inviteFriend->verticalScrollBar()->setSingleStep(6);
    this->view_inviteFriend->setResizeMode(QListView::Fixed);

    this->btn_add = new QPushButton("添加", this->widget_card);
    this->btn_add->setObjectName("btn_add");
    this->btn_add->resize(80, 36);
    this->btn_add->move(this->widget_card->width() - 24 - this->btn_add->width(),
                        this->btn_addPersonal->pos().y() + this->btn_addPersonal->height() + 18);//166

    this->btn_cancel = new QPushButton("取消", this->widget_card);
    this->btn_cancel->setObjectName("btn_cancel");
    this->btn_cancel->resize(70, 36);
    this->btn_cancel->move(this->btn_add->pos().x() - 12 - this->btn_cancel->width(), this->btn_add->pos().y());

    connect(this->btn_cancel, &QPushButton::clicked, this, [this](){
        this->hide();
        this->deleteLater();
    });

    connect(this->btn_add, &QPushButton::clicked, this, [this](){
        if(this->currentMode == AddFriend)
        {
            QString sid_email = this->edit_IDOrEmail->text().trimmed();
            QString verMsg = this->edit_verMsg->text().trimmed();
            if(sid_email.isEmpty())
            {
                ToastManager::getToastManager(this).error("待添加人信息不能为空");
                return;
            }
            QRegularExpression regExp_email("^[0-9a-zA-Z._%+\\-]+@[0-9a-zA-Z.\\-]+\\.[a-zA-Z]{2,}$");
            QRegularExpression regExp_sid("^\\d{10}$");
            if(regExp_email.match(sid_email).hasMatch())
            {
                TcpLongConnection::getTcpClient().sendAddNewFriendRequest("", sid_email, verMsg);
            }
            else if(regExp_sid.match(sid_email).hasMatch())
            {
                TcpLongConnection::getTcpClient().sendAddNewFriendRequest(sid_email, "", verMsg);
            }
            else
            {
                ToastManager::getToastManager(this).error("输入为非邮箱或者用户ID");
                return;
            }
        }
        else if(currentMode == JoinGroup)
        {
            QString groupID = this->edit_IDOrEmail->text().trimmed();
            QString verMsg = this->edit_verMsg->text().trimmed();
            if(groupID.isEmpty())
            {
                ToastManager::getToastManager(this).error("群聊ID不能为空");
                return;
            }
            QRegularExpression regExp_gid("^\\d{9,10}$");
            if(!regExp_gid.match(groupID).hasMatch())
            {
                ToastManager::getToastManager(this).error("请输入正确的群号");
                return;
            }

            TcpLongConnection::getTcpClient().sendApplyJoinGroupRequest(groupID, verMsg);
        }
        else if(currentMode == CreateGroup)
        {
            QString groupName = this->edit_IDOrEmail->text().trimmed();
            QString inviteMsg = this->edit_verMsg->text().trimmed();
            if(groupName.isEmpty())
            {
                ToastManager::getToastManager(this).error("群聊名称不能为空");
                return;
            }

            QModelIndexList selectedRows = this->view_inviteFriend->selectionModel()->selectedIndexes();
            QList<QString> invitesList;
            for(const QModelIndex& index : std::as_const(selectedRows))
                invitesList.append(index.data(FriendDelegate::UIDRole).toString());

            if(invitesList.isEmpty())
            {
                ToastManager::getToastManager(this).error("未选择被邀请的好友");
                return;
            }

            //发送tcp
            TcpLongConnection::getTcpClient().sendCreateGroupRequest(groupName, invitesList, inviteMsg);
        }

        this->hide();
        this->deleteLater();
    });

    connect(this->btn_addPersonal, &QPushButton::clicked, this, [this](){
        changeMode(AddFriend);
    });

    connect(this->btn_joinGroup, &QPushButton::clicked, this, [this](){
        changeMode(JoinGroup);
    });

    connect(this->btn_createGroup, &QPushButton::clicked, this, [this](){
        changeMode(CreateGroup);
    });

    const auto& infos = FriendManage::getFriendManage().getAllFriend();
    for(const auto& info : infos)
    {
        QStandardItem* item = new QStandardItem();
        item->setData(info.uid, FriendDelegate::UIDRole);
        item->setData(info.sid, FriendDelegate::SIDRole);
        item->setData(info.username, FriendDelegate::UsernameRole);
        QPixmap pix = FriendManage::getFriendManage().getFriendAvatar(info.conversationID, QSize(38, 38));
        pix.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
        item->setData(pix, FriendDelegate::AvatarRole);
        item->setData(info.email, FriendDelegate::EmailRole);
        item->setData(info.isOnline, FriendDelegate::IsOnlineRole);
        this->model->appendRow(item);
    }

    connect(this->delegate, &FriendDelegate::itemClicked, this, [this](){
        QModelIndexList selectedRows = this->view_inviteFriend->selectionModel()->selectedIndexes();
        for(const QModelIndex& index : std::as_const(selectedRows))
        {
            qDebug()<<index.data(FriendDelegate::UsernameRole).toString();
        }
    });
}

void AddNewFriendWidget::initStyle()
{
    this->setStyleSheet(R"(
        #widget_addNewFriend
        {
            background: rgba(0, 0, 0, 0.35);
        }
        #widget_card
        {
            background: #ffffff;
            border-radius: 14px;
        }
        #label_bigTitle
        {
            color: #1b1b23;
            font-size: 18px;
            font-weight: bold;
        }
        #label_smallTitle
        {
            color: #8a8a99;
            font-size: 12px;
        }
        #label_UserIDOrEmail,#label_varMsg
        {
            color: #1f2937;
            font-size: 12px;
            font-weight: bold;
        }
        #label_atIcon
        {
            color: #8a8a99;
            font-size: 14px;
            font-weight: bold;
            background: transparent;
        }
        #edit_IDOrEmail,#edit_varMsg
        {
            background: #f7f7fb;
            border: 1px solid #e4e4ec;
            border-radius: 8px;
            padding: 6px 10px;
            padding-left: 30px;
            font-size: 12px;
            color: #1b1b23;
            margin: 1px;
        }
        #edit_varMsg
        {
            padding: 6px 10px;
            padding-left: 20px;
        }
        #edit_IDOrEmail:focus,#edit_varMsg:focus
        {
            background: #ffffff;
            border: 1px solid #4648d4;
        }
        #btn_cancel
        {
            background: #f0f0f4;
            border: none;
            border-radius: 8px;
            color: #4b4b58;
            font-size: 13px;
            font-weight: bold;
        }
        #btn_cancel:hover
        {
            background: #e4e4ec;
        }
        #btn_add
        {
            background: #4648d4;
            border: none;
            border-radius: 8px;
            color: #ffffff;
            font-size: 13px;
            font-weight: bold;
        }
        #btn_add:hover
        {
            background: #6063ee;
        }
        #btn_add:pressed
        {
            background: #2f2ebe;
        }
        #btn_addPersonal,
        #btn_joinGroup,
        #btn_createGroup
        {
            background-color: rgba(249, 250, 251, 255);
            border: 1px solid rgba(229, 231, 235, 255);
            border-radius: 10px;
            padding: 0px;
            color: rgba(75, 85, 99, 255);
            font-size: 12px;
            font-weight: bold;
        }
        #btn_addPersonal:hover,
        #btn_joinGroup:hover,
        #btn_createGroup:hover
        {
            background-color: rgba(243, 244, 246, 255);
            border-color: rgba(209, 213, 219, 255);
        }
        #btn_addPersonal:pressed,
        #btn_joinGroup:pressed,
        #btn_createGroup:pressed
        {
            background-color: rgba(229, 231, 235, 255);
            border-color: rgba(156, 163, 175, 255);
        }
        #btn_addPersonal:checked,
        #btn_joinGroup:checked,
        #btn_createGroup:checked
        {
            background-color: rgba(238, 242, 255, 255);
            border-color: rgba(199, 210, 254, 255);
            color: rgba(99, 102, 241, 255);
        }
        #view_inviteFriend
        {
            background: transparent;
            border: 1px solid black;
            border-radius: 10px;
            margin-top: 2px;
            padding: 5px;
        }
                                #view_inviteFriend QScrollBar:vertical
                                {
                                    width: 5px;
                                    background: transparent;
                                    margin: 0px;
                                    margin-top: 2px;
                                }
                                #view_inviteFriend QScrollBar::handle:vertical
                                {
                                    background: rgba(0, 0, 0, 0.2);
                                    border-radius: 2px;
                                    min-height: 30px;
                                    margin-right: 1px;
                                }
                                #view_inviteFriend QScrollBar::handle:vertical:hover
                                {
                                    background: rgba(0, 0, 0, 0.35);
                                }
                                #view_inviteFriend QScrollBar::add-line:vertical,
                                #view_inviteFriend QScrollBar::sub-line:vertical
                                {
                                    height: 0px;
                                }
                                #view_inviteFriend QScrollBar::add-page:vertical,
                                #view_inviteFriend QScrollBar::sub-page:vertical
                                {
                                    background: none;
                                }
    )");
}

void AddNewFriendWidget::changeMode(Mode mode)
{
    switch(mode)
    {
    case AddFriend:
    {
        this->label_bigTitle->setText("添加好友");
        this->label_smallTitle->setText("输入对方信息即可添加好友");
        this->label_UserIDOrEmail->setText("用户ID 或 邮箱");
        this->edit_IDOrEmail->setPlaceholderText("name@example.com 或 123456789");
        this->edit_IDOrEmail->setMaxLength(254);
        this->label_verMsg->setText("验证消息(选填)");
        this->btn_add->setText("添加");
        this->view_inviteFriend->hide();
        this->btn_add->move(this->widget_card->width() - 24 - this->btn_add->width(),
                            this->btn_addPersonal->pos().y() + this->btn_addPersonal->height() + 18);
        this->btn_cancel->move(this->btn_add->pos().x() - 12 - this->btn_cancel->width(), this->btn_add->pos().y());
        this->widget_card->resize(360, 340);
        this->widget_card->move((this->width() - this->widget_card->width()) / 2,
                                (this->height() - this->widget_card->height()) / 2);
        break;
    }
    case JoinGroup:
    {
        this->label_bigTitle->setText("加入群聊");
        this->label_smallTitle->setText("输入群聊ID即可加入群聊");
        this->label_UserIDOrEmail->setText("群聊ID");
        this->edit_IDOrEmail->setPlaceholderText("123456789");
        this->edit_IDOrEmail->setMaxLength(18);
        this->label_verMsg->setText("验证消息(选填)");
        this->btn_add->setText("加入");
        this->view_inviteFriend->hide();
        this->btn_add->move(this->widget_card->width() - 24 - this->btn_add->width(),
                            this->btn_addPersonal->pos().y() + this->btn_addPersonal->height() + 18);
        this->btn_cancel->move(this->btn_add->pos().x() - 12 - this->btn_cancel->width(), this->btn_add->pos().y());
        this->widget_card->resize(360, 340);
        this->widget_card->move((this->width() - this->widget_card->width()) / 2,
                                (this->height() - this->widget_card->height()) / 2);
        break;
    }
    case CreateGroup:
    {
        this->label_bigTitle->setText("创建群聊");
        this->label_smallTitle->setText("选择邀请进入群聊的好友即可创建群聊");
        this->label_UserIDOrEmail->setText("群聊名称");
        this->edit_IDOrEmail->setPlaceholderText("贝利亚大王的群");
        this->edit_IDOrEmail->setMaxLength(64);
        this->label_verMsg->setText("邀请消息(选填)");
        this->btn_add->setText("创建");
        this->view_inviteFriend->show();
        this->btn_add->move(this->widget_card->width() - 24 - this->btn_add->width(),
                            this->view_inviteFriend->pos().y() + this->view_inviteFriend->height() + 18);
        this->btn_cancel->move(this->btn_add->pos().x() - 12 - this->btn_cancel->width(), this->btn_add->pos().y());
        this->widget_card->resize(360, 540);
        this->widget_card->move((this->width() - this->widget_card->width()) / 2,
                                (this->height() - this->widget_card->height()) / 2);
        break;
    }
    default:
        break;
    }
    this->currentMode = mode;
}

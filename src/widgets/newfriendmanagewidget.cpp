#include "newfriendmanagewidget.h"

NewFriendManageWidget::NewFriendManageWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);
    this->setObjectName("this");
    initWidget();
    initstyle();
}

void NewFriendManageWidget::addRequestsItem(QString uid, QString sid, QString username, QString avatar_url, QString verMsg)
{
    //添加项目
    QStandardItem* item = new QStandardItem();
    item->setData(uid, FriendApplyDelegate::UIDRole);
    item->setData(sid, FriendApplyDelegate::SIDRole);
    item->setData(username, FriendApplyDelegate::UsernameRole);
    //item->setData(avatar_url, FriendApplyDelegate::AvatarRole);
    item->setData(verMsg, FriendApplyDelegate::VerMsgRole);
    this->model->appendRow(item);
}

void NewFriendManageWidget::initWidget()
{
    this->widget_friendApply = new QWidget(this);
    this->widget_friendApply->setObjectName("widget_friendApply");
    this->widget_friendApply->resize(this->width(),56);
    this->widget_friendApply->move(0,0);

    this->label_friendApply = new QLabel("好友申请管理",this->widget_friendApply);
    this->label_friendApply->setObjectName("label_friendApply");
    this->label_friendApply->setAlignment(Qt::AlignVCenter);
    QFont font = this->label_friendApply->font();
    font.setWeight(QFont::DemiBold);
    this->label_friendApply->setFont(font);
    this->label_friendApply->setMaximumHeight(this->widget_friendApply->height());
    this->label_friendApply->setMinimumHeight(this->widget_friendApply->height());
    this->label_friendApply->move(0,(this->widget_friendApply->height() - this->label_friendApply->height())/2);

    this->listView_friendApplyManage = new QListView(this);
    this->listView_friendApplyManage->setObjectName("listView_friendApplyManage");
    this->listView_friendApplyManage->resize(this->width(),this->height() - this->widget_friendApply->height());
    this->listView_friendApplyManage->move(0,this->widget_friendApply->height());
    this->listView_friendApplyManage->setMouseTracking(true);
    this->listView_friendApplyManage->viewport()->setMouseTracking(true);

    this->model = new QStandardItemModel(this);
    this->listView_friendApplyManage->setModel(this->model);

    this->delegate = new FriendApplyDelegate(this);
    this->listView_friendApplyManage->setItemDelegate(this->delegate);

    this->listView_friendApplyManage->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->listView_friendApplyManage->setSelectionMode(QAbstractItemView::NoSelection);
    this->listView_friendApplyManage->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->listView_friendApplyManage->setResizeMode(QListView::Fixed);

    connect(this->delegate, &FriendApplyDelegate::agreeClicked, this, [](QString UID, const QModelIndex& index){
        TcpLongConnection::getTcpClient().sendHandleNewFriendRequest(UserInfo::getUserInfo().getUID(), UID, true);
    });

    connect(this->delegate, &FriendApplyDelegate::refuseClicked, this, [](QString UID, const QModelIndex& index){
        TcpLongConnection::getTcpClient().sendHandleNewFriendRequest(UserInfo::getUserInfo().getUID(), UID, false);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newFriendRequests, this, [this](QString uid, QString sid, QString username, QString avatar_url, QString verMsg){
        addRequestsItem(uid, sid, username, avatar_url, verMsg);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::cleanNewFriendRequestsList, this, [this](){
        this->model->clear();
    });
}

void NewFriendManageWidget::initstyle()
{
    this->setStyleSheet(R"(
                            #widget_friendApply
                            {
                                border-bottom:1px solid #e0e0e0;
                            }
                            #label_friendApply
                            {
                                color: rgba(17, 24, 39, 255);
                                padding-left: 24px;
                                font-size:13px;
                            }
                            #listView_friendApplyManage
                            {
                                border-radius:0px;
                                background:transparent;
                                padding-bottom: 10px;
                            }
                                )");
}

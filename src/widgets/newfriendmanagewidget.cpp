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
    item->setData("", FriendApplyDelegate::AvatarRole);

    item->setData(verMsg, FriendApplyDelegate::VerMsgRole);
    this->model->appendRow(item);

    if(!avatar_url.isEmpty())
    {
        //防止头像下载成功后item野指针
        QPersistentModelIndex index = this->model->indexFromItem(item);
        QPointer<QStandardItemModel> modelPtr(this->model);
        HttpShortConnection::getHttpClient().getAvatar(avatar_url, 3, [index, modelPtr](const QPixmap& avatar){
            QMetaObject::invokeMethod(qApp, [index, modelPtr, avatar](){
                if(modelPtr && index.isValid())
                {
                    modelPtr->setData(index, avatar, FriendApplyDelegate::AvatarRole);
                }
            }, Qt::QueuedConnection);
        }, false);
    }
    emit RequestsNumberChange(this->model->rowCount());
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
    this->listView_friendApplyManage->verticalScrollBar()->setSingleStep(8);
    this->listView_friendApplyManage->setResizeMode(QListView::Fixed);

    this->widget_status = new QWidget(this->listView_friendApplyManage);
    this->widget_status->setObjectName("widget_status");
    this->widget_status->resize(this->listView_friendApplyManage->width(),this->listView_friendApplyManage->height());
    this->widget_status->move(0, 0);
    this->widget_status->hide();

    this->label_loadingIcon = new LoadingLabel(this->widget_status);
    this->label_loadingIcon->setObjectName("label_loadingIcon");
    this->label_loadingIcon->setPixmap(QIcon(":/default/images/loading.png").pixmap(24));
    this->label_loadingIcon->resize(30, 30);
    this->label_loadingIcon->move((this->widget_status->width() - this->label_loadingIcon->width())/2,
                              (this->widget_status->height() - this->label_loadingIcon->height())/2 - 40);
    this->label_loadingIcon->hide();

    this->animation_loading = new QPropertyAnimation(this->label_loadingIcon, "angle", this);
    this->animation_loading->setStartValue(0);
    this->animation_loading->setEndValue(360);
    this->animation_loading->setDuration(2000);
    this->animation_loading->setLoopCount(-1);
    this->animation_loading->start();

    this->label_loading = new QLabel("正在加载好友申请...", this->widget_status);
    this->label_loading->setObjectName("label_loading");
    this->label_loading->resize(this->widget_status->width(), 40);
    this->label_loading->move(0, (this->widget_status->height() - this->label_loading->height())/2);
    this->label_loading->setAlignment(Qt::AlignCenter);
    this->label_loading->hide();

    this->label_empty = new QLabel("暂无好友申请", this->widget_status);
    this->label_empty->setObjectName("label_empty");
    this->label_empty->resize(this->widget_status->width(), 40);
    this->label_empty->move(0, (this->widget_status->height() - this->label_empty->height())/2 - 30);
    this->label_empty->setAlignment(Qt::AlignCenter);
    this->label_empty->hide();

    this->btn_refresh = new QPushButton("刷新", this->widget_status);
    this->btn_refresh->setObjectName("btn_refresh");
    this->btn_refresh->resize(64, 32);
    this->btn_refresh->move((this->widget_status->width() - this->btn_refresh->width())/2,
                            this->label_loading->pos().y() + this->label_loading->height() - 25);
    this->btn_refresh->hide();

    changeStatus(Empty);

    connect(this->delegate, &FriendApplyDelegate::agreeClicked, this, [this](QString UID, const QModelIndex& index){
        this->waitingHandleIndex = QPersistentModelIndex(index);
        TcpLongConnection::getTcpClient().sendHandleNewFriendRequest(UID, true);
    });

    connect(this->delegate, &FriendApplyDelegate::refuseClicked, this, [this](QString UID, const QModelIndex& index){
        this->waitingHandleIndex = QPersistentModelIndex(index);
        TcpLongConnection::getTcpClient().sendHandleNewFriendRequest(UID, false);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newFriendRequests, this, [this](QString uid, QString sid, QString username, QString avatar_url, QString verMsg){
        addRequestsItem(uid, sid, username, avatar_url, verMsg);
        changeStatus(Hide);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::cleanNewFriendRequestsList, this, [this](){
        this->model->clear();
        emit RequestsNumberChange(this->model->rowCount());
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newFriendRequestsState, this, [this](bool isEmpty){
        if(isEmpty)
        {
            this->model->clear();
            emit RequestsNumberChange(this->model->rowCount());
            changeStatus(Empty);
        }
        else
        {
            changeStatus(Loading);
        }
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newFriendRequestsHandleResult, this, [this](bool isSuccess){
        //处理index删除还是保留
        if(isSuccess)
        {
            if(this->waitingHandleIndex.isValid())
            {
                this->model->removeRow(this->waitingHandleIndex.row());
                emit RequestsNumberChange(this->model->rowCount());
            }
        }
        changeStatus(Hide);
        if(this->model->rowCount() == 0)
            changeStatus(Empty);
    });


    connect(this->btn_refresh, &QPushButton::clicked, this, [this](){
        changeStatus(Loading);
        TcpLongConnection::getTcpClient().getNewFriendRequestsList();
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
                            #listView_friendApplyManage QScrollBar:vertical
                            {
                                width: 6px;
                                background: transparent;
                                margin: 0px;
                                margin-top: 2px;
                            }
                            #listView_friendApplyManage QScrollBar::handle:vertical
                            {
                                background: rgba(0, 0, 0, 0.2);
                                border-radius: 3px;
                                min-height: 30px;
                            }
                            #listView_friendApplyManage QScrollBar::handle:vertical:hover
                            {
                                background: rgba(0, 0, 0, 0.35);
                            }
                            #listView_friendApplyManage QScrollBar::add-line:vertical,
                            #listView_friendApplyManage QScrollBar::sub-line:vertical
                            {
                                height: 0px;
                            }
                            #listView_friendApplyManage QScrollBar::add-page:vertical,
                            #listView_friendApplyManage QScrollBar::sub-page:vertical
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
                            }
                            #label_empty
                            {
                                color:#6B7280;
                                font-size:15px;
                                font-weight:600;
                            }
                            #btn_refresh
                            {
                                border:none;
                                border-radius:12px;
                                color:white;
                                background:#4648d4;
                            }
                            #btn_refresh:hover
                            {
                                background:#6063ee;
                            }
                            #btn_refresh:pressed
                            {
                                background:#2f2ebe;
                            }
                                )");
}

void NewFriendManageWidget::changeStatus(Status status)
{
    if(status == Loading)
    {
        this->widget_status->setProperty("status", "loading");
        this->animation_loading->start();
        this->label_loadingIcon->show();
        this->animation_loading->start();
        this->label_loading->show();

        this->label_empty->hide();
        this->btn_refresh->hide();
        this->widget_status->show();
    }
    else if(status == Empty)
    {
        this->widget_status->setProperty("status", "empty");
        this->animation_loading->stop();

        this->label_loadingIcon->hide();
        this->label_loading->hide();

        this->label_empty->show();
        this->btn_refresh->show();
        this->widget_status->show();
    }
    else if(status == Hide)
    {
        this->widget_status->hide();
        this->animation_loading->stop();
    }
    this->widget_status->style()->unpolish(this->widget_status);
    this->widget_status->style()->polish(this->widget_status);
}

#include "newfriendmanagewidget.h"

NewFriendManageWidget::NewFriendManageWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);
    this->setObjectName("this");
    initWidget();
    initstyle();
}

void NewFriendManageWidget::addFriendRequestsItem(QString uid, QString sid, QString username, QString avatar_url, QString verMsg)
{
    //添加项目
    QStandardItem* item = new QStandardItem();
    item->setData(uid, FriendApplyDelegate::UIDRole);
    item->setData(sid, FriendApplyDelegate::SIDRole);
    item->setData(username, FriendApplyDelegate::UsernameRole);

    QPixmap defAvatar(":/default/images/defaultAvatar.png");
    defAvatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
    item->setData(defAvatar, FriendApplyDelegate::AvatarRole);

    item->setData(verMsg, FriendApplyDelegate::VerMsgRole);
    item->setData(false, FriendApplyDelegate::IsGroupRequest);
    this->model->appendRow(item);

    QString loadUrl = avatar_url.isEmpty() ? ":/default/images/defaultAvatar.png" : avatar_url;

    //防止头像下载成功后item野指针
    QPersistentModelIndex index = this->model->indexFromItem(item);
    QPointer<QStandardItemModel> modelPtr(this->model);

    ImageCacheManager::getManager().loadImage(loadUrl, [this, modelPtr, index](const QPixmap& pix){
        if(!pix.isNull())
        {
            if(modelPtr && index.isValid())
            {
                QPixmap pix_copy = pix;
                pix_copy.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                modelPtr->setData(index, pix_copy, FriendApplyDelegate::AvatarRole);
            }
            return;
        }
        ImageCacheManager::getManager().loadImage(":/default/images/defaultAvatar.png", [this, modelPtr, index](const QPixmap& pix){
            if(pix.isNull())
                return;
            if(modelPtr && index.isValid())
            {
                QPixmap pix_copy = pix;
                pix_copy.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                modelPtr->setData(index, pix_copy, FriendApplyDelegate::AvatarRole);
            }
        }, false, 0, 0, QSize(48, 48));

    }, false, 0, 0, QSize(48, 48));

    emit RequestsNumberChange(this->model->rowCount());
}

void NewFriendManageWidget::addGroupRequestsItem(const QString &requestID, const QString &groupName, int source, const QString &displayName, const QString &displayAvatarUrl, const QString &verMsg, const QString &applicantSID, const QString &inviterName)
{
    QStandardItem* item = new QStandardItem();
    item->setData(requestID, FriendApplyDelegate::RequestID);
    item->setData(groupName, FriendApplyDelegate::GroupName);
    item->setData(source, FriendApplyDelegate::Source);
    item->setData(displayName, FriendApplyDelegate::UsernameRole);
    item->setData(verMsg, FriendApplyDelegate::VerMsgRole);
    item->setData(applicantSID, FriendApplyDelegate::SIDRole);
    item->setData(inviterName, FriendApplyDelegate::InviterName);
    item->setData(true, FriendApplyDelegate::IsGroupRequest);

    QPixmap defAvatar(":/default/images/defaultAvatar.png");
    defAvatar.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
    item->setData(defAvatar, FriendApplyDelegate::AvatarRole);

    this->model->appendRow(item);

    QString loadUrl = displayAvatarUrl.isEmpty() ? ":/default/images/defaultAvatar.png" : displayAvatarUrl;

    //防止头像下载成功后item野指针
    QPersistentModelIndex index = this->model->indexFromItem(item);
    QPointer<QStandardItemModel> modelPtr(this->model);

    ImageCacheManager::getManager().loadImage(loadUrl, [this, modelPtr, index](const QPixmap& pix){
        if(!pix.isNull())
        {
            if(modelPtr && index.isValid())
            {
                QPixmap pix_copy = pix;
                pix_copy.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                modelPtr->setData(index, pix_copy, FriendApplyDelegate::AvatarRole);
            }
            return;
        }
        ImageCacheManager::getManager().loadImage(":/default/images/defaultAvatar.png", [this, modelPtr, index](const QPixmap& pix){
            if(pix.isNull())
                return;
            if(modelPtr && index.isValid())
            {
                QPixmap pix_copy = pix;
                pix_copy.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
                modelPtr->setData(index, pix_copy, FriendApplyDelegate::AvatarRole);
            }
        }, false, 0, 0, QSize(48, 48));

    }, false, 0, 0, QSize(48, 48));

    emit RequestsNumberChange(this->model->rowCount());
}

void NewFriendManageWidget::initWidget()
{
    this->widget_friendApply = new QWidget(this);
    this->widget_friendApply->setObjectName("widget_friendApply");
    this->widget_friendApply->resize(this->width(),56);
    this->widget_friendApply->move(0,0);

    this->label_friendApply = new QLabel("申请管理",this->widget_friendApply);
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

    this->label_loading = new QLabel("正在加载申请列表...", this->widget_status);
    this->label_loading->setObjectName("label_loading");
    this->label_loading->resize(this->widget_status->width(), 40);
    this->label_loading->move(0, (this->widget_status->height() - this->label_loading->height())/2);
    this->label_loading->setAlignment(Qt::AlignCenter);
    this->label_loading->hide();

    this->label_empty = new QLabel("暂无新申请", this->widget_status);
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

    connect(this->delegate, &FriendApplyDelegate::agreeFriendClicked, this, [this](const QString& UID, const QModelIndex& index){
        if(!index.data(FriendApplyDelegate::IsGroupRequest).toBool())
            TcpLongConnection::getTcpClient().sendHandleNewFriendRequest(UID, true);
    });

    connect(this->delegate, &FriendApplyDelegate::refuseFriendClicked, this, [this](const QString& UID, const QModelIndex& index){
        if(!index.data(FriendApplyDelegate::IsGroupRequest).toBool())
            TcpLongConnection::getTcpClient().sendHandleNewFriendRequest(UID, false);
    });

    connect(this->delegate, &FriendApplyDelegate::agreeGroupClicked, this, [this](const QString& requestID, const QModelIndex& index){
        if(index.data(FriendApplyDelegate::IsGroupRequest).toBool())
            TcpLongConnection::getTcpClient().sendHandleNewGroupJoinRequest(requestID, true);
    });

    connect(this->delegate, &FriendApplyDelegate::refuseGroupClicked, this, [this](const QString& requestID, const QModelIndex& index){
        if(index.data(FriendApplyDelegate::IsGroupRequest).toBool())
            TcpLongConnection::getTcpClient().sendHandleNewGroupJoinRequest(requestID, false);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newFriendRequests, this, [this](QString uid, QString sid, QString username, QString avatar_url, QString verMsg){
        addFriendRequestsItem(uid, sid, username, avatar_url, verMsg);
        changeStatus(Hide);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newGroupJoinRequest, this, [this](const QString& requestID, const QString& groupName, int source, const QString& displayName,
                                                                                                      const QString& displayAvatarUrl, const QString& verMsg,
                                                                                                      const QString& applicantSID, const QString& inviterName){
        addGroupRequestsItem(requestID, groupName, source, displayName, displayAvatarUrl, verMsg, applicantSID, inviterName);
        changeStatus(Hide);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::cleanNewFriendRequestsList, this, [this](){
        for(int i = this->model->rowCount() - 1; i >= 0; i--)
        {
            QModelIndex index = this->model->index(i, 0);
            if(!index.data(FriendApplyDelegate::IsGroupRequest).toBool())
                this->model->removeRow(i);
        }
        emit RequestsNumberChange(this->model->rowCount());
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::cleanNewGroupJoinRequestsList, this, [this](){
        for(int i = this->model->rowCount() - 1; i >= 0; i--)
        {
            QModelIndex index = this->model->index(i, 0);
            if(index.data(FriendApplyDelegate::IsGroupRequest).toBool())
                this->model->removeRow(i);
        }
        emit RequestsNumberChange(this->model->rowCount());
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newFriendRequestsState, this, [this](bool isEmpty){
        if(isEmpty)
        {
            for(int i = this->model->rowCount() - 1; i >= 0; i--)
            {
                QModelIndex index = this->model->index(i, 0);
                if(!index.data(FriendApplyDelegate::IsGroupRequest).toBool())
                    this->model->removeRow(i);
            }
            emit RequestsNumberChange(this->model->rowCount());

            if(this->model->rowCount() > 0)
                changeStatus(Hide);
            else
                changeStatus(Empty);
        }
        else
        {
            changeStatus(Loading);
        }
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newGroupJoinRequestsState, this, [this](bool isEmpty){
        if(isEmpty)
        {
            for(int i = this->model->rowCount() - 1; i >= 0; i--)
            {
                QModelIndex index = this->model->index(i, 0);
                if(index.data(FriendApplyDelegate::IsGroupRequest).toBool())
                    this->model->removeRow(i);
            }
            emit RequestsNumberChange(this->model->rowCount());

            if(this->model->rowCount() > 0)
                changeStatus(Hide);
            else
                changeStatus(Empty);
        }
        else
        {
            changeStatus(Loading);
        }
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newFriendRequestsHandleResult, this, [this](const QString& handleUID, bool isSuccess){
        //处理index删除还是保留
        if(isSuccess)
        {
            for(int i = 0; i < this->model->rowCount(); i++)
            {
                QModelIndex index = this->model->index(i, 0);
                if(index.data(FriendApplyDelegate::UIDRole).toString() == handleUID)
                {
                    this->model->removeRow(i);
                    break;
                }
            }
            emit RequestsNumberChange(this->model->rowCount());
        }

        changeStatus(Hide);
        if(this->model->rowCount() == 0)
            changeStatus(Empty);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newGroupJoinRequestsHandleResult, this, [this](const QString& handleRequestID, bool isSuccess){
        //处理index删除还是保留
        if(isSuccess)
        {
            for(int i = 0; i < this->model->rowCount(); i++)
            {
                QModelIndex index = this->model->index(i, 0);
                if(index.data(FriendApplyDelegate::RequestID).toString() == handleRequestID)
                {
                    this->model->removeRow(i);
                    break;
                }
            }
            emit RequestsNumberChange(this->model->rowCount());
        }
        changeStatus(Hide);
        if(this->model->rowCount() == 0)
            changeStatus(Empty);
    });

    connect(this->btn_refresh, &QPushButton::clicked, this, [this](){
        changeStatus(Loading);
        TcpLongConnection::getTcpClient().getNewFriendRequestsList();
        TcpLongConnection::getTcpClient().getNewGroupJoinRequestsList();
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

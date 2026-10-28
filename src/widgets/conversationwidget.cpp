#include "conversationwidget.h"

ConversationWidget::ConversationWidget(int width, int height, ConversationItem* item, QWidget *parent)
    : QWidget{parent}, item(item)
{
    this->setObjectName("widget_conversation");
    this->resize(width, height);
    this->widget_header = new QWidget(this);
    this->widget_header->setObjectName("widget_header");
    this->widget_header->resize(this->width(), 56);
    this->widget_header->move(0,0);

    QString name;
    bool isOnline = false;

    ConversationItem::ConversationType type = item->getConversationType();
    if(type == ConversationItem::ConversationType::Personal)
    {
        FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(item->getConversationID());
        if(info.uid.isEmpty())
        {
            name = "未知用户";
        }
        else
        {
            name = info.username;
            isOnline = info.isOnline;
        }
    }
    else if(type == ConversationItem::ConversationType::Group)
    {
        GroupManager::GroupInfo info = GroupManager::getGroupManage().getGroupInfo(this->item->getConversationID());
        name = info.groupName.isEmpty() ? "未知群聊" : info.groupName;
    }

    //初始化 更多 按钮
    this->btn_more = new QPushButton(this->widget_header);
    this->btn_more->setObjectName("btn_more");
    this->btn_more->resize(32,32);
    this->btn_more->move(this->widget_header->width() - this->btn_more->width() - 16,
                         (this->widget_header->height() - this->btn_more->height())/2);
    this->btn_more->setIcon(QIcon(":/default/images/more.png"));
    this->btn_more->setIconSize(QSize(16,16));
    this->btn_more->setCheckable(true);
    this->btn_more->setChecked(false);

    this->label_name = new QLabel(name, this->widget_header);
    this->label_name->setObjectName("label_name");
    this->label_name->setMaximumHeight(24);
    this->label_name->setMinimumHeight(24);
    this->label_name->move(16,(this->widget_header->height() - this->label_name->height())/2);
    this->label_name->setFixedWidth(this->widget_header->width() - this->label_name->pos().x() - (this->widget_header->width() - this->btn_more->pos().x()));

    QFont font = this->label_name->font();
    font.setPointSize(12);
    font.setWeight(QFont::Bold);
    this->label_name->setFont(font);

    QString show_name;
    QFontMetrics fm(font);

    if(type == ConversationItem::ConversationType::Personal)
    {
        //初始化 在线状态图标
        this->label_statusIcon = new QLabel(this->widget_header);
        this->label_statusIcon->setObjectName("label_statusIcon");
        this->label_statusIcon->setFixedSize(8, 8);

        if(isOnline)
            this->label_statusIcon->setProperty("status", "online");
        else
            this->label_statusIcon->setProperty("status", "offline");

        //初始化 在线状态文字
        QString status_text = isOnline ? "在线" : "离线";
        this->label_statusText = new QLabel(status_text, this->widget_header);
        this->label_statusText->setObjectName("label_statusText");
        this->label_statusText->setMaximumHeight(20);
        this->label_statusText->setMinimumHeight(20);

        QFontMetrics fmStatus(this->label_statusText->font());
        int statusWidth = 8 + 8 + fmStatus.horizontalAdvance(status_text) + 8;

        show_name = fm.elidedText(name, Qt::ElideRight, this->label_name->width() - statusWidth);
    }
    else
    {
        this->label_statusIcon = nullptr;
        this->label_statusText = nullptr;


        int64_t memberCount = GroupManager::getGroupManage().getGroupInfo(this->item->getConversationID()).memberInfos.size();
        QString memberCountStr = "(" + QString::number(memberCount) + ")";
        int width_memberCount = fm.horizontalAdvance(memberCountStr);

        show_name = fm.elidedText(name, Qt::ElideRight, this->label_name->width() - width_memberCount) + memberCountStr;
    }
    this->label_name->setText(show_name);
    if(type == ConversationItem::ConversationType::Personal)
        updateStatusIconPos();

    this->timer_loading = new QTimer(this);
    connect(this->timer_loading, &QTimer::timeout, this, [this](){
        this->loadingAngle = (this->loadingAngle + 5) % 360;
        this->listView_messages->viewport()->update();
        if(loadingCount == 0)
            this->timer_loading->stop();
    });

    //初始化 消息显示 部分
    this->listView_messages = new QListView(this);
    this->listView_messages->setObjectName("listView_messages");
    this->listView_messages->resize(this->width(),(this->height() - this->widget_header->height()) * 0.65);
    this->listView_messages->move(0, this->widget_header->height());

    this->model = new MessageModel(&(item->getMessagesManager()), this);
    this->listView_messages->setModel(this->model);

    this->delegate = new ConversationDelegate(&this->loadingAngle, this);
    this->listView_messages->setItemDelegate(this->delegate);

    this->listView_messages->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->listView_messages->setSelectionMode(QAbstractItemView::NoSelection);
    this->listView_messages->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->listView_messages->verticalScrollBar()->setSingleStep(10);
    this->listView_messages->setUniformItemSizes(false);
    this->listView_messages->setResizeMode(QListView::Adjust);

    connect(this->listView_messages->verticalScrollBar(), &QScrollBar::rangeChanged, this->listView_messages, [this](int, int){
        //防止视图宽度变小导致缓存未更新
        this->listView_messages->doItemsLayout();
    });

    //初始化 编辑框 区域
    this->widget_editRegion = new QWidget(this);
    this->widget_editRegion->setObjectName("widget_editRegion");
    this->widget_editRegion->resize(this->width() * 0.98,(this->height() - this->listView_messages->height() - this->widget_header->height()) * 0.9);
    int editRegion_mheight = this->height() - this->widget_header->height() - this->listView_messages->height();
    this->widget_editRegion->move((this->width() - this->widget_editRegion->width()) / 2,
                     this->listView_messages->pos().y() + this->listView_messages->height() + (editRegion_mheight - this->widget_editRegion->height()) / 2);

    //初始化 编辑框
    this->edit_message = new ChatTextEdit(this->widget_editRegion);
    this->edit_message->setObjectName("edit_message");
    this->edit_message->setPlaceholderText("输入些什么...");
    this->edit_message->setAcceptRichText(true);
    this->edit_message->resize(this->widget_editRegion->width() * 0.98, this->widget_editRegion->height() * 0.78);
    this->edit_message->move((this->widget_editRegion->width() - this->edit_message->width()) / 2,
                             0);

    connect(this->edit_message,&QTextEdit::textChanged,this,[=](){
        QString text = this->edit_message->toPlainText();
        if(text.length() > 10000)
        {
            //设置截断信号
            this->edit_message->blockSignals(true);
            int cursorPos = this->edit_message->textCursor().position();
            text.truncate(10000);
            this->edit_message->setPlainText(text);
            // 恢复光标位置（不超出文本长度）
            QTextCursor cursor = this->edit_message->textCursor();
            cursor.setPosition(qMin(cursorPos, text.length()));
            this->edit_message->setTextCursor(cursor);
            this->edit_message->blockSignals(false);
        }
    });

    //初始化 文件 按钮
    this->btn_file = new QPushButton(this->widget_editRegion);
    this->btn_file->setObjectName("btn_file");
    int fileSize = this->widget_editRegion->height() - this->edit_message->pos().y() - this->edit_message->height();
    this->btn_file->resize(fileSize * 0.8,fileSize * 0.8);
    this->btn_file->setIcon(QIcon(":/default/images/file.png"));
    this->btn_file->setIconSize(QSize(this->btn_file->width() * 0.8,this->btn_file->height() * 0.8));
    int file_h = this->edit_message->pos().y() + this->edit_message->height();
    this->btn_file->move(15, file_h + (this->widget_editRegion->height() - file_h - this->btn_file->height())/2);

    connect(this->btn_file, &QPushButton::clicked, this, [this](){
        QString filePath = QFileDialog::getOpenFileName(this, "选择文件", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation), "所有文件 (*)");
        if(filePath.isEmpty())
            return;

        QString suffix = QFileInfo(filePath).suffix().toLower();
        if(suffix == "jpg" || suffix == "jpeg" || suffix == "png")
        {
            QImage image(filePath);
            if(image.isNull())
            {
                ToastManager::getToastManager(this).error("图片读取失败");
                return;
            }

            this->edit_message->insertFileToEdit(filePath, Image);
        }
        else if(suffix == "mp4" || suffix == "mov" || suffix == "webm")
            this->edit_message->insertFileToEdit(filePath, Video);
        else
            this->edit_message->insertFileToEdit(filePath, File);
    });

    //初始化 发送 按钮
    this->btn_send = new QPushButton(this->widget_editRegion);
    this->btn_send->setObjectName("btn_send");
    int sendSize = this->widget_editRegion->height() - this->edit_message->pos().y() - this->edit_message->height();
    this->btn_send->resize(sendSize*0.8,sendSize*0.8);
    this->btn_send->setIcon(QIcon(":/default/images/send.png"));
    this->btn_send->setIconSize(QSize(this->btn_send->width()*0.4,this->btn_send->height()*0.4));
    int send_h = this->edit_message->pos().y() + this->edit_message->height();
    this->btn_send->move(this->widget_editRegion->width() * 0.99 - this->btn_send->width(),
                         send_h + (this->widget_editRegion->height() - send_h - this->btn_send->height())/2);

    connect(this->btn_send,&QPushButton::clicked,this,[this](){
        this->edit_message->saveBlocks();
        QList<ChatTextEdit::MessageBlock>& blocks = this->edit_message->getAllBlocks();

        for(auto& block : blocks)
        {
            const QString& content = block.content.trimmed();
            if(content.trimmed().isEmpty())
                continue;

            QString tempMsgID = QUuid::createUuid().toString();
            block.tempID = tempMsgID;

            Message msg;
            msg.tempMsgID = tempMsgID;
            msg.timeStamp = QDateTime::currentSecsSinceEpoch();
            msg.senderUID = UserInfo::getUserInfo().getUID();
            msg.status = Sending;
            msg.convSeq = this->item->getMessagesManager().getNextConvSeq();

            msg.contentType = block.type;
            msg.content = block.content;

            if(msg.contentType != ContentType::Text)
            {
                msg.info.url = content;
                QJsonObject obj;
                obj["Url"] = block.content;

                if(msg.contentType == ContentType::Image)
                {
                    obj["ThumbnailUrl"] = block.thumbnailUrl;
                    msg.info.thumbnailUrl = block.thumbnailUrl;
                }
                else if(msg.contentType == ContentType::File)
                {
                    QFileInfo info(block.content);
                    if(info.exists())
                    {
                        obj["Name"] = info.fileName();
                        obj["Size"] = QString::number(info.size());
                        msg.info.name = info.fileName();
                        msg.info.size = info.size();
                    }
                }

                msg.content = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
                msg.info.url = block.content;
            }

            this->item->addNewMessage(msg);
            this->loadingCount++;
            QTimer::singleShot(0, this, [this](){
                if(this->loadingCount > 0 && !this->timer_loading->isActive())
                    this->timer_loading->start(30);
            });
        }
        handleNextBlock();
        this->edit_message->clear();
        this->listView_messages->scrollToBottom();
    });

    this->label_deletedTip = new QLabel("对方已不是您的好友，无法发送消息", this->widget_editRegion);
    this->label_deletedTip->setObjectName("label_deletedTip");
    this->label_deletedTip->resize(this->widget_editRegion->width(), this->widget_editRegion->height());
    this->label_deletedTip->move(0, 0);
    this->label_deletedTip->setAlignment(Qt::AlignCenter);
    updateDeletedState(item->getIsDeleted());

    //初始化更多弹窗
    GroupManager::GroupInfo info = GroupManager::getGroupManage().getGroupInfo(this->item->getConversationID());
    this->widget_menu = new MoreWidget(item->getConversationType() == ConversationItem::ConversationType::Personal, info.ownerUID == UserInfo::getUserInfo().getUID(), this);

    connect(this->btn_more,&QPushButton::toggled,this,[=](bool checked){
        if(checked)
        {
            this->widget_menu->move(this->btn_more->mapToGlobal(QPoint(0 - this->widget_menu->width() + this->btn_more->width(),
                                                                       this->btn_more->height())));
            this->widget_menu->show();
            this->widget_menu->activateWindow();
            this->widget_menu->setFocus();
        }
        else
        {
            this->widget_menu->hide();
        }
    });
    connect(this->widget_menu,&MoreWidget::closed, this->btn_more, [=](){
        if(this->btn_more->underMouse())
            return;
        this->btn_more->setChecked(false);
    });

    connect(this->widget_menu, &MoreWidget::switchToInfoWidget, this, [this](){
        if(this->item->getIsDeleted())
        {
            if(this->item->getConversationType() == ConversationItem::ConversationType::Group)
                ToastManager::getToastManager(this).error("您已不在该群聊中");
            else
                ToastManager::getToastManager(this).error("对方已不是您的好友");
            return;
        }
        emit switchToInfoWidget();
    });

    connect(this->widget_menu, &MoreWidget::clearMessages, this, [this](){
        DatabaseManager::getDatabaseManager().clearChatMessagesOfConversationID(this->item->getConversationID());
        this->item->clearMessages();

        this->loadingCount = 0;
        this->timer_loading->stop();
        this->isFirstLoaded = false;
        this->topMsgID.clear();
        this->isLoadingOlder = false;
        this->isLoadingNewer = false;
        this->isWaitingJump = false;
        this->listView_messages->scrollToBottom();
    });

    connect(this->widget_menu, &MoreWidget::deletedConversation, this, [this](){
        if(this->item->getIsDeleted())
            return;

        if(this->item->getConversationType() == ConversationItem::ConversationType::Personal)
            TcpLongConnection::getTcpClient().sendDeleteFriend(this->item->getConversationID());
        else
        {
            if(GroupManager::getGroupManage().getGroupInfo(this->item->getConversationID()).ownerUID == UserInfo::getUserInfo().getUID())
                TcpLongConnection::getTcpClient().sendDismissGroup(this->item->getConversationID());//解散
            else
                TcpLongConnection::getTcpClient().sendExitGroup(this->item->getConversationID());
        }
    });

    initStyle();
    startReFlashTimeStamp();

    connect(this->item, &ConversationItem::PushNewMessage, this, [this](const Message &msg){
        this->listView_messages->doItemsLayout();
        QScrollBar* scrollBar = this->listView_messages->verticalScrollBar();
        int height = this->listView_messages->viewport()->height();
        if(this->isVisible() && scrollBar->maximum() - scrollBar->value() < (height / 2))
        {
            this->listView_messages->scrollToBottom();
            setActive(true);
        }
        else if(scrollBar->maximum() - scrollBar->value() < (height / 2))
            this->listView_messages->scrollToBottom();
    });

    connect(this->item, &ConversationItem::messageStatusChange, this, [this](){
        this->listView_messages->doItemsLayout();
        if(this->loadingCount > 0)
        {
            this->loadingCount--;
        }
    });

    connect(this->item, &ConversationItem::deletedStateChanged, this, [this](bool isDeleted){
        updateDeletedState(isDeleted);
    });

    connect(this->delegate, &ConversationDelegate::ReSendClicked, this, [this](const QString& tempMsgID){
        if(this->item->getIsDeleted())
            return;

        int index = this->item->getMessagesManager().indexOfMsg(tempMsgID);
        if(index < 0)
            return;

        this->item->getMessagesManager().retryMessage(index);
        this->listView_messages->doItemsLayout();

        const Message& msg = this->item->getMessagesManager().getLastMessage();

        this->loadingCount++;
        QTimer::singleShot(0, this, [this](){
            if(this->loadingCount > 0 && !this->timer_loading->isActive())
                this->timer_loading->start(30);
        });

        if(msg.contentType == ContentType::Text || msg.info.url.startsWith("http"))
            TcpLongConnection::getTcpClient().sendMessageTo(this->item->getConversationID(), msg.content, msg.tempMsgID, msg.contentType);
        else
        {
            ChatTextEdit::MessageBlock block;
            block.type = msg.contentType;
            block.tempID = msg.tempMsgID;
            block.content = msg.content;

            this->edit_message->getAllBlocks().append(block);
            handleNextBlock();
        }
        this->listView_messages->scrollToBottom();
    });

    connect(this->delegate, &ConversationDelegate::ReloadImageClicked, this, [this](const QString& msgID){
        this->item->reloadImage(msgID);
    });

    connect(this->delegate, &ConversationDelegate::previewImageClicked, this, [this](const QString& url){
        QPixmap pix = ImageCacheManager::getManager().fastLoadImage(url, 0, 0, QSize());
        if(!pix.isNull())
        {
            ImagePreviewWidget::getPreviewWidget().setPixmapUrl(url);
            ImagePreviewWidget::getPreviewWidget().setPixmap(pix);
            ImagePreviewWidget::getPreviewWidget().show();
            return;
        }
        else
        {
            ImageCacheManager::getManager().loadImage(url, [this, url](const QPixmap& pix){
                if(pix.isNull())
                    return;
                ImagePreviewWidget::getPreviewWidget().setPixmapUrl(url);
                ImagePreviewWidget::getPreviewWidget().setPixmap(pix);
                ImagePreviewWidget::getPreviewWidget().show();
            }, false, 0, 0, QSize());
        }
    });

    connect(this->delegate, &ConversationDelegate::previewVideoClicked, this, [this](const QString& url, const qint64& videoSize){
        VideoPreviewWidget::getPreviewWidget().setVideoUrl(url, videoSize);
        VideoPreviewWidget::getPreviewWidget().show();
    });

    connect(this->delegate, &ConversationDelegate::CancelUploadClicked, this, [this](const QString& tempMsgID){
        int index = this->item->getMessagesManager().indexOfMsg(tempMsgID);
        if(index < 0)
            return;

        const Message& msg = this->item->getMessagesManager().getMessages().at(index);
        QString filePath = msg.info.url;

        if(this->item->cancelUpload(tempMsgID))
            HttpShortConnection::getHttpClient().cancelUpload(filePath);
    });

    connect(this->delegate, &ConversationDelegate::fileDownloadClicked, this, [this](const QString& msgID){
        int index = this->item->getMessagesManager().indexOfMsg(msgID);
        if(index < 0)
            return;
        const Message msg = this->item->getMessagesManager().getMessages().at(index);

        if(msg.info.url.isEmpty() || !msg.info.url.startsWith("http"))
            return;

        if(msg.info.name.isEmpty())
            return;

        QString dir = GlobalVariable::getPosOfDownloadFile();
        QDir().mkpath(dir);
        QString target = QDir(dir).filePath(msg.info.name);

        bool isExists = GlobalVariable::isDownloadFileExists(msg.info.name);
        this->item->getMessagesManager().setDownloadExists(msg.info.url, isExists);
        if(isExists)
        {
            QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(target)});
            return;
        }
        else
        {
            target = HttpShortConnection::getHttpClient().resolveTargetPath(target);
            bool needDownload = !this->item->getMessagesManager().isDownloading(msg.info.url);
            this->item->getMessagesManager().registerDownload(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID, msg.info.url);
            if(needDownload)
                HttpShortConnection::getHttpClient().downloadFile(msg.info.url, target);
        }
    });

    connect(this->delegate, &ConversationDelegate::cancelDownloadClicked, this, [this](const QString& msgID){
        int index = this->item->getMessagesManager().indexOfMsg(msgID);
        if(index < 0)
            return;

        const Message& msg = this->item->getMessagesManager().getMessages().at(index);
        QString url = msg.info.url;

        if(this->item->getMessagesManager().cancelDownload(msgID))
            HttpShortConnection::getHttpClient().cancelDownloadFile(url);
    });

    connect(this->item, &ConversationItem::finishLoadedImage, this, [this](){
        this->loadingCount--;
    });

    connect(this->item, &ConversationItem::startLoadingImage, this, [this](){
        this->loadingCount++;
        QTimer::singleShot(0, this, [this](){
            if(this->loadingCount > 0 && !this->timer_loading->isActive())
                this->timer_loading->start(30);
        });
    });

    connect(this->item, &ConversationItem::loadMessagesFinished, this, [this](MessagesManager::LoadDir dir){
        if(dir == MessagesManager::LoadDir::Older)
        {
            this->topMsgID.clear();
            this->isLoadingOlder = false;
        }
        else if(dir == MessagesManager::LoadDir::Newer)
            this->isLoadingNewer = false;
    });

    connect(this->item, &ConversationItem::messagesInserted, this, [this](){
        bool isFirstLoad = !this->isFirstLoaded;
        if(isFirstLoad)
            this->isFirstLoaded = true;

        if(this->isWaitingJump)
            return;

        if(isFirstLoad)
        {
            this->listView_messages->scrollToBottom();
            return;
        }

        if(this->topMsgID.isEmpty())
            return;

        int newRow = this->item->getMessagesManager().indexOfMsg(this->topMsgID);
        this->topMsgID.clear();

        if(newRow < 0)
            return;

        this->isAdjustNow = true;
        this->listView_messages->scrollTo(this->model->index(newRow, 0), QAbstractItemView::PositionAtTop);
        this->isAdjustNow = false;
    });

    connect(this->item, &ConversationItem::jumpToFinished, this, [this](int row, const QString& msgID){
        this->isWaitingJump = false;
        if(row < 0 || row >= this->model->rowCount())
            return;

        this->listView_messages->scrollTo(this->model->index(row, 0), QAbstractItemView::PositionAtCenter);
        this->item->getMessagesManager().setHighLightMsgID(msgID);
    });

    connect(this->item, &ConversationItem::jumpToFailed, this, [this](const QString& msgID){
        this->isWaitingJump = false;
        ToastManager::getToastManager(this).error("跳转失败: 找不到该消息");
    });

    connect(this->listView_messages->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value){
        if(isAdjustNow)
            return;

        QModelIndex centerIndex = this->listView_messages->indexAt(this->listView_messages->viewport()->rect().center());
        if(!centerIndex.isValid())
            return;

        int centerRow = centerIndex.row();
        qint64 centerConvSeq = centerIndex.data(MessageModel::ConvSeqRole).toLongLong();

        int rangeIndex = this->item->getMessagesManager().findRangeIndexByconvSeq(centerConvSeq);
        if(rangeIndex < 0)
            return;

        const QList<MessagesManager::Range>& ranges = this->item->getMessagesManager().getRanges();

        int row_first = this->item->getMessagesManager().indexOfConvSeqOrAfter(ranges[rangeIndex].first);
        int row_last = this->item->getMessagesManager().indexOfConvSeqOrBefore(ranges[rangeIndex].last);
        if(row_first < 0 || row_last < 0)
            return;

        int rowsToFirst = centerRow - row_first;
        int rowsToLast = row_last - centerRow;

        if(rowsToFirst <= 6 && !this->isLoadingOlder)
        {
            QModelIndex topIndex = this->listView_messages->indexAt(QPoint(1, 1));
            if(topIndex.isValid())
                this->topMsgID = topIndex.data(MessageModel::MessageIDRole).toString();

            this->isLoadingOlder = true;
            this->item->getMessagesManager().loadMessages(MessagesManager::LoadDir::Older, ranges[rangeIndex].first, 20);
        }

        if(rowsToLast <= 6 && !this->isLoadingNewer)
        {
            this->isLoadingNewer = true;
            this->item->getMessagesManager().loadMessages(MessagesManager::LoadDir::Newer, ranges[rangeIndex].last, 20);
        }
    });

    installEventFilter(this);
    this->listView_messages->installEventFilter(this);
    this->listView_messages->viewport()->installEventFilter(this);
    this->edit_message->installEventFilter(this);
    this->widget_header->installEventFilter(this);
    this->widget_editRegion->installEventFilter(this);
    this->label_deletedTip->installEventFilter(this);

    this->item->getMessagesManager().loadMessages(MessagesManager::LoadDir::Older, INT64_MAX, 20);
}

void ConversationWidget::updateFriendUsername(const QString &username)
{
    QFontMetrics fm(this->label_name->font());
    int statusWidth = 0;

    if(this->label_statusIcon && this->label_statusText)
    {
        QFontMetrics fmStatus(this->label_statusText->font());
        statusWidth = 8 + 8 + fmStatus.horizontalAdvance(this->label_statusText->text()) + 8;
    }

    QString show_name = fm.elidedText(username, Qt::ElideRight, this->label_name->width() - statusWidth);

    this->label_name->setText(show_name);
    updateStatusIconPos();
    this->update();
}

void ConversationWidget::updateFriendStatus(bool isOnline)
{
    if(this->item->getConversationType() == ConversationItem::ConversationType::Group)
        return;

    QString status_text;
    if(isOnline)
    {
        this->label_statusIcon->setProperty("status", "online");
        status_text = "在线";
    }
    else
    {
        this->label_statusIcon->setProperty("status", "offline");
        status_text = "离线";
    }
    this->label_statusText->setText(status_text);
    updateStatusIconPos();

    this->label_statusIcon->style()->unpolish(this->label_statusIcon);
    this->label_statusIcon->style()->polish(this->label_statusIcon);
    this->update();
}

void ConversationWidget::setActive(bool isActive)
{
    this->item->setActive(isActive);
    if(isActive)
        this->item->clearUnRead();
}

void ConversationWidget::jumpToMessage(const QString &msgID, qint64 convSeq)
{
    this->isWaitingJump = true;
    this->item->getMessagesManager().jumpToMessage(msgID, convSeq);
}

void ConversationWidget::updateGroupInfo()
{
    if(this->item->getConversationType() != ConversationItem::ConversationType::Group)
        return;

    GroupManager::GroupInfo info = GroupManager::getGroupManage().getGroupInfo(this->item->getConversationID());
    if(info.conversationID.isEmpty())
        return;

    QFont font = this->label_name->font();
    QFontMetrics fm(font);

    int64_t memberCount = info.memberInfos.size();
    QString memberCountStr = "(" + QString::number(memberCount) + ")";
    int width_memberCount = fm.horizontalAdvance(memberCountStr);

    // 动态计算可用宽度
    int availableWidth = this->widget_header->width() - this->label_name->pos().x() - (this->widget_header->width() - this->btn_more->pos().x()) - width_memberCount - 16;

    QString show_name = fm.elidedText(info.groupName, Qt::ElideRight, availableWidth);
    show_name += memberCountStr;

    this->label_name->setText(show_name);
    this->label_name->adjustSize();
}

bool ConversationWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        this->item->clearUnRead();
    }
    return QWidget::eventFilter(obj, event);
}

void ConversationWidget::showEvent(QShowEvent *event)
{
    if(!this->isFirstTimeOpen)
    {
        this->isFirstTimeOpen = true;
        this->listView_messages->scrollToBottom();
    }
}

void ConversationWidget::initStyle()
{
    this->setStyleSheet(R"(
                            #widget_conversation
                            {
                                background-color: #ffffff;
                                border-left: none;
                            }
                            #widget_header
                            {
                                background-color: rgba(252, 255, 255, 255);
                                border-bottom: 1px solid #e0e0e0;
                            }
                            #label_name
                            {
                                color:rgba(31, 41, 55, 255);
                                font-family: "Microsoft YaHei";
                            }
                            #label_statusText
                            {
                                color: rgba(107, 114, 128, 255);
                            }
                            #label_statusIcon[status="online"]
                            {
                                background-color: #66FF00;
                                border-radius: 4px;
                                border: none;
                            }
                            #label_statusIcon[status="offline"]
                            {
                                background-color: rgba(209, 213, 219, 255);
                                border-radius: 4px;
                                border: none;
                            }
                            #btn_more
                            {
                                border:none;
                                background:transparent;
                                border-radius:16px;
                            }
                            #btn_more:hover
                            {
                                background-color: rgba(243, 244, 246, 255);
                            }
                            #btn_more:pressed
                            {
                                background-color: rgba(229, 231, 235, 255);
                            }
                            #listView_messages
                            {
                                background-color: #f9f9f9;
                                border-bottom: 1px solid #e0e0e0;
                            }
                                #listView_messages QScrollBar:vertical
                                {
                                    width: 4px;
                                    background: transparent;
                                    margin: 0px;
                                    margin-top: 2px;
                                }
                                #listView_messages QScrollBar::handle:vertical
                                {
                                    background: rgba(0, 0, 0, 0.2);
                                    border-radius: 2px;
                                    min-height: 30px;
                                }
                                #listView_messages QScrollBar::handle:vertical:hover
                                {
                                    background: rgba(0, 0, 0, 0.35);
                                }
                                #listView_messages QScrollBar::add-line:vertical,
                                #listView_messages QScrollBar::sub-line:vertical
                                {
                                    height: 0px;
                                }
                                #listView_messages QScrollBar::add-page:vertical,
                                #listView_messages QScrollBar::sub-page:vertical
                                {
                                    background: none;
                                }
                            #widget_editRegion[deleted="false"]
                            {
                                background-color: #ffffff;
                                border: 1px solid rgba(99, 102, 241, 255);
                                border-radius: 8px;
                            }
                            #widget_editRegion[deleted="true"]
                            {
                                background-color: #ffffff;
                                border: 1px solid rgba(229, 231, 235, 255);
                                border-radius: 8px;
                            }
                            #edit_message QScrollBar:vertical
                            {
                                width:6px;
                                margin:0px;
                                background:transparent;
                            }
                            #edit_message QScrollBar::sub-line:vertical,
                            #edit_message QScrollBar::add-line:vertical
                            {
                                width:0px;
                                height:0px;
                            }
                            #edit_message QScrollBar::add-page:vertical,
                            #edit_message QScrollBar::sub-page:vertical
                            {
                                background:none;
                            }
                            #edit_message QScrollBar::handle:vertical
                            {
                                background: rgba(209, 213, 219, 255);
                                border-radius: 3px;
                                min-height: 30px;
                            }
                            #edit_message QScrollBar::handle:vertical:hover
                            {
                                background: rgba(156, 163, 175, 255);
                            }
                            #edit_message
                            {
                                border:none;
                                outline:none;
                                border-bottom: 1px solid rgba(229, 231, 235, 255);
                                font-size: 13px;
                                font-weight:bold;
                                color: rgba(31, 41, 55, 255);
                                background: transparent;
                                padding: 8px 6px;
                            }
                            #edit_message:focus
                            {
                                border:none;
                                outline:none;
                                border-bottom: 1px solid rgba(229, 231, 235, 255);
                            }
                            #btn_file
                            {
                                background-color: transparent;
                                border: none;
                                border-radius: 6px;
                            }
                            #btn_file:hover
                            {
                                background-color: rgba(243, 244, 246, 255);
                            }
                            #btn_file:pressed
                            {
                                background-color: rgba(229, 231, 235, 255);
                            }
                            #btn_send
                            {
                                background-color: rgba(99, 102, 241, 255);
                                border: none;
                                border-radius: 6px;
                            }
                            #btn_send:hover
                            {
                                background-color: rgba(79, 70, 229, 255);
                            }
                            #btn_send:pressed
                            {
                                background-color: rgba(67, 56, 202, 255);
                            }
                            #label_deletedTip
                            {
                                color: rgba(107, 114, 128, 255);
                                font-size: 13px;
                                font-weight: 600;
                                background: transparent;
                            }
                                )");
}

void ConversationWidget::startReFlashTimeStamp()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime nextTime = now.date().addDays(1).startOfDay();
    qint64 msecsToTime = now.msecsTo(nextTime);
    QTimer::singleShot(msecsToTime, this, [this](){
        //更新视图上的时间（00：00更新）
        this->listView_messages->viewport()->update();
        startReFlashTimeStamp();
    });
}

void ConversationWidget::updateStatusIconPos()
{
    if(!this->label_statusIcon || !this->label_statusText)
        return;

    QFontMetrics fm(this->label_name->font());
    const QString text = this->label_name->text();

    int textWidth = qMin(fm.horizontalAdvance(text), this->label_name->width());
    int textEndX = this->label_name->pos().x() + textWidth;

    this->label_statusIcon->move(textEndX + 8, (this->widget_header->height() - this->label_statusIcon->height()) / 2);
    this->label_statusText->move(this->label_statusIcon->pos().x() + this->label_statusIcon->width() + 8,
                                 (this->widget_header->height() - this->label_statusText->height()) / 2);
}

void ConversationWidget::updateDeletedState(bool isDeleted)
{
    this->edit_message->setVisible(!isDeleted);
    this->btn_file->setVisible(!isDeleted);
    this->btn_send->setVisible(!isDeleted);
    this->label_deletedTip->setVisible(isDeleted);

    if(isDeleted)
    {
        if(this->item->getConversationType() == ConversationItem::ConversationType::Group)
            this->label_deletedTip->setText("您已不在该群聊中，无法发送消息");
        else
            this->label_deletedTip->setText("对方已不是您的好友，无法发送消息");
    }

    this->widget_editRegion->setProperty("deleted", isDeleted);
    this->widget_editRegion->style()->unpolish(this->widget_editRegion);
    this->widget_editRegion->style()->polish(this->widget_editRegion);
}

void ConversationWidget::handleNextBlock()
{
    ChatTextEdit::MessageBlock block;
    bool isVaild = false;
    while(this->edit_message->hasBlocks())
    {
        block = this->edit_message->nextBlock();
        if(block.content.trimmed().isEmpty() || block.tempID.isEmpty())
            continue;
        else
        {
            isVaild = true;
            break;
        }
    }

    if(!isVaild)
        return;

    const QString content = block.content.trimmed();

    if(block.type == ContentType::Text)
    {
        TcpLongConnection::getTcpClient().sendMessageTo(this->item->getConversationID(), content, block.tempID, ContentType::Text);
        handleNextBlock();
    }
    else if(block.type == ContentType::Image)
    {
        QString url = block.content;
        QString contentJson = block.content;

        if(block.content.startsWith("{"))
            url = QJsonDocument::fromJson(block.content.toUtf8()).object()["Url"].toString();
        else
        {
            QJsonObject obj;
            obj["Url"] = url;
            contentJson = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        }
        if(url.startsWith("http"))
        {
            TcpLongConnection::getTcpClient().sendMessageTo(this->item->getConversationID(), contentJson, block.tempID, ContentType::Image);
            handleNextBlock();
            return;
        }

        QString filePath = ImageCacheManager::getManager().getCacheFilePath(url);
        if(filePath.isEmpty())
        {
            this->item->updateMessageStatus(false, block.tempID, QString(), -1, -1);
            handleNextBlock();
            return;
        }

        QImage img(filePath);

        HttpShortConnection::getHttpClient().uploadMedia(HttpShortConnection::MediaType::Image, filePath, [this, block, filePath](const HttpShortConnection::UploadResult& result){
            QImageReader reader(filePath);
            QSize size = reader.size();
            int width = size.width();
            int height = size.height();

            QJsonObject obj;
            obj["Url"] = result.url;
            obj["Width"] = QString::number(width);
            obj["Height"] = QString::number(height);
            obj["ThumbnailUrl"] = result.thumbnailUrl;
            QString newJson = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));

            if(!result.thumbnailUrl.isEmpty())
                ImageCacheManager::getManager().migrateThumbnail(block.thumbnailUrl, result.thumbnailUrl);

            this->item->updateMessageContent(block.tempID, newJson);
            TcpLongConnection::getTcpClient().sendMessageTo(this->item->getConversationID(), newJson, block.tempID, ContentType::Image);
            handleNextBlock();
        }, true, [this, block](const QString& info){
            this->item->updateMessageStatus(false, block.tempID, QString(), -1, -1);
            handleNextBlock();
        });
    }
    else if(block.type == Video)
    {
        QString url = block.content;
        QString contentJson = block.content;
        if(block.content.startsWith("{"))
            url = QJsonDocument::fromJson(block.content.toUtf8()).object()["Url"].toString();
        else
        {
            QJsonObject obj;
            obj["Url"] = url;
            contentJson = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        }

        if(url.startsWith("http"))
        {
            TcpLongConnection::getTcpClient().sendMessageTo(this->item->getConversationID(), contentJson, block.tempID, ContentType::Video);
            handleNextBlock();
            return;
        }

        QString filePath = url;
        if(!QFile::exists(filePath))
        {
            this->item->updateMessageStatus(false, block.tempID, QString(), -1, -1);
            handleNextBlock();
            return;
        }

        this->item->getMessagesManager().registerUpload(block.tempID, filePath);
        VideoUtils::extractAsync(filePath, [this, block, filePath, contentJson](const VideoUtils::VideoInfo& info){
            int index = this->item->getMessagesManager().indexOfMsg(block.tempID);
            if(index < 0)
                return;
            if(this->item->getMessagesManager().getMessages().at(index).status != Sending)
            {
                this->item->getMessagesManager().unregisterUpload(block.tempID);
                handleNextBlock();
                return;
            }

            if(!info.valid)
            {
                this->item->getMessagesManager().unregisterUpload(block.tempID);
                this->item->updateMessageStatus(false, block.tempID, QString(), -1, -1);
                handleNextBlock();
                return;
            }

            QString tempUrl = "local://" + QUuid::createUuid().toString();
            ImageCacheManager::getManager().insertThumbnail(tempUrl, QPixmap::fromImage(info.thumbnail));

            QJsonObject obj = QJsonDocument::fromJson(contentJson.toUtf8()).object();
            obj["Url"] = filePath;
            obj["Width"] = QString::number(info.width);
            obj["Height"] = QString::number(info.height);
            obj["ThumbnailUrl"] = tempUrl;
            obj["Duration"] = QString::number(info.duration);
            obj["Size"] = QString::number(QFileInfo(filePath).size());

            QString qJson = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
            this->item->updateMessageContent(block.tempID, qJson);

            //上传视频
            HttpShortConnection::getHttpClient().uploadMedia(HttpShortConnection::MediaType::Video, filePath, [=](const HttpShortConnection::UploadResult& result){
                int index = this->item->getMessagesManager().indexOfMsg(block.tempID);
                if(index < 0)
                    return;
                if(this->item->getMessagesManager().getMessages().at(index).status != Sending)
                {
                    this->item->getMessagesManager().unregisterUpload(block.tempID);
                    handleNextBlock();
                    return;
                }

                this->item->getMessagesManager().unregisterUpload(block.tempID);
                QJsonObject newObj = obj;
                newObj["Url"] = result.url;

                QString thumFilePath = ImageCacheManager::getManager().getCacheFilePath(tempUrl);
                if(thumFilePath.isEmpty())
                {
                    QString contentJson = QString::fromUtf8(QJsonDocument(newObj).toJson(QJsonDocument::Compact));

                    this->item->updateMessageContent(block.tempID, contentJson);
                    TcpLongConnection::getTcpClient().sendMessageTo(this->item->getConversationID(), contentJson, block.tempID, block.type);
                    handleNextBlock();
                    return;
                }

                //上传缩略图
                HttpShortConnection::getHttpClient().uploadMedia(HttpShortConnection::MediaType::Image, thumFilePath, [=](const HttpShortConnection::UploadResult& result){
                    int index = this->item->getMessagesManager().indexOfMsg(block.tempID);
                    if(index < 0)
                        return;
                    if(this->item->getMessagesManager().getMessages().at(index).status != Sending)
                    {
                        handleNextBlock();
                        return;
                    }

                    QJsonObject newObj1 = newObj;
                    ImageCacheManager::getManager().migrateThumbnail(tempUrl, result.url);

                    newObj1["ThumbnailUrl"] = result.url;
                    QString contentJson = QString::fromUtf8(QJsonDocument(newObj1).toJson(QJsonDocument::Compact));

                    this->item->updateMessageContent(block.tempID, contentJson);
                    TcpLongConnection::getTcpClient().sendMessageTo(this->item->getConversationID(), contentJson, block.tempID, block.type);
                    handleNextBlock();
                }, false, [=](const QString&){
                    int index = this->item->getMessagesManager().indexOfMsg(block.tempID);
                    if(index < 0)
                        return;
                    if(this->item->getMessagesManager().getMessages().at(index).status != Sending)
                    {
                        handleNextBlock();
                        return;
                    }

                    QJsonObject newObj1 = newObj;
                    newObj1["ThumbnailUrl"] = "";
                    QString contentJson = QString::fromUtf8(QJsonDocument(newObj1).toJson(QJsonDocument::Compact));

                    this->item->updateMessageContent(block.tempID, contentJson);
                    TcpLongConnection::getTcpClient().sendMessageTo(this->item->getConversationID(), contentJson, block.tempID, block.type);
                    handleNextBlock();
                }, false);
            }, true, [=](const QString&){
                int index = this->item->getMessagesManager().indexOfMsg(block.tempID);
                if(index < 0)
                    return;
                if(this->item->getMessagesManager().getMessages().at(index).status != Sending)
                {
                    handleNextBlock();
                    return;
                }

                this->item->getMessagesManager().unregisterUpload(block.tempID);
                this->item->updateMessageStatus(false, block.tempID, QString(), -1, -1);
                handleNextBlock();
            }, false);
        });
        handleNextBlock();
        return;
    }
    else if(block.type == File)
    {
        QString url = block.content;
        QString contentJson = block.content;
        if(block.content.startsWith("{"))
            url = QJsonDocument::fromJson(block.content.toUtf8()).object()["Url"].toString();
        else
        {
            QJsonObject obj;
            obj["Url"] = url;
            contentJson = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        }

        if(url.startsWith("http"))
        {
            TcpLongConnection::getTcpClient().sendMessageTo(this->item->getConversationID(), contentJson, block.tempID, ContentType::File);
            handleNextBlock();
            return;
        }

        QString filePath = url;
        if(!QFile::exists(filePath))
        {
            this->item->updateMessageStatus(false, block.tempID, QString(), -1, -1);
            handleNextBlock();
            return;
        }

        this->item->getMessagesManager().registerUpload(block.tempID, filePath);
        HttpShortConnection::getHttpClient().uploadMedia(HttpShortConnection::MediaType::File, filePath, [this, block, filePath, contentJson](const HttpShortConnection::UploadResult& result){
            QJsonObject obj = QJsonDocument::fromJson(contentJson.toUtf8()).object();
            obj["Url"] = result.url;
            obj["Name"] = QFileInfo(filePath).fileName();
            obj["Size"] = QString::number(QFileInfo(filePath).size());
            QString newJson = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));

            this->item->getMessagesManager().unregisterUpload(block.tempID);
            this->item->updateMessageContent(block.tempID, newJson);
            this->item->getMessagesManager().setDownloadExists(result.url, true);

            TcpLongConnection::getTcpClient().sendMessageTo(this->item->getConversationID(), newJson, block.tempID, block.type);

            QPointer<ConversationWidget> self(this);
            QString srcPath = filePath;
            QString uploadedUrl = result.url;
            auto future = QtConcurrent::run([self, srcPath, uploadedUrl](){
                QString dir = GlobalVariable::getPosOfDownloadFile();
                if(dir.isEmpty())
                    return;

                QDir().mkpath(dir);
                QString target = QDir(dir).filePath(QFileInfo(srcPath).fileName());

                if(!QFile::exists(target))
                    QFile::copy(srcPath, target);
            });

            handleNextBlock();
        }, false, [this, block, filePath](const QString& info){
            this->item->getMessagesManager().unregisterUpload(block.tempID);
            this->item->updateMessageStatus(false, block.tempID, QString(), -1, -1);
            handleNextBlock();
        }, false);
    }
}

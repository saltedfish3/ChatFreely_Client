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

    FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(item->getConversationID());
    QString username;
    bool isOnline = false;

    if(info.uid.isEmpty())
    {
        username = "未知用户";
    }
    else
    {
        username = info.username;
        isOnline = info.isOnline;
    }

    QString show_username;
    if(username.length() >= 30)
        show_username = username.left(24) + "...";
    else
        show_username = username;

    this->label_name = new QLabel(show_username, this->widget_header);
    this->label_name->setObjectName("label_name");
    this->label_name->setMaximumHeight(24);
    this->label_name->setMinimumHeight(24);
    QFont font = this->label_name->font();
    font.setPointSize(12);
    font.setWeight(QFont::Bold);
    this->label_name->setFont(font);
    this->label_name->adjustSize();
    this->label_name->move(16,(this->widget_header->height() - this->label_name->height())/2);

    //初始化 在线状态图标
    this->label_statusIcon = new QLabel(this->widget_header);
    this->label_statusIcon->setObjectName("label_statusIcon");
    this->label_statusIcon->setFixedSize(8, 8);
    this->label_statusIcon->adjustSize();
    this->label_statusIcon->move(this->label_name->pos().x() + this->label_name->width() + 8,
                       (this->widget_header->height() - this->label_statusIcon->height())/2);
    if(isOnline)
        this->label_statusIcon->setProperty("status", "online");
    else
        this->label_statusIcon->setProperty("status", "offline");

    //初始化 在线状态文字
    QString status_text;
    if(isOnline)
        status_text = "在线";
    else
        status_text = "离线";
    this->label_statusText = new QLabel(status_text, this->widget_header);
    this->label_statusText->setObjectName("label_statusText");
    this->label_statusText->setMaximumHeight(20);
    this->label_statusText->setMinimumHeight(20);
    font = this->label_statusText->font();
    font.setPointSize(11);
    this->label_statusText->adjustSize();
    this->label_statusText->move(this->label_statusIcon->pos().x() + this->label_statusIcon->width() + 8,
                                 (this->widget_header->height() - this->label_statusText->height())/2);

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
    //initMoreMenu(this,more);

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
    this->listView_messages->resize(this->width(),(this->height() - this->widget_header->height()) * 0.75);
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

    //初始化 编辑框 区域
    this->widget_editRegion = new QWidget(this);
    this->widget_editRegion->setObjectName("widget_editRegion");
    this->widget_editRegion->resize(this->width() * 0.98,(this->height() - this->listView_messages->height() - this->widget_header->height()) * 0.9);
    int editRegion_mheight = this->height() - this->widget_header->height() - this->listView_messages->height();
    this->widget_editRegion->move((this->width() - this->widget_editRegion->width()) / 2,
                     this->listView_messages->pos().y() + this->listView_messages->height() + (editRegion_mheight - this->widget_editRegion->height()) / 2);

    //初始化 编辑框
    this->edit_message = new QPlainTextEdit(this->widget_editRegion);
    this->edit_message->setObjectName("edit_message");
    this->edit_message->setPlaceholderText("输入些什么...");
    this->edit_message->resize(this->widget_editRegion->width() * 0.98, this->widget_editRegion->height() * 0.6);
    this->edit_message->move((this->widget_editRegion->width() - this->edit_message->width()) / 2,
                             this->widget_editRegion->height() * 0.07);

    connect(this->edit_message,&QPlainTextEdit::textChanged,this,[=](){
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
        QString content = this->edit_message->toPlainText().trimmed();
        if(content.isEmpty())
            return;
        QString tempMsgID = QUuid::createUuid().toString();

        Message msg;
        msg.tempMsgID = tempMsgID;
        msg.content = content;
        msg.timeStamp = QDateTime::currentSecsSinceEpoch();
        msg.senderUID = UserInfo::getUserInfo().getUID();
        msg.status = Sending;
        msg.convSeq = this->item->getMessagesManager().getNextConvSeq();

        this->item->addNewMessage(msg);
        this->loadingCount++;
        QTimer::singleShot(0, this, [this](){
            if(this->loadingCount > 0 && !this->timer_loading->isActive())
                this->timer_loading->start(30);
        });

        TcpLongConnection::getTcpClient().sendMessageTo(this->item->getConversationID(), content, tempMsgID);
        this->edit_message->clear();
        this->listView_messages->scrollToBottom();
    });

    //初始化更多弹窗
    this->widget_menu = new MoreWidget(this);

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
        else if(this->isVisible())
        {
            if(msg.senderUID != UserInfo::getUserInfo().getUID())
                this->item->addUnReadCount();
        }
    });

    connect(this->item, &ConversationItem::messageStatusChange, this, [this](){
        this->listView_messages->doItemsLayout();
        if(this->loadingCount > 0)
        {
            this->loadingCount--;
        }
    });

    connect(this->delegate, &ConversationDelegate::ReSendClicked, this, [this](const QString& tempMsgID){
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

        TcpLongConnection::getTcpClient().sendMessageTo(this->item->getConversationID(), msg.content, msg.tempMsgID);
        this->listView_messages->scrollToBottom();
    });

    connect(this->listView_messages->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value){
        if(value <= (this->listView_messages->verticalScrollBar()->minimum() + this->listView_messages->height()*0.2))
        {
            Message msg = this->item->getMessagesManager().getFrontMessage();
            if(!msg.senderUID.isEmpty())
            {
                if(msg.convSeq > 1)
                    this->item->loadHistoryMessages();
            }
        }
    });

    connect(this->item, &ConversationItem::firstLoadingMessages, this, [this](){
        this->listView_messages->scrollToBottom();
    });

    installEventFilter(this);
    this->listView_messages->installEventFilter(this);
    this->listView_messages->viewport()->installEventFilter(this);
    this->edit_message->installEventFilter(this);
    this->widget_header->installEventFilter(this);
    this->widget_editRegion->installEventFilter(this);

    this->item->loadHistoryMessages();
}

void ConversationWidget::updateFriendUsername(const QString &username)
{
    this->label_name->setText(username);
    this->update();
}

void ConversationWidget::updateFriendStatus(bool isOnline)
{
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

bool ConversationWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        this->item->clearUnRead();
    }
    return QWidget::eventFilter(obj, event);
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
                            #widget_editRegion
                            {
                                background-color: #ffffff;
                                border: 1px solid rgba(99, 102, 241, 255);
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
                                padding: 0px;
                            }
                            #edit_message:focus
                            {
                                border:none;
                                outline:none;
                                border-bottom: 1px solid rgba(229, 231, 235, 255);
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

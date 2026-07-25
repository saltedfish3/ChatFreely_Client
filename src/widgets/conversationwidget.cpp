#include "conversationwidget.h"

ConversationWidget::ConversationWidget(int width, int height, const QString &username, bool isOnline, const QString &uid, const QPixmap &avatar, QWidget *parent)
    : QWidget{parent}, uid(uid), avatar(avatar)
{
    this->setObjectName("widget_conversation");
    this->resize(width, height);

    this->widget_header = new QWidget(this);
    this->widget_header->setObjectName("widget_header");
    this->widget_header->resize(this->width(), 56);
    this->widget_header->move(0,0);

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
        if(loadingCount != 0)
            this->timer_loading->start(30);
        else
            this->timer_loading->stop();
    });

    //初始化 消息显示 部分
    this->listView_messages = new QListView(this);
    this->listView_messages->setObjectName("listView_messages");
    this->listView_messages->resize(this->width(),(this->height() - this->widget_header->height()) * 0.75);
    this->listView_messages->move(0, this->widget_header->height());

    this->model = new QStandardItemModel(this);
    this->listView_messages->setModel(this->model);

    this->delegate = new ConversationDelegate(&this->loadingAngle, this);
    this->listView_messages->setItemDelegate(this->delegate);

    this->listView_messages->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->listView_messages->setSelectionMode(QAbstractItemView::NoSelection);
    this->listView_messages->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->listView_messages->verticalScrollBar()->setSingleStep(8);
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

        addMessageItem(true, content, QDateTime::currentSecsSinceEpoch(), tempMsgID);

        this->listView_messages->scrollToBottom();
        this->loadingCount++;
        if(!this->timer_loading->isActive())
            this->timer_loading->start(30);
        // TcpLongConnection::getTcpClient().sendMessageTo(this->uid, content);
        this->edit_message->clear();
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
}

void ConversationWidget::addMessageItem(bool isMyself, const QString &content, int64_t timestamp, QString tempMsgID)
{
    QDateTime msgTime = QDateTime::fromSecsSinceEpoch(timestamp);
    int64_t lastTimeStamp = 0;
    for(int i = this->model->rowCount() - 1; i >= 0; i--)
    {
        QModelIndex index = this->model->index(i, 0);
        QVariant vi_isTimeStamp = index.data(ConversationDelegate::IsTimestampRole);
        QVariant vi_isBottomSpace = index.data(ConversationDelegate::IsBottomSpaceRole);
        bool isTimeStamp = vi_isTimeStamp.isValid() ? vi_isTimeStamp.toBool() : false;
        bool isBottomSpace = vi_isBottomSpace.isValid() ? vi_isBottomSpace.toBool() : false;
        if(isTimeStamp || isBottomSpace)
        {
            if(isBottomSpace)
                this->model->removeRow(i);
            continue;
        }
        lastTimeStamp = index.data(ConversationDelegate::TimeStamp).toLongLong();
        break;
    }
    if(lastTimeStamp == 0 || (timestamp - lastTimeStamp) > 300)
    {
        //插入时间戳
        QDateTime now = QDateTime::currentDateTime();
        QString timeStr;
        //非本年
        if(msgTime.date().year() != now.date().year())
        {
            timeStr = msgTime.toString("yyyy年M月d日 hh:mm");
        }
        else if(msgTime.date() == now.date())
        {
            //今天
            timeStr = msgTime.toString("hh:mm");
        }
        else if(msgTime.date() == now.date().addDays(-1))
        {
            //昨天
            timeStr = QString("昨天 %1").arg(msgTime.toString("hh:mm"));
        }
        else if(msgTime.date() == now.date().addDays(-2))
        {
            //前天
            timeStr = QString("前天 %1").arg(msgTime.toString("hh:mm"));
        }
        else
        {
            int mondayOfTime = now.date().dayOfWeek() - 1;
            QDate thisMonday = now.date().addDays(-mondayOfTime);
            QDate thisSunday = thisMonday.addDays(6);
            if(msgTime.date() >= thisMonday && msgTime.date() <= thisSunday)
            {
                QStringList weekDays = {"","星期一","星期二","星期三","星期四","星期五","星期六","星期日"};
                timeStr = QString("%1 %2").arg(weekDays.at(msgTime.date().dayOfWeek()), "hh:mm");
            }
            else
            {
                timeStr = msgTime.toString("M月d日 hh:mm");
            }
        }
        QStandardItem* item = new QStandardItem();
        item->setData(true, ConversationDelegate::IsTimestampRole);
        item->setData(timeStr, ConversationDelegate::ContentRole);
        this->model->appendRow(item);
    }
    QStandardItem* item = new QStandardItem();
    item->setData(isMyself, ConversationDelegate::IsMyselfRole);
    item->setData(content, ConversationDelegate::ContentRole);
    item->setData(ConversationDelegate::Sending, ConversationDelegate::MessageStatusRole);
    item->setData(tempMsgID, ConversationDelegate::TempMsgIDRole);
    item->setData(timestamp, ConversationDelegate::TimeStamp);
    this->model->appendRow(item);

    QStandardItem* item_bottomSpace = new QStandardItem();
    item_bottomSpace->setData(true, ConversationDelegate::IsBottomSpaceRole);
    this->model->appendRow(item_bottomSpace);
}

QString ConversationWidget::getLastMessage()
{
    for(int i = this->model->rowCount() - 1; i >= 0; i--)
    {
        QModelIndex index = this->model->index(i, 0);
        QVariant vi_isTimeStamp = index.data(ConversationDelegate::IsTimestampRole);
        QVariant vi_isBottomSpace = index.data(ConversationDelegate::IsBottomSpaceRole);
        bool isTimeStamp = vi_isTimeStamp.isValid() ? vi_isTimeStamp.toBool() : false;
        bool isBottomSpace = vi_isBottomSpace.isValid() ? vi_isBottomSpace.toBool() : false;
        if(isTimeStamp || isBottomSpace)
            continue;
        return index.data(ConversationDelegate::ContentRole).toString();
    }
    return QString();
}

QString ConversationWidget::getLastMessageTime()
{
    for(int i = this->model->rowCount() - 1; i >= 0; i--)
    {
        QModelIndex index = this->model->index(i, 0);
        QVariant vi_isTimeStamp = index.data(ConversationDelegate::IsTimestampRole);
        QVariant vi_isBottomSpace = index.data(ConversationDelegate::IsBottomSpaceRole);
        bool isTimeStamp = vi_isTimeStamp.isValid() ? vi_isTimeStamp.toBool() : false;
        bool isBottomSpace = vi_isBottomSpace.isValid() ? vi_isBottomSpace.toBool() : false;
        if(isBottomSpace)
            continue;
        if(isTimeStamp)
            return index.data(ConversationDelegate::ContentRole).toString();
    }
    return QString();
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

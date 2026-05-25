#include "chatwidget.h"

ChatWidget::ChatWidget(int width, int height,QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);
    this->setObjectName("this");

    initSearchWidget();
    initSearchStyle();
    initListWidget();
    initListStyle();
    initStackedConversation();
}

void ChatWidget::initSearchWidget()
{
    this->widget_search = new QWidget(this);
    this->widget_search->setObjectName("widget_search");
    this->widget_search->setAttribute(Qt::WA_StyledBackground,true);
    this->widget_search->setGeometry(0,0,260,64);

    this->edit_search = new QLineEdit(this->widget_search);
    this->edit_search->setObjectName("edit_search");
    this->edit_search->setPlaceholderText("搜索对话...");
    this->edit_search->setMaxLength(50);
    searchIcon = new QAction(QIcon(":/default/images/search.png"),"",this->edit_search);
    this->edit_search->addAction(searchIcon,QLineEdit::LeadingPosition);
    this->edit_search->resize(228,36);
    this->edit_search->move((this->widget_search->width() - this->edit_search->width()) / 2,(this->widget_search->height() - this->edit_search->height()) / 2);
}

void ChatWidget::initSearchStyle()
{
    this->setStyleSheet(R"(
                                    #widget_search
                                    {
                                        background: #ffffff;
                                        border-radius: 0;
                                        border-right: 1px solid #e0e0e0;
                                        border-bottom: 1px solid #e0e0e0;
                                    }
                                    #edit_search
                                    {
                                        background-color: #efecf8;
                                        border: 1px solid #f3f4f6;
                                        border-radius: 8px;
                                        padding-left: 0px;
                                        padding-right: 15px;
                                        color: #1b1b23;
                                        font-size: 12px;
                                        font-weight:bold;
                                        font-family: "Microsoft YaHei";
                                    }
                                    #edit_search:focus
                                    {
                                        background-color: rgba(255, 255, 255, 255);
                                        border: 1px solid rgba(99, 102, 241, 255);
                                    }
                                    )");
}

void ChatWidget::initListWidget()
{
    //userList
    this->listWidget_userList = new QListWidget(this);
    this->listWidget_userList->setObjectName("listWidget_userList");
    this->listWidget_userList->setGeometry(0, this->widget_search->height(), this->widget_search->width(), this->height());
}

void ChatWidget::initListStyle()
{
    this->listWidget_userList->setStyleSheet(R"(
                                            #listWidget_userList
                                            {
                                                background: #ffffff;
                                                border-radius: 0;
                                                border-right: 1px solid #e0e0e0;
                                            }
                                            )");
}

void ChatWidget::addConversation()
{
    QWidget* conversation = new QWidget(this->stackedWidget_Conversation);
    conversation->setObjectName("conversation");
    conversation->resize(this->stackedWidget_Conversation->width(), this->stackedWidget_Conversation->height());
    conversation->move(0,0);

    //初始化 头部 信息栏
    QWidget* header = new QWidget(conversation);
    header->setObjectName("header");
    header->resize(conversation->width(), 56);
    header->move(0,0);

    //初始化 对方用户名
    QLabel* name = new QLabel("Sarah Miller",header);
    name->setObjectName("name");
    name->setMaximumHeight(24);
    name->setMinimumHeight(24);
    QFont font = name->font();
    font.setPointSize(12);
    font.setWeight(QFont::Bold);
    name->setFont(font);
    name->adjustSize();
    name->move(16,(header->height() - name->height())/2);

    //初始化 在线状态图标
    QLabel* status_color = new QLabel(header);
    status_color->setObjectName("status_color");
    status_color->setFixedSize(8, 8);
    status_color->adjustSize();
    status_color->move(name->pos().x() + name->width() + 8,(header->height() - status_color->height())/2);

    //初始化 在线状态文字
    QLabel* status = new QLabel("在线",header);
    status->setObjectName("status");
    status->setMaximumHeight(20);
    status->setMinimumHeight(20);
    font = status->font();
    font.setPointSize(12);
    status->adjustSize();
    status->move(status_color->pos().x() + status_color->width() + 8,(header->height() - status->height())/2);

    //初始化 更多 按钮
    QPushButton* more = new QPushButton(header);
    more->setObjectName("more");
    more->resize(32,32);
    more->move(header->width() - more->width() - 16, (header->height() - more->height())/2);
    more->setIcon(QIcon(":/default/images/more.png"));
    more->setIconSize(QSize(16,16));
    more->setCheckable(true);
    more->setChecked(false);
    initMoreMenu(conversation,more);

    //初始化 消息显示 部分
    QListView* messages = new QListView(conversation);
    messages->setObjectName("messages");
    messages->resize(conversation->width(),(conversation->height() - header->height()) * 0.75);
    messages->move(0, header->height());

    //初始化 编辑框 区域
    QWidget* editRegion = new QWidget(conversation);
    editRegion->setObjectName("editRegion");
    editRegion->resize(conversation->width() * 0.98,(conversation->height() - messages->height() - header->height()) * 0.9);
    int editRegion_mheight = conversation->height() - header->height() - messages->height();
    editRegion->move((conversation->width() - editRegion->width()) / 2, messages->pos().y() + messages->height() + (editRegion_mheight - editRegion->height()) / 2);

    //初始化 编辑框
    QPlainTextEdit* edit = new QPlainTextEdit(editRegion);
    edit->setObjectName("edit");
    edit->setPlaceholderText("输入些什么...");
    edit->resize(editRegion->width() * 0.98, editRegion->height() * 0.6);
    edit->move((editRegion->width() - edit->width()) / 2, editRegion->height() * 0.07);

    connect(edit,&QPlainTextEdit::textChanged,this,[=](){
        QString text = edit->toPlainText();
        if(text.length() > 10000)
        {
            //设置截断信号
            edit->blockSignals(true);
            int cursorPos = edit->textCursor().position();
            text.truncate(10000);
            edit->setPlainText(text);
            // 恢复光标位置（不超出文本长度）
            QTextCursor cursor = edit->textCursor();
            cursor.setPosition(qMin(cursorPos, text.length()));
            edit->setTextCursor(cursor);
            edit->blockSignals(false);
        }
    });

    //初始化 发送 按钮
    QPushButton* send = new QPushButton(editRegion);
    send->setObjectName("send");
    int sendSize = editRegion->height() - edit->pos().y() - edit->height();
    send->resize(sendSize*0.8,sendSize*0.8);
    send->setIcon(QIcon(":/default/images/send.png"));
    send->setIconSize(QSize(send->width()*0.4,send->height()*0.4));
    int send_h = edit->pos().y() + edit->height();
    send->move(editRegion->width() * 0.99 - send->width(), send_h + (editRegion->height() - send_h - send->height())/2);
    connect(send,&QPushButton::clicked,this,[edit](){
        if(edit->toPlainText().trimmed().length() == 0)
            qDebug()<<"请输入文本";
    });

    initConversationStyle(conversation);

    this->stackedWidget_Conversation->addWidget(conversation);
    this->stackedWidget_Conversation->setCurrentWidget(conversation);
}

void ChatWidget::initConversationStyle(QWidget* conversation)
{
    conversation->setStyleSheet(R"(
                            #conversation
                            {
                                background-color: #ffffff;
                                border-left: none;
                            }
                            #header
                            {
                                background-color: rgba(252, 255, 255, 255);
                                border-bottom: 1px solid #e0e0e0;
                            }
                            #name
                            {
                                color:rgba(31, 41, 55, 255);
                                font-family: "Microsoft YaHei";
                            }
                            #status
                            {
                                color: rgba(107, 114, 128, 255);
                            }
                            #status_color
                            {
                                background-color: #66FF00;
                                border-radius: 4px;
                                border: none;
                            }
                            #more
                            {
                                border:none;
                                background:transparent;
                                border-radius:16px;
                            }
                            #more:hover
                            {
                                background-color: rgba(243, 244, 246, 255);
                            }
                            #more:pressed
                            {
                                background-color: rgba(229, 231, 235, 255);
                            }
                            #messages
                            {
                                background-color: #f9f9f9;
                                border-bottom: 1px solid #e0e0e0;
                            }
                            #editRegion
                            {
                                background-color: #ffffff;
                                border: 1px solid rgba(99, 102, 241, 255);
                                border-radius: 8px;
                            }
                            #edit QScrollBar:vertical
                            {
                                width:6px;
                                margin:0px;
                                background:transparent;
                            }
                            #edit QScrollBar::sub-line:vertical,
                            #edit QScrollBar::add-line:vertical
                            {
                                width:0px;
                                height:0px;
                            }
                            #edit QScrollBar::add-page:vertical,
                            #edit QScrollBar::sub-page:vertical
                            {
                                background:none;
                            }
                            #edit QScrollBar::handle:vertical
                            {
                                background: rgba(209, 213, 219, 255);
                                border-radius: 3px;
                                min-height: 30px;
                            }
                            #edit QScrollBar::handle:vertical:hover
                            {
                                background: rgba(156, 163, 175, 255);
                            }
                            #edit
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
                            #edit:focus
                            {
                                border:none;
                                outline:none;
                                border-bottom: 1px solid rgba(229, 231, 235, 255);
                            }
                            #send
                            {
                                background-color: rgba(99, 102, 241, 255);
                                border: none;
                                border-radius: 6px;
                            }
                            #send:hover
                            {
                                background-color: rgba(79, 70, 229, 255);
                            }
                            #send:pressed
                            {
                                background-color: rgba(67, 56, 202, 255);
                            }
                                )");
}

void ChatWidget::initMoreMenu(QWidget* conversation,QPushButton* more)
{
    MoreWidget* menu = new MoreWidget(conversation);

    connect(more,&QPushButton::toggled,this,[=](bool checked){
        if(checked)
        {
            menu->move(more->mapToGlobal(QPoint(0 - menu->width() + more->width(),more->height())));
            menu->show();
            menu->activateWindow();
            menu->setFocus();
        }
        else
        {
            menu->hide();
        }
    });
    connect(menu,&MoreWidget::closed,more,[=](){
        if(more->underMouse())
            return;
        more->setChecked(false);
    });
}

void ChatWidget::initStackedConversation()
{
    this->stackedWidget_Conversation = new QStackedWidget(this);
    this->stackedWidget_Conversation->resize(this->width() - this->widget_search->width(),this->height());
    this->stackedWidget_Conversation->move(this->widget_search->width(), 0);

    addConversation();
}

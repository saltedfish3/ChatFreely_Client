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
    this->widget_card->resize(360,290);//220
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

    this->btn_add = new QPushButton("添加", this->widget_card);
    this->btn_add->setObjectName("btn_add");
    this->btn_add->resize(80, 36);
    this->btn_add->move(this->widget_card->width() - 24 - this->btn_add->width(),
                        this->edit_verMsg->pos().y() + this->edit_verMsg->height() + 18);//166

    this->btn_cancel = new QPushButton("取消", this->widget_card);
    this->btn_cancel->setObjectName("btn_cancel");
    this->btn_cancel->resize(70, 36);
    this->btn_cancel->move(this->btn_add->pos().x() - 12 - this->btn_cancel->width(), this->btn_add->pos().y());

    connect(this->btn_cancel, &QPushButton::clicked, this, [this](){
        this->hide();
        this->deleteLater();
    });

    connect(this->btn_add, &QPushButton::clicked, this, [this](){
        QString sid_email = this->edit_IDOrEmail->text().trimmed();
        QString verMsg = this->edit_verMsg->text().trimmed();
        if(sid_email == "")
        {
            ToastManager::getToastManager(true).error("待添加人信息不能为空", this);
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
            ToastManager::getToastManager(true).error("输入为非邮箱或者用户ID", this);
            return;
        }

        this->hide();
        this->deleteLater();
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
    )");
}

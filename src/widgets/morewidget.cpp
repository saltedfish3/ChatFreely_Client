#include "morewidget.h"

MoreWidget::MoreWidget(bool isFriend, bool isOwner, QWidget *parent)
    : QWidget{parent}
{
    this->setMaximumWidth(150);
    this->setMinimumWidth(150);
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setFocusPolicy(Qt::StrongFocus);
    initMore(isFriend, isOwner);
    initStyle();
}

void MoreWidget::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    this->hide();
    emit closed();
}

void MoreWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(QRectF(this->rect()), 8, 8);
    painter.setBrush(QColor(255, 255, 255, 255));
    painter.setPen(QPen(QColor(229, 231, 235, 255), 1));
    painter.drawPath(path);
}

void MoreWidget::initMore(bool isFriend, bool isOwner)
{
    QString showText = isFriend ? "用户" : "群聊";
    QPushButton* show_user_info = new QPushButton("查看" + showText + "信息",this);
    show_user_info->resize(this->width() - 2,30);
    show_user_info->move(1,10);

    QLabel* label_showInfo = new QLabel(show_user_info);
    label_showInfo->setPixmap(QIcon(":/default/images/userInfo.png").pixmap(16));
    label_showInfo->resize(show_user_info->height(),show_user_info->height());
    label_showInfo->move(12,(show_user_info->height() - label_showInfo->height())/2);

    QPushButton* clear_msg = new QPushButton("清空聊天记录",this);
    clear_msg->resize(this->width() - 2,30);
    clear_msg->move(1, show_user_info->pos().y() + show_user_info->height());

    QLabel* label_clearMsg = new QLabel(clear_msg);
    label_clearMsg->setPixmap(QIcon(":/default/images/clearMsg.png").pixmap(16));
    label_clearMsg->resize(clear_msg->height(),clear_msg->height());
    label_clearMsg->move(12,(clear_msg->height() - label_clearMsg->height())/2);

    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::NoFrame);
    line->resize(this->width() - 20, 1);
    line->move(10, clear_msg->pos().y() + clear_msg->height() + 4);

    QString showDeleteText = isFriend ? "删除用户" : "群聊";
    if(!isFriend)
        showDeleteText.push_front(isOwner ? "解散" : "退出");
    QPushButton* delete_user = new QPushButton(showDeleteText,this);
    delete_user->resize(this->width() - 2,30);
    delete_user->move(1, line->pos().y() + line->height() + 4);

    QLabel* label_deleteUser = new QLabel(delete_user);
    label_deleteUser->setPixmap(QIcon(":/default/images/deleteUser.png").pixmap(16));
    label_deleteUser->resize(delete_user->height(),delete_user->height());
    label_deleteUser->move(12,(delete_user->height() - label_deleteUser->height())/2);

    connect(show_user_info,&QPushButton::clicked,this,[=](){
        emit switchToInfoWidget();
        this->hide();
    });

    connect(clear_msg,&QPushButton::clicked,this,[=](){
        emit clearMessages();
        this->hide();
    });

    connect(delete_user,&QPushButton::clicked,this,[=](){
        emit deletedConversation();
        this->hide();
    });

    this->setFixedHeight(delete_user->pos().y() + delete_user->height() + 10);
}

void MoreWidget::initStyle()
{
    this->setStyleSheet(R"(
                    QFrame
                    {
                        background-color: rgba(229, 231, 235, 255);
                    }
                    QPushButton
                    {
                        text-align: left;
                        border:none;
                        border-radius:none;
                        padding: 6px 0px 6px 40px;
                        font-size: 12px;
                        font-weight:bold;
                        color: rgba(55, 65, 81, 255);
                    }
                    QPushButton:hover
                    {
                        background-color: rgba(243, 244, 246, 255);
                        color: rgba(55, 65, 81, 255);
                    }
                    QLabel
                    {
                        border:none;
                        background:transparent;
                    }
                        )");
}

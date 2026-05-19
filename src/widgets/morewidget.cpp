#include "morewidget.h"

MoreWidget::MoreWidget(QWidget *parent)
    : QWidget{parent}
{
    this->setMaximumWidth(150);
    this->setMinimumWidth(150);
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setFocusPolicy(Qt::StrongFocus);
    initMore();
    initStyle();
}

void MoreWidget::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    this->hide();
    emit closed();
}

void MoreWidget::initMore()
{
    QPushButton* show_user_info = new QPushButton("查看用户信息",this);
    show_user_info->resize(this->width() - 2,30);
    show_user_info->move(1,10);

    QLabel* label_showInfo = new QLabel(show_user_info);
    label_showInfo->setPixmap(QIcon(":/default/images/userInfo.png").pixmap(16));
    label_showInfo->resize(show_user_info->height(),show_user_info->height());
    label_showInfo->move(12,(show_user_info->height() - label_showInfo->height())/2);

    QPushButton* no_voice = new QPushButton("消息免打扰",this);
    no_voice->resize(this->width() - 2,30);
    no_voice->move(1, show_user_info->pos().y() + show_user_info->height());

    QLabel* label_noVoice = new QLabel(no_voice);
    label_noVoice->setPixmap(QIcon(":/default/images/noNoise.png").pixmap(16));
    label_noVoice->resize(no_voice->height(),no_voice->height());
    label_noVoice->move(12,(no_voice->height() - label_noVoice->height())/2);

    QPushButton* clear_msg = new QPushButton("清空聊天记录",this);
    clear_msg->resize(this->width() - 2,30);
    clear_msg->move(1, no_voice->pos().y() + no_voice->height());

    QLabel* label_clearMsg = new QLabel(clear_msg);
    label_clearMsg->setPixmap(QIcon(":/default/images/clearMsg.png").pixmap(16));
    label_clearMsg->resize(clear_msg->height(),clear_msg->height());
    label_clearMsg->move(12,(clear_msg->height() - label_clearMsg->height())/2);

    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->resize(this->width() - 2, 1);
    line->move(5, clear_msg->pos().y() + clear_msg->height() + 4);

    QPushButton* delete_user = new QPushButton("删除用户",this);
    delete_user->resize(this->width() - 2,30);
    delete_user->move(1, line->pos().y() + line->height() + 4);

    QLabel* label_deleteUser = new QLabel(delete_user);
    label_deleteUser->setPixmap(QIcon(":/default/images/deleteUser.png").pixmap(16));
    label_deleteUser->resize(delete_user->height(),delete_user->height());
    label_deleteUser->move(12,(delete_user->height() - label_deleteUser->height())/2);

    connect(show_user_info,&QPushButton::clicked,this,[=](){
        qDebug()<<"show_user_info";
        this->hide();
    });
    connect(no_voice,&QPushButton::clicked,this,[=](){
        qDebug()<<"no_voice";
        this->hide();
    });
    connect(clear_msg,&QPushButton::clicked,this,[=](){
        qDebug()<<"clear_msg";
        this->hide();
    });
    connect(delete_user,&QPushButton::clicked,this,[=](){
        qDebug()<<"delete_user";
        this->hide();
    });
}

void MoreWidget::initStyle()
{
    this->setStyleSheet(R"(
                    QWidget
                    {
                        background:rgba(255, 255, 255, 255);
                        border: 1px solid rgba(229, 231, 235, 255);
                        border-radius: 8px;
                        padding: 4px 0px;
                    }
                    QFrame
                    {
                        background-color: rgba(229, 231, 235, 255);
                        margin: 4px 0px;
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

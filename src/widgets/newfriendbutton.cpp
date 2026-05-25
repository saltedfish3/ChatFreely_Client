#include "newfriendbutton.h"

NewFriendButton::NewFriendButton(int width, int height, QWidget *parent)
    : QPushButton{parent}
{
    this->resize(width,height);
    initButton();
    initStyle();
}

void NewFriendButton::enterEvent(QEnterEvent *e)
{
    QPushButton::enterEvent(e);
    this->label_Icon->setStyleSheet(R"(
                                #label_Icon
                                {
                                    background:#DADAFF;
                                }
                                    )");
    this->label_newFriend->setStyleSheet(R"(
                                #label_newFriend
                                {
                                    color: #4648d4;
                                }
                                    )");
    this->label_arrow->setPixmap(QIcon(":/default/images/activate_rightArrow.png").pixmap(16,16));
}

void NewFriendButton::leaveEvent(QEvent *e)
{
    QPushButton::leaveEvent(e);
    if(this->isChecked())
        return;
    this->label_Icon->setStyleSheet(R"(
                                #label_Icon
                                {
                                    background:#EAE7FF;
                                }
                                    )");
    this->label_newFriend->setStyleSheet(R"(
                                #label_newFriend
                                {
                                    color: #1f2937;
                                }
                                    )");
    this->label_arrow->setPixmap(QIcon(":/default/images/normal_rightArrow.png").pixmap(16,16));
}

void NewFriendButton::nextCheckState()
{
    if(!this->isChecked())
        QPushButton::nextCheckState();
}

void NewFriendButton::initButton()
{
    this->label_Icon = new QLabel(this);
    this->label_Icon->setObjectName("label_Icon");
    this->label_Icon->setPixmap(QIcon(":/default/images/addFriend.png").pixmap(24,24));
    this->label_Icon->setAlignment(Qt::AlignCenter);
    this->label_Icon->resize(36,36);
    this->label_Icon->move(16,(this->height() - this->label_Icon->height())/2);

    this->label_newFriend = new QLabel("新的朋友",this);
    this->label_newFriend->setObjectName("label_newFriend");
    this->label_newFriend->setMaximumHeight(20);
    this->label_newFriend->setMinimumHeight(20);
    this->label_newFriend->move(60,10);

    this->label_checkApply = new QLabel("查看好友申请",this);
    this->label_checkApply->setObjectName("label_checkApply");
    this->label_checkApply->setMaximumHeight(18);
    this->label_checkApply->setMinimumHeight(18);
    this->label_checkApply->move(60,this->label_newFriend->height()+6);

    this->label_arrow = new QLabel(this);
    this->label_arrow->setObjectName("label_arrow");
    this->label_arrow->setPixmap(QIcon(":/default/images/normal_rightArrow.png").pixmap(16,16));
    this->label_arrow->resize(24,24);
    this->label_arrow->move(this->width() - this->label_arrow->width() - 10,(this->height() - this->label_arrow->height())/2);
}

void NewFriendButton::initStyle()
{
    this->setStyleSheet(R"(
                                QPushButton
                                {
                                    border-radius:0px;
                                }
                                QPushButton:checked
                                {
                                    border-left:2px solid #4648d4;
                                    background:#f5f2fe;
                                }
                                QPushButton:checked:hover
                                {
                                    border-left:2px solid #4648d4;
                                    background:#f5f2fe;
                                }
                                QPushButton:hover
                                {
                                    background:#F3F4F6;
                                }
                                #label_Icon
                                {
                                    border-radius:18px;
                                    background:#EAE7FF;
                                }
                                #label_newFriend
                                {
                                    background:transparent;
                                    border: none;
                                    font-size: 12px;
                                    font-weight:bold;
                                    color: #1f2937;
                                }
                                #label_checkApply
                                {
                                    background:transparent;
                                    border: none;
                                    font-size: 12px;
                                    color: #6b7280;
                                }
                                #label_arrow
                                {
                                    color: #9ca3af;
                                }
)");
}

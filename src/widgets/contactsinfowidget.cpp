#include "contactsinfowidget.h"

ContactsInfoWidget::ContactsInfoWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);
    init();
    initStyle();
    this->setObjectName("this");
}

void ContactsInfoWidget::init()
{
    this->label_avatar = new QLabel(this);
    this->label_avatar->resize(136,136);
    this->label_avatar->setObjectName("label_avatar");
    this->label_avatar->move((this->width() - this->label_avatar->width())/2,64);
    setRadius(QIcon(":/default/images/defaultAvatar.png"),this->label_avatar,128);

    this->label_username = new QLabel("用户名",this);
    this->label_username->resize(this->width(),32);
    this->label_username->setAlignment(Qt::AlignCenter);
    this->label_username->setObjectName("label_username");
    this->label_username->move(0,this->label_avatar->pos().y() + this->label_avatar->height() + 10);

    this->label_userID = new QLabel("ID:124216457457",this);
    this->label_userID->resize(this->width(),16);
    this->label_userID->setAlignment(Qt::AlignCenter);
    this->label_userID->setObjectName("label_userID");
    this->label_userID->move(0,this->label_username->pos().y() + this->label_username->height() + 4);

    this->label_status = new QLabel("在线",this);
    this->label_status->resize(60,24);
    this->label_status->setAlignment(Qt::AlignCenter);
    this->label_status->setObjectName("label_status");
    this->label_status->move((this->width()-this->label_status->width())/2,
                             this->label_userID->pos().y() + this->label_userID->height() + 16);

    this->label_status_icon = new QLabel(this->label_status);
    this->label_status_icon->setObjectName("label_status_icon");
    this->label_status_icon->resize(8,8);
    this->label_status_icon->move(12,(this->label_status->height() - this->label_status_icon->height())/2);

    this->btn_sendMsg = new QPushButton("发送消息",this);
    this->btn_sendMsg->setObjectName("btn_sendMsg");
    this->btn_sendMsg->resize(150,48);
    this->btn_sendMsg->move(78,this->label_status->pos().y() + this->label_status->height() + 32);

    this->label_sendMsg_icon = new QLabel(this->btn_sendMsg);
    this->label_sendMsg_icon->setObjectName("label_sendMsg_icon");
    this->label_sendMsg_icon->resize(20,20);
    this->label_sendMsg_icon->setPixmap(QIcon(":/default/images/sendMsg.png").pixmap(20));
    this->label_sendMsg_icon->move(30,(this->btn_sendMsg->height() - this->label_sendMsg_icon->height())/2);

    this->btn_deleteFriend = new QPushButton("删除好友",this);
    this->btn_deleteFriend->setObjectName("btn_deleteFriend");
    this->btn_deleteFriend->resize(150,48);
    this->btn_deleteFriend->move(this->btn_sendMsg->pos().x()+this->btn_sendMsg->width()+20,
                                 this->label_status->pos().y() + this->label_status->height() + 32);

    this->label_deleteFriend = new QLabel(this->btn_deleteFriend);
    this->label_deleteFriend->setObjectName("label_deleteFriend");
    this->label_deleteFriend->resize(20,20);
    this->label_deleteFriend->setPixmap(QIcon(":/default/images/deleteUser.png").pixmap(20));
    this->label_deleteFriend->move(30,(this->btn_deleteFriend->height() - this->label_deleteFriend->height())/2);

    this->widget_emailRegion = new QWidget(this);
    this->widget_emailRegion->setObjectName("widget_emailRegion");
    this->widget_emailRegion->resize(380,80);
    this->widget_emailRegion->setAttribute(Qt::WA_StyledBackground);
    this->widget_emailRegion->move((this->width() - this->widget_emailRegion->width())/2,
                                   this->btn_deleteFriend->pos().y() + this->btn_deleteFriend->height() + 48);

    this->label_emailChinese = new QLabel("电子邮箱",this->widget_emailRegion);
    this->label_emailChinese->setObjectName("label_emailChinese");
    this->label_emailChinese->resize(this->widget_emailRegion->width()-40,20);
    this->label_emailChinese->move((this->widget_emailRegion->width() - this->label_emailChinese->width())/2,16);

    this->label_email = new QLabel("saltedfish.tk@gmail.com",this->widget_emailRegion);
    this->label_email->setObjectName("label_email");
    this->label_email->resize(this->widget_emailRegion->width()-40,24);
    this->label_email->move((this->widget_emailRegion->width() - this->label_email->width())/2,
                            this->label_emailChinese->pos().y()+this->label_emailChinese->height());
}

void ContactsInfoWidget::initStyle()
{
    this->setStyleSheet(QString(R"(
                    #label_avatar
                    {
                        border-radius:%1px;
                    }
                    #label_username
                    {
                        font-size: 24px;
                        font-weight:bold;
                        color:rgba(17, 24, 39, 255);
                    }
                    #label_userID
                    {
                        font-size:14px;
                        color:rgba(107, 114, 128, 255);
                    }
                    #label_status
                    {
                        background-color: rgba(220, 252, 231, 255);
                        color:rgba(21, 128, 61, 255);
                        font-size: 12px;
                        border-radius: 12px;
                        padding-left:15px;
                    }
                    #label_status_icon
                    {
                        border-radius:%2px;
                        background:rgba(34, 197, 94, 255);
                    }
                    #btn_sendMsg
                    {
                        background-color: rgba(99, 102, 241, 255);
                        color: rgba(255, 255, 255, 255);
                        border-radius: 8px;
                        font-size: 14px;
                        font-weight: bold;
                        padding-left:30px;
                    }
                    #btn_sendMsg:hover
                    {
                        background-color: rgba(79, 70, 229, 255);
                    }
                    #btn_sendMsg:pressed
                    {
                        background-color: rgba(67, 56, 202, 255);
                    }
                    #btn_deleteFriend
                    {
                        background-color: rgba(254, 226, 226, 255);
                        color: rgba(185, 28, 28, 255);
                        border-radius: 8px;
                        font-size: 14px;
                        padding-left:30px;
                    }
                    #btn_deleteFriend:hover
                    {
                        background-color: rgba(255, 201, 206, 255);
                    }
                    #btn_deleteFriend:pressed
                    {
                        background-color: rgba(255, 179, 186, 255);
                        color: rgba(190, 18, 60, 255);
                    }
                    #widget_emailRegion
                    {
                        background-color: rgba(255, 255, 255, 255);
                        border: 1px solid rgba(0, 0, 0, 13);
                        border-radius: 12px;
                    }
                    #label_emailChinese
                    {
                        font-family: "Microsoft YaHei";
                        font-size: 12px;
                        color: rgba(107, 114, 128, 255);
                    }
                    #label_email
                    {
                        font-family: "Microsoft YaHei";
                        font-size: 13px;
                        font-weight:bold;
                        color: rgba(17, 24, 39, 255);

                    }
                        )").arg(this->label_avatar->height()/2).arg(this->label_status_icon->height()/2));
}

void ContactsInfoWidget::setRadius(QIcon pic, QLabel *label, int hei_wid)
{
    QPixmap pixmap = pic.pixmap(hei_wid);
    //适配高DPI
    pixmap.setDevicePixelRatio(pixmap.devicePixelRatioF());

    QPixmap roundedPix(pixmap.size());
    roundedPix.fill(Qt::transparent);
    roundedPix.setDevicePixelRatio(pixmap.devicePixelRatio());

    QPainter painter(&roundedPix);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addEllipse(0,0,hei_wid,hei_wid);

    painter.setClipPath(path);
    painter.drawPixmap(0,0,pixmap);
    painter.end();

    label->setPixmap(roundedPix);
}

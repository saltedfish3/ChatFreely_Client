#include "mainwidget.h"

MainWidget::MainWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);
    initSideBar();
    initSideBarDefaultStyle();
    initStackedChat();
}

void MainWidget::buttonColorChange(QAbstractButton* button)
{
    for(auto btn : this->sideBarGroup->buttons())
    {
        QString objname = btn->objectName();
        if(btn == button)
        {
            btn->setStyleSheet(QString(R"(
                                #%1
                                {
                                    border-radius: 0;
                                    border-left: 2px solid rgba(99, 102, 241, 255);
                                    background: rgba(238, 242, 255, 255);
                                    padding-top: -20px;
                                    padding-left: -2px;
                                }
                                #%1 QLabel
                                {
                                    color: rgba(99, 102, 241, 255);
                                    font-size: 11px;
                                    font-weight:bold;
                                }
                                #%1:hover
                                {
                                    background: rgba(238, 242, 255, 255);
                                }
                            )").arg(btn->objectName()));
            if(objname == "btn_chats")
                btn->setIcon(QIcon(":/default/images/activate_chats.png"));
            else if(objname == "btn_contacts")
                btn->setIcon(QIcon(":/default/images/activate_contacts.png"));
            else if(objname == "btn_setting")
                btn->setIcon(QIcon(":/default/images/activate_setting.png"));
        }
        else//未选中
        {
            btn->setStyleSheet(QString(R"(
                                #%1
                                {
                                    border-radius: 0;
                                    border-left: none;
                                    background: transparent;
                                    padding-top: -20px;
                                    padding-left: -2px;
                                }
                                #%1 QLabel
                                {
                                    color: rgba(107, 114, 128, 255);
                                    font-size: 11px;
                                    font-weight:bold;
                                }
                                #%1:hover
                                {
                                    background: rgba(243, 244, 246, 255);
                                }
                            )").arg(btn->objectName()));
            if(objname == "btn_chats")
                btn->setIcon(QIcon(":/default/images/normal_chats.png"));
            else if(objname == "btn_contacts")
                btn->setIcon(QIcon(":/default/images/normal_contacts.png"));
            else if(objname == "btn_setting")
                btn->setIcon(QIcon(":/default/images/normal_setting.png"));
        }
    }
}

void MainWidget::initSideBar()
{
    this->widget_sideBar = new QWidget(this);
    this->widget_sideBar->setObjectName("widget_sideBar");
    this->widget_sideBar->resize(64,this->height());
    this->widget_sideBar->move(0,0);

    this->label_avatar = new QLabel(this->widget_sideBar);
    this->label_avatar->setObjectName("label_avatar");
    this->label_avatar->resize(40,40);
    setRadius(QIcon(":/default/images/defaultAvatar.png"),this->label_avatar,this->label_avatar->width());
    this->label_avatar->move((this->widget_sideBar->width() - this->label_avatar->width()) / 2,16);

    connect(&UserInfo::getUserInfo(), &UserInfo::updateAvatar, this, [this](QPixmap avatar){
        setRadius(QIcon(avatar), this->label_avatar, this->label_avatar->width());
    });

    this->sideBarGroup = new QButtonGroup(this->widget_sideBar);
    //设置互斥
    this->sideBarGroup->setExclusive(true);
    connect(this->sideBarGroup,&QButtonGroup::buttonClicked,this,&MainWidget::buttonColorChange);
    QPushButton* temp = nullptr;

    //初始化 聊天 按钮
    temp = new QPushButton(this->widget_sideBar);
    temp->setObjectName("btn_chats");

    temp->resize(this->widget_sideBar->width() - 1,this->widget_sideBar->width() + 6);
    temp->setIcon(QIcon(":/default/images/activate_chats.png"));
    temp->setIconSize(QSize(24,24));
    temp->move(0,72);
    temp->setCheckable(true);
    temp->setChecked(true);
    sideBarGroup->addButton(temp);

    this->label_chats = new QLabel("聊天",temp);
    this->label_chats->setObjectName("label_chats");
    this->label_chats->setAlignment(Qt::AlignCenter);
    this->label_chats->resize(temp->width(), 24);
    this->label_chats->move(0,temp->height() - this->label_chats->height() - 10);
    this->sideBarButton.insert(temp->objectName(),temp);

    //初始化 联系人 按钮
    temp = new QPushButton(this->widget_sideBar);
    temp->setObjectName("btn_contacts");
    temp->resize(this->widget_sideBar->width() - 1,this->widget_sideBar->width() + 6);
    temp->setIcon(QIcon(":/default/images/normal_contacts.png"));
    temp->setIconSize(QSize(20,20));
    temp->move(0,this->sideBarButton["btn_chats"]->pos().y() + this->sideBarButton["btn_chats"]->height() + 5);
    sideBarGroup->addButton(temp);

    this->label_contacts = new QLabel("联系人",temp);
    this->label_contacts->setObjectName("label_contacts");
    this->label_contacts->setAlignment(Qt::AlignCenter);
    this->label_contacts->resize(temp->width(), 24);
    this->label_contacts->move(0,temp->height() - this->label_contacts->height() - 10);
    temp->setCheckable(true);
    temp->setChecked(false);
    this->sideBarButton.insert(temp->objectName(),temp);

    //初始化 设置 按钮
    temp = new QPushButton(this->widget_sideBar);
    temp->setObjectName("btn_setting");
    temp->resize(this->widget_sideBar->width() - 1,this->widget_sideBar->width() + 6);
    temp->setIcon(QIcon(":/default/images/normal_setting.png"));
    temp->setIconSize(QSize(32,32));
    temp->move(0,this->sideBarButton["btn_contacts"]->pos().y() + this->sideBarButton["btn_contacts"]->height() + 5);
    sideBarGroup->addButton(temp);


    this->label_setting = new QLabel("设置",temp);
    this->label_setting->setObjectName("label_setting");
    this->label_setting->setAlignment(Qt::AlignCenter);
    this->label_setting->resize(temp->width(), 24);
    this->label_setting->move(0,temp->height() - this->label_setting->height() - 10);
    temp->setCheckable(true);
    temp->setChecked(false);
    this->sideBarButton.insert(temp->objectName(),temp);
}

void MainWidget::initSideBarDefaultStyle()
{
    this->widget_sideBar->setStyleSheet(R"(
                                        #widget_sideBar
                                        {
                                            background: rgba(255, 255, 255, 255);
                                            border-right: 1px solid rgba(0, 0, 0, 26);
                                        }
                                        #label_avatar
                                        {
                                            border-radius: 20px;
                                            background-color: rgba(243, 244, 246, 255);
                                        }
                                #btn_chats
                                {
                                    border-radius: 0;
                                    border-left: 2px solid rgba(99, 102, 241, 255);
                                    background: rgba(238, 242, 255, 255);
                                    padding-top: -20px;
                                    padding-left: -2px;
                                }
                                #btn_chats QLabel
                                {
                                    color: rgba(99, 102, 241, 255);
                                    font-size: 11px;
                                    font-weight:bold;
                                }
                                #btn_chats:hover
                                {
                                    background: transparent;
                                }

                                        #btn_contacts,
                                        #btn_setting
                                        {
                                            border-radius: 0;
                                            border-left: none;
                                            background: transparent;
                                            padding-top: -20px;
                                            padding-left: -2px;
                                        }

                                        #btn_contacts QLabel,
                                        #btn_setting QLabel
                                        {
                                            color: rgba(107, 114, 128, 255);
                                            font-size: 11px;
                                            font-weight:bold;
                                        }

                                        #btn_contacts:hover,
                                        #btn_setting:hover
                                        {
                                            background: rgba(243, 244, 246, 255);
                                        }
)");
}

void MainWidget::initStackedChat()
{
    this->stackedWidget_Chat = new QStackedWidget(this);
    this->stackedWidget_Chat->setObjectName("stackedWidget_Chat");
    this->stackedWidget_Chat->resize(this->width() - this->widget_sideBar->width(), this->height());
    this->stackedWidget_Chat->move(this->widget_sideBar->width(),0);

    initPage();
}

void MainWidget::initPage()
{
    //初始化预热页面
    QPixmap warmup(this->stackedWidget_Chat->size());
    warmup.fill(Qt::transparent);

    this->widget_chat = new ChatWidget(this->stackedWidget_Chat->width(),this->stackedWidget_Chat->height());
    this->widget_chat->ensurePolished();
    this->widget_chat->render(&warmup);

    this->stackedWidget_Chat->addWidget(this->widget_chat);
    this->widget_contacts = new ContactsWidget(this->stackedWidget_Chat->width(),this->stackedWidget_Chat->height());
    this->widget_contacts->ensurePolished();
    this->widget_contacts->render(&warmup);

    this->stackedWidget_Chat->addWidget(this->widget_contacts);
    this->widget_setting = new SettingWidget(this->stackedWidget_Chat->width(),this->stackedWidget_Chat->height());
    //提前绘制好样式和控件
    this->widget_setting->ensurePolished();
    this->widget_setting->render(&warmup);
    this->stackedWidget_Chat->addWidget(this->widget_setting);

    QPushButton* btn = nullptr;
    btn = this->sideBarButton.value("btn_chats");
    if(btn)
        connect(btn,&QPushButton::clicked,this,[=](){
            this->stackedWidget_Chat->setCurrentWidget(this->widget_chat);
        });

    btn = this->sideBarButton.value("btn_contacts");
    if(btn)
        connect(btn,&QPushButton::clicked,this,[=](){
            this->stackedWidget_Chat->setCurrentWidget(this->widget_contacts);
        });

    btn = this->sideBarButton.value("btn_setting");
    if(btn)
        connect(btn,&QPushButton::clicked,this,[=](){
            this->stackedWidget_Chat->setCurrentWidget(this->widget_setting);
        });

    this->stackedWidget_Chat->setCurrentWidget(this->widget_chat);
}

void MainWidget::setRadius(QIcon pic, QLabel *label, int hei_wid)
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






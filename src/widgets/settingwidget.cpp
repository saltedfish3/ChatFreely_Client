#include "settingwidget.h"

SettingWidget::SettingWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);
    initWidget();
}

SettingWidget::~SettingWidget()
{
    this->is_stop = true;
    if(this->occupy_worker.joinable())
        this->occupy_worker.join();
}

void SettingWidget::setRadius(QIcon pic, QPushButton *btn, int hei_wid)
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

    btn->setIcon(roundedPix);
    btn->setIconSize(QSize(hei_wid,hei_wid));
}

void SettingWidget::initWidget()
{
    this->scrollArea = new QScrollArea(this);
    this->scrollArea->setObjectName("scrollArea");
    this->scrollArea->resize(this->width(),this->height() - 10);
    this->scrollArea->move(0,0);

    this->widget_holy = new QWidget();
    this->widget_holy->setObjectName("widget_holy");
    this->widget_holy->resize(this->scrollArea->width(),this->scrollArea->height());

    this->scrollArea->setWidget(this->widget_holy);

    this->label_settingTitle = new QLabel("设置中心",this->widget_holy);
    this->label_settingTitle->setObjectName("label_settingTitle");
    this->label_settingTitle->setAlignment(Qt::AlignCenter);
    this->label_settingTitle->resize(this->widget_holy->width(),32);
    this->label_settingTitle->move(0,40);

    this->label_settingLittleTitle = new QLabel("管理个人信息及系统偏好",this->widget_holy);
    this->label_settingLittleTitle->setObjectName("label_settingLittleTitle");
    this->label_settingLittleTitle->setAlignment(Qt::AlignCenter);
    this->label_settingLittleTitle->resize(this->widget_holy->width(),20);
    this->label_settingLittleTitle->move(0,this->label_settingTitle->pos().y() + this->label_settingTitle->height() + 8);

    initPersonalDataWidget();
    initSystemDataWidget();

    this->line = new QFrame(this->widget_holy);
    this->line->setObjectName("line");
    this->line->resize(this->widget_systemData->width(),1);
    this->line->move(this->widget_systemData->pos().x(),
                     this->widget_systemData->pos().y()+this->widget_systemData->height() + 32);


    this->btn_UnLoginMyself = new QPushButton("退出账号",this->widget_holy);
    this->btn_UnLoginMyself->setObjectName("btn_UnLoginMyself");
    this->btn_UnLoginMyself->resize(100,36);
    this->btn_UnLoginMyself->move(this->line->pos().x(),this->line->pos().y()+this->line->height()+24);

    this->btn_saveAllChange = new QPushButton("保存所有修改",this->widget_holy);
    this->btn_saveAllChange->setObjectName("btn_saveAllChange");
    this->btn_saveAllChange->resize(100,36);
    this->btn_saveAllChange->move(this->widget_systemData->pos().x()+this->widget_systemData->width() - this->btn_saveAllChange->width(),
                                  this->btn_UnLoginMyself->pos().y());

    connect(this->btn_saveAllChange,&QPushButton::clicked,this,[this](){
        GlobalVariable::saveAllChange();
        this->label_occupyNumber->setText(GlobalVariable::getChatRecordSize());
    });

    this->btn_cancel = new QPushButton("取消",this->widget_holy);
    this->btn_cancel->setObjectName("btn_cancel");
    this->btn_cancel->resize(80,36);
    this->btn_cancel->move(this->btn_saveAllChange->pos().x() - this->btn_cancel->width() - 12,
                           this->btn_UnLoginMyself->pos().y());

    connect(this->btn_cancel,&QPushButton::clicked,this,[this](){
        GlobalVariable::clearAllChange();
        this->edit_fileSavePos->setText(GlobalVariable::getPosOfDownloadFile());
        this->edit_chatRecordSavePos->setText(GlobalVariable::getPosOfChatRecord());
    });

    this->label_version = new QLabel(QString(AppEnglishName) +" v" + QString(AppVersion),this->widget_holy);
    this->label_version->setObjectName("label_version");
    this->label_version->setAlignment(Qt::AlignCenter);
    this->label_version->resize(this->widget_holy->width(),20);
    this->label_version->move(0,this->btn_UnLoginMyself->pos().y()+this->btn_UnLoginMyself->height()+40);

    int contentHeight = this->label_version->pos().y()
                        + this->label_version->height() + 20;
    this->widget_holy->setMinimumHeight(contentHeight);
    this->scrollArea->setWidgetResizable(true);

    initThisStyle();
}

void SettingWidget::initThisStyle()
{
    this->setStyleSheet(QString(R"(
                #scrollArea
                {
                    background:transparent;
                    border:none;
                }
                #widget_holy
                {
                    background:transparent;
                    border:none;
                }
                #label_settingTitle
                {
                    color: rgba(17, 24, 39, 255);
                    font-size: 24px;
                    font-weight: bold;
                }
                #label_settingLittleTitle
                {
                    color: rgba(107, 114, 128, 255);
                    font-size: 14px;
                }
                QScrollBar:vertical
                {
                    width: 6px;
                    background: transparent;
                    margin: 0px;
                }
                QScrollBar::handle:vertical
                {
                    background: rgba(0, 0, 0, 0.2);
                    border-radius: 3px;
                    min-height: 30px;
                }
                QScrollBar::handle:vertical:hover
                {
                    background: rgba(0, 0, 0, 0.35);
                }
                QScrollBar::add-line:vertical,
                QScrollBar::sub-line:vertical
                {
                    height: 0px;
                }
                QScrollBar::add-page:vertical,
                QScrollBar::sub-page:vertical
                {
                    background: none;
                }
                #btn_saveAllChange
                {
                    background-color: rgba(99, 102, 241, 255);
                    border: none;
                    border-radius: 8px;
                    color: rgba(255, 255, 255, 255);
                    font-size: 12px;
                    font-weight:bold;
                }
                #btn_saveAllChange:hover
                {
                    background-color: rgba(79, 70, 229, 255);
                }
                #btn_saveAllChange:pressed
                {
                    background-color: rgba(67, 56, 202, 255);
                }
                #btn_cancel
                {
                    background-color: transparent;
                    border: none;
                    border-radius: 8px;
                    color: rgba(75, 85, 99, 255);
                    font-size: 12px;
                    font-weight:bold;
                }
                #btn_cancel:hover
                {
                    background-color: rgba(243, 244, 246, 255);
                    border: none;
                    color: rgba(17, 24, 39, 255);
                }
                #btn_cancel:pressed
                {
                    background-color: rgba(229, 231, 235, 255);
                    border: none;
                    color: rgba(17, 24, 39, 255);
                }
                #btn_UnLoginMyself
                {
                    background-color: transparent;
                    border: none;
                    border-radius: 8px;
                    color: rgba(220, 38, 38, 255);
                    font-size: 12px;
                    font-weight:bold;
                    padding: 8px 16px;
                    text-align: left;
                }
                #btn_UnLoginMyself:hover
                {
                    background-color: transparent;
                    color: rgba(185, 28, 28, 255);
                }
                #btn_UnLoginMyself:pressed
                {
                    background-color: transparent;
                    color: rgba(153, 27, 27, 255);
                }
                #line
                {
                    background-color: rgba(229, 231, 235, 255);
                    border: none;
                }
                #label_version
                {
                    color:rgba(156, 163, 175, 255);
                    font-size: 10px;
                    font-weight: 400;
                    background:transparent;
                }
                                    #widget_personalData
                                    {
                                        background-color: #ffffff;
                                        border: 1px solid rgba(239, 234, 254, 255);
                                        border-radius: 12px;
                                    }
                                    #label_personalData
                                    {
                                        font-size: 16px;
                                        font-family:'Segoe UI Variable';
                                        font-weight:bold;
                                        color:#111827;
                                        padding-left:%1px;
                                    }
                                    #btn_avatar
                                    {
                                        background-color: rgba(255, 255, 255, 255);
                                        border: 2px solid rgba(243, 244, 246, 255);
                                        border-radius:%3px;
                                    }
                                    #label_camera_icon
                                    {
                                        background-color: rgba(99, 102, 241, 255);
                                        border: 2px solid rgba(255, 255, 255, 255);
                                        border-radius: %2px;
                                    }
                                    #label_username,#label_userID,#label_email
                                    {
                                        font-size: 14px;
                                        font-weight:bold;
                                        color:#4b5563;
                                    }
                                    #edit_username,#edit_userID,#edit_email
                                    {
                                        background-color: rgba(249, 250, 251, 255);;
                                        border: 1px solid rgba(229, 231, 235, 255);
                                        border-radius: 8px;
                                        padding: 8px 12px;
                                        font-size: 14px;
                                        color: rgba(55, 65, 81, 255);
                                    }
                                    #edit_username:focus,#edit_userID:focus,#edit_email:focus
                                    {
                                        border: 1px solid rgba(99, 102, 241, 255);
                                        background-color: rgba(255, 255, 255, 255);;
                                    }
                                    #edit_username:disabled,#edit_userID:disabled,#edit_email:disabled
                                    {
                                        border: 1px solid rgba(229, 231, 235, 255);
                                        background-color: rgba(243, 244, 246, 255);
                                        color: rgba(156, 163, 175, 255);
                                    }


                                    #widget_systemData
                                    {
                                        background-color: #ffffff;
                                        border: 1px solid rgba(239, 234, 254, 255);
                                        border-radius: 12px;
                                    }
                                    #label_system_setting
                                    {
                                        font-size: 16px;
                                        font-family:'Segoe UI Variable';
                                        font-weight:bold;
                                        color:#111827;
                                        padding-left:%4px;
                                    }
                                    #label_bigTitle_fileSave,#label_bigTitle_chatRecord
                                    {
                                        color: rgba(156, 163, 175, 255);
                                        font-size: 16px;
                                        font-weight:500;
                                        padding-left:20px;
                                    }
                                    #label_fileSavePos,#label_chatRecordSavePos,#label_saveSpaceManage
                                    {
                                        color: rgba(75, 85, 99, 255);
                                        font-size: 13px;
                                        padding-left:20px;
                                    }
                                    #label_occupyNow
                                    {
                                        color: rgba(75, 85, 99, 255);
                                        font-size: 12px;
                                        padding-left:20px;
                                    }
                                    #label_occupyNumber
                                    {
                                        color: rgba(75, 85, 99, 255);
                                        font-size: 12px;
                                        padding-left:75px;
                                    }
                                    #edit_fileSavePos,#edit_chatRecordSavePos
                                    {
                                        background-color: rgba(249, 250, 251, 255);;
                                        border: 1px solid rgba(229, 231, 235, 255);
                                        border-radius: 8px;
                                        padding: 8px 12px;
                                        font-size: 13px;
                                        font-weight:bold;
                                        color: rgba(55, 65, 81, 255);
                                    }
                                    #edit_fileSavePos:focus,#edit_chatRecordSavePos:focus
                                    {
                                        border: 1px solid rgba(99, 102, 241, 255);
                                        background-color: rgba(255, 255, 255, 255);;
                                    }
                                    #edit_fileSavePos:disabled,#edit_chatRecordSavePos:disabled
                                    {
                                        border: 1px solid rgba(229, 231, 235, 255);
                                        background-color: rgba(243, 244, 246, 255);
                                        color: rgba(156, 163, 175, 255);
                                    }
                                    #btn_changechatRecordSavePos,#btn_changeFileSavePos
                                    {
                                        background-color: rgba(255, 255, 255, 255);
                                        border: 1px solid rgba(209, 213, 219, 255);
                                        border-radius: 6px;
                                        color: rgba(75, 85, 99, 255);
                                        font-size: 13px;
                                        font-weight:bold;
                                    }
                                    #btn_changechatRecordSavePos:hover,#btn_changeFileSavePos:hover
                                    {
                                        background-color: rgba(249, 250, 251, 255);
                                        border-color: rgba(99, 102, 241, 255);
                                        color: rgba(99, 102, 241, 255);
                                    }
                                    #btn_changechatRecordSavePos:pressed,#btn_changeFileSavePos:pressed
                                    {
                                        background-color: rgba(243, 244, 246, 255);
                                    }
                                    #btn_clearOccupy
                                    {
                                        background-color: rgba(254, 242, 242, 255);
                                        border: 1px solid rgba(252, 165, 165, 255);
                                        border-radius: 6px;
                                        color: rgba(220, 38, 38, 255);
                                        font-size: 11px;
                                    }
                                    #btn_clearOccupy:hover
                                    {
                                        background-color: rgba(220, 38, 38, 255);
                                        border-color: rgba(220, 38, 38, 255);
                                        color: rgba(255, 255, 255, 255);
                                    }
                                    #btn_clearOccupy:pressed
                                    {
                                        background-color: rgba(185, 28, 28, 255);
                                        border-color: rgba(185, 28, 28, 255);
                                        color: rgba(255, 255, 255, 255);
                                    }
                        )").arg(this->label_personalData_icon->width() + this->label_personalData_icon->pos().x()+8)
                            .arg(this->label_camera_icon->height()/2)
                            .arg(this->btn_avatar->width()/2)
                        .arg(this->label_system_setting_icon->width() + this->label_system_setting_icon->pos().x()+8));
}

void SettingWidget::initPersonalDataWidget()
{
    this->widget_personalData = new QWidget(this->widget_holy);
    this->widget_personalData->setObjectName("widget_personalData");
    this->widget_personalData->resize(560,380);
    this->widget_personalData->move((this->scrollArea->width() - this->widget_personalData->width())/2,
                                    this->label_settingLittleTitle->pos().y()+this->label_settingLittleTitle->height()+40);
    this->widget_personalData->setAttribute(Qt::WA_StyledBackground);

    this->label_personalData = new QLabel("个人资料",this->widget_personalData);
    this->label_personalData->setObjectName("label_personalData");
    this->label_personalData->resize(this->width(),24);
    this->label_personalData->move(0,24);

    this->label_personalData_icon = new QLabel(this->label_personalData);
    this->label_personalData_icon->setObjectName("label_personalData_icon");
    this->label_personalData_icon->setPixmap(QIcon(":/default/images/personalData.png").pixmap(20));
    this->label_personalData_icon->resize(20,20);
    this->label_personalData_icon->move(24,(this->label_personalData->height() - this->label_personalData_icon->height())/2);

    this->btn_avatar = new QPushButton(this->widget_personalData);
    this->btn_avatar->setObjectName("btn_avatar");
    this->btn_avatar->resize(84,84);
    setRadius(QIcon(":/default/images/defaultAvatar.png"),this->btn_avatar,80);
    this->btn_avatar->move((this->widget_personalData->width() - this->btn_avatar->width())/2,
                           this->label_personalData->pos().y()+this->label_personalData->height()+20);

    this->label_camera_icon = new QLabel(this->btn_avatar);
    this->label_camera_icon->setObjectName("label_camera_icon");
    this->label_camera_icon->resize(24,24);
    this->label_camera_icon->setAlignment(Qt::AlignCenter);
    this->label_camera_icon->setPixmap(QIcon(":/default/images/camera.png").pixmap(16));
    this->label_camera_icon->move(this->btn_avatar->width() - this->label_camera_icon->width() -5,
                                  this->btn_avatar->height()-this->label_camera_icon->height() -5);

    this->label_username = new QLabel("昵称",this->widget_personalData);
    this->label_username->setObjectName("label_username");
    this->label_username->resize(60,20);
    this->label_username->move(24,this->btn_avatar->height() + this->btn_avatar->pos().y() + 22);

    this->label_userID = new QLabel("畅聊ID",this->widget_personalData);
    this->label_userID->setObjectName("label_userID");
    this->label_userID->resize(60,20);
    this->label_userID->move(this->label_username->pos().x()+this->label_username->width()+212,
                             this->label_username->pos().y());

    this->edit_username = new QLineEdit(this->widget_personalData);
    this->edit_username->setObjectName("edit_username");
    this->edit_username->setText("用户昵称");
    this->edit_username->resize(240,40);
    this->edit_username->move(24,this->label_username->pos().y()+this->label_username->height()+8);

    this->edit_userID = new QLineEdit(this->widget_personalData);
    this->edit_userID->setObjectName("edit_userID");
    this->edit_userID->setText("12345678910");
    this->edit_userID->setEnabled(false);
    this->edit_userID->resize(240,40);
    this->edit_userID->move(this->edit_username->pos().x()+this->edit_username->width()+32,this->edit_username->pos().y());

    this->label_email = new QLabel("电子邮箱",this->widget_personalData);
    this->label_email->setObjectName("label_email");
    this->label_email->resize(80,20);
    this->label_email->move(this->edit_username->pos().x(),
                             this->edit_username->pos().y()+this->edit_username->height()+24);

    this->edit_email = new QLineEdit(this->widget_personalData);
    this->edit_email->setObjectName("edit_email");
    this->edit_email->setText("saltedfish.tk@gmail.com");
    this->edit_email->setEnabled(false);
    this->edit_email->resize(512,40);
    this->edit_email->move(this->label_email->pos().x(),
                           this->label_email->pos().y()+this->label_email->height()+8);
}

void SettingWidget::initSystemDataWidget()
{
    this->widget_systemData = new QWidget(this->widget_holy);
    this->widget_systemData->setObjectName("widget_systemData");
    this->widget_systemData->resize(this->widget_personalData->width(),400);
    this->widget_systemData->move(this->widget_personalData->pos().x(),
                                    this->widget_personalData->pos().y()+this->widget_personalData->height()+20);
    this->widget_systemData->setAttribute(Qt::WA_StyledBackground);

    this->label_system_setting = new QLabel("系统设置",this->widget_systemData);
    this->label_system_setting->setObjectName("label_system_setting");
    this->label_system_setting->resize(this->widget_systemData->width(),24);
    this->label_system_setting->move(0,24);

    this->label_system_setting_icon = new QLabel(this->label_system_setting);
    this->label_system_setting_icon->setObjectName("label_system_setting_icon");
    this->label_system_setting_icon->resize(24,24);
    this->label_system_setting_icon->move(24,(this->label_system_setting->height() - this->label_system_setting_icon->height())/2);
    this->label_system_setting_icon->setPixmap(QIcon(":/default/images/systemSetting.png").pixmap(24));

    this->label_bigTitle_fileSave = new QLabel("文件存储",this->widget_systemData);
    this->label_bigTitle_fileSave->setObjectName("label_bigTitle_fileSave");
    this->label_bigTitle_fileSave->resize(this->widget_systemData->width(),18);
    this->label_bigTitle_fileSave->move(0,this->label_system_setting->pos().y()+this->label_system_setting->height()+20);

    this->label_fileSavePos = new QLabel("下载文件保存位置",this->widget_systemData);
    this->label_fileSavePos->setObjectName("label_fileSavePos");
    this->label_fileSavePos->resize(this->widget_systemData->width(),20);
    this->label_fileSavePos->move(0,this->label_bigTitle_fileSave->pos().y()+this->label_bigTitle_fileSave->height()+16);

    this->edit_fileSavePos = new QLineEdit(this->widget_systemData);
    this->edit_fileSavePos->setObjectName("edit_fileSavePos");
    this->edit_fileSavePos->setReadOnly(true);
    this->edit_fileSavePos->setText(GlobalVariable::getPosOfDownloadFile());

    this->edit_fileSavePos->move(20,this->label_fileSavePos->pos().y()+this->label_fileSavePos->height()+8);

    this->btn_changeFileSavePos = new QPushButton("浏览",this->widget_systemData);
    this->btn_changeFileSavePos->setObjectName("btn_changeFileSavePos");
    this->btn_changeFileSavePos->resize(64,40);

    this->edit_fileSavePos->resize(this->widget_systemData->width()-40-btn_changeFileSavePos->width() - 10,this->btn_changeFileSavePos->height());
    this->btn_changeFileSavePos->move(this->edit_fileSavePos->pos().x()+this->edit_fileSavePos->width()+10,
                                      this->edit_fileSavePos->pos().y());

    connect(this->btn_changeFileSavePos,&QPushButton::clicked,this,[this](){
        QString dir = QFileDialog::getExistingDirectory(this,"请选择下载文件保存目录",GlobalVariable::getPosOfDownloadFile());
        if(dir.isEmpty())
            return;
        GlobalVariable::setPosOfDownloadFile(dir);
        this->edit_fileSavePos->setText(dir);
        //文件转移
    });

    this->label_bigTitle_chatRecord = new QLabel("聊天记录",this->widget_systemData);
    this->label_bigTitle_chatRecord->setObjectName("label_bigTitle_chatRecord");
    this->label_bigTitle_chatRecord->resize(this->widget_systemData->width(),18);
    this->label_bigTitle_chatRecord->move(0,this->edit_fileSavePos->pos().y()+this->edit_fileSavePos->height()+28);

    this->label_chatRecordSavePos = new QLabel("聊天记录保存路径",this->widget_systemData);
    this->label_chatRecordSavePos->setObjectName("label_chatRecordSavePos");
    this->label_chatRecordSavePos->resize(this->widget_systemData->width(),20);
    this->label_chatRecordSavePos->move(0,this->label_bigTitle_chatRecord->pos().y()+this->label_bigTitle_chatRecord->height()+16);

    this->edit_chatRecordSavePos = new QLineEdit(this->widget_systemData);
    this->edit_chatRecordSavePos->setObjectName("edit_chatRecordSavePos");
    this->edit_chatRecordSavePos->setReadOnly(true);
    this->edit_chatRecordSavePos->setText(GlobalVariable::getPosOfChatRecord());

    this->edit_chatRecordSavePos->move(20,this->label_chatRecordSavePos->pos().y()+this->label_chatRecordSavePos->height()+4);

    this->btn_changechatRecordSavePos = new QPushButton("修改",this->widget_systemData);
    this->btn_changechatRecordSavePos->setObjectName("btn_changechatRecordSavePos");
    this->btn_changechatRecordSavePos->resize(64,40);

    this->edit_chatRecordSavePos->resize(this->widget_systemData->width()-40-btn_changechatRecordSavePos->width() - 10,
                                         this->btn_changechatRecordSavePos->height());
    this->btn_changechatRecordSavePos->move(this->edit_chatRecordSavePos->pos().x()+this->edit_chatRecordSavePos->width()+10,
                                      this->edit_chatRecordSavePos->pos().y());

    connect(this->btn_changechatRecordSavePos,&QPushButton::clicked,this,[this](){
        QString dir = QFileDialog::getExistingDirectory(this,"请选择聊天文件保存目录",GlobalVariable::getPosOfChatRecord());
        if(dir.isEmpty())
            return;
        GlobalVariable::setPosOfChatRecord(dir);
        this->edit_chatRecordSavePos->setText(dir);
        //文件转移
    });

    this->label_saveSpaceManage = new QLabel("存储空间管理",this->widget_systemData);
    this->label_saveSpaceManage->setObjectName("label_saveSpaceManage");
    this->label_saveSpaceManage->resize(this->widget_systemData->width(),20);
    this->label_saveSpaceManage->move(0,this->edit_chatRecordSavePos->pos().y()+this->edit_chatRecordSavePos->height()+30);

    this->label_occupyNow = new QLabel("当前占用:",this->widget_systemData);
    this->label_occupyNow->setObjectName("label_occupyNow");
    this->label_occupyNow->resize(this->widget_systemData->width(),18);
    this->label_occupyNow->move(0,this->label_saveSpaceManage->pos().y()+this->label_saveSpaceManage->height()+4);

    this->label_occupyNumber = new QLabel(this->label_occupyNow);
    this->label_occupyNumber->setObjectName("label_occupyNumber");
    this->label_occupyNumber->resize(this->label_occupyNow->width(),this->label_occupyNow->height());
    this->label_occupyNumber->move(0,0);
    this->label_occupyNumber->setText(GlobalVariable::getChatRecordSize());

    this->is_stop = false;
    this->occupy_worker = std::thread([this](){
        while(!this->is_stop)
        {
            this->label_occupyNumber->setText(GlobalVariable::getChatRecordSize());
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    });

    QTimer* timer = new QTimer(this->widget_holy);
    connect(timer,&QTimer::timeout,this,[this](){
        this->label_occupyNumber->setText(GlobalVariable::getChatRecordSize());
    });
    timer->start(10000);

    this->btn_clearOccupy = new QPushButton("清空聊天记录",this->widget_systemData);
    this->btn_clearOccupy->setObjectName("btn_clearOccupy");
    this->btn_clearOccupy->resize(80,this->label_occupyNow->pos().y()+this->label_occupyNow->height()-this->label_saveSpaceManage->pos().y() - 8);
    this->btn_clearOccupy->move(this->btn_changechatRecordSavePos->pos().x()+this->btn_changechatRecordSavePos->width()-this->btn_clearOccupy->width(),
                                this->label_saveSpaceManage->pos().y() + 4);

    connect(this->btn_clearOccupy,&QPushButton::clicked,this,[this](){
        QMessageBox::StandardButton reply = QMessageBox::question(this,"确认清空？","确定要清空所有聊天记录吗？",QMessageBox::Yes|QMessageBox::Cancel);
        if(reply == QMessageBox::Yes)
        {
            GlobalVariable::clearAllChatRecord();
            this->label_occupyNumber->setText(GlobalVariable::getChatRecordSize());
        }
    });
}


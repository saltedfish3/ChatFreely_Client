#include "loginwidget.h"

LoginWidget::LoginWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);
    initWidget();
    initLoginStyle();
}

void LoginWidget::initWidget()
{
    //初始化 “欢迎回来” 标签
    this->label_welcomeBack = new QLabel("欢迎回来",this);
    this->label_welcomeBack->setObjectName("label_welcomeBack");
    this->label_welcomeBack->setAlignment(Qt::AlignCenter);
    this->label_welcomeBack->resize(440,40);
    this->label_welcomeBack->move(180,68);

    //初始化 “登录您的” 标签
    this->label_welcomeBack2 = new QLabel("登录您的 "+ QString(AppEnglishName) +" 账号",this);
    this->label_welcomeBack2->setObjectName("label_welcomeBack2");
    this->label_welcomeBack2->setAlignment(Qt::AlignCenter);
    this->label_welcomeBack2->resize(440,24);
    this->label_welcomeBack2->move(180,118);

    //初始化 "电子邮箱" 标签
    this->label_loginEmail = new QLabel("电子邮箱",this);
    this->label_loginEmail->setObjectName("label_loginEmail");
    this->label_loginEmail->resize(440,20);
    this->label_loginEmail->move(180,164);

    //初始化 "电子邮箱" 输入框
    this->edit_loginEmail = new QLineEdit(this);
    this->edit_loginEmail->setObjectName("edit_loginEmail");
    this->edit_loginEmail->setPlaceholderText("请输入邮箱地址");
    this->edit_loginEmail->setMaxLength(254);
    this->edit_loginEmail->resize(440,52);
    this->edit_loginEmail->move(180,192);

    //初始化 "密码" 标签
    this->label_loginPassd = new QLabel("密码",this);
    this->label_loginPassd->setObjectName("label_loginPassd");
    this->label_loginPassd->resize(220,20);
    this->label_loginPassd->move(180,268);

    //初始化 "密码" 输入框
    this->edit_loginPassd = new QLineEdit(this);
    this->edit_loginPassd->setObjectName("edit_loginPassd");
    this->edit_loginPassd->setPlaceholderText("请输入密码");
    this->edit_loginPassd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    this->edit_loginPassd->setMaxLength(128);
    this->edit_loginPassd->resize(440,52);
    this->edit_loginPassd->move(180,296);

    //初始化 “忘记密码” 按钮
    this->btn_forgetPassd = new QPushButton("忘记密码?",this);
    this->btn_forgetPassd->setObjectName("btn_forgetPassd");
    this->btn_forgetPassd->resize(100,20);
    this->btn_forgetPassd->move(535,268);

    //初始化 “立即登录” 按钮
    this->btn_loginNow = new QPushButton("立即登录",this);
    this->btn_loginNow->setObjectName("btn_loginNow");
    this->btn_loginNow->resize(440,48);
    this->btn_loginNow->move(180,this->edit_loginPassd->pos().y() + this->edit_loginPassd->height() + 32);
    connect(this->btn_loginNow,&QPushButton::pressed,this,&LoginWidget::onButtonPressed);
    connect(this->btn_loginNow,&QPushButton::released,this,&LoginWidget::onButtonReleased);
    connect(this->btn_loginNow,&QPushButton::clicked,this,[=](){
        emit changeWidget(GlobalVariable::MainPage::Main);
    });

    //初始化 “还没有账号” 标签
    this->label_noAccount = new QLabel("还没有账号?",this);
    this->label_noAccount->setObjectName("label_noAccount");
    this->label_noAccount->resize(98,24);
    this->label_noAccount->move(325,460);

    //初始化 “立即注册” 按钮
    this->btn_goRegisterNow = new QPushButton("立即注册",this);
    this->btn_goRegisterNow->setObjectName("btn_goRegisterNow");
    this->btn_goRegisterNow->resize(70,24);
    this->btn_goRegisterNow->move(414,460);
    connect(this->btn_goRegisterNow,&QPushButton::clicked,this,&LoginWidget::sendSignalsChangeToRegister);

    //初始化 “版本号” 标签
    this->label_version = new QLabel(QString(AppEnglishName) +" v" + QString(AppVersion),this);
    this->label_version->setObjectName("label_version");
    this->label_version->setAlignment(Qt::AlignCenter);
    this->label_version->resize(440,20);
    this->label_version->move(180,this->height() - this->label_version->height() - 30);
}

//按钮 按下动画
void LoginWidget::onButtonPressed()
{
    QPushButton* send = qobject_cast<QPushButton*>(sender());
    if(!send)
        return;
    send->resize(send->width() - 8,send->height() - 2);
    send->move(send->pos().x() + 4,send->pos().y() + 1);
}

//按钮 松开动画
void LoginWidget::onButtonReleased()
{
    QPushButton* send = qobject_cast<QPushButton*>(sender());
    if(!send)
        return;
    send->resize(send->width() + 8,send->height() + 2);
    send->move(send->pos().x() - 4,send->pos().y() - 1);
}

//发送切换到 注册窗口 信号
void LoginWidget::sendSignalsChangeToRegister()
{
    emit changeWidget(GlobalVariable::MainPage::Register);
}

//初始化 登录界面 样式表
void LoginWidget::initLoginStyle()
{
    //Login Style
    this->setStyleSheet(R"(
                                        #widget_login
                                        {
                                            background:transparent;
                                        }
                                        #label_welcomeBack
                                        {
                                            color:rgba(17, 24, 39, 255);
                                            font-family: "Microsoft YaHei";
                                            font-size: 30px;
                                            font-weight: 700;
                                            background:transparent;
                                        }
                                        #label_welcomeBack2
                                        {
                                            color:rgba(107, 114, 128, 255);
                                            font-family: "Microsoft YaHei";
                                            font-size: 16px;
                                            font-weight: 500;
                                            background:transparent;
                                        }
                                        #label_loginEmail,
                                        #label_loginPassd
                                        {
                                            color:rgba(55, 65, 81, 255);
                                            font-family: "Microsoft YaHei";
                                            font-size: 14px;
                                            font-weight: 550;
                                            background:transparent;
                                        }
                                        #label_noAccount
                                        {
                                            color:rgba(107, 114, 128, 255);
                                            font-family: "Microsoft YaHei";
                                            font-size: 14px;
                                            font-weight: 400;
                                            background:transparent;
                                        }
                                        #label_version
                                        {
                                            color:rgba(156, 163, 175, 255);
                                            font-family: "Microsoft YaHei";
                                            font-size: 10px;
                                            font-weight: 400;
                                            background:transparent;
                                        }

                                        #edit_loginEmail,
                                        #edit_loginPassd
                                        {
                                            background: rgba(255, 255, 255, 255);
                                            border: 1px solid rgba(229, 231, 235, 255);
                                            border-radius: 8px;
                                            padding-left: 16px;
                                            padding-right: 16px;
                                            color: rgba(17, 24, 39, 255);
                                            font-size: 13px;
                                            font-weight:bold;
                                            selection-background-color: rgba(99, 102, 241, 100);
                                        }
                                        #edit_loginEmail:hover,
                                        #edit_loginPassd:hover
                                        {
                                            border: 1px solid rgba(209, 213, 219, 255);
                                        }
                                        #edit_loginEmail:focus,
                                        #edit_loginPassd:focus
                                        {
                                            border: 2px solid rgba(99, 102, 241, 255);
                                            padding-left:15px;
                                        }
                                        #edit_loginEmail::placeholder,
                                        #edit_loginPassd::placeholder
                                        {
                                            color: rgba(156, 163, 175, 255);
                                        }

                                        #btn_forgetPassd
                                        {
                                            color: rgba(99, 102, 241, 255);
                                            font-family: "Microsoft YaHei";
                                            font-size: 14px;
                                            font-weight: 550;
                                            background: transparent;
                                            border: none;
                                        }
                                        #btn_forgetPassd:hover
                                        {
                                            color: rgba(79, 70, 229, 255);
                                        }
                                        #btn_loginNow
                                        {
                                            color: rgba(255, 255, 255, 255);
                                            font-family: "Microsoft YaHei";
                                            font-size: 16px;
                                            font-weight: 600;
                                            background: rgba(99, 102, 241, 255);
                                            border: none;
                                            border-radius: 8px;
                                        }
                                        #btn_loginNow:hover
                                        {
                                            background: rgba(79, 70, 229, 255);
                                        }
                                        #btn_loginNow:pressed
                                        {
                                            background: rgba(67, 56, 202, 255);
                                        }
                                        #btn_goRegisterNow
                                        {
                                            color: rgba(99, 102, 241, 255);
                                            font-family: "Microsoft YaHei";
                                            font-size: 14px;
                                            font-weight: 500;
                                            text-align: left;
                                            background: transparent;
                                            border: none;
                                            padding: 0px;
                                        }
                                        #btn_goRegisterNow:hover
                                        {
                                            color: rgba(79, 70, 229, 255);
                                        }
)");
}

#include "registerwidget.h"

RegisterWidget::RegisterWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);
    initWidget();
    initRegisterSytle();
}

void RegisterWidget::initWidget()
{
    //初始化 “创建账号” 标签
    this->label_createAccount = new QLabel("创建账号",this);
    this->label_createAccount->setObjectName("label_createAccount");
    this->label_createAccount->setAlignment(Qt::AlignCenter);
    this->label_createAccount->resize(440,40);
    this->label_createAccount->move(180,44);

    //初始化 “立即加入” 标签
    this->label_joinApp = new QLabel("立即加入 "+ QString(AppEnglishName) +" 社区",this);
    this->label_joinApp->setObjectName("label_joinApp");
    this->label_joinApp->setAlignment(Qt::AlignCenter);
    this->label_joinApp->resize(440,24);
    this->label_joinApp->move(180,94);

    //初始化 "用户名" 标签
    this->label_regUsername = new QLabel("用户名",this);
    this->label_regUsername->setObjectName("label_regUsername");
    this->label_regUsername->resize(440,20);
    this->label_regUsername->move(180,142);

    //初始化 "用户名" 输入框
    this->edit_regUsername = new QLineEdit(this);
    this->edit_regUsername->setObjectName("edit_regUsername");
    this->edit_regUsername->setPlaceholderText("请输入用户名");
    this->edit_regUsername->setMaxLength(30);
    this->edit_regUsername->resize(440,52);
    this->edit_regUsername->move(180,166);

    //初始化 "电子邮箱" 标签
    this->label_regEmail = new QLabel("电子邮箱",this);
    this->label_regEmail->setObjectName("label_regEmail");
    this->label_regEmail->resize(440,20);
    this->label_regEmail->move(180,230);

    //初始化 "电子邮箱" 输入框
    this->edit_regEmail = new QLineEdit(this);
    this->edit_regEmail->setObjectName("edit_regEmail");
    this->edit_regEmail->setPlaceholderText("请输入邮箱地址");
    this->edit_regEmail->setMaxLength(254);
    this->edit_regEmail->resize(440,52);
    this->edit_regEmail->move(180,254);

    //初始化 "密码" 标签
    this->label_regPassd = new QLabel("密码",this);
    this->label_regPassd->setObjectName("label_regPassd");
    this->label_regPassd->resize(210,20);
    this->label_regPassd->move(180,318);

    //初始化 "密码" 输入框
    this->edit_regPassd = new QLineEdit(this);
    this->edit_regPassd->setObjectName("edit_regPassd");
    this->edit_regPassd->setPlaceholderText("请输入密码");
    this->edit_regPassd->setMaxLength(128);
    this->edit_regPassd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    this->edit_regPassd->resize(210,52);
    this->edit_regPassd->move(180,342);

    //初始化 "确认密码" 标签
    this->label_regConfirmPassd = new QLabel("确认密码",this);
    this->label_regConfirmPassd->setObjectName("label_regConfirmPassd");
    this->label_regConfirmPassd->resize(210,20);
    this->label_regConfirmPassd->move(410,318);

    //初始化 "确认密码" 输入框
    this->edit_regConfirmPassd = new QLineEdit(this);
    this->edit_regConfirmPassd->setObjectName("edit_regConfirmPassd");
    this->edit_regConfirmPassd->setPlaceholderText("请再次输入密码");
    this->edit_regConfirmPassd->setMaxLength(128);
    this->edit_regConfirmPassd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    this->edit_regConfirmPassd->resize(210,52);
    this->edit_regConfirmPassd->move(410,342);

    //初始化 “立即注册” 按钮
    this->btn_registerNow = new QPushButton("立即注册",this);
    this->btn_registerNow->setObjectName("btn_registerNow");
    this->btn_registerNow->resize(440,48);
    this->btn_registerNow->move(180,414);
    connect(this->btn_registerNow,&QPushButton::pressed,this,&RegisterWidget::onButtonPressed);
    connect(this->btn_registerNow,&QPushButton::released,this,&RegisterWidget::onButtonReleased);

    //初始化 “已经有账号” 标签
    this->label_haveAccount = new QLabel("已经有账号?",this);
    this->label_haveAccount->setObjectName("label_haveAccount");
    this->label_haveAccount->resize(98,24);
    this->label_haveAccount->move(326,486);

    //初始化 "立即登录" 按钮
    this->btn_goLoginNow = new QPushButton("立即登录",this);
    this->btn_goLoginNow->setObjectName("btn_goLoginNow");
    this->btn_goLoginNow->resize(70,24);
    this->btn_goLoginNow->move(414,486);
    connect(this->btn_goLoginNow,&QPushButton::clicked,this,&RegisterWidget::sendSignalChangeToLogin);
}

void RegisterWidget::initRegisterSytle()
{
    this->setStyleSheet(R"(
                                            #label_createAccount
                                            {
                                                color:rgba(17, 24, 39, 255);
                                                font-family: "Microsoft YaHei";
                                                font-size: 30px;
                                                font-weight: 700;
                                                background:transparent;
                                            }
                                            #label_joinApp
                                            {
                                                color:rgba(107, 114, 128, 255);
                                                font-family: "Microsoft YaHei";
                                                font-size: 16px;
                                                font-weight: 500;
                                                background:transparent;
                                            }
                                            #label_regUsername,
                                            #label_regEmail,
                                            #label_regPassd,
                                            #label_regConfirmPassd
                                            {
                                                color:rgba(55, 65, 81, 255);
                                                font-family: "Microsoft YaHei";
                                                font-size: 14px;
                                                font-weight: 550;
                                                background:transparent;
                                            }

                                        #edit_regUsername,
                                        #edit_regEmail,
                                        #edit_regPassd,
                                        #edit_regConfirmPassd
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
                                        #edit_regUsername:hover,
                                        #edit_regEmail:hover,
                                        #edit_regPassd:hover,
                                        #edit_regConfirmPassd:hover
                                        {
                                            border: 1px solid rgba(209, 213, 219, 255);
                                        }
                                        #edit_regUsername:focus,
                                        #edit_regEmail:focus,
                                        #edit_regPassd:focus,
                                        #edit_regConfirmPassd:focus
                                        {
                                            border: 2px solid rgba(99, 102, 241, 255);
                                            padding-left:15px;
                                        }
                                        #edit_regUsername::placeholder,
                                        #edit_regEmail::placeholder,
                                        #edit_regPassd::placeholder,
                                        #edit_regConfirmPassd::placeholder
                                        {
                                            color: rgba(156, 163, 175, 255);
                                        }

                                        #btn_registerNow
                                        {
                                            color: rgba(255, 255, 255, 255);
                                            font-family: "Microsoft YaHei";
                                            font-size: 16px;
                                            font-weight: 600;
                                            background: rgba(99, 102, 241, 255);
                                            border: none;
                                            border-radius: 8px;
                                        }
                                        #btn_registerNow:hover{
                                            background: rgba(79, 70, 229, 255);
                                        }
                                        #btn_registerNow:pressed{
                                            background: rgba(67, 56, 202, 255);
                                        }

                                        #label_haveAccount{
                                            color:rgba(107, 114, 128, 255);
                                            font-family: "Microsoft YaHei";
                                            font-size: 14px;
                                            font-weight: 400;
                                            background:transparent;
                                        }

                                        #btn_goLoginNow{
                                            color: rgba(99, 102, 241, 255);
                                            font-family: "Microsoft YaHei";
                                            font-size: 14px;
                                            font-weight: 500;
                                            text-align: left;
                                            background: transparent;
                                            border: none;
                                            padding: 0px;
                                        }
                                        #btn_goLoginNow:hover{
                                            color: rgba(79, 70, 229, 255);
                                        }
)");
}

//按钮 按下动画
void RegisterWidget::onButtonPressed()
{
    QPushButton* send = qobject_cast<QPushButton*>(sender());
    if(!send)
        return;
    send->resize(send->width() - 8,send->height() - 2);
    send->move(send->pos().x() + 4,send->pos().y() + 1);
}

//按钮 松开动画
void RegisterWidget::onButtonReleased()
{
    QPushButton* send = qobject_cast<QPushButton*>(sender());
    if(!send)
        return;
    send->resize(send->width() + 8,send->height() + 2);
    send->move(send->pos().x() - 4,send->pos().y() - 1);
}

//发送 跳转到登录界面 信号
void RegisterWidget::sendSignalChangeToLogin()
{
    emit changeWidget(GlobalVariable::MainPage::Login);
}

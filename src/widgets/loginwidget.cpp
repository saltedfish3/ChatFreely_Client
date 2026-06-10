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
    this->edit_loginEmail->setProperty("hasError", false);
    connect(this->edit_loginEmail, &QLineEdit::textChanged, this, [=](){
        clearError(this->edit_loginEmail, this->label_emailError);
    });

    //初始化 "电子邮箱" 错误标签
    this->label_emailError = new QLabel("", this);
    this->label_emailError->setObjectName("label_emailError");
    this->label_emailError->resize(440, 20);
    this->label_emailError->move(180, this->edit_loginEmail->pos().y() + this->edit_loginEmail->height() + 4);
    this->label_emailError->hide();

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
    this->edit_loginPassd->setProperty("hasError", false);
    connect(this->edit_loginPassd, &QLineEdit::textChanged, this, [=](){
        clearError(this->edit_loginPassd, this->label_passdError);
    });

    //初始化 "密码" 错误标签
    this->label_passdError = new QLabel("", this);
    this->label_passdError->setObjectName("label_passdError");
    this->label_passdError->resize(440, 20);
    this->label_passdError->move(180, this->edit_loginPassd->pos().y() + this->edit_loginPassd->height() + 4);
    this->label_passdError->hide();

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
        QString email = this->edit_loginEmail->text().trimmed();
        QString password = this->edit_loginPassd->text().trimmed();
        QRegularExpression regExp_email("^[0-9a-zA-Z._%+\\-]+@[0-9a-zA-Z.\\-]+\\.[a-zA-Z]{2,}$");
        QString info = QString();
        if(email.isEmpty())
        {
            info = "邮箱不能为空";
        }
        else if(!regExp_email.match(email).hasMatch())
        {
            info = "邮箱格式不正确";
        }
        if(!info.isEmpty())
        {
            showError(this->edit_loginEmail, this->label_emailError, info);
            return;
        }

        QRegularExpression regExp_password("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[!@#$%^&*])[a-zA-Z\\d!@#$%^&*]{8,30}$");
        if(password.isEmpty())
        {
            info = "密码不能为空";
        }
        else if(!regExp_password.match(password).hasMatch())
        {
            info = "密码需包含大小写字母、数字和特殊符号(!@#$%^&*)，8-30位";
        }
        if(!info.isEmpty())
        {
            showError(this->edit_loginPassd, this->label_passdError, info);
            return;
        }
        this->setDisabled(true);
        this->btn_loginNow->setDisabled(true);
        this->btn_loginNow->setText("立即登录中...");
        this->animation_loading->start();
        this->label_loading->show();
        ChatClient::getChatClient().sendLogin(email, password);
        //超时计时器启动
    });

    this->animation_btn_loginNow = new QPropertyAnimation(this->btn_loginNow,"geometry",this);
    this->animation_btn_loginNow->setStartValue(this->btn_loginNow->geometry());
    this->animation_btn_loginNow->setEndValue(this->btn_loginNow->geometry().adjusted(4,0,-4,-1));
    this->animation_btn_loginNow->setDuration(50);

    //初始化 “加载” 图标
    this->label_loading = new LoadingLabel(this->btn_loginNow);
    this->label_loading->resize(24,24);
    this->label_loading->setPixmap(QIcon(":/default/images/loading.png").pixmap(24));
    this->label_loading->move((this->btn_loginNow->width() - this->label_loading->width())/2 - 55,
                              (this->btn_loginNow->height() - this->label_loading->height())/2);
    this->label_loading->hide();

    this->animation_loading = new QPropertyAnimation(this->label_loading,"angle",this);
    this->animation_loading->setStartValue(0);
    this->animation_loading->setEndValue(360);
    this->animation_loading->setDuration(2000);
    this->animation_loading->setLoopCount(-1);
    this->animation_loading->start();

    //初始化 "跳转" 时钟
    this->timer_jump = new QTimer(this);
    this->timer_jump->setInterval(2000);
    this->timer_jump->setSingleShot(true);
    connect(this->timer_jump,&QTimer::timeout,this,[=](){
        emit changeWidget(GlobalVariable::MainPage::Main);
        this->setDisabled(false);
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

    //初始化 "顶部 错误提示" 标签
    this->label_toast = new QLabel("", this);
    this->label_toast->setObjectName("label_toast");
    this->label_toast->setAlignment(Qt::AlignCenter);
    this->label_toast->resize(this->width(), 44);
    this->label_toast->move(0, 0);
    this->label_toast->hide();

    this->timer_toast = new QTimer(this);
    this->timer_toast->setSingleShot(true);
    connect(this->timer_toast, &QTimer::timeout, this, [=](){
        this->label_toast->hide();
    });

    connect(&ChatClient::getChatClient(), &ChatClient::LoginState, this, &LoginWidget::getLoginState);
}

//按钮 按下动画
void LoginWidget::onButtonPressed()
{
    QPushButton* send = qobject_cast<QPushButton*>(sender());
    if(!send)
        return;
    // send->resize(send->width() - 8,send->height() - 2);
    this->animation_btn_loginNow->setDirection(QPropertyAnimation::Direction::Forward);
    // send->move(send->pos().x() + 2,send->pos().y() + 1);
    this->animation_btn_loginNow->start();

}

//按钮 松开动画
void LoginWidget::onButtonReleased()
{
    QPushButton* send = qobject_cast<QPushButton*>(sender());
    if(!send)
        return;
    this->animation_btn_loginNow->setDirection(QPropertyAnimation::Direction::Backward);
    // send->move(send->pos().x() - 2,send->pos().y() - 1);
    this->animation_btn_loginNow->start();
    //send->resize(send->width() + 8,send->height() + 2);

}

//发送切换到 注册窗口 信号
void LoginWidget::sendSignalsChangeToRegister()
{
    emit changeWidget(GlobalVariable::MainPage::Register);
}

void LoginWidget::getLoginState(bool isSuccess, QString from, QString info)
{
    this->btn_loginNow->setDisabled(false);
    this->btn_loginNow->setText("立即登录");
    this->animation_loading->stop();
    this->label_loading->hide();
    if(isSuccess)
    {
        showToast(info + " 将在2秒后为您跳转...",isSuccess);
        this->timer_jump->start();
        return;
    }
    if(from.isEmpty())
    {
        showToast(info);
    }
    else if(from == "Email")
    {
        showError(this->edit_loginEmail,this->label_emailError,info);
    }
    else if(from == "Password")
    {
        showError(this->edit_loginPassd,this->label_passdError,info);
    }
    else
    {
        showToast(info);
    }
    this->setDisabled(false);
}

//显示错误标签
void LoginWidget::showError(QLineEdit* edit, QLabel* label, const QString& msg)
{
    label->setText(msg);
    label->show();
    edit->setProperty("hasError", true);
    edit->style()->unpolish(edit);
    edit->style()->polish(edit);
}

//清除错误标签
void LoginWidget::clearError(QLineEdit* edit, QLabel* label)
{
    label->hide();
    label->setText("");
    edit->setProperty("hasError", false);
    edit->style()->unpolish(edit);
    edit->style()->polish(edit);
}

void LoginWidget::showToast(const QString &msg, bool isSuccess)
{
    this->label_toast->setText(msg);
    this->label_toast->setProperty("toastType",isSuccess);
    this->label_toast->style()->unpolish(this->label_toast);
    this->label_toast->style()->polish(this->label_toast);
    this->label_toast->show();
    this->label_toast->raise();
    this->timer_toast->start(3000);
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
                                        #label_emailError,
                                        #label_passdError
                                        {
                                            color: rgba(239, 68, 68, 255);
                                            font-size: 12px;
                                            background: transparent;
                                        }
                                            #label_toast[toastType="false"]
                                            {
                                                background: rgba(254, 226, 226, 255);
                                                color: rgba(185, 28, 28, 255);
                                                border-bottom: 1px solid rgba(252, 165, 165, 255);
                                            }
                                            #label_toast[toastType="true"]
                                            {
                                                background: rgba(220, 252, 231, 255);
                                                color: rgba(22, 101, 52, 255);
                                                border-bottom: 1px solid rgba(134, 239, 172, 255);
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
                                        #edit_loginEmail[hasError="true"],
                                        #edit_loginPassd[hasError="true"]
                                        {
                                            border: 1px solid rgba(239, 68, 68, 255);
                                            padding-left: 16px;
                                        }
                                        #edit_loginEmail:hover,
                                        #edit_loginPassd:hover
                                        {
                                            border: 1px solid rgba(209, 213, 219, 255);
                                        }
                                        #edit_loginEmail:focus,
                                        #edit_loginPassd:focus
                                        {
                                            border: 1px solid rgba(99, 102, 241, 255);
                                            padding-left:16px;
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
                                        #btn_loginNow:disabled
                                        {
                                            background-color: rgba(99, 102, 241, 180);
                                            border: none;
                                            border-radius: 8px;
                                            color: rgba(255, 255, 255, 220);
                                            font-size: 16px;
                                            font-weight: 600;
                                            padding-left:20px;
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

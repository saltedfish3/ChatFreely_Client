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
    this->label_createAccount->move(180,34);//44

    //初始化 “立即加入” 标签
    this->label_joinApp = new QLabel("立即加入 "+ QString(AppEnglishName) +" 社区",this);
    this->label_joinApp->setObjectName("label_joinApp");
    this->label_joinApp->setAlignment(Qt::AlignCenter);
    this->label_joinApp->resize(440,24);
    this->label_joinApp->move(180,84);//94

    //初始化 "用户名" 标签
    this->label_regUsername = new QLabel("用户名",this);
    this->label_regUsername->setObjectName("label_regUsername");
    this->label_regUsername->resize(440,20);
    this->label_regUsername->move(180,120);//142

    //初始化 "用户名" 输入框
    this->edit_regUsername = new QLineEdit(this);
    this->edit_regUsername->setObjectName("edit_regUsername");
    this->edit_regUsername->setPlaceholderText("请输入用户名");
    this->edit_regUsername->setMaxLength(30);
    this->edit_regUsername->resize(440,52);
    this->edit_regUsername->move(this->label_regUsername->pos().x(),
                                 this->label_regUsername->pos().y()+this->label_regUsername->height() + 2);//166
    this->edit_regUsername->setProperty("hasError", false);
    connect(this->edit_regUsername, &QLineEdit::textChanged, this, [=](){
        clearError(this->edit_regUsername, this->label_usernameError);
    });

    //初始化 "用户名" 错误标签
    this->label_usernameError = new QLabel("", this);
    this->label_usernameError->setObjectName("label_usernameError");
    this->label_usernameError->resize(440, 20);
    this->label_usernameError->move(this->edit_regUsername->pos().x(),
                                    this->edit_regUsername->pos().y() + this->edit_regUsername->height()+2);
    this->label_usernameError->hide();

    //初始化 "电子邮箱" 标签
    this->label_regEmail = new QLabel("电子邮箱",this);
    this->label_regEmail->setObjectName("label_regEmail");
    this->label_regEmail->resize(440,20);
    this->label_regEmail->move(this->edit_regUsername->pos().x(),
                               this->edit_regUsername->pos().y()+this->edit_regUsername->height() + 20);//230

    //初始化 "电子邮箱" 输入框
    this->edit_regEmail = new QLineEdit(this);
    this->edit_regEmail->setObjectName("edit_regEmail");
    this->edit_regEmail->setPlaceholderText("请输入邮箱地址");
    this->edit_regEmail->setMaxLength(254);
    this->edit_regEmail->resize(440,52);
    this->edit_regEmail->move(this->label_regEmail->pos().x(),
                              this->label_regEmail->pos().y()+this->label_regEmail->height() + 2);//254
    connect(this->edit_regEmail, &QLineEdit::textChanged, this, [=](){
        clearError(this->edit_regEmail, this->label_emailError);
    });

    //初始化 "电子邮箱" 错误标签
    this->label_emailError = new QLabel("", this);
    this->label_emailError->setObjectName("label_emailError");
    this->label_emailError->resize(440, 20);
    this->label_emailError->move(this->label_regEmail->pos().x(),
                                 this->edit_regEmail->pos().y() + this->edit_regEmail->height()+2);
    this->label_emailError->hide();

    //初始化 "密码" 标签
    this->label_regPassd = new QLabel("密码",this);
    this->label_regPassd->setObjectName("label_regPassd");
    this->label_regPassd->resize(210,20);
    this->label_regPassd->move(this->edit_regEmail->pos().x(),
                               this->edit_regEmail->pos().y()+this->edit_regEmail->height()+20);//318

    //初始化 "密码" 输入框
    this->edit_regPassd = new QLineEdit(this);
    this->edit_regPassd->setObjectName("edit_regPassd");
    this->edit_regPassd->setPlaceholderText("请输入密码");
    this->edit_regPassd->setMaxLength(128);
    this->edit_regPassd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    this->edit_regPassd->resize(210,52);
    this->edit_regPassd->move(this->label_regPassd->pos().x(),
                              this->label_regPassd->pos().y()+this->label_regPassd->height()+2);//342
    this->edit_regPassd->setProperty("hasError", false);
    connect(this->edit_regPassd, &QLineEdit::textChanged, this, [=](){
        clearError(this->edit_regPassd, this->label_passdError);
    });

    //初始化 "密码" 错误标签
    this->label_passdError = new QLabel("", this);
    this->label_passdError->setObjectName("label_passdError");
    this->label_passdError->resize(440, 20);
    this->label_passdError->move(this->label_regPassd->pos().x(),
                                 this->edit_regPassd->pos().y() + this->edit_regPassd->height()+2);
    this->label_passdError->hide();

    //初始化 "确认密码" 标签
    this->label_regConfirmPassd = new QLabel("确认密码",this);
    this->label_regConfirmPassd->setObjectName("label_regConfirmPassd");
    this->label_regConfirmPassd->resize(210,20);
    this->label_regConfirmPassd->move(this->label_regPassd->pos().x()+this->label_regPassd->width()+20,
                                      this->label_regPassd->pos().y());//318

    //初始化 "确认密码" 输入框
    this->edit_regConfirmPassd = new QLineEdit(this);
    this->edit_regConfirmPassd->setObjectName("edit_regConfirmPassd");
    this->edit_regConfirmPassd->setPlaceholderText("请再次输入密码");
    this->edit_regConfirmPassd->setMaxLength(128);
    this->edit_regConfirmPassd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    this->edit_regConfirmPassd->resize(210,52);
    this->edit_regConfirmPassd->move(this->label_regConfirmPassd->pos().x(),
                                     this->label_regConfirmPassd->pos().y()+this->label_regConfirmPassd->height()+2);//342
    this->edit_regConfirmPassd->setProperty("hasError", false);
    connect(this->edit_regConfirmPassd, &QLineEdit::textChanged, this, [=](){
        clearError(this->edit_regConfirmPassd, this->label_confirmPError);
    });

    //初始化 "确认密码" 错误标签
    this->label_confirmPError = new QLabel("", this);
    this->label_confirmPError->setObjectName("label_confirmPError");
    this->label_confirmPError->resize(210, 20);
    this->label_confirmPError->move(this->label_regConfirmPassd->pos().x(),
                                    this->edit_regConfirmPassd->pos().y() + this->edit_regConfirmPassd->height()+2);
    this->label_confirmPError->hide();

    //初始化 “跳转登录” 时钟
    this->timer_jump = new QTimer(this);
    this->timer_jump->setInterval(2500);
    this->timer_jump->setSingleShot(true);
    connect(this->timer_jump, &QTimer::timeout, this, [=](){
        emit changeWidget(GlobalVariable::MainPage::Login);
        this->edit_regUsername->clear();
        this->edit_regEmail->clear();
        this->edit_regPassd->clear();
        this->edit_regConfirmPassd->clear();
    });

    //初始化 “立即注册” 按钮
    this->btn_registerNow = new QPushButton("立即注册",this);
    this->btn_registerNow->setObjectName("btn_registerNow");
    this->btn_registerNow->resize(440,48);
    this->btn_registerNow->move(180,414);
    connect(this->btn_registerNow,&QPushButton::pressed,this,&RegisterWidget::onButtonPressed);
    connect(this->btn_registerNow,&QPushButton::released,this,&RegisterWidget::onButtonReleased);
    connect(this->btn_registerNow,&QPushButton::clicked,this,[=](){
        QString username = this->edit_regUsername->text().trimmed();
        QString email = this->edit_regEmail->text().trimmed();
        QString password = this->edit_regPassd->text().trimmed();
        QString conPassword = this->edit_regConfirmPassd->text().trimmed();
        QString info = "";
        if(username.isEmpty())
        {
            showError(this->edit_regUsername,this->label_usernameError,"用户名不能为空");
            return;
        }
        if(email.isEmpty())
        {
            info = "邮箱不能为空";
        }
        else
        {
            QRegularExpression regExp_email("^[0-9a-zA-Z._%+\\-]+@[0-9a-zA-z.\\-]+\\.[a-zA-Z]{2,}$");
            if(!regExp_email.match(email).hasMatch())
            {
                info = "邮箱格式不合规";
            }
        }
        if(!info.isEmpty())
        {
            showError(this->edit_regEmail,this->label_emailError, info);
            return;
        }

        if(password.isEmpty())
        {
            info = "密码不能为空";
        }
        else
        {
            QRegularExpression regExp_password("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[!@#$%^&*])[a-zA-Z\\d!@#$%^&*]{8,30}$");
            if(!regExp_password.match(password).hasMatch())
            {
                info = "密码需包含大小写字母、数字和特殊符号(!@#$%^&*)，8-30位";
            }
        }
        if(!info.isEmpty())
        {
            showError(this->edit_regPassd,this->label_passdError, info);
            return;
        }

        if(conPassword != password)
        {
            showError(this->edit_regConfirmPassd,this->label_confirmPError, "密码与确认密码不同");
            return;
        }
        this->setDisabled(true);
        this->btn_registerNow->setDisabled(true);
        this->btn_registerNow->setText("立即注册中...");
        this->label_loading->show();
        this->animation_loading->start();
        TcpLongConnection::getTcpClient().sendRegister(username,email,password);
    });

    this->animation_btn_registerNow = new QPropertyAnimation(this->btn_registerNow,"geometry",this);
    this->animation_btn_registerNow->setStartValue(this->btn_registerNow->geometry());
    this->animation_btn_registerNow->setEndValue(this->btn_registerNow->geometry().adjusted(4,0,-4,-1));
    this->animation_btn_registerNow->setDuration(50);

    //初始化 “加载” 图标
    this->label_loading = new LoadingLabel(this->btn_registerNow);
    this->label_loading->resize(24,24);
    this->label_loading->setPixmap(QIcon(":/default/images/loading.png").pixmap(24));
    this->label_loading->move((this->btn_registerNow->width() - this->label_loading->width())/2 - 55,
                              (this->btn_registerNow->height() - this->label_loading->height())/2);
    this->label_loading->hide();

    this->animation_loading = new QPropertyAnimation(this->label_loading,"angle",this);
    this->animation_loading->setStartValue(0);
    this->animation_loading->setEndValue(360);
    this->animation_loading->setDuration(2000);
    this->animation_loading->setLoopCount(-1);

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

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::RegisterState, this, &RegisterWidget::getRegisterState);
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
                                            #label_usernameError,
                                            #label_emailError,
                                            #label_passdError,
                                            #label_confirmPError
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
                                            margin-top:1px;
                                            margin-left:1px;
                                        }
                                        #edit_regEmail
                                        {
                                            margin-top: 0px;
                                            margin-left: 0px;
                                        }
                                        #edit_regUsername[hasError="true"],
                                        #edit_regEmail[hasError="true"],
                                        #edit_regPassd[hasError="true"],
                                        #edit_regConfirmPassd[hasError="true"]
                                        {
                                            border: 1px solid rgba(239, 68, 68, 255);
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
                                            border: 1px solid rgba(99, 102, 241, 255);
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
                                        #btn_registerNow:disabled
                                        {
                                            background-color: rgba(99, 102, 241, 180);
                                            border: none;
                                            border-radius: 8px;
                                            color: rgba(255, 255, 255, 220);
                                            font-size: 16px;
                                            font-weight: 600;
                                            padding-left:20px;
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

void RegisterWidget::showToast(const QString &msg, bool isSuccess)
{
    if(isSuccess)
        ToastManager::getToastManager(false).success(msg, this->window(), this);
    else
        ToastManager::getToastManager(false).error(msg, this->window(), this);
}

//按钮 按下动画
void RegisterWidget::onButtonPressed()
{
    QPushButton* send = qobject_cast<QPushButton*>(sender());
    if(!send)
        return;
    // send->resize(send->width() - 8,send->height() - 2);
    // send->move(send->pos().x() + 4,send->pos().y() + 1);
    this->animation_btn_registerNow->setDirection(QPropertyAnimation::Direction::Forward);
    this->animation_btn_registerNow->start();
}

//按钮 松开动画
void RegisterWidget::onButtonReleased()
{
    QPushButton* send = qobject_cast<QPushButton*>(sender());
    if(!send)
        return;
    // send->resize(send->width() + 8,send->height() + 2);
    // send->move(send->pos().x() - 4,send->pos().y() - 1);
    this->animation_btn_registerNow->setDirection(QPropertyAnimation::Direction::Backward);
    this->animation_btn_registerNow->start();
}

//发送 跳转到登录界面 信号
void RegisterWidget::sendSignalChangeToLogin()
{
    emit changeWidget(GlobalVariable::MainPage::Login);
}

void RegisterWidget::showError(QLineEdit *edit, QLabel *label, const QString &msg)
{
    label->setText(msg);
    label->show();
    edit->setProperty("hasError", true);
    edit->style()->unpolish(edit);
    edit->style()->polish(edit);
}

void RegisterWidget::clearError(QLineEdit *edit, QLabel *label)
{
    label->setText("");
    label->hide();
    edit->setProperty("hasError", false);
    edit->style()->unpolish(edit);
    edit->style()->polish(edit);
}

void RegisterWidget::getRegisterState(bool isSuccess, QString from, QString info)
{
    this->setDisabled(false);
    this->btn_registerNow->setDisabled(false);
    this->btn_registerNow->setText("立即注册");
    this->label_loading->hide();
    this->animation_loading->stop();
    if(isSuccess)
    {
        showToast(info, true);
        this->timer_jump->start();
        return;
    }
    if(from.isEmpty())
    {
        showToast(info);
    }
    else if(from == "Email")
    {
        showError(this->edit_regEmail,this->label_emailError,info);
    }
    else if(from == "Password")
    {
        showError(this->edit_regPassd,this->label_passdError,info);
    }
    else if(from == "Username")
    {
        showError(this->edit_regUsername,this->label_usernameError,info);
    }
    else
    {
        showToast(info);
    }
}

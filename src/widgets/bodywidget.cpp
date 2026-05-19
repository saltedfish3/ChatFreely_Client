#include "bodywidget.h"

BodyWidget::BodyWidget(int width, int height, int radius, QWidget* parent)
    : QWidget(parent), radius(radius), pos_widget(QPoint())
{
    //默认窗口数据
    if (width <= 0)
        width = 300;
    if (height <= 0)
        height = 400;
    if (radius <= 0)
        this->radius = 0;
    this->resize(width,height);

    //设置无边框和透明背景
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    //获取屏幕大小让窗口居中
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen)
    {
        QRect screenRect = screen->availableGeometry();
        move((screenRect.width() - this->width()) / 2, (screenRect.height() - this->height()) / 2);
    }

    //安装过滤器
    this->installEventFilter(this);
    //
    initTitleBar();
    initStackWidget();
}

void BodyWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);//抗锯齿
    QPainterPath path;
    QRect rect = QRect(QRect(0, 0, this->width(), this->height()));
    path.addRoundedRect(rect, this->radius, this->radius);
    painter.setBrush(QColor(255, 255, 255));
    painter.setPen(QPen(QColor(0, 0, 0, 26), 1));
    painter.drawPath(path);
}

bool BodyWidget::eventFilter(QObject* obj, QEvent* ev)
{
    //过滤窗口拖动事件
    if (obj == this)
    {
        QMouseEvent* qme = static_cast<QMouseEvent*>(ev);
        if(!qme)
            return QObject::eventFilter(obj, ev);
        if (ev->type() == QEvent::MouseButtonPress)
        {
            this->pos_widget = qme->pos();
        }
        else if (ev->type() == QEvent::MouseButtonRelease)
        {
            this->pos_widget = QPoint();
        }
        else if (ev->type() == QEvent::MouseMove && this->pos_widget.isNull() == false)
        {
            if(qme->pos().y() <= this->widget_titleBar->height())
                this->move(qme->globalPosition().toPoint() - this->pos_widget);
            else
                this->pos_widget = QPoint();
        }
    }
    return QObject::eventFilter(obj, ev);
}

//初始化 标题栏
void BodyWidget::initTitleBar()
{
    //初始化标题栏
    this->widget_titleBar = new TitleBarWidget(this->width(),40,this->radius,this);
    this->widget_titleBar->setObjectName("titleBar");
    this->widget_titleBar->move(0,0);
    connect(this->widget_titleBar,&TitleBarWidget::closeApp,this,&BodyWidget::close);
    connect(this->widget_titleBar,&TitleBarWidget::minimizeApp,this,&BodyWidget::showMinimized);
}

void BodyWidget::initStackWidget()
{
    //初始化页面
    this->stackedWidget_page = new QStackedWidget(this);
    this->stackedWidget_page->setObjectName("stackedWidget_page");
    this->stackedWidget_page->resize(this->width(),this->height() - this->widget_titleBar->height());
    this->stackedWidget_page->move(0,this->widget_titleBar->height());

    initLoginWidget();
    initRegisterWidget();
    initMainWidget();

    this->stackedWidget_page->setCurrentWidget(this->widget_login);
}

//初始化 登录界面
void BodyWidget::initLoginWidget()
{
    //初始化登录界面
    this->widget_login = new LoginWidget(this->stackedWidget_page->width(),this->stackedWidget_page->height(),this->stackedWidget_page);
    this->widget_login->setObjectName("widget_login");

    connect(this->widget_login,&LoginWidget::changeWidget,this,&BodyWidget::changeWidget);
    this->stackedWidget_page->addWidget(this->widget_login);
}

//初始化 注册界面
void BodyWidget::initRegisterWidget()
{
    //初始化注册界面
    this->widget_register = new RegisterWidget(this->stackedWidget_page->width(),this->stackedWidget_page->height(),this->stackedWidget_page);
    this->widget_register->setObjectName("widget_register");
    connect(this->widget_register,&RegisterWidget::changeWidget,this,&BodyWidget::changeWidget);
    this->stackedWidget_page->addWidget(this->widget_register);
}

//初始化 主界面
void BodyWidget::initMainWidget()
{
    this->widget_main = new MainWidget(this->stackedWidget_page->width(),this->stackedWidget_page->height());
    this->widget_main->setObjectName("widget_main");

    this->stackedWidget_page->addWidget(this->widget_main);
}

//改变容器页码
void BodyWidget::changeWidget(WidgetID id)
{
    if(id == WidgetID::Login)
        this->stackedWidget_page->setCurrentWidget(this->widget_login);
    else if(id == WidgetID::Register)
        this->stackedWidget_page->setCurrentWidget(this->widget_register);
    else if(id == WidgetID::Main)
        this->stackedWidget_page->setCurrentWidget(this->widget_main);
}




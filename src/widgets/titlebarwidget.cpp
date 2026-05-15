#include "titlebarwidget.h"

TitleBarWidget::TitleBarWidget(int width, int height, int radius, QWidget *parent)
    : QWidget{parent},radius(radius)
{
    this->resize(width,height);
    if(radius <=0 )
        this->radius = 0;

    this->setAttribute(Qt::WA_StyledBackground, true);

    initWidget();
    initTitleBarStyle();
}

void TitleBarWidget::sendClose()
{
    emit closeApp();
}

void TitleBarWidget::sendMinimize()
{
    emit minimizeApp();
}

void TitleBarWidget::initWidget()
{
    //App图标标签
    this->label_logo = new QLabel(this);
    this->label_logo->setObjectName("label_logo");
    this->label_logo->resize(16,16);
    this->label_logo->setPixmap(QPixmap(":/icon/images/favicon.ico").scaled(16,16));
    this->label_logo->move(12,(this->height() - this->label_logo->height())/2);

    //App名字标签
    this->label_logoText = new QLabel(QString(AppEnglishName) + " " + QString(AppChineseName),this);
    this->label_logoText->setObjectName("label_logoText");
    this->label_logoText->setMinimumHeight(this->height());
    this->label_logoText->setMaximumHeight(this->height());
    this->label_logoText->move(this->label_logo->pos().x() + this->label_logo->width() + 8, 0);

    //关闭按钮
    this->btn_close = new CloseButton(50,this->height(),this);
    //去掉焦点边框
    this->btn_close->setFlat(true);
    this->btn_close->setObjectName("btn_close");


    //最小化按钮
    this->btn_min = new MinimizeButton(50,this->height(),this);
    this->btn_min->setFlat(true);
    this->btn_min->setObjectName("btn_min");
    this->btn_close->move(this->width() - btn_close->width(), 0);
    this->btn_min->move(this->width() - btn_close->width() - btn_min->width(), 0);

    connect(this->btn_close,&QPushButton::clicked,this,&TitleBarWidget::sendClose);
    connect(this->btn_min,&QPushButton::clicked,this,&TitleBarWidget::sendMinimize);
}

void TitleBarWidget::initTitleBarStyle()
{
    this->setStyleSheet(QString(R"(
                            TitleBarWidget
                            {
                                background:rgba(255, 255, 255, 255);
                                border-top-left-radius: %1px;
                                border-top-right-radius: %1px;
                                border-bottom:1px solid #F1F5F9;
                                border-top:1px solid rgba(0, 0, 0, 10);
                                border-left:1px solid rgba(0, 0, 0, 10);
                                border-right:1px solid rgba(0, 0, 0, 10);
                            }
                            #label_logoText
                            {
                                font-size:13px;
                                color:rgba(107, 114, 128, 255);
                                font-weight: 600;
                                font-family: "Microsoft YaHei";
                                background: transparent;
                                padding-top:-2px;
                            }
                            #btn_close
                            {
                                background:transparent;
                                border-top-right-radius: %2px;
                            }
                            #btn_close:hover
                            {
                                background: rgba(215, 30, 45, 255);
                            }
                            #btn_close:pressed
                            {
                                background: rgba(185, 25, 40, 255);
                            }
                            #btn_min
                            {
                                background:transparent;
                                border: none;
                            }
                            #btn_min:hover
                            {
                                background: rgba(0, 0, 0, 26);
                            }
                            #btn_min:pressed
                            {
                                background: rgba(0, 0, 0, 51);
                            }
)").arg(this->radius).arg(this->radius - 1));
}

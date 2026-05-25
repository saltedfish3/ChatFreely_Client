#include "newfriendmanagewidget.h"

NewFriendManageWidget::NewFriendManageWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);
    this->setObjectName("this");
    initWidget();
    initstyle();
}

void NewFriendManageWidget::initWidget()
{
    this->widget_friendApply = new QWidget(this);
    this->widget_friendApply->setObjectName("widget_friendApply");
    this->widget_friendApply->resize(this->width(),56);
    this->widget_friendApply->move(0,0);

    this->label_friendApply = new QLabel("好友申请管理",this->widget_friendApply);
    this->label_friendApply->setObjectName("label_friendApply");
    this->label_friendApply->setAlignment(Qt::AlignVCenter);
    QFont font = this->label_friendApply->font();
    font.setWeight(QFont::DemiBold);
    this->label_friendApply->setFont(font);
    this->label_friendApply->setMaximumHeight(this->widget_friendApply->height());
    this->label_friendApply->setMinimumHeight(this->widget_friendApply->height());
    this->label_friendApply->move(0,(this->widget_friendApply->height() - this->label_friendApply->height())/2);

    this->listWidget_friendApplyManage = new QListWidget(this);
    this->listWidget_friendApplyManage->setObjectName("listWidget_friendApplyManage");
    this->listWidget_friendApplyManage->resize(this->width(),this->height() - this->widget_friendApply->height());
    this->listWidget_friendApplyManage->move(0,this->widget_friendApply->height());

}

void NewFriendManageWidget::initstyle()
{
    this->setStyleSheet(R"(
                            #widget_friendApply
                            {
                                border-bottom:1px solid #e0e0e0;
                            }
                            #label_friendApply
                            {
                                color: rgba(17, 24, 39, 255);
                                padding-left: 24px;
                                font-size:14px;
                            }
                            #listWidget_friendApplyManage
                            {
                                border-radius:0px;
                                background:transparent;
                            }
                                )");
}

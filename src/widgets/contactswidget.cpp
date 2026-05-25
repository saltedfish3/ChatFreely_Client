#include "contactswidget.h"

ContactsWidget::ContactsWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);
    this->setObjectName("this");

    initSideBar();
    initSideBarStyle();

    initSideUn();
    initSideUnStyle();

    initStacked();
}

void ContactsWidget::initSideBar()
{
    this->widget_sideBar = new QWidget(this);
    this->widget_sideBar->setObjectName("widget_sideBar");
    this->widget_sideBar->resize(260,this->height());
    this->widget_sideBar->move(0,0);

    this->btn_addFriend = new QPushButton("添加联系人",this->widget_sideBar);
    this->btn_addFriend->setObjectName("btn_addFriend");
    this->btn_addFriend->resize(this->widget_sideBar->width() - 32,35);
    this->btn_addFriend->move((this->widget_sideBar->width() - this->btn_addFriend->width())/2 , (this->widget_sideBar->width() - this->btn_addFriend->width())/2);

    this->edit_searchFriend = new QLineEdit(this->widget_sideBar);
    this->edit_searchFriend->setObjectName("edit_searchFriend");
    this->edit_searchFriend->setPlaceholderText("搜索联系人");
    QAction* searchIcon = new QAction(QIcon(":/default/images/search.png"),"",this->edit_searchFriend);
    this->edit_searchFriend->addAction(searchIcon,QLineEdit::LeadingPosition);
    this->edit_searchFriend->resize(this->widget_sideBar->width() - 32,36);
    this->edit_searchFriend->move((this->widget_sideBar->width() - this->edit_searchFriend->width())/2 ,
                                  this->btn_addFriend->pos().y() + this->btn_addFriend->height() + (this->widget_sideBar->width() - this->btn_addFriend->width())/2);

}

void ContactsWidget::initSideBarStyle()
{
    this->widget_sideBar->setStyleSheet(R"(
                                #widget_sideBar
                                {
                                    background:#ffffff;
                                    border-right:1px solid #e0e0e0;
                                }
                                #btn_addFriend
                                {
                                    color:#ffffff;
                                    background: #4648d4;
                                    border:none;
                                    border-radius: 8px;
                                    font-size: 13px;
                                    font-weight: bold;
                                }
                                #btn_addFriend:hover
                                {
                                    background: #6063ee;
                                }
                                #btn_addFriend:pressed
                                {
                                    background: #2f2ebe;
                                }
                                #edit_searchFriend
                                {
                                    background: #efecf8;
                                    border: 1px solid #f3f4f6;
                                    border-radius: 8px;
                                    padding: 8px 12px 8px 0px;
                                    font-size: 12px;
                                    font-weight:bold;
                                    color: #1b1b23;
                                }
                                #edit_searchFriend:focus
                                {
                                    background: #ffffff;
                                    border: 1px solid #4648d4;
                                }
                                        )");
}

void ContactsWidget::initSideUn()
{
    this->widget_sideUn = new QWidget(this->widget_sideBar);
    this->widget_sideUn->setObjectName("widget_sideUn");
    int upPos = this->edit_searchFriend->pos().y() + this->edit_searchFriend->height() + (this->widget_sideBar->width() - this->edit_searchFriend->width()) / 2;
    this->widget_sideUn->resize(this->widget_sideBar->width() - 1,this->widget_sideBar->height() - upPos);
    this->widget_sideUn->move(0,upPos);

    this->btn_newFriend = new NewFriendButton(this->widget_sideUn->width(),55,this->widget_sideUn);
    this->btn_newFriend->setObjectName("btn_newFriend");
    this->btn_newFriend->move(0, 10);
    this->btn_newFriend->setCheckable(true);
    this->btn_newFriend->setChecked(false);
    connect(this->btn_newFriend,&QPushButton::clicked,this,[=](){
        this->stackedWidget->setCurrentWidget(this->widget_newFriend);
    });

    this->label_myFriend = new QLabel("我的好友",this->widget_sideUn);
    this->label_myFriend->setObjectName("label_myFriend");
    this->label_myFriend->move((this->widget_sideBar->width()-this->btn_addFriend->width())/2,
                               this->btn_newFriend->pos().y()+this->btn_newFriend->height()+8);

    this->listView_myFriend = new QListView(this->widget_sideUn);
    this->listView_myFriend->setObjectName("listView_myFriend");
    this->listView_myFriend->resize(this->widget_sideUn->width(),
                                    this->widget_sideUn->height() - this->label_myFriend->pos().y() - this->label_myFriend->height());
    this->listView_myFriend->move(0,this->label_myFriend->pos().y() + this->label_myFriend->height());
}

void ContactsWidget::initSideUnStyle()
{
    this->widget_sideUn->setStyleSheet(R"(
                                #widget_sideUn
                                {
                                    border-top:1px solid #e0e0e0;
                                    background:#ffffff;
                                }
                                #label_myFriend
                                {
                                    color:#1f2937;
                                    font-size:12px;
                                    font-weight:bold;
                                }
                                #listView_myFriend
                                {
                                    background: transparent;
                                    border-radius:0px;
                                }
                                        )");
}

void ContactsWidget::initStacked()
{
    this->stackedWidget = new QStackedWidget(this);
    this->stackedWidget->setObjectName("stackedWidget");
    this->stackedWidget->resize(this->width() - this->widget_sideBar->width(),this->height());
    this->stackedWidget->move(this->widget_sideBar->width(),0);

    this->widget_noSelect = new QWidget();
    this->widget_noSelect->resize(this->stackedWidget->width(),this->stackedWidget->height());

    this->label_noSelect = new QLabel("Please choose....",this->widget_noSelect);
    this->label_noSelect->setObjectName("label_noSelect");
    this->label_noSelect->resize(this->widget_noSelect->width(),this->widget_noSelect->height());
    this->label_noSelect->setAlignment(Qt::AlignCenter);
    QFont font = this->label_noSelect->font();
    font.setPixelSize(16);
    font.setWeight(QFont::Bold);
    this->label_noSelect->setFont(font);

    this->label_noSelect->move(0,0);
    this->label_noSelect->setStyleSheet(R"(
                            #label_noSelect
                            {
                                color:black;
                            }
                                    )");

    this->widget_newFriend = new NewFriendManageWidget(this->stackedWidget->width(),this->stackedWidget->height());
    this->widget_contactsInfo = new ContactsInfoWidget(this->stackedWidget->width(),this->stackedWidget->height());

    this->stackedWidget->addWidget(this->widget_noSelect);
    this->stackedWidget->addWidget(this->widget_newFriend);
    this->stackedWidget->addWidget(this->widget_contactsInfo);

    this->stackedWidget->setCurrentWidget(this->widget_noSelect);
}

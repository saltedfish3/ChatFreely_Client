#ifndef BODYWIDGET_H
#define BODYWIDGET_H

#include <QWidget>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qguiapplication.h>
#include <qscreen.h>
#include <qevent.h>
#include <QString>
#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>

#include "closebutton.h"
#include "minimizebutton.h"
#include "GlobalVariable.h"
#include "loginwidget.h"
#include "registerwidget.h"
#include "titlebarwidget.h"
#include "mainwidget.h"

class BodyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BodyWidget(int width,int height,int radius,QWidget *parent = nullptr);

signals:


private slots:
    void changeWidget(WidgetID id);

private:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* obj,QEvent* ev) override;

    void initTitleBar();
    void initStackWidget();
    void initLoginWidget();
    void initRegisterWidget();
    void initMainWidget();

    int radius;
    QPoint pos_widget;

    QStackedWidget* stackedWidget_page;

    TitleBarWidget* widget_titleBar;
    LoginWidget* widget_login;
    RegisterWidget* widget_register;
    MainWidget* widget_main;
};

#endif // BODYWIDGET_H

#ifndef TITLEBARWIDGET_H
#define TITLEBARWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTextLayout>
#include "minimizebutton.h"
#include "closebutton.h"
#include "../utils/GlobalVariable.h"


class TitleBarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBarWidget(int width, int height, int radius, QWidget *parent = nullptr);
    int leftLimit();
    int rightLimit();

signals:
    void closeApp();
    void minimizeApp();

private slots:
    void sendClose();
    void sendMinimize();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void initWidget();
    void initTitleBarStyle();

    int radius;

    MinimizeButton* btn_min;
    CloseButton* btn_close;
    QLabel* label_logo;
    QLabel* label_logoText;
};

#endif // TITLEBARWIDGET_H

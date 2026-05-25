#ifndef NEWFRIENDMANAGEWIDGET_H
#define NEWFRIENDMANAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QListWidget>

class NewFriendManageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NewFriendManageWidget(int width, int height, QWidget *parent = nullptr);

signals:

private:
    void initWidget();
    void initstyle();

    QLabel* label_friendApply;

    QWidget* widget_friendApply;

    QListWidget* listWidget_friendApplyManage;
};

#endif // NEWFRIENDMANAGEWIDGET_H

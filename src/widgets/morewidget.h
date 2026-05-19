#ifndef MOREWIDGET_H
#define MOREWIDGET_H

#include <QWidget>
#include <QFrame>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>

class MoreWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MoreWidget(QWidget *parent = nullptr);

signals:
    void closed();
protected:
    void focusOutEvent(QFocusEvent* event) override;

private:
    void initMore();
    void initStyle();
};

#endif // MOREWIDGET_H

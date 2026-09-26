#ifndef RATEWIDGET_H
#define RATEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QPainter>
#include <QPainterPath>
#include <QButtonGroup>

class RateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RateWidget(QToolButton* btn, QWidget *parent = nullptr);
    void showAtBtn();

signals:
    void rateSelected(qreal rate);
    void closed();

protected:
    void paintEvent(QPaintEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    void initUI();
    void initStyle();

    QButtonGroup* group;
    QToolButton* btn;
};

#endif // RATEWIDGET_H

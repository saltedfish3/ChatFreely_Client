#include "ratewidget.h"

RateWidget::RateWidget(QToolButton* btn, QWidget *parent)
    : QWidget{parent}, btn(btn)
{
    this->setObjectName("this");

    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setFocusPolicy(Qt::StrongFocus);

    initUI();
    initStyle();
}

void RateWidget::showAtBtn()
{
    if(!this->btn)
        return;

    this->setFixedWidth(this->btn->width());
    this->adjustSize();

    QPoint pos = this->btn->mapToGlobal(QPoint(0, 0));
    pos.setY(pos.y() - this->height() - 6);

    this->move(pos);
    this->show();
    this->activateWindow();
    this->setFocus();
}

void RateWidget::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    this->hide();
    emit closed();
}

void RateWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(QRect(this->rect()), 6, 6);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(30, 30, 30, 200));
    painter.drawPath(path);
}

void RateWidget::initUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(2);

    this->group = new QButtonGroup(this);

    const qreal rates[] = {0.5, 0.75, 1.0, 1.25, 1.5, 2.0};
    for(const qreal& rate : rates)
    {
        auto* btn = new QPushButton(QString::number(rate) + "x", this);
        btn->setFixedHeight(30);
        btn->setCheckable(true);

        this->group->addButton(btn);
        if(qFuzzyCompare(rate, 1.0))
            btn->setChecked(true);

        connect(btn, &QPushButton::clicked, this, [this, rate](){
            emit rateSelected(rate);
            this->hide();
            emit closed();
        });

        layout->addWidget(btn);
    }
}

void RateWidget::initStyle()
{
    this->setStyleSheet(R"(
        #this QPushButton
        {
            background: transparent;
            border: none;
            color: white;
            text-align: center;
            font-size: 12px;
            border-radius: 4px;
        }
        #this QPushButton:hover
        {
            background: #5B9BD5;
        }
        #this QPushButton:pressed
        {
            background: #4A8AC4;
        }
        #this QPushButton:checked {
            background: #5B9BD5;
            color: white;
        }
        #this QPushButton:checked:hover {
            background: #4A8AC4;
        }
    )");
}

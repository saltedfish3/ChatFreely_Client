#include "toastmessage.h"

ToastMessage::ToastMessage(Type type, const QString &msgs, QWidget *parent)
    : QWidget{parent}
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);

    struct ColorTable
    {
        QRgb bg, border, iconColor, textColor;
        QString icon;
    };

    static const QMap<Type, ColorTable> themes = {
        { Type::Success, { qRgba(240,253,244,255), qRgba(134,239,172,255), qRgba(22,163,74,255),  qRgba(21,128,61,255),  "✓" }},
        { Type::Error,   { qRgba(254,242,242,255), qRgba(252,165,165,255), qRgba(220,38,38,255),  qRgba(185,28,28,255),  "✕" }},
        { Type::Info,    { qRgba(239,246,255,255), qRgba(147,197,253,255), qRgba(37,99,235,255),  qRgba(29,78,216,255),  "i" }},
        { Type::Warning, { qRgba(255,251,235,255), qRgba(253,211,77,255),  qRgba(217,119,6,255),  qRgba(180,83,9,255),   "!" }}
    };

    const auto& t = themes[type];
    this->bgColor = QColor(t.bg);
    this->borderColor = QColor(t.border);

    //水平layout
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(14, 10, 10, 10);
    layout->setSpacing(10);

    // 图标圆形背景
    this->icon = new QLabel(t.icon, this);
    this->icon->setFixedSize(22, 22);
    this->icon->setAlignment(Qt::AlignCenter);
    this->icon->setStyleSheet(QString(
                              "background: transparent;"
                              "color: %1;"
                              "font-size: 14px;"
                              "font-weight: bold;"
                                  ).arg(QColor(t.iconColor).name(QColor::HexArgb)));

    this->msg = new QLabel(msgs, this);
    this->msg->setStyleSheet(QString("color: %1; font-size: 13px;").arg(QColor(t.textColor).name(QColor::HexArgb)));
    this->msg->setWordWrap(false);

    layout->addWidget(this->icon);
    layout->addWidget(this->msg, 1);

    adjustSize();
    setFixedHeight(44);
    setMinimumWidth(parent->width() * 0.4);
    setMaximumWidth(parent->width() * 0.6);
    adjustSize();

    // 动画
    auto* opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(1.0);
    setGraphicsEffect(opacityEffect);

    this->fadeIn = new QPropertyAnimation(opacityEffect, "opacity", this);
    this->fadeIn->setDuration(200);
    this->fadeIn->setStartValue(0.0);
    this->fadeIn->setEndValue(1.0);

    this->fadeOut = new QPropertyAnimation(opacityEffect, "opacity", this);
    this->fadeOut->setDuration(200);
    this->fadeOut->setStartValue(1.0);
    this->fadeOut->setEndValue(0.0);
    connect(this->fadeOut, &QPropertyAnimation::finished, this, &QWidget::close);

    this->autoClose = new QTimer(this);
    this->autoClose->setSingleShot(true);
    this->autoClose->setInterval(3000);
    connect(this->autoClose, &QTimer::timeout, this, [this]() {
        this->fadeOut->start();
    });
}

qreal ToastMessage::opacity() const
{
    return this->m_opacity;
}

void ToastMessage::setOpacity(qreal val)
{
    this->m_opacity = val;
    setWindowOpacity(val);
    update();
}

void ToastMessage::showToast()
{
    show();
    this->fadeIn->start();
    this->autoClose->start();
}

void ToastMessage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    //填充背景（无边框）
    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawRoundedRect(rect(), 10, 10);

    //绘制边框（完全内缩 0.5px，防止被窗口边缘切割）
    QPen pen(borderColor, 1);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    //浮点矩形，向内缩 0.5，右边/下边对称缩进
    QRectF borderRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    painter.drawRoundedRect(borderRect, 10, 10);
}



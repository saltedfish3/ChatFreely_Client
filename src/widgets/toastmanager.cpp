#include "toastmanager.h"

ToastManager &ToastManager::getToastManager(bool isMainPage)
{
    //若为true代表是聊天界面
    static ToastManager tm;
    static ToastManager maintm;
    return isMainPage ? maintm : tm;
}

void ToastManager::show(ToastMessage::Type type, const QString &text, QWidget *parent, QWidget* mapping)
{
    auto* toast = new ToastMessage(type, text, parent);

    this->active.removeIf([](ToastMessage* t){
        return !t || !t->isVisible();
    });

    connect(toast, &QObject::destroyed, this, [this, parent, mapping](QObject* obj){
        this->active.removeAll(static_cast<ToastMessage*>(obj));
        relayout(parent, mapping);
    });

    this->active.prepend(toast);
    toast->showToast();
    relayout(parent, mapping);
}

void ToastManager::success(const QString &msg, QWidget *parent, QWidget* mapping)
{
    show(ToastMessage::Success, msg, parent, mapping);
}

void ToastManager::error(const QString &msg, QWidget *parent, QWidget* mapping)
{
    show(ToastMessage::Error, msg, parent, mapping);
}

void ToastManager::info(const QString &msg, QWidget *parent, QWidget* mapping)
{
    show(ToastMessage::Info, msg, parent, mapping);
}

void ToastManager::warning(const QString &msg, QWidget *parent, QWidget* mapping)
{
    show(ToastMessage::Warning, msg, parent, mapping);
}

ToastManager::ToastManager(QObject *parent)
    : QObject{parent}
{}

void ToastManager::relayout(QWidget *parent, QWidget* mapping)
{
    if(!parent)
        return;

    int baseX = 0, baseY = 0;
    int refWidth = parent->width();
    if(mapping)
    {
        QPoint origin = mapping->mapTo(parent, QPoint(0, 0));
        baseX = origin.x();
        baseY = origin.y();
        refWidth = mapping->width();
    }

    int offset = 16;
    for(auto* toast : std::as_const(this->active))
    {
        if(!toast || !toast->isVisible())
            continue;

        int x = baseX + (refWidth - toast->width()) / 2;
        int y = baseY + offset;

        toast->move(x, y);
        offset += toast->height() + 8;
    }
}

#include "toastmanager.h"

ToastManager &ToastManager::getToastManager()
{
    static ToastManager tm;
    return tm;
}

void ToastManager::show(ToastMessage::Type type, const QString &text, QWidget *parent)
{
    auto* toast = new ToastMessage(type, text, parent);

    this->active.removeIf([](ToastMessage* t){
        return !t || !t->isVisible();
    });

    connect(toast, &QObject::destroyed, this, [this, parent](QObject* obj){
        this->active.removeAll(static_cast<ToastMessage*>(obj));
        relayout(parent);
    });

    this->active.prepend(toast);
    toast->showToast();
    relayout(parent);
}

void ToastManager::success(const QString &msg, QWidget *parent)
{
    show(ToastMessage::Success, msg, parent);
}

void ToastManager::error(const QString &msg, QWidget *parent)
{
    show(ToastMessage::Error, msg, parent);
}

void ToastManager::info(const QString &msg, QWidget *parent)
{
    show(ToastMessage::Info, msg, parent);
}

void ToastManager::warning(const QString &msg, QWidget *parent)
{
    show(ToastMessage::Warning, msg, parent);
}

ToastManager::ToastManager(QObject *parent)
    : QObject{parent}
{}

void ToastManager::relayout(QWidget *parent)
{
    if(!parent)
        return;

    int offset = 16;
    for(auto* toast : this->active)
    {
        if(!toast || !toast->isVisible())
            continue;

        int x = (parent->width() - toast->width()) / 2;
        int y = offset;
        toast->move(x, y);
        offset += toast->height() + 8;
    }
}

#include "toastmanager.h"

ToastManager &ToastManager::getToastManager()
{
    static ToastManager tm;
    return tm;
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

    int offset = 16;
    for(auto* toast : std::as_const(this->active))
    {
        if(!toast || !toast->isVisible())
            continue;
        int x = 0;
        if(!mapping)
            x = (parent->width() - toast->width()) / 2;
        else
            x = (parent->width() - mapping->width()) + (mapping->width() - toast->width()) / 2;
        int y = offset;
        toast->move(x, y);
        offset += toast->height() + 8;
    }
}

#include "toastmanager.h"

QSet<QWidget*> ToastManager::set_anchor;
QHash<QWidget*, QPointer<ToastManager>> ToastManager::hash_instances;

void ToastManager::registerAnchor(QWidget *widget)
{
    if(widget)
        set_anchor.insert(widget);
}

ToastManager &ToastManager::getToastManager(QWidget* widget)
{
    static ToastManager fallback(nullptr);

    if(!widget)
        return fallback;

    QWidget* anchor = nullptr;
    for(QWidget* w = widget; w; w = w->parentWidget())
    {
        if(set_anchor.contains(w))
        {
            anchor = w;
            break;
        }
    }

    if(!anchor)
        anchor = widget->window();

    if(!anchor)
        return fallback;

    auto it = hash_instances.find(anchor);
    if(it != hash_instances.end() && !it.value().isNull())
        return *it.value().data();

    ToastManager* tmg = new ToastManager(anchor);
    hash_instances[anchor] = tmg;
    return *tmg;
}

void ToastManager::show(ToastMessage::Type type, const QString &text, QWidget* mapping)
{
    if(!this->anchor)
        return;

    auto* toast = new ToastMessage(type, text, this->anchor);

    this->active.removeIf([](const QPointer<ToastMessage>& t){
        return t.isNull() || !t->isVisible();
    });

    connect(toast, &QObject::destroyed, this, [this, mapping](QObject* obj){
        this->active.removeIf([obj](const QPointer<ToastMessage>& t){
            return t.data() == obj;
        });
        relayout(mapping);
    });

    this->active.prepend(toast);
    toast->showToast();
    relayout(mapping);
}

void ToastManager::success(const QString &msg, QWidget* mapping)
{
    show(ToastMessage::Success, msg, mapping);
}

void ToastManager::error(const QString &msg, QWidget* mapping)
{
    show(ToastMessage::Error, msg, mapping);
}

void ToastManager::info(const QString &msg, QWidget* mapping)
{
    show(ToastMessage::Info, msg, mapping);
}

void ToastManager::warning(const QString &msg, QWidget* mapping)
{
    show(ToastMessage::Warning, msg, mapping);
}

ToastManager::ToastManager(QWidget *anchor)
    : QObject{anchor}, anchor(anchor)
{
    if(anchor)
    {
        connect(anchor, &QObject::destroyed, this, [anchor](){
            set_anchor.remove(anchor);
            hash_instances.remove(anchor);
        });
    }
}

void ToastManager::relayout(QWidget* mapping)
{
    if(!this->anchor)
        return;

    int baseX = 0;
    int baseY = 0;
    int refWidth = this->anchor->width();

    if(mapping && mapping->window() == this->anchor->window())
    {
        QPoint origin = mapping->mapTo(this->anchor, QPoint(0, 0));
        baseX = origin.x();
        baseY = origin.y();
        refWidth = mapping->width();
    }

    int offset = 16;
    for(auto& toast : std::as_const(this->active))
    {
        if(toast.isNull() || !toast->isVisible())
            continue;

        int x = baseX + (refWidth - toast->width()) / 2;
        int y = baseY + offset;

        toast->move(x, y);
        offset += toast->height() + 8;
    }
}

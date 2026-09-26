#ifndef TOASTMANAGER_H
#define TOASTMANAGER_H

#include <QObject>
#include <QPointer>
#include <QList>
#include "toastmessage.h"

class ToastManager : public QObject
{
    Q_OBJECT
public:
    static void registerAnchor(QWidget* widget);
    static ToastManager& getToastManager(QWidget* widget);

    void success(const QString& msg, QWidget* mapping = nullptr);
    void error  (const QString& msg, QWidget* mapping = nullptr);
    void info   (const QString& msg, QWidget* mapping = nullptr);
    void warning(const QString& msg, QWidget* mapping = nullptr);

signals:

private:
    explicit ToastManager(QWidget *anchor = nullptr);
    void show(ToastMessage::Type type, const QString& text, QWidget* mapping = nullptr);
    void relayout(QWidget* mapping = nullptr);

    //当前
    QPointer<QWidget> anchor;
    QList<QPointer<ToastMessage>> active;

    //全局
    static QSet<QWidget*> set_anchor;
    static QHash<QWidget*, QPointer<ToastManager>> hash_instances;
};

#endif // TOASTMANAGER_H

#ifndef TOASTMANAGER_H
#define TOASTMANAGER_H

#include <QObject>
#include "toastmessage.h"

class ToastManager : public QObject
{
    Q_OBJECT
public:
    static ToastManager& getToastManager();

    void show(ToastMessage::Type type, const QString& text, QWidget* parent);
    void success(const QString& msg, QWidget* parent);
    void error  (const QString& msg, QWidget* parent);
    void info   (const QString& msg, QWidget* parent);
    void warning(const QString& msg, QWidget* parent);

signals:

private:
    explicit ToastManager(QObject *parent = nullptr);

    void relayout(QWidget* parent);

    QList<ToastMessage*> active;
};

#endif // TOASTMANAGER_H

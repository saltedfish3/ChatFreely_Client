#ifndef TOASTMANAGER_H
#define TOASTMANAGER_H

#include <QObject>
#include "toastmessage.h"

class ToastManager : public QObject
{
    Q_OBJECT
public:
    static ToastManager& getToastManager(bool isMainPage);

    void show(ToastMessage::Type type, const QString& text, QWidget* parent, QWidget* mapping = nullptr);
    void success(const QString& msg, QWidget* parent, QWidget* mapping = nullptr);
    void error  (const QString& msg, QWidget* parent, QWidget* mapping = nullptr);
    void info   (const QString& msg, QWidget* parent, QWidget* mapping = nullptr);
    void warning(const QString& msg, QWidget* parent, QWidget* mapping = nullptr);

signals:

private:
    explicit ToastManager(QObject *parent = nullptr);

    void relayout(QWidget* parent, QWidget* mapping = nullptr);

    QList<ToastMessage*> active;
};

#endif // TOASTMANAGER_H

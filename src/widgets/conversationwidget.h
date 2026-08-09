#ifndef CONVERSATIONWIDGET_H
#define CONVERSATIONWIDGET_H

#include <QWidget>
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QListView>
#include <QPlainTextEdit>
#include <QMenu>
#include <QScrollBar>
#include <QDateTime>
#include <QTimer>
#include <QUuid>
#include "morewidget.h"
#include "conversationdelegate.h"
#include "../network/tcplongconnection.h"
#include "toastmanager.h"
#include "../utils/userinfo.h"
#include "../chat/conversationitem.h"
#include "messagemodel.h"

class ConversationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConversationWidget(int width, int height, ConversationItem* item, QWidget *parent = nullptr);

    void updateFriendUsername(const QString& username);
    void updateFriendStatus(bool isOnline);
    void setActive(bool isActive);

signals:

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void initStyle();
    void startReFlashTimeStamp();

    QWidget* widget_header;
    QLabel* label_name;
    QLabel* label_statusIcon;
    QLabel* label_statusText;
    QPushButton* btn_more;
    MoreWidget* widget_menu;
    QListView* listView_messages;
    QWidget* widget_editRegion;
    QPlainTextEdit* edit_message;
    QPushButton* btn_send;

    QTimer* timer_loading;
    int loadingCount = 0;
    int loadingAngle = 0;

    MessageModel* model;
    ConversationDelegate* delegate;

    ConversationItem* item;
    int64_t theBestConvSeq = 0;
};

#endif // CONVERSATIONWIDGET_H

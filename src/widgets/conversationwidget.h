#ifndef CONVERSATIONWIDGET_H
#define CONVERSATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListView>
#include <QPlainTextEdit>
#include <QMenu>
#include <QStandardItemModel>
#include <QScrollBar>
#include <QDateTime>
#include <QTimer>
#include <QUuid>
#include "morewidget.h"
#include "conversationdelegate.h"
#include "../network/tcplongconnection.h"
#include "toastmanager.h"

class ConversationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConversationWidget(int width, int height,
                                const QString& username, bool isOnline,
                                const QString& uid, const QPixmap& avatar, QWidget *parent = nullptr);
    void addMessageItem(bool isMyself, const QString& content, int64_t timestamp, QString tempMsgID);
    QString getLastMessage();
    QString getLastMessageTime();
    void updateResp(QString messageID, int64_t timestamp, bool isSuccess);

signals:

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

    QStandardItemModel* model;
    ConversationDelegate* delegate;

    QString uid;
    QPixmap avatar;
};

#endif // CONVERSATIONWIDGET_H

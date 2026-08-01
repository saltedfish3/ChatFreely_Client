#ifndef CONVERSATIONWIDGET_H
#define CONVERSATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListView>
#include <QPlainTextEdit>
#include <QMenu>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QScrollBar>
#include <QDateTime>
#include <QTimer>
#include <QUuid>
#include "morewidget.h"
#include "conversationdelegate.h"
#include "../network/tcplongconnection.h"
#include "toastmanager.h"
#include "../utils/userinfo.h"

class ConversationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConversationWidget(int width, int height,
                                const QString& username, bool isOnline,
                                const QString& friendUID, const QPixmap& friendAvatar, QWidget *parent = nullptr);
    void addMessageItem(bool isMyself, const QString& content, int64_t timestamp, QString messageID, int64_t convSeq = 0);
    QString getLastMessage();
    QString getLastMessageTime();
    void updateResp(bool isSuccess, QString tempMsgID, int64_t timestamp = 0, QString messageID = "", int64_t convSeq = 0);
    void updateFriendAvatar(const QPixmap& avatar);
    void updateFriendUsername(const QString& username);
    void updateFriendStatus(bool isOnline);

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
    QSortFilterProxyModel* sortModel;
    ConversationDelegate* delegate;

    QString friendUID;
    QPixmap friendAvatar;
    int64_t theBestConvSeq = 0;
};

#endif // CONVERSATIONWIDGET_H

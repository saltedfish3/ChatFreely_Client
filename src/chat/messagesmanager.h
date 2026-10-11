#ifndef MESSAGESMANAGER_H
#define MESSAGESMANAGER_H

#include <QObject>
#include <QHash>
#include "message.h"
#include "../database/databasemanager.h"
#include "../utils/imagecachemanager.h"
#include "../utils/videoutils.h"

class MessagesManager : public QObject
{
    Q_OBJECT
public:
    struct Range
    {
        qint64 first;
        qint64 last;
    };

    enum class LoadDir
    {
        Older,
        Newer,
        Around
    };

    explicit MessagesManager(const QString& conversationID, QObject* parent = nullptr);

    void addMessage(const Message& msg, bool isStoreDB = true);
    void addMessages(const QList<Message>& msgs, qint64 convSeqFirst, qint64 convSeqLast, bool isStoreDB = true);

    bool updateMessageStatus(const QString& tempMsgID, const QString& newServerMsgID, Status status, int64_t newTimeStamp, int64_t newConvSeq);
    bool updateMessageContent(const QString& tempMsgID, const QString& content);
    void clearMessages();
    void loadMessages(LoadDir dir, qint64 cursorConvSeq, int limit = 20);
    void jumpToMessage(const QString& msgID, qint64 convSeq);

    Message getLastMessage() const;
    Message getFrontMessage() const;
    const QList<Message>& getMessages() const;

    int indexOfMsg(const QString& msgID) const;
    void removeOfIndex(int index);

    void retryMessage(int index);
    bool cancelUpload(const QString& tempMsgID);
    bool cancelDownload(const QString& msgID);
    void reLoadImage(int index);
    qint64 getNextConvSeq();

    //上传进度
    void registerUpload(const QString& tempID, const QString& filePath);
    void unregisterUpload(const QString& tempID);
    int getUploadProgres(const QString& filePath) const;

    //下载进度
    void registerDownload(const QString& msgID, const QString& url);
    void unregisterDownload(const QString& msgID);
    int getDownloadProgress(const QString& msgID) const;
    bool isDownloaded(const QString& msgID);
    bool isDownloading(const QString& url) const;
    void setDownloadExists(const QString& url, bool exists);

    bool isMediaExpired(const QString& url) const;

    //ranges
    const QList<Range>& getRanges() const;
    int indexOfConvSeqOrAfter(qint64 convSeq) const;
    int indexOfConvSeqOrBefore(qint64 convSeq) const;
    int findRangeIndexByconvSeq(qint64 convSeq) const;

    //高亮
    void setHighLightMsgID(const QString& msgID);
    QString getHighLightMsgID() const;

signals:
    void messageAdd(int row);
    void messagesAdd(int first, int end);
    void messageUpdate(int row);
    void messageRemove(int row);
    void messageMove(int oldRow, int newRow);
    void resetModel();
    void uploadProgressUpdate(const QString& tempID, const QString& filePath, int percent);
    void mediaExpired(const QString& url);
    void mediaAvailable(const QString& url);

    void downloadProgressUpdate(const QString& msgID, const QString& url, int percent);
    void updateDownloadStatus(const QString& url);

    //通知item
    void startLoadingImage();
    void finishLoadedImage();
    void messagesInserted();
    void loadMessagesFinished(MessagesManager::LoadDir dir);

    //跳转
    void jumpToFinished(int row, const QString& msgID);
    void jumpToFailed(const QString& msgID);

    //高亮
    void highLightChanged(const QString& oldMsgID, const QString& newMsgID);

private:
    QList<Message> messages;
    QHash<QString, int> index_message;//消息索引(消息ID，行号)

    QString conversationID;
    qint64 lastConvSeq = 0;

    QList<Range> ranges;
    quint64 loadGen_older = 0;
    quint64 loadGen_newer = 0;
    quint64 loadGen_around = 0;
    quint64 loadGen_jump = 0;

    QString highLightMsgID;
    QTimer* timer_highLight;

    //上传
    QHash<QString, QString> hash_tempIDToFilePath;//tempID - filePath
    QHash<QString, QList<QString>> hash_registerUploadID;//filePath - [tempID]
    QHash<QString, int> hash_uploadProgress;//filePath - percent

    //下载
    QHash<QString, QString> hash_tempIDToDownloadUrl;//msgID - Url
    QHash<QString, QList<QString>> hash_registerDownloadID;//Url - [msgID]
    QHash<QString, int> hash_downloadProgress;//Url - percent
    QSet<QString> set_downloadFileExists;//url

    QSet<QString> set_expiredMediaUrl;
    QSet<QString> set_checkingMediaUrl;

    int findInsertIndex(qint64 convSeq) const;
    void saveMoveMessage(int row);
    void calcShowTimestamp(int row);
    void addIndex(const Message& msg, int index);
    void rebuildIndex();

    void checkMediaExpiredStatus(const Message& msg);

    void updateRanges(qint64 convSeqFirst, qint64 convSeqLast);
};

#endif // MESSAGESMANAGER_H

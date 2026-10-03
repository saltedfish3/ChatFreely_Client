#ifndef CHATTEXTEDIT_H
#define CHATTEXTEDIT_H

#include <QTextEdit>
#include <QMimeData>
#include <QImage>
#include <QDir>
#include <QUuid>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QCryptographicHash>
#include <QTextBlock>
#include "../utils/GlobalVariable.h"
#include "../utils/imagecachemanager.h"
#include "../chat/message.h"

class ChatTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    enum EditProperty
    {
        UrlPro = QTextFormat::UserProperty + 1,
        TypePro,
        SizePro,
        ThumbnailUrlPro
    };

    struct MessageBlock
    {
        ContentType type;
        QString content;
        QString thumbnailUrl;
        QString tempID;
    };

    explicit ChatTextEdit(QWidget *parent = nullptr);
    void saveBlocks();
    bool hasBlocks();
    MessageBlock nextBlock();
    QList<MessageBlock>& getAllBlocks();

    void insertFileToEdit(const QString& filePath, ContentType type);

protected:
    bool canInsertFromMimeData(const QMimeData* source) const override;
    void insertFromMimeData(const QMimeData* source) override;

signals:

private:
    void insertImageToEdit(const QImage& image, const QString& url, const QString& thumbnailUrl);
    QImage addRoundedAndPadding(const QImage& pic, int radius, int padding, qreal dpr);

    QList<MessageBlock> blocks;

};

#endif // CHATTEXTEDIT_H

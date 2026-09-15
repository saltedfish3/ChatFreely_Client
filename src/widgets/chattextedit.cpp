#include "chattextedit.h"

ChatTextEdit::ChatTextEdit(QWidget *parent)
    : QTextEdit{parent}
{}

void ChatTextEdit::saveBlocks()
{
    QTextDocument* doc = document();
    QTextBlock block = doc->begin();
    QString text;

    for(; block.isValid(); block = block.next())
    {
        for(QTextBlock::iterator it = block.begin(); !it.atEnd(); it++)
        {
            QTextFragment fragment = it.fragment();
            if(!fragment.isValid())
                continue;

            QTextCharFormat fmt = fragment.charFormat();
            QString fragText = fragment.text();
            if(fmt.isImageFormat())
            {
                if(!text.trimmed().isEmpty())
                {
                    MessageBlock b;
                    b.type = ContentType::Text;
                    b.content = text.trimmed();
                    text.clear();
                    this->blocks.append(b);
                }

                QTextImageFormat imgFmt = fmt.toImageFormat();
                QString url = imgFmt.property(UrlPro).toString();
                if(url.isEmpty())
                    url = imgFmt.name();

                MessageBlock b;
                b.type = ContentType::Image;
                b.content = url;
                this->blocks.append(b);
            }
            else
                text += fragText;
        }

        if(block.next().isValid())
            text += '\n';
    }

    //处理末尾文字
    if(!text.trimmed().isEmpty())
    {
        MessageBlock block;
        block.type = ContentType::Text;
        block.content = text.trimmed();
        this->blocks.append(block);
    }
}

bool ChatTextEdit::hasBlocks()
{
    return !this->blocks.isEmpty();
}

QList<ChatTextEdit::MessageBlock>& ChatTextEdit::getAllBlocks()
{
    return this->blocks;
}

ChatTextEdit::MessageBlock ChatTextEdit::nextBlock()
{
    return this->blocks.takeFirst();
}

bool ChatTextEdit::canInsertFromMimeData(const QMimeData *source) const
{
    return source->hasImage();
}

void ChatTextEdit::insertFromMimeData(const QMimeData *source)
{
    if(source->hasImage())
    {
        QImage image = qvariant_cast<QImage>(source->imageData());
        if(image.isNull())
            return;

        //载入缓存
        QString filename = "local://" + QUuid::createUuid().toString();
        ImageCacheManager::getManager().insertCache(filename, QPixmap::fromImage(image));

        insertImageToEdit(image, filename);
    }
    else
        QTextEdit::insertFromMimeData(source);
}

void ChatTextEdit::insertImageToEdit(const QImage& image, const QString& url)
{
    const int maxWidth = this->width() * 0.5;
    const int maxHeight = this->height() * 0.7;

    QSize tempSize = image.size();
    if(tempSize.width() > maxWidth || tempSize.height() > maxHeight)
        tempSize.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);

    QImage scaled = image.scaled(tempSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    qreal dpr = this->devicePixelRatioF();

    scaled = addRoundedAndPadding(scaled, 5, 1, dpr);

    QPixmap pm = QPixmap::fromImage(scaled);
    pm.setDevicePixelRatio(dpr);

    QString tempUrl = "temp://image_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    document()->addResource(QTextDocument::ImageResource, QUrl(tempUrl), pm);

    QTextCursor cursor = textCursor();
    QTextImageFormat format;
    format.setName(tempUrl);
    format.setWidth(scaled.width() / dpr);
    format.setHeight(scaled.height() / dpr);

    format.setProperty(UrlPro, url);

    cursor.insertImage(format);
}

QImage ChatTextEdit::addRoundedAndPadding(const QImage &pic, int radius, int padding, qreal dpr)
{
    if(pic.isNull())
        return QImage();

    QSize logicalSize = pic.size() + QSize(2 * padding, 2 * padding);
    QSize physicalSize = logicalSize * dpr;

    const int ss = 2;
    QImage mask(physicalSize * ss, QImage::Format_ARGB32_Premultiplied);
    mask.fill(Qt::transparent);
    {
        QPainter maskPainter(&mask);
        maskPainter.setRenderHint(QPainter::Antialiasing);
        QRectF maskRect(padding * dpr * ss, padding * dpr * ss,
                        pic.width() * dpr * ss, pic.height() * dpr * ss);
        QPainterPath path;
        path.addRoundedRect(maskRect, radius * dpr * ss, radius * dpr * ss);
        maskPainter.fillPath(path, Qt::black);
    }
    mask = mask.scaled(physicalSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QImage picScaledToDpr = pic.scaled(pic.size() * dpr, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                                .convertToFormat(QImage::Format_ARGB32_Premultiplied);

    QImage result(physicalSize, QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawImage(QPointF(padding * dpr, padding * dpr), picScaledToDpr);

    //合成模式
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawImage(0, 0, mask);
    painter.end();

    result.setDevicePixelRatio(dpr);
    return result;
}

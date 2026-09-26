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

                int typeInt = imgFmt.property(TypePro).toInt();
                ContentType type = (typeInt == 0) ? ContentType::Image : static_cast<ContentType>(typeInt);

                MessageBlock b;
                b.type = type;
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

void ChatTextEdit::insertFileToEdit(const QString &filePath, ContentType type)
{
    if(type == Image)
    {
        QImage image(filePath);
        if(image.isNull())
            return;

        QString filename = "local://" + QUuid::createUuid().toString();
        ImageCacheManager::getManager().insertCache(filename, QPixmap::fromImage(image), 1.0);
        insertImageToEdit(image, filename);
        return;
    }

    //视频及其它文件
    QFileInfo info(filePath);
    QString fileName = info.fileName();
    qint64 fileSize = info.size();

    const int cardWidth = 250;
    const int cardHeight = 60;
    const int padding = 10;
    const int cardRadius = 8;
    const QSize iconSize(36, 36);

    qreal dpr = this->devicePixelRatioF();

    QPixmap card(cardWidth * dpr, cardHeight * dpr);
    card.setDevicePixelRatio(dpr);
    card.fill(Qt::transparent);

    {
        QPainter painter(&card);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addRoundedRect(QRectF(0, 0, cardWidth, cardHeight).adjusted(2, 2, -2, -2), cardRadius, cardRadius);
        painter.fillPath(path, QColor(229, 231, 235));

        //绘制图标
        int textX = padding + 10;
        int rightMargin = padding + 10;
        int textWidth = cardWidth - textX - rightMargin - iconSize.width();

        QFont font = this->font();
        font.setPointSize(9);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(QColor(55, 65, 81));

        QFontMetrics fm(font);
        QString showName = fm.elidedText(fileName, Qt::ElideMiddle, textWidth);
        painter.drawText(QRect(textX, 8, textWidth, 20), Qt::AlignVCenter | Qt::AlignLeft, showName);

        QString sizeText;
        if(fileSize < 1024)
            sizeText = QString::number(fileSize) + " B";
        else if(fileSize < 1024 * 1024)
            sizeText = QString::number(fileSize / 1024.0, 'f', 2) + " KB";
        else if(fileSize < 1024LL * 1024 * 1024)
            sizeText = QString::number(fileSize / 1024.0 / 1024.0, 'f', 2) + " MB";
        else
            sizeText = QString::number(fileSize / 1024.0 / 1024.0 / 1024.0, 'f', 2) + " GB";

        font.setBold(false);
        font.setPointSize(8);
        painter.setFont(font);
        painter.setPen(QColor(107, 114, 128));
        painter.drawText(QRectF(textX, cardHeight - 26, textWidth, 18), Qt::AlignVCenter | Qt::AlignLeft, sizeText);

        QPixmap icon(":/default/images/showFile.png");
        icon = icon.scaled(iconSize * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        icon.setDevicePixelRatio(dpr);

        QRect iconRect(textX + textWidth, (cardHeight - iconSize.height()) / 2, iconSize.width(), iconSize.height());
        QPoint drawPos(iconRect.x() + (iconRect.width() - icon.width()) / 2, iconRect.y() + (iconRect.height() - icon.height()) / 2 + 4);
        painter.drawPixmap(drawPos, icon);
    }

    QString tempUrl = "temp://file_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    document()->addResource(QTextDocument::ImageResource, QUrl(tempUrl), card);

    QTextCursor cursor = textCursor();
    QTextImageFormat format;
    format.setName(tempUrl);
    format.setWidth(cardWidth);
    format.setHeight(cardHeight);

    format.setProperty(UrlPro, filePath);
    format.setProperty(TypePro, static_cast<int>(type));
    format.setProperty(SizePro, fileSize);

    cursor.insertImage(format);

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
        ImageCacheManager::getManager().insertCache(filename, QPixmap::fromImage(image), 1.0);

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
    format.setProperty(TypePro, static_cast<int>(Image));

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

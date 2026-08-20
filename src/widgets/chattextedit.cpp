#include "chattextedit.h"

ChatTextEdit::ChatTextEdit(QWidget *parent)
    : QTextEdit{parent}
{}

bool ChatTextEdit::canInsertFromMimeData(const QMimeData *source) const
{
    return source->hasImage();
}

void ChatTextEdit::insertFromMimeData(const QMimeData *source)
{
    if(source->hasImage())
    {
        QImage image = qvariant_cast<QImage>(source->imageData());
        QString imagePath = QDir::tempPath() + "/chat_image_" + QUuid::createUuid().toString() + ".png";
        image.save(imagePath);

        insertImageToEdit(imagePath);
    }
    else
        QTextEdit::insertFromMimeData(source);
}

void ChatTextEdit::insertImageToEdit(const QString &imagePath)
{
    QImage temp(imagePath);
    if(temp.isNull())
        return;

    const int maxWidth = this->width() * 0.5;
    const int maxHeight = this->height() * 0.7;

    QSize tempSize = temp.size();
    if(tempSize.width() > maxWidth || tempSize.height() > maxHeight)
        tempSize.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);

    QImage scaled = temp.scaled(tempSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    qreal dpr = this->devicePixelRatioF();

    scaled = addRoundedAndPadding(scaled, 5, 1, dpr);
    QString scaledPath = QDir::tempPath() + "/chat_image_" + QUuid::createUuid().toString() + ".png";
    scaled.save(scaledPath);

    QPixmap pm = QPixmap::fromImage(scaled);
    pm.setDevicePixelRatio(dpr);
    document()->addResource(QTextDocument::ImageResource, QUrl::fromLocalFile(scaledPath), pm);

    QTextCursor cursor = textCursor();
    QTextImageFormat format;
    format.setName(QUrl::fromLocalFile(scaledPath).toString());
    format.setWidth(scaled.width() / dpr);
    format.setHeight(scaled.height() / dpr);
    cursor.insertImage(format);

    this->list_normalImagePaths.append(scaledPath);
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

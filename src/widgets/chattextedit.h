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

class ChatTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit ChatTextEdit(QWidget *parent = nullptr);

protected:
    bool canInsertFromMimeData(const QMimeData* source) const override;
    void insertFromMimeData(const QMimeData* source) override;

signals:

private:
    void insertImageToEdit(const QString& imagePath);
    QImage addRoundedAndPadding(const QImage& pic, int radius, int padding, qreal dpr);
    QList<QString> list_normalImagePaths;

};

#endif // CHATTEXTEDIT_H

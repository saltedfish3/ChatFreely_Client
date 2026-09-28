#ifndef IMAGEPREVIEWWIDGET_H
#define IMAGEPREVIEWWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include "imageview.h"
#include "closebutton.h"
#include "minimizebutton.h"
#include "titlebarwidget.h"
#include "../utils/imagecachemanager.h"
#include "toastmanager.h"
#include "../network/tcplongconnection.h"

class ImagePreviewWidget : public QWidget
{
    Q_OBJECT
public:
    enum class Edge
    {
        Left,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        None
    };
    static ImagePreviewWidget& getPreviewWidget();
    ImagePreviewWidget& operator=(const ImagePreviewWidget&) = delete;
    ImagePreviewWidget(const ImagePreviewWidget&) = delete;

    void setPixmap(const QPixmap& pix);
    void setPixmapUrl(const QString& url);

signals:

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* obj,QEvent* ev) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    explicit ImagePreviewWidget(int width = 600, int height = 800, QWidget *parent = nullptr);
    ImagePreviewWidget::Edge edgeAt(const QPoint& pos);
    void updateCursor(Edge edge);
    void updateSaveButton();

    void initStyle();

    Edge edge = Edge::None;
    QPoint pos_startScaleGlobal;
    QRect rect_startGeometry;

    TitleBarWidget* widget_titleBar;
    QPushButton* btn_rotateLeft;
    QPushButton* btn_rotateRight;
    QPushButton* btn_zoomIn;
    QPushButton* btn_zoomOut;
    QPushButton* btn_save;
    QPoint pos_widget;

    bool isSaved = false;
    QString pixmapUrl;

    ImageView* view;
};

#endif // IMAGEPREVIEWWIDGET_H

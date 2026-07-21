#ifndef NEWFRIENDBUTTON_H
#define NEWFRIENDBUTTON_H

#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QRect>
#include <QPoint>

class NewFriendButton : public QPushButton
{
    Q_OBJECT
public:
    explicit NewFriendButton(int width, int height, QWidget *parent = nullptr);

    void setUnProcessedRequests(size_t num);
signals:

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void nextCheckState() override;
    void paintEvent(QPaintEvent* event) override;

private:
    void initButton();
    void initStyle();

    QLabel* label_newFriend;
    QLabel* label_Icon;
    QLabel* label_checkApply;
    QLabel* label_arrow;

    size_t unProcessedRequests = 0;
};

#endif // NEWFRIENDBUTTON_H

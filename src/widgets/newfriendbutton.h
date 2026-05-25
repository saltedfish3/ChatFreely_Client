#ifndef NEWFRIENDBUTTON_H
#define NEWFRIENDBUTTON_H

#include <QPushButton>
#include <QLabel>

class NewFriendButton : public QPushButton
{
    Q_OBJECT
public:
    explicit NewFriendButton(int width, int height, QWidget *parent = nullptr);

    void setNotRead(size_t num);
signals:

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void nextCheckState() override;

private:
    void initButton();
    void initStyle();

    QLabel* label_newFriend;
    QLabel* label_Icon;
    QLabel* label_checkApply;
    QLabel* label_arrow;
    QLabel* label_notRead;
};

#endif // NEWFRIENDBUTTON_H

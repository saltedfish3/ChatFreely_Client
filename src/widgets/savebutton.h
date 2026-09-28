#ifndef SAVEBUTTON_H
#define SAVEBUTTON_H

#include <QToolButton>
#include <QPainter>

class SaveButton : public QToolButton
{
    Q_OBJECT
public:
    enum class State
    {
        None,
        Downloading,
        Paused,
        Finished
    };
    explicit SaveButton(QWidget *parent = nullptr);
    void setState(State state);
    State getState() const;

    void setProgress(int percent);
    int getProgress() const;

signals:

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    State currentState = State::None;
    int currentProgress = 0;

    bool isHovered = false;

    QIcon icon_normal;
    QIcon icon_finished;
};

#endif // SAVEBUTTON_H

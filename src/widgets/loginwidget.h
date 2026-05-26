#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include "../utils/GlobalVariable.h"

class LoginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoginWidget(int width, int height, QWidget *parent = nullptr);

signals:
    void changeWidget(GlobalVariable::MainPage id);

private slots:

    void onButtonPressed();
    void onButtonReleased();

    void sendSignalsChangeToRegister();

private:
    void initWidget();
    void initLoginStyle();

    QLabel* label_loginEmail;
    QLabel* label_loginPassd;
    QLabel* label_welcomeBack;
    QLabel* label_welcomeBack2;
    QLabel* label_noAccount;
    QLabel* label_version;

    QLineEdit* edit_loginEmail;
    QLineEdit* edit_loginPassd;

    QPushButton* btn_loginNow;
    QPushButton* btn_forgetPassd;
    QPushButton* btn_goRegisterNow;
};

#endif // LOGINWIDGET_H

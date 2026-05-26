#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QPropertyAnimation>
#include "../utils/GlobalVariable.h"

class RegisterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RegisterWidget(int width, int height, QWidget *parent = nullptr);

signals:
    void changeWidget(GlobalVariable::MainPage id);

private slots:
    void onButtonPressed();
    void onButtonReleased();

    void sendSignalChangeToLogin();

private:
    void initWidget();
    void initRegisterSytle();

    QLabel* label_createAccount;
    QLabel* label_joinApp;
    QLabel* label_regUsername;
    QLabel* label_regEmail;
    QLabel* label_regPassd;
    QLabel* label_regConfirmPassd;
    QLabel* label_haveAccount;

    QLineEdit* edit_regEmail;
    QLineEdit* edit_regPassd;
    QLineEdit* edit_regConfirmPassd;
    QLineEdit* edit_regUsername;

    QPushButton* btn_goLoginNow;
    QPushButton* btn_registerNow;

    QPropertyAnimation* animation_btn_registerNow;
};

#endif // REGISTERWIDGET_H

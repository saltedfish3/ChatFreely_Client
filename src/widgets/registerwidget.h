#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QStyle>
#include <QRegularExpressionValidator>
#include "../utils/GlobalVariable.h"
#include "loadinglabel.h"
#include "../network/tcplongconnection.h"

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

    void showError(QLineEdit* edit, QLabel* label, const QString& msg);
    void clearError(QLineEdit* edit, QLabel* label);
    void getRegisterState(bool isSuccess ,QString from, QString info);

private:
    void initWidget();
    void initRegisterSytle();
    //顶部错误提示
    void showToast(const QString& msg, bool isSuccess = false);

    QLabel* label_createAccount;
    QLabel* label_joinApp;
    QLabel* label_regUsername;
    QLabel* label_regEmail;
    QLabel* label_regPassd;
    QLabel* label_regConfirmPassd;
    QLabel* label_emailError;
    QLabel* label_passdError;
    QLabel* label_usernameError;
    QLabel* label_confirmPError;
    QLabel* label_haveAccount;

    //顶部错误提示悬浮标签
    QLabel* label_toast;
    QTimer* timer_toast;

    //一定时间跳转到登录界面
    QTimer* timer_jump;

    QLineEdit* edit_regEmail;
    QLineEdit* edit_regPassd;
    QLineEdit* edit_regConfirmPassd;
    QLineEdit* edit_regUsername;

    QPushButton* btn_goLoginNow;
    QPushButton* btn_registerNow;
    LoadingLabel* label_loading;

    QPropertyAnimation* animation_btn_registerNow;
    QPropertyAnimation* animation_loading;
};

#endif // REGISTERWIDGET_H

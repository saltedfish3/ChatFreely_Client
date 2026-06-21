#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QStyle>
#include <QPropertyAnimation>
#include <QRegularExpressionValidator>
#include <QStyle>
#include <QTimer>
#include "../network/tcplongconnection.h"
#include "loadinglabel.h"
#include "../utils/GlobalVariable.h"
#include "toastmanager.h"

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

    void getLoginState(bool isSuccess ,QString from, QString info);
private:
    void initWidget();
    void initLoginStyle();
    void showError(QLineEdit* edit, QLabel* label, const QString& msg);
    void clearError(QLineEdit* edit, QLabel* label);
    //顶部错误提示
    void showToast(const QString& msg, bool isSuccess = false);

    QLabel* label_loginEmail;
    QLabel* label_loginPassd;
    QLabel* label_emailError;
    QLabel* label_passdError;
    QLabel* label_welcomeBack;
    QLabel* label_welcomeBack2;
    QLabel* label_noAccount;
    QLabel* label_version;

    //顶部错误提示悬浮标签
    QLabel* label_toast;
    QTimer* timer_toast;


    QLineEdit* edit_loginEmail;
    QRegularExpressionValidator* regular_email;
    QLineEdit* edit_loginPassd;
    QRegularExpressionValidator* regular_password;

    QPushButton* btn_loginNow;
    LoadingLabel* label_loading;
    QTimer* timer_jump;

    QPushButton* btn_forgetPassd;
    QPushButton* btn_goRegisterNow;

    QPropertyAnimation* animation_btn_loginNow;
    QPropertyAnimation* animation_loading;
};

#endif // LOGINWIDGET_H

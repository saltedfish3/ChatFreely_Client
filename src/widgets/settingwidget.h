#ifndef SETTINGWIDGET_H
#define SETTINGWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QCheckBox>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QTimer>
#include <thread>
#include <atomic>
#include <QMessageBox>
#include <chrono>
#include "../utils/GlobalVariable.h"

class SettingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingWidget(int width, int height, QWidget *parent = nullptr);
    ~SettingWidget();
signals:

private:

    void setRadius(QIcon pic, QPushButton* btn, int hei_wid);

    void initWidget();
    void initThisStyle();

    void initPersonalDataWidget();
    void initSystemDataWidget();

    std::atomic<bool> is_stop;
    std::thread occupy_worker;

    QScrollArea* scrollArea;
    QWidget* widget_holy;
    QLabel* label_settingTitle;
    QLabel* label_settingLittleTitle;

    QWidget* widget_personalData;
    QLabel* label_personalData_icon;
    QLabel* label_personalData;
    QPushButton* btn_avatar;
    QLabel* label_camera_icon;
    QLabel* label_changeAvatar;
    QLabel* label_username;
    QLabel* label_userID;
    QLabel* label_email;
    QLineEdit* edit_username;
    QLineEdit* edit_userID;
    QLineEdit* edit_email;

    QWidget* widget_systemData;
    QLabel* label_system_setting;
    QLabel* label_system_setting_icon;
    QLabel* label_bigTitle_fileSave;
    QLabel* label_bigTitle_chatRecord;

    QLabel* label_fileSavePos;
    QLineEdit* edit_fileSavePos;
    QPushButton* btn_changeFileSavePos;
    QLabel* label_chatRecordSavePos;
    QLineEdit* edit_chatRecordSavePos;
    QPushButton* btn_changechatRecordSavePos;
    QLabel* label_saveSpaceManage;
    QLabel* label_occupyNow;
    QLabel* label_occupyNumber;
    QPushButton* btn_clearOccupy;

    QFrame* line;

    QPushButton* btn_UnLoginMyself;
    QPushButton* btn_cancel;
    QPushButton* btn_saveAllChange;

    QLabel* label_version;
};

#endif // SETTINGWIDGET_H

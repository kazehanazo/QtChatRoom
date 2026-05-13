#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#pragma once

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

signals:
    void changePassword();   // 修改密码信号
    void logout();           // 退出登录信号

private slots:
    void onChangePwdPushButtonClicked();
    void onLogoutPushButtonClicked();

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H

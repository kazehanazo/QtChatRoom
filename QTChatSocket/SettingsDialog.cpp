#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent): QDialog(parent), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    connect(ui->changePwdPushButton, &QPushButton::clicked, this, &SettingsDialog::onChangePwdPushButtonClicked);
    connect(ui->logoutPushButton, &QPushButton::clicked, this, &SettingsDialog::onLogoutPushButtonClicked);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::onChangePwdPushButtonClicked()
{
    emit changePassword();
}

void SettingsDialog::onLogoutPushButtonClicked()
{
    emit logout();
}



#ifndef LOGIN_H
#define LOGIN_H

#pragma once

#include <QDialog>
#include <QEvent>
#include <QMouseEvent>
#include <QMovie>
#include <QLabel>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QScreen>
#include <QTcpSocket>
#include <QSettings>

#include "DisplayAnimation.h"
#include "SocketBusiness.h"
#include "UserInfoSDK.h"
#include "Register.h"
#include "MyMainWindow.h"

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT
public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

private slots:
    void onLoginSuccess(const UserInfo& info);
    void onLoginFailed(const QString& reason);
    void onUserEdited(const QString& user);
    void onLoginButtonClicked();
    void onRegisterButtonClicked();
    void on_userCloseToolButton_clicked();
    void onCloseToolButtonClicked();

signals:

protected:
    void showEvent(QShowEvent *event) override;
    //void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent  *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void loadUsers();

    Ui::Login *ui;
    QString host;
    int port;
    QPoint mDragPos;
    bool mDragging = false;
    QLabel *gifLabel = nullptr;
    QMovie *movie = nullptr;
    Register *mRegister = nullptr;
    DisplayAnimation *display;
    bool isClosed = false;
    bool mTokenLoginEnabled = true;
    struct loginUserInfo
    {
        int userId;
        QString username;
        QString token;
        QByteArray profile;
        qint64 lastLoginTime;
    };
    QList<loginUserInfo> userList;

};


#endif // LOGIN_H

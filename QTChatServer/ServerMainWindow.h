#ifndef SERVERMAINWINDOW_H
#define SERVERMAINWINDOW_H

#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QWidget>
#include <QString>
#include <QDate>
#include "ChatServer.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class ServerMainWindow;
}
QT_END_NAMESPACE

class ServerMainWindow : public QWidget
{
    Q_OBJECT

public:
    ServerMainWindow(QWidget *parent = nullptr);
    ~ServerMainWindow();

private slots:
    void appendMsg(const QString& msg);
    void onUserOnline(const QString &username);
    void onUserOffline(const QString &username);

    void on_serverONButton_clicked();
    void on_serverOFFButton_clicked();

signals:
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    ChatServer server;
    QListWidget *userList;
    Ui::ServerMainWindow *ui;
};
#endif // SERVERMAINWINDOW_H

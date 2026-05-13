#ifndef CHATSERVER_H
#define CHATSERVER_H

#pragma once
#include <QObject>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QThread>
#include <QDebug>
#include <QHash>
#include <QMap>
#include <QTimer>

#include "ThreadPool.h"
#include "SqlConnPool.h"
#include "Log.h"
#include "ChatSocket.h"

struct clientInfo
{
    int userId;
    QString username;
    ChatSocket* client;
    QTcpSocket* socket;
    QDateTime lastActive;
};

class ChatServer: public QTcpServer
{
    Q_OBJECT
public:
    ChatServer(QObject *parent = nullptr);
    bool start(QString host, quint16 port);
    void stop();
protected:
    void incomingConnection(qintptr socketDescriptor) override; //重写QTcpServer中的连接虚函数

private slots:
    void checkTimeoutClients();
    void userOnline(int userId, const QString &username, QTcpSocket* socket);
    void userOffline(int userId, const QString &username, QTcpSocket* socket);
    void addUserRequest(int userId, int friendId, const QString &username, const QByteArray &profile, const QString &leaveMsg);
    void friendApplyResult(int userId, int friendId, qint64 timeMs, const QString &status);
    void acceptFriendApplyMsg(int userId, int friendId, const QString &leaveMsg, const QString &replyMsg, qint64 timeMs1, qint64 timeMs2);
    void userInfoUpdated(int userId, const QString& username, const QByteArray& profile, QList<int> userIdList);
    void deletedFriend(int userId, int friendId);
    void newMessage(int friendId, int sendId, int type, const QString &content, qint64 timeMs);

signals:
    void timeoutClient(const QString &msg);
    void userListChanged(const QStringList &users);
    void checkLogin(const QString &msg, const QString &username, QTcpSocket* socket);
    void checkRegister(const QString &msg, const QString &username, QTcpSocket* socket);
    void onUserOnline(const QString &username);
    void onUserOffline(const QString &username);


private:
    QHash<qintptr, ChatSocket*> pendingClients; // 存储尚未登录的客户端
    QMap<int, clientInfo> activeClients; // 存储已登录的客户端
    QTimer* clientTimer;
    QString mHost;
    quint16 mPort;
    const int TIMEOUT_MS = 180000;
};

#endif // CHATSERVER_H

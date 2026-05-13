#ifndef CHATSOCKET_H
#define CHATSOCKET_H

#pragma once
#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QPointer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QUuid>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <atomic>

#include "ThreadPool.h"
#include "SqlConnPool.h"
#include "SqlConnRAII.h"
#include "Log.h"

class ChatSocket: public QObject
{
    Q_OBJECT
public:
    explicit ChatSocket(qintptr socketDescriptor, QObject* parent = nullptr);
    ~ChatSocket();
    void start();

public slots:
    void responeHandle(const QByteArray &respone);

private slots:
    void readyReadHandle();
    void sendUserOnline(int userId, const QString &username);
    void sendAddUserRequest(int userId, int friendId, const QString &username, const QByteArray &profile, const QString &leaveMsg);
    void sendAcceptFriendApply(int userId, int friendId, const QString &leaveMsg, const QString &replyMsg, qint64 timeMs1, qint64 timeMs2);
    void sendRejectFriendApply(int userId, int friendId, qint64 timeMs1);
    void sendUserInfoUpdated(int userId, const QString &username, const QByteArray &profile, QList<int> userIdList);
    void sendDeletedFriend(int userId, int friendId);
    void sendNewMessage(int friendId, int sendId, int type, const QString &content, qint64 timeMs);
    void disConnection();

signals:
    void userOnline(int userId, const QString &username, QTcpSocket* socket);
    void userOffline(int userId, const QString &username, QTcpSocket* socket);
    void addUserRequest(int userId, int friendId, const QString &username, const QByteArray &profile, const QString &leaveMsg);
    void friendApplyResult(int userId, int friendId, qint64 timeStamp, const QString &status);
    void acceptFriendApplyMsg(int userId, int friendId, const QString &leaveMsg, const QString &replyMsg, qint64 timeMs1, qint64 timeMs2);
    void userInfoUpdatedBroadcast(int userId, const QString &username, const QByteArray &profile, QList<int> userIdList);
    void deletedFriend(int userId, int friendId);
    void newMessage(int friendId, int sendId, int type, const QString &content, qint64 timeMs);

private:
    QByteArray createSalt(int length);
    QByteArray createHash(const QByteArray &pwd);
    bool verifyPassword(const QString &pwd, const QByteArray &salt, const QByteArray &dbPwd);
    QByteArray createToken(const QString &username);

    void processBusiness(const QJsonDocument &doc);
    QByteArray checkLogin(const QString &user, const QString &pwd);
    QByteArray checkTokenLogin(const QString &username, const QString &token);
    QByteArray checkRegister(const QString &username, const QString &phonenumber, const QString &pwd);
    QByteArray checkChangePassword(int userId, const QString &oldPwd, const QString &newPwd);
    QByteArray checkSearchUser(const QString &keyword);
    QByteArray checkAddUser(int userId, int friendId, const QString &leaveMsg);
    QByteArray checkAcceptFriendApply(int friendId, int userId);
    QByteArray checkRejectFriendApply(int friendId, int userId);
    QByteArray checkDeleteUser(int userId, int friendId);
    QByteArray checkUpdatedUserInfo(int userId, const QString &username, const QByteArray &profile);
    QByteArray checkNewMessage(int chatId, int sendId, int type, const QString &content);

    void pullFrinedList(int userId);
    void pullFriendApply(int userId);
    void pullOfflineMessage(int userId);

    QTcpSocket *mSocket;
    qintptr descriptor;
    QByteArray mBuffer;
    std::atomic<int> mUserId = -1;
    QString mUsername;
};

#endif // CHATSOCKET_H


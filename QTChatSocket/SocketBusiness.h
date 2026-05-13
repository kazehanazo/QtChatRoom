#ifndef SOCKETBUSINESS_H
#define SOCKETBUSINESS_H

#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "UserInfoSDK.h"
#include "MessageStore.h"
#include "ChatStore.h"
#include "FriendApplyStore.h"

class SocketBusiness : public QObject
{
    Q_OBJECT
public:
    static SocketBusiness& instance();

    // 连接服务器
    void connectToServer();
    // 退出服务器
    void disconnectFromServer();
    // 发送登录请求
    void sendLogin(const QString& user, const QString& password);
    void sendTokenLogin(const QString& username, const QString& token);
    // 发送注册请求
    void sendRegister(const QString& username, const QString& phonenumber, const QString& password);
    // 发送修改密码请求
    void sendChangePassword(int userId, const QString &oldPwd, const QString &newPwd);
    // 发送修改用户信息请求
    void sendUpdatedUserInfo(int userId, const QString& username, const QByteArray& profile);
    // 发送搜索用户请求
    void sendSearchUser(const QString& keyword);
    // 发送添加好友请求
    void sendAddUser(int userId, int friendId, const QString& leaveMsg);
    // 发送删除好友请求
    void sendDeleteUser(int userId, int friendId);
    // 发送接受好友申请请求
    void sendAcceptFriendApply(int friendId, int userId);
    // 发送拒绝好友申请请求
    void sendRejectFriendApply(int friendId, int userId);
    // 发送信息请求
    void sendNewMessage(int chatId, int sendId, int type, const QString& content);

private slots:
    void onReadyRead();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError err);
signals:
    // 登录信号
    void loginSuccess(const UserInfo& info);
    void loginFailed(const QString& reason);

    // 注册信号
    void registerSuccess();
    void registerFailed(const QString& reason);

    // 修改密码信号
    void changePasswordSuccess(const QString& leaveMsg);
    void changePasswordFailed(const QString& leaveMsg);

    // 搜索用户信号
    void searchUserResult(const QList<UserInfo>& users);

    // 添加用户信号
    void sendFriendApplyResult(const QString& leaveMsg);
    void acceptFriendApplyResult(const QString& leaveMsg);
    void rejectFriendApplyResult(const QString& leaveMsg);
    void friendAdded(int friendId);

    // 删除用户信号
    void friendDeleted(int friendId);
    void deleteFriendResult(const QString& leaveMsg);

    // 用户信息更新信号
    void updatedUserInfoResult(const QString& msg);

private:
    explicit SocketBusiness(QObject *parent = nullptr);
    SocketBusiness(const SocketBusiness&) = delete;
    SocketBusiness& operator=(const SocketBusiness&) = delete;
    void sendJson(const QJsonObject& json);

    const QString host = "127.0.0.1";
    const int port = 7777;
    QTcpSocket* socket;
    QByteArray mBuffer;
};

#endif // SOCKETBUSINESS_H

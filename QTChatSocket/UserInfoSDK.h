#ifndef USERINFOSDK_H
#define USERINFOSDK_H

#pragma once

#include <QObject>
#include <QString>
#include <QSettings>
#include <QPixmap>
#include <QFileDialog>
#include <QInputDialog>
#include <QPainter>
#include <QPainterPath>
#include <QList>

struct UserInfo
{
    int userId = -1;
    QString username;
    QByteArray profile;
    // 处理用户搜索与好友申请
    int isFriend = -1; // 0=不是好友 1=好友
    int applyStatus = -1;  // -1=无申请 0=待处理 1=同意 2=拒绝
    int isSelf = -1;  // -1=无 0=对方发的 1=我发的
};

class UserInfoSDK : public QObject
{
    Q_OBJECT
public:
    static UserInfoSDK& instance();

    QPixmap makeProfile(const QPixmap &src, int width, int height);
    QSharedPointer<const UserInfo> getUser(int userId) const;
    QList<QSharedPointer<const UserInfo>> getFriendList() const;
    void clearAll();
    void updateUser(const UserInfo& info);

signals:
    /* ===== 核心信号 ===== */
    void userInfoUpdated(int userId);

private:
    explicit UserInfoSDK(QObject* parent = nullptr);
    UserInfoSDK(const UserInfoSDK&) = delete;
    UserInfoSDK& operator=(const UserInfoSDK&) = delete;

    QHash<qint64, QSharedPointer<UserInfo>> mUserMap;
};

#endif // USERINFOSDK_H

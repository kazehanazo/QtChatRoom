#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#pragma once

#include <QString>
#include <QPixmap>

enum class MessageLocation
{
    Left,
    Right
};
enum class MessageType
{
    Text,
    Image,
};
enum class FriendApplyStatus
{
    Pending,
    Accepted,
    Rejected
};

/* ================= 消息模型 ================= */
struct MessageData
{
    int sendId;
    int chatId;
    MessageType type;
    QString content;
    qint64 timestamp;
    bool isSelf;
};

/* ================= 会话模型 ================= */
struct ChatData
{
    int chatId;
    MessageData lastMessage;
    qint64 lastTimestamp;
    int unreadCount = 0;
};

/* ================= 好友申请模型 ================= */
struct FriendApplyData
{
    int friendId;
    FriendApplyStatus status;
    QString leaveMsg;
    qint64 timestamp;
};
#endif // MESSAGEDATA_H

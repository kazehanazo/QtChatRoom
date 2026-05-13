#ifndef FRIENDAPPLYSTORE_H
#define FRIENDAPPLYSTORE_H

#pragma once

#include <QObject>
#include <QList>
#include <QDateTime>

#include "MessageModel.h"

class FriendApplyStore : public QObject
{
    Q_OBJECT
public:
    static FriendApplyStore& instance();
    void addFriendApply(const FriendApplyData& apply);
    void removeFriendApply(int userId);
    const FriendApplyData* getFriendApply(int friendId) const;
    QList<FriendApplyData> getAllFriendApplies() const;
    void clearAll();
    void updateStatus(int friendId, qint64 timestamp, FriendApplyStatus status);

signals:
    void friendApplyAdded(const FriendApplyData& data);
    void friendApplyUpdated(const FriendApplyData& data);

private:
    explicit FriendApplyStore(QObject *parent = nullptr);
    FriendApplyStore(const FriendApplyStore&) = delete;
    FriendApplyStore& operator=(const FriendApplyStore&) = delete;

    QList<FriendApplyData> mFriendApplys;
};

#endif // FRIENDAPPLYSTORE_H

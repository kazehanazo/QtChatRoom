#include "FriendApplyStore.h"

FriendApplyStore &FriendApplyStore::instance()
{
    static FriendApplyStore instance;
    return instance;
}

FriendApplyStore::FriendApplyStore(QObject *parent): QObject{parent}
{

}

void FriendApplyStore::addFriendApply(const FriendApplyData& apply)
{
    mFriendApplys.prepend(apply);
    emit friendApplyUpdated(apply);
}

void FriendApplyStore::removeFriendApply(int userId)
{
    for (int i = 0; i < mFriendApplys.size(); ++i)
    {
        if (mFriendApplys[i].friendId == userId)
        {
            mFriendApplys.removeAt(i);
            break;
        }
    }
}

const FriendApplyData* FriendApplyStore::getFriendApply(int friendId) const
{
    for (auto& apply : mFriendApplys)
    {
        if (apply.friendId == friendId) return &apply;
    }
    return nullptr;
}

QList<FriendApplyData> FriendApplyStore::getAllFriendApplies() const
{
    return mFriendApplys;
}

void FriendApplyStore::clearAll()
{
    mFriendApplys.clear();
}

void FriendApplyStore::updateStatus(int friendId, qint64 timestamp, FriendApplyStatus status)
{
    for (auto& apply : mFriendApplys)
    {
        if (apply.friendId == friendId)
        {
            apply.status = status;
            apply.timestamp = timestamp;
            emit friendApplyUpdated(apply);
            break;
        }
    }
}



#include "MessageStore.h"

MessageStore& MessageStore::instance()
{
    static MessageStore instance;
    return instance;
}

MessageStore::MessageStore(QObject* parent): QObject(parent)
{

}

void MessageStore::addMessage(int chatId, const MessageData &msg)
{
    {
        QWriteLocker locker(&mLock);
        mMessages[chatId].append(msg);
    }
    emit messageAdded(msg);
}

void MessageStore::addMessages(int chatId, const QList<MessageData> &msgs)
{
    if (msgs.isEmpty()) return;
    {
        QWriteLocker locker(&mLock);
        auto& list = mMessages[chatId];
        list.append(msgs);
    }
}

QList<MessageData> MessageStore::getMessages(int chatId) const
{
    QReadLocker locker(&mLock);
    return mMessages.value(chatId);
}

int MessageStore::messageCount(int chatId) const
{
    QReadLocker locker(&mLock);
    return mMessages.value(chatId).size();
}

void MessageStore::clearMessage(int chatId)
{
    QWriteLocker locker(&mLock);
    mMessages.remove(chatId);
}

void MessageStore::clearAll()
{
    QWriteLocker locker(&mLock);
    mMessages.clear();
}

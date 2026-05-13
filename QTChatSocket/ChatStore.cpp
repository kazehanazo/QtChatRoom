#include "ChatStore.h"

ChatStore &ChatStore::instance()
{
    static ChatStore instance;
    return instance;
}

void ChatStore::addChat(const ChatData &data)
{
    auto chat = getChatData(data.chatId);
    if (!chat)
    {
        mChatList.prepend(data);
        emit chatUpdated(data.chatId);
        return;
    }
}

void ChatStore::removeChat(int chatId)
{
    for (int i = 0; i < mChatList.size(); ++i)
    {
        if (mChatList[i].chatId == chatId)
        {
            mChatList.removeAt(i);
            break;
        }
    }
}

QList<ChatData> ChatStore::getAllChat() const
{
    return mChatList;
}

void ChatStore::clearAll()
{
    mChatList.clear();
}

ChatData *ChatStore::getChatData(int chatId)
{
    for (auto& chat : mChatList)
    {
        if (chat.chatId == chatId) return &chat;
    }
    return nullptr;
}

ChatStore::ChatStore(QObject *parent) : QObject{parent}
{
    connect(&MessageStore::instance(), &MessageStore::messageAdded, this, &ChatStore::onMessageAdded);
}

void ChatStore::onMessageAdded(const MessageData &msg)
{
    auto chat = getChatData(msg.chatId);
    if (!chat)
    {
        ChatData newChat;
        newChat.chatId = msg.chatId;
        newChat.lastMessage = msg;
        newChat.unreadCount = 1;
        newChat.lastTimestamp = msg.timestamp;
        mChatList.prepend(newChat);
        emit chatUpdated(msg.chatId);
        return;
    }
    chat->lastMessage = msg;
    chat->unreadCount++;
    emit chatUpdated(msg.chatId);
}



#ifndef CHATSTORE_H
#define CHATSTORE_H

#pragma once

#include <QObject>
#include <QDebug>
#include <QList>
#include <QDateTime>

#include "MessageStore.h"

class ChatStore: public QObject
{
    Q_OBJECT
public:
    static ChatStore& instance();
    void addChat(const ChatData& data);
    void removeChat(int chatId);
    ChatData* getChatData(int chatId);
    QList<ChatData> getAllChat() const;
    void clearAll();

signals:
    void chatUpdated(int chatId);

private slots:
    void onMessageAdded(const MessageData& msg);

private:
    explicit ChatStore(QObject* parent = nullptr);
    ChatStore(const ChatStore&) = delete;
    ChatStore& operator=(const ChatStore&) = delete;


    QList<ChatData> mChatList;
};

#endif // MESSAGEMANAGER_H

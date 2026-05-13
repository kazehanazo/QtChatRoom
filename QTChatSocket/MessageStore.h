#ifndef MESSAGESTORE_H
#define MESSAGESTORE_H

#pragma once

#include <QObject>
#include <QDebug>
#include <QHash>
#include <QList>
#include <QReadWriteLock>

#include "MessageModel.h"

class MessageStore: public QObject
{
    Q_OBJECT
public:
    static MessageStore& instance();
    void addMessage(int chatId, const MessageData& msg);
    void addMessages(int chatId, const QList<MessageData>& msgs);
    QList<MessageData> getMessages(int chatId) const;
    int messageCount(int chatId) const;
    void clearMessage(int chatId);
    void clearAll();

signals:
    void messageAdded(const MessageData &msg);

private:
    explicit MessageStore(QObject* parent = nullptr);
    MessageStore(const MessageStore&) = delete;
    MessageStore& operator=(const MessageStore&) = delete;


    QHash<int, QList<MessageData>> mMessages;
    mutable QReadWriteLock mLock;
};

#endif // MESSAGESTORE_H

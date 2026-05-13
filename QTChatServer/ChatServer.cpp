#include "ChatServer.h"

ChatServer::ChatServer(QObject *parent): QTcpServer(parent)
{

    clientTimer = new QTimer(this);
    connect(clientTimer, &QTimer::timeout, this, &ChatServer::checkTimeoutClients);
    clientTimer->start(60000);
}

bool ChatServer::start(QString host, quint16 port)
{
    mHost = host;
    mPort = port;
    if(this->listen(QHostAddress(mHost), mPort))
    {
        qDebug() << "ChatServer listening the port: " << port;
        QString msg= QString("服务器%1已开启，开始监听端口: %2").arg(mHost).arg(mPort);
        LOG_INFO(msg);
        return true;
    }
    qWarning() << "ChatServer failed to start: " << errorString();
    QString msg = QString("服务器%1监听端口: %2失败，%3").arg(mHost).arg(mPort).arg(errorString());
    LOG_WARN(msg);
    return false;
}

void ChatServer::stop()
{
    this->close();
    qDebug() << "ChatServer stopped listening";
    if (clientTimer) clientTimer->stop();
    for (auto it = activeClients.begin(); it != activeClients.end(); ++it)
    {
        const auto& info = it.value();
        if (info.socket)
        {
            qDebug() << "Disconnecting user:" << info.username;
            info.socket->disconnectFromHost();
            if (info.socket->state() != QAbstractSocket::UnconnectedState) info.socket->abort();
        }
        // ChatSocket* client = qobject_cast<ChatSocket*>(info.socket->parent());
        // if (client)
        // {
        //     QThread* thread = client->thread();
        //     // 正常退出
        //     client->deleteLater();
        //     if (thread)
        //     {
        //         thread->quit();
        //         thread->wait();
        //         thread->deleteLater();
        //     }
        // }
        emit onUserOffline(info.username);
    }
    activeClients.clear();
    QString msg = QString("服务器%1已关闭，停止监听接口%2").arg(mHost).arg(mPort);
    LOG_INFO(msg);
    mHost.clear();
    mPort = 0;
}

void ChatServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "New connection accepted, socketDescriptor = " << socketDescriptor;
    QThread *thread = new QThread(this);
    ChatSocket* client = new ChatSocket(socketDescriptor);
    client->moveToThread(thread);
    // 保存到pendingClients中，用于后续登录时匹配
    pendingClients.insert(socketDescriptor, client);
    // 自动清理：当client销毁时，从pendingClients中移除
    connect(client, &ChatSocket::destroyed, this, [this, socketDescriptor]() { pendingClients.remove(socketDescriptor); });
    connect(thread, &QThread::started,client, &ChatSocket::start);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(client, &ChatSocket::destroyed, thread, &QThread::quit);
    connect(client, &ChatSocket::userOnline, this, &ChatServer::userOnline);
    connect(client, &ChatSocket::userOffline, this, &ChatServer::userOffline);
    connect(client, &ChatSocket::addUserRequest, this, &ChatServer::addUserRequest);
    connect(client, &ChatSocket::friendApplyResult, this, &ChatServer::friendApplyResult);
    connect(client, &ChatSocket::acceptFriendApplyMsg, this, &ChatServer::acceptFriendApplyMsg);
    connect(client, &ChatSocket::userInfoUpdatedBroadcast, this, &ChatServer::userInfoUpdated);
    connect(client, &ChatSocket::deletedFriend, this, &ChatServer::deletedFriend);
     connect(client, &ChatSocket::newMessage, this, &ChatServer::newMessage);
    thread->start();
}


void ChatServer::checkTimeoutClients()
{
    QDateTime nowTime = QDateTime::currentDateTime();
    for (auto it = activeClients.begin(); it != activeClients.end(); )
    {
        if (it.value().lastActive.msecsTo(nowTime) > TIMEOUT_MS)
        {
            const auto& info = it.value();
            QString msg = QString("用户 [%1] 超时断开连接").arg(info.username);
            qDebug() << msg;
            LOG_INFO(msg);
            if (info.socket)
            {
                info.socket->disconnectFromHost();
            }
            emit onUserOffline(info.username);
            it = activeClients.erase(it);
        }
        else
        {
            ++it;
        }
    }

}

void ChatServer::userOnline(int userId, const QString &username, QTcpSocket* socket)
{
    qDebug() << username << "上线";
    // 通过 socket 的描述符找到对应的 ChatSocket*
    qintptr desc = socket->socketDescriptor();
    ChatSocket* client = pendingClients.take(desc); // 取出并移除
    if (!client)
    {
        qWarning() << "userOnline: cannot find ChatSocket for descriptor" << desc;
        return;
    }
    clientInfo info;
    info.userId = userId;
    info.username = username;
    info.client = client;
    info.socket = socket;
    info.lastActive = QDateTime::currentDateTime();
    activeClients[userId] = info;
    QString msg = "登录成功,用户" + username + "上线了";
    LOG_INFO(msg);
    emit onUserOnline(username);
}

void ChatServer::userOffline(int userId, const QString &username, QTcpSocket *socket)
{
    qDebug() << username << "下线";
    activeClients.remove(userId);
    QString msg = " 用户" + username + "下线了";
    LOG_INFO(msg);
    emit onUserOffline(username);
}

void ChatServer::addUserRequest(int userId, int friendId, const QString &username, const QByteArray &profile, const QString &leaveMsg)
{
    // 查找被申请者是否在线
    if (!activeClients.contains(friendId))
    {
        qDebug() << "friendId:" << friendId << "offline, skip push";
        return;
    }
    const auto& info = activeClients.value(friendId);
    if (!info.socket || info.socket->state() != QAbstractSocket::ConnectedState) return;

    QJsonObject json;
    json["type"] = "applyResultToRecipient";
    json["fromUserId"] = userId;
    json["fromUsername"] = username;
    json["fromProfile"] = QString(profile.toBase64());;
    json["leaveMsg"] = leaveMsg;
    json["timestamp"] = QDateTime::currentSecsSinceEpoch();
    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";

    // 通过invokeMethod调用工作线程中的socket进行数据包写入
    QPointer<ChatSocket> clientPtr(info.client);
    QMetaObject::invokeMethod(info.client, [clientPtr, data]() {
        if (clientPtr)
        {
            clientPtr->responeHandle(data);
        }
    }, Qt::QueuedConnection);
    qDebug() << "push addUserRequest from" << userId << "to" << friendId;
}

void ChatServer::friendApplyResult(int userId, int friendId, qint64 timeMs, const QString &status)
{
    // 查找被申请者是否在线
    if (!activeClients.contains(friendId))
    {
        qDebug() << "friendId:" << friendId << "offline, skip push";
        return;
    }
    const auto& info = activeClients.value(friendId);
    if (!info.socket || info.socket->state() != QAbstractSocket::ConnectedState) return;
    QJsonObject json;
    json["type"] = "friendApplyResult";
    json["userId"] = userId;
    json["status"] = status;
    json["timestamp"] = timeMs;
    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";

    QPointer<ChatSocket> clientPtr(info.client);
    QMetaObject::invokeMethod(info.client, [clientPtr, data]() {
        if (clientPtr)
        {
            clientPtr->responeHandle(data);
        }
    }, Qt::QueuedConnection);
    qDebug() << "push addUserRequest from" << friendId << "to" << userId;
}

void ChatServer::acceptFriendApplyMsg(int userId, int friendId, const QString &leaveMsg, const QString &replyMsg, qint64 timeMs1, qint64 timeMs2)
{
    // 查找被申请者是否在线
    if (!activeClients.contains(friendId))
    {
        qDebug() << "friendId:" << friendId << "offline, skip push";
        return;
    }
    const auto& info = activeClients.value(friendId);
    if (!info.socket || info.socket->state() != QAbstractSocket::ConnectedState) return;
    // 发送好友双方第一次的对话信息
    QJsonArray msgs;
    QJsonObject msg1;
    QJsonObject msg2;
    msg1["chatId"] = userId;
    msg1["sendId"] = friendId;
    msg1["msgType"] = 0;
    msg1["content"] = leaveMsg;
    msg1["timestamp"] = timeMs1;
    msg1["isSelf"] = false;
    msgs.append(msg1);
    msg2["chatId"] = userId;
    msg2["sendId"] = userId;
    msg2["msgType"] = 0;
    msg2["content"] = replyMsg;
    msg2["timestamp"] = timeMs2;
    msg2["isSelf"] = true;
    msgs.append(msg2);
    QJsonObject json;
    json["type"] = "newMessages";
    json["messages"] = msgs;
    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";

    QPointer<ChatSocket> clientPtr(info.client);
    QMetaObject::invokeMethod(info.client, [clientPtr, data]() {
        if (clientPtr)
        {
            clientPtr->responeHandle(data);
        }
    }, Qt::QueuedConnection);
    qDebug() << "push acceptFriendApplyMsg from" << userId << "to" << friendId;
}

void ChatServer::userInfoUpdated(int userId, const QString &username, const QByteArray &profile, QList<int> userIdList)
{
    QJsonObject json;
    json["type"] = "friendInfoUpdated";
    json["userId"] = userId;
    json["username"] = username;
    json["profile"] = QString(profile.toBase64());
    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";

    // 向在线好友发送
    for (int userId : userIdList)
    {
        if (activeClients.contains(userId))
        {
            const auto& info = activeClients.value(userId);
            if (info.client)
            {
                QPointer<ChatSocket> clientPtr(info.client);
                QMetaObject::invokeMethod(info.client, [clientPtr, data]() {
                    if (clientPtr)
                    {
                        clientPtr->responeHandle(data);
                    }
                }, Qt::QueuedConnection);
            }
        }
    }
}

void ChatServer::deletedFriend(int userId, int friendId)
{
    // 查找被申请者是否在线
    if (!activeClients.contains(friendId))
    {
        qDebug() << "friendId:" << friendId << "offline, skip push";
        return;
    }
    const auto& info = activeClients.value(friendId);
    if (!info.socket || info.socket->state() != QAbstractSocket::ConnectedState) return;
    QJsonObject json;
    json["type"] = "receiveDeleteFriend";
    json["userId"] = userId;
    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";

    QPointer<ChatSocket> clientPtr(info.client);
    QMetaObject::invokeMethod(info.client, [clientPtr, data]() {
        if (clientPtr)
        {
            clientPtr->responeHandle(data);
        }
    }, Qt::QueuedConnection);
    qDebug() << "push deletedFriend from" << userId << "to" << friendId;
}

void ChatServer::newMessage(int friendId, int sendId, int type, const QString &content, qint64 timeMs)
{
    // 查找被申请者是否在线
    if (!activeClients.contains(friendId))
    {
        qDebug() << "friendId:" << friendId << "offline, skip push";
        return;
    }
    const auto& info = activeClients.value(friendId);
    if (!info.socket || info.socket->state() != QAbstractSocket::ConnectedState) return;

    QJsonObject json;
    json["type"] = "newMessage";
    json["chatId"] = sendId;
    json["sendId"] = sendId;
    json["msgType"] = type;
    json["content"] = content;
    json["timestamp"] = timeMs;
    json["isSelf"] = false;
    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";

    QPointer<ChatSocket> clientPtr(info.client);
    QMetaObject::invokeMethod(info.client, [clientPtr, data]() {
        if (clientPtr)
        {
            clientPtr->responeHandle(data);
        }
    }, Qt::QueuedConnection);
    qDebug() << "push newMessage to" << friendId;
}

#include "SocketBusiness.h"

SocketBusiness& SocketBusiness::instance()
{
    static SocketBusiness instance;
    return instance;
}

SocketBusiness::SocketBusiness(QObject *parent) : QObject{parent}
{
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &SocketBusiness::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &SocketBusiness::onDisconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &SocketBusiness::onError);
}

void SocketBusiness::connectToServer()
{
    if (socket->state() == QAbstractSocket::ConnectedState) return;
    socket->abort();
    socket->connectToHost(host, port);
    if(!socket->waitForConnected(30000))
    {
        qDebug() << "Connection falied!";
        return;
    }
    qDebug() << "Connected to server success!";
}

void SocketBusiness::disconnectFromServer()
{
    if (socket->state() == QAbstractSocket::ConnectedState)
    {
        socket->disconnectFromHost();
        socket->waitForDisconnected(3000);
    }
}

void SocketBusiness::sendJson(const QJsonObject &json)
{
    QJsonDocument doc(json);
    QByteArray body = doc.toJson(QJsonDocument::Compact);
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << (quint32)body.size();
    data.append(body);
    socket->write(data);
    socket->flush();
}
void SocketBusiness::sendLogin(const QString &user, const QString &password)
{
    qDebug() << "start login";
    connectToServer();
    QJsonObject json;
    json["type"] = "login";
    json["user"] = user;
    json["password"] = password;
    sendJson(json);
}

void SocketBusiness::sendTokenLogin(const QString &username, const QString &token)
{
    qDebug() << "start tokenLogin";
    connectToServer();
    QJsonObject json;
    json["type"] = "tokenLogin";
    json["username"] = username;
    json["token"] = token;
    sendJson(json);
}

void SocketBusiness::sendRegister(const QString &username, const QString &phonenumber, const QString &password)
{
    qDebug() << "start register";
    connectToServer();
    QJsonObject json;
    json["type"] = "register";
    json["username"] = username;
    json["phonenumber"] = phonenumber;
    json["password"] = password;
    sendJson(json);
}

void SocketBusiness::sendChangePassword(int userId, const QString &oldPwd, const QString &newPwd)
{
    qDebug() << "start changePassword";
    connectToServer();
    QJsonObject json;
    json["type"] = "changePassword";
    json["userId"] = userId;
    json["oldPassword"] = oldPwd;
    json["newPassword"] = newPwd;
    sendJson(json);
}

void SocketBusiness::sendUpdatedUserInfo(int userId, const QString &username, const QByteArray &profile)
{
    qDebug() << "start updatedUserInfo";
    connectToServer();
    QJsonObject json;
    json["type"] = "updatedUserInfo";
    json["userId"] = userId;
    json["username"] = username;
    json["profile"] = QString(profile.toBase64());
    sendJson(json);
}

void SocketBusiness::sendSearchUser(const QString &keyword)
{
    qDebug() << "start searchUser";
    connectToServer();
    QJsonObject json;
    json["type"] = "searchUser";
    json["keyword"] = keyword;
    sendJson(json);
}

void SocketBusiness::sendAddUser(int userId, int friendId, const QString& leaveMsg)
{
    qDebug() << "start addUser";
    connectToServer();
    QJsonObject json;
    json["type"] = "addUser";
    json["userId"] = userId;
    json["friendId"] = friendId;
    json["leaveMsg"] = leaveMsg;
    sendJson(json);
}

void SocketBusiness::sendDeleteUser(int userId, int friendId)
{
    qDebug() << "start deleteUser";
    connectToServer();
    QJsonObject json;
    json["type"] = "deleteUser";
    json["userId"] = userId;
    json["friendId"] = friendId;
    sendJson(json);
}

void SocketBusiness::sendAcceptFriendApply(int friendId, int userId)
{
    qDebug() << "start acceptFriendApply";
    connectToServer();
    QJsonObject json;
    json["type"] = "acceptFriendApply";
    json["friendId"] = friendId;
    json["userId"] = userId;
    sendJson(json);
}

void SocketBusiness::sendRejectFriendApply(int friendId, int userId)
{
    qDebug() << "start rejectFriendApply";
    connectToServer();
    QJsonObject json;
    json["type"] = "rejectFriendApply";
    json["friendId"] = friendId;
    json["userId"] = userId;
    sendJson(json);
}

void SocketBusiness::sendNewMessage(int chatId, int sendId, int type, const QString &content)
{
    qDebug() << "start sendNewMessage";
    connectToServer();
    QJsonObject json;
    json["type"] = "newMessage";
    json["chatId"] = chatId;
    json["sendId"] = sendId;
    json["MsgType"] = type;
    json["content"] = content;
    sendJson(json);
}

void SocketBusiness::onReadyRead()
{
    mBuffer.append(socket->readAll());
    while (true)
    {
        int pos = mBuffer.indexOf('\n');
        if (pos == -1) break;
        QByteArray line = mBuffer.left(pos);
        mBuffer.remove(0, pos + 1);  // 移除该行及换行符
        if (line.trimmed().isEmpty()) continue;
        qDebug() << "Server send line:" << line;
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error != QJsonParseError::NoError)
        {
            qWarning() << "JSON parse error:" << err.errorString();
            continue;
        }
        QJsonObject json = doc.object();
        if(json["type"].toString() == "login")
        {
            if(json["status"].toString() == "ok")
            {
                int userId = json["userId"].toVariant().toInt();
                QString username = json["username"].toString();
                QByteArray profile = QByteArray::fromBase64(json["profile"].toString().toUtf8());
                QString token = json["token"].toString();
                qint64 lastLoginTime = json["lastLoginTime"].toVariant().toLongLong();
                QSettings settings("users.ini", QSettings::IniFormat);
                settings.beginGroup(QString("users/%1").arg(userId));
                QString oldUsername = settings.value("username").toString();
                QString oldToken = settings.value("token").toString();
                QByteArray oldProfile = settings.value("profile").toByteArray();
                if(oldUsername != username)
                {
                    settings.setValue("username", username);
                }
                settings.setValue("token", token);
                if(oldToken != token)
                {
                    settings.setValue("token", token);
                }
                if(oldProfile != profile)
                {
                    settings.setValue("profile", profile);
                }
                settings.setValue("lastLoginTime", lastLoginTime);
                settings.endGroup();
                UserInfo info;
                info.userId = userId;
                info.username = username;
                info.profile = profile;
                emit loginSuccess(info);
            }
            else
            {
                emit loginFailed(json["reason"].toString("请重新登录"));
            }
        }
        else if (json["type"].toString() == "register")
        {
            if (json["status"].toString() == "ok")
            {
                emit registerSuccess();
            }
            else
            {
                emit registerFailed(json["reason"].toString("请重新注册"));
            }
        }
        else if (json["type"].toString() == "changePassword")
        {
            if (json["status"].toString() == "ok")
            {
                emit changePasswordSuccess(json["msg"].toString());
            }
            else
            {
                emit changePasswordFailed(json["msg"].toString());
            }
        }
        else if (json["type"].toString() == "searchUser")
        {
            QList<UserInfo> users;
            for (const auto &user : json["userlist"].toArray())
            {

                QJsonObject obj = user.toObject();
                UserInfo info;
                info.userId = obj["userId"].toVariant().toInt();
                info.username = obj["username"].toString();
                info.profile = QByteArray::fromBase64(obj["profile"].toString().toUtf8());
                info.isFriend = obj["isFriend"].toVariant().toInt();
                info.applyStatus = obj["applyStatus"].toVariant().toInt();
                info.isSelf = obj["isSelf"].toVariant().toInt();
                users.append(info);
                UserInfoSDK::instance().updateUser(info);
                qWarning() << "[UserInfoSDK]:" << info.isFriend <<  info.isSelf <<  info.applyStatus;
            }
            emit searchUserResult(users);
        }
        else if (json["type"].toString() == "applyResultToSender")
        {
            if (json["status"].toString() == "ok")
            {
                UserInfo info;
                info.userId = json["friendId"].toVariant().toInt();
                info.isFriend = 0;
                info.applyStatus = 0;
                info.isSelf = 1;
                UserInfoSDK::instance().updateUser(info);

                FriendApplyData data;
                data.friendId = json["friendId"].toInt();
                data.leaveMsg = "等待验证";
                data.status = FriendApplyStatus::Pending;
                data.timestamp = json["timestamp"].toVariant().toLongLong();
                FriendApplyStore::instance().addFriendApply(data);
                QString msg = json["msg"].toString();
                emit sendFriendApplyResult(msg);
            }
            else
            {
                QString msg = json["msg"].toString();
                emit sendFriendApplyResult(msg);
            }
        }
        else if (json["type"].toString() == "applyResultToRecipient")
        {
            UserInfo info;
            info.userId = json["fromUserId"].toVariant().toInt();
            info.username = json["fromUsername"].toString();
            info.profile = QByteArray::fromBase64(json["fromProfile"].toString().toUtf8());
            info.isFriend = 0;
            info.applyStatus = 0;
            info.isSelf = 0;
            UserInfoSDK::instance().updateUser(info);

            FriendApplyData data;
            data.friendId = json["fromUserId"].toVariant().toInt();
            data.leaveMsg = json["leaveMsg"].toString();
            data.timestamp = json["timestamp"].toVariant().toLongLong();
            FriendApplyStore::instance().addFriendApply(data);
        }
        else if (json["type"].toString() == "acceptFriendApply")
        {
            int friendId = json["friendId"].toVariant().toInt();
            QString msg = json["msg"].toString();
            qint64 timestamp = json["timestamp"].toVariant().toLongLong();
            if (json["status"].toString() == "ok")
            {
                UserInfo info;
                info.userId = friendId;
                info.isFriend = 1;
                info.applyStatus = 1;
                UserInfoSDK::instance().updateUser(info);

                FriendApplyStore::instance().updateStatus(friendId, timestamp, FriendApplyStatus::Accepted);
                emit acceptFriendApplyResult(msg);
                emit friendAdded(friendId);
            }
            else
            {
                emit acceptFriendApplyResult(msg);
            }
        }
        else if (json["type"].toString() == "rejectFriendApply")
        {
            int friendId = json["friendId"].toVariant().toInt();
            QString msg = json["msg"].toString();
            qint64 timestamp = json["timestamp"].toVariant().toLongLong();
            if (json["status"].toString() == "ok")
            {
                UserInfo info;
                info.userId = friendId;
                info.isFriend = 0;
                info.applyStatus = 2;
                UserInfoSDK::instance().updateUser(info);
                FriendApplyStore::instance().updateStatus(friendId, timestamp, FriendApplyStatus::Rejected);
                emit rejectFriendApplyResult(msg);
            }
            else
            {
                emit rejectFriendApplyResult(msg);
            }
        }
        else if (json["type"].toString() == "friendApplyResult")
        {
            UserInfo info;
            int userId = json["userId"].toVariant().toInt();
            qint64 timestamp = json["timestamp"].toVariant().toLongLong();
            if (json["status"].toString() == "ok")
            {
                info.userId = userId;
                info.isFriend = 1;
                info.applyStatus = 1;
                UserInfoSDK::instance().updateUser(info);
                FriendApplyStore::instance().updateStatus(userId, timestamp, FriendApplyStatus::Accepted);
                emit friendAdded(userId);
            }
            else
            {
                info.userId = userId;
                info.isFriend = 0;
                info.applyStatus = 2;
                UserInfoSDK::instance().updateUser(info);
                FriendApplyStore::instance().updateStatus(userId, timestamp, FriendApplyStatus::Rejected);
            }
        }
        else if (json["type"].toString() == "userInfoUpdated")
        {
            if (json["status"].toString() == "ok")
            {
                UserInfo info;
                info.userId = json["userId"].toVariant().toInt();
                info.username = json["username"].toString();
                info.profile = QByteArray::fromBase64(json["profile"].toString().toUtf8());
                UserInfoSDK::instance().updateUser(info);
                emit updatedUserInfoResult(json["msg"].toString());
            }
            else
            {
                emit updatedUserInfoResult(json["msg"].toString());
            }
        }
        else if (json["type"].toString() == "friendInfoUpdated")
        {
            UserInfo info;
            info.userId = json["userId"].toVariant().toInt();
            info.username = json["username"].toString();
            info.profile = QByteArray::fromBase64(json["profile"].toString().toUtf8());
            UserInfoSDK::instance().updateUser(info);
        }
        else if (json["type"].toString() == "deleteFriend")
        {
            if (json["status"].toString() == "ok")
            {
                emit friendDeleted(json["friendId"].toVariant().toInt());
                emit deleteFriendResult(json["msg"].toString());
            }
            else
            {
                emit deleteFriendResult(json["msg"].toString());
            }
        }
        else if (json["type"].toString() == "receiveDeleteFriend")
        {
            emit friendDeleted(json["userId"].toVariant().toInt());
        }
        else if (json["type"].toString() == "pullFriendList")
        {
            for (const auto &user : json["friends"].toArray())
            {
                QJsonObject obj = user.toObject();
                UserInfo info;
                info.userId = obj["userId"].toVariant().toInt();
                info.username = obj["username"].toString();
                info.profile = QByteArray::fromBase64(obj["profile"].toString().toUtf8());
                info.isFriend = 1;
                info.isSelf = 0;
                info.applyStatus = 1;
                UserInfoSDK::instance().updateUser(info);
            }
        }
        else if (json["type"].toString() == "pullFriendApplyList")
        {
            UserInfo info;
            FriendApplyData data;
            FriendApplyStatus status = FriendApplyStatus::Pending;
            for (const auto &apply : json["applies"].toArray())
            {
                QJsonObject obj = apply.toObject();

                info.userId = obj["friendId"].toVariant().toInt();
                info.username = obj["friendUsername"].toString();
                info.profile = QByteArray::fromBase64(obj["friendProfile"].toString().toUtf8());
                info.isFriend = obj["isFriend"].toVariant().toInt();
                info.applyStatus = obj["applyStatus"].toVariant().toInt();
                info.isSelf = obj["isSelf"].toVariant().toInt();
                UserInfoSDK::instance().updateUser(info);

                switch (info.applyStatus)
                {
                    case 0:
                        status = FriendApplyStatus::Pending;
                        break;
                    case 1:
                        status = FriendApplyStatus::Accepted;
                        break;
                    case 2:
                        status = FriendApplyStatus::Rejected;
                        break;
                    default:
                        status = FriendApplyStatus::Pending;
                        break;
                }
                data.friendId = obj["friendId"].toInt();
                data.status = status;
                data.leaveMsg = obj["leaveMsg"].toString();
                data.timestamp = obj["timestamp"].toVariant().toLongLong();
                FriendApplyStore::instance().addFriendApply(data);
            }
        }
        else if (json["type"].toString() == "newMessage")
        {
            MessageData data;
            data.chatId = json["chatId"].toVariant().toInt();
            data.sendId = json["sendId"].toVariant().toInt();
            data.content = json["content"].toString();
            data.type = (json["msgType"].toVariant().toInt() == 0) ? MessageType::Text : MessageType::Image;
            data.timestamp = json["timestamp"].toVariant().toLongLong();
            data.isSelf = json["isSelf"].toBool();
            MessageStore::instance().addMessage(data.chatId, data);
        }
        else if (json["type"].toString() == "newMessages")
        {
            ChatStore::instance();
            MessageData data;
            QJsonArray msgs = json["messages"].toArray();
            for(const QJsonValue &msg : msgs)
            {
                QJsonObject obj = msg.toObject();
                data.chatId = obj["chatId"].toVariant().toInt();
                data.sendId = obj["sendId"].toVariant().toInt();
                data.content = obj["content"].toString();
                data.type = (obj["msgType"].toVariant().toInt() == 0) ? MessageType::Text : MessageType::Image;
                data.timestamp = obj["timestamp"].toVariant().toLongLong();
                data.isSelf = obj["isSelf"].toBool();
                MessageStore::instance().addMessage(data.chatId, data);
            }
        }
    }
    return;
}

void SocketBusiness::onDisconnected()
{
    qWarning() << "server disconnected!";
}

void SocketBusiness::onError(QAbstractSocket::SocketError err)
{
    qWarning() << "Socket error:" << err;
}



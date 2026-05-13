#include "ChatSocket.h"

ChatSocket::ChatSocket(qintptr socketDescriptor, QObject *parent): descriptor(socketDescriptor), mUserId(-1), mSocket(nullptr)
{
    mUsername.clear();
}

ChatSocket::~ChatSocket()
{
    if(mSocket)
    {
        mUserId = -1;
        mUsername.clear();
        mSocket->close();
        mSocket->deleteLater();
        mSocket = nullptr;
    }
}

void ChatSocket::start()
{
    mSocket = new QTcpSocket();
    QString msg;
    if(!mSocket->setSocketDescriptor(descriptor))
    {
        msg = "CahtSocket setting descriptor failed!";
        qDebug() << msg;
        LOG_WARN(msg);
        mSocket->deleteLater();
        mSocket = nullptr;
        deleteLater();
        return;
    }
    msg = QString("用户连接：Client_%1已进入服务器").arg(descriptor);
    LOG_INFO(msg);
    connect(mSocket, &QTcpSocket::readyRead, this, &ChatSocket::readyReadHandle, Qt::DirectConnection);
    connect(mSocket, &QTcpSocket::disconnected, this, &ChatSocket::disConnection, Qt::DirectConnection);
}

void ChatSocket::responeHandle(const QByteArray &respone)
{
    if(mSocket && mSocket->state() == QAbstractSocket::ConnectedState)
    {
        mSocket->write(respone);
    }
}

void ChatSocket::readyReadHandle()
{
    Q_ASSERT(mSocket);
    mBuffer.append(mSocket->readAll());
    while(true)
    {
        if (mBuffer.size() < 4) return;
        QDataStream stream(mBuffer);
        stream.setByteOrder(QDataStream::BigEndian);
        quint32 length;
        stream >> length;
        if (mBuffer.size() - 4 < length) return;
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromRawData(mBuffer.constData() + 4, static_cast<int>(length)));
        mBuffer.remove(0, 4 + length);
        QPointer<ChatSocket> guard(this);
        int currentUserId = mUserId.load();
        ThreadPool::Instance().addTask([guard, doc, currentUserId] {
            if (guard) {
                guard->processBusiness(doc);
            }
        });
    }
}

void ChatSocket::sendUserOnline(int userId, const QString &username)
{
    mUserId = userId;
    mUsername = username;
    pullFrinedList(userId);
    pullFriendApply(userId);
    pullOfflineMessage(userId);
    emit userOnline(mUserId, mUsername, mSocket);
}

void ChatSocket::sendAddUserRequest(int userId, int friendId, const QString &username, const QByteArray &profile, const QString &leaveMsg)
{
    emit addUserRequest(userId, friendId, username, profile, leaveMsg);
}

void ChatSocket::sendAcceptFriendApply(int userId, int friendId, const QString &leaveMsg, const QString &replyMsg, qint64 timeMs1, qint64 timeMs2)
{
    QJsonArray msgs;
    QJsonObject obj;
    obj["chatId"] = friendId;
    obj["sendId"] = friendId;
    obj["msgType"] = 0;
    obj["content"] = leaveMsg;
    obj["timestamp"] = timeMs1;
    obj["isSelf"] = false;
    msgs.append(obj);
    obj["chatId"] = friendId;
    obj["sendId"] = userId;
    obj["msgType"] = 0;
    obj["content"] = replyMsg;
    obj["timestamp"] = timeMs2;
    obj["isSelf"] = true;
    msgs.append(obj);
    QJsonObject json;
    json["type"] = "newMessages";
    json["messages"] = msgs;
    QByteArray respone = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    emit friendApplyResult(userId, friendId, timeMs2, "ok");
    emit acceptFriendApplyMsg(userId, friendId, leaveMsg, replyMsg, timeMs1, timeMs2);
    responeHandle(respone);
}

void ChatSocket::sendRejectFriendApply(int userId, int friendId, qint64 timeMs)
{
    emit friendApplyResult(userId, friendId, timeMs, "false");
}

void ChatSocket::sendUserInfoUpdated(int userId, const QString &username, const QByteArray &profile, QList<int> userIdList)
{
    emit userInfoUpdatedBroadcast(userId, username, profile, userIdList);
}

void ChatSocket::sendDeletedFriend(int userId, int friendId)
{
    emit deletedFriend(userId, friendId);
}

void ChatSocket::sendNewMessage(int friendId, int sendId, int type, const QString &content, qint64 timeMs)
{
    emit newMessage(friendId, sendId, type, content, timeMs);
}

void ChatSocket::disConnection()
{
    if(mUserId != -1)
    {
        emit userOffline(mUserId, mUsername, mSocket);
    }
    else
    {
        QString msg = QString("用户连接：Client_%1已退出服务器").arg(descriptor);
        LOG_INFO(msg);
    }
    if(mSocket)
    {
        mSocket->deleteLater();
        mSocket = nullptr;
    }
    this->deleteLater();
}

QByteArray ChatSocket::createSalt(int length)
{
    QByteArray salt;
    for (int i = 0; i < length; ++i) {
        salt.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
    return salt;
}

QByteArray ChatSocket::createHash(const QByteArray &pwd)
{
    return QCryptographicHash::hash(pwd, QCryptographicHash::Sha256);
}

bool ChatSocket::verifyPassword(const QString &pwd, const QByteArray &salt, const QByteArray &dbPwd)
{
    return dbPwd == createHash(pwd.toUtf8() + salt);
}

QByteArray ChatSocket::createToken(const QString &username)
{
    QByteArray token = username.toUtf8()
                       + QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8()
                       + QByteArray::number(QDateTime::currentMSecsSinceEpoch())
                       + QByteArray::number(QRandomGenerator::global()->generate64());
    return QCryptographicHash::hash(token, QCryptographicHash::Sha256).toHex();
}

void ChatSocket::processBusiness(const QJsonDocument &doc)
{
    if(!doc.isObject()) return;
    QJsonObject json = doc.object();
    QString type = json.value("type").toString();
    QByteArray respone;

    if(type == "login")
    {      
        QString user = json.value("user").toString();
        QString pwd = json.value("password").toString();
        QString msg = QString("用户%1发起了登录请求").arg(user);
        LOG_INFO(msg);
        respone = checkLogin(user, pwd);
    }
    else if (type == "tokenLogin")
    {
        QString username = json.value("username").toString();
        QString token = json.value("token").toString();
        QString msg = QString("用户%1发起了token登录请求").arg(username);
        LOG_INFO(msg);
        respone = checkTokenLogin(username, token);
    }
    else if(type == "register")
    {
        QString username = json.value("username").toString();
        QString phonenumber = json.value("phonenumber").toString();
        QString pwd = json.value("password").toString();
        QString msg = QString("用户%1发起了注册请求").arg(username);
        LOG_INFO(msg);
        respone = checkRegister(username, phonenumber, pwd);
    }
    else if(type == "changePassword")
    {
        int userId = json.value("userId").toInt();
        QString oldPwd = json.value("oldPassword").toString();
        QString newPwd = json.value("newPassword").toString();
        QString msg = QString("用户发起了修改密码请求").arg(userId);
        LOG_INFO(msg);
        respone = checkChangePassword(userId, oldPwd, newPwd);
    }
    else if(type == "searchUser")
    {
        QString keyword = json.value("keyword").toString();
        QString msg = QString("用户发起搜索请求，关键词：%1").arg(keyword);
        LOG_INFO(msg);
        respone = checkSearchUser(keyword);
    }
    else if(type == "addUser")
    {
        int userId = json.value("userId").toInt();
        int friendId = json.value("friendId").toInt();
        QString leaveMsg = json.value("leaveMsg").toString();
        QString msg = QString("用户发起添加好友请求，添加用户id：%1").arg(friendId);
        LOG_INFO(msg);
        respone = checkAddUser(userId, friendId, leaveMsg);
    }
    else if(type == "acceptFriendApply")
    {
        int friendId = json.value("friendId").toInt();
        int userId = json.value("userId").toInt();
        QString msg = QString("用户发起通过好友申请请求");
        LOG_INFO(msg);
        respone = checkAcceptFriendApply(friendId, userId);
    }
    else if(type == "rejectFriendApply")
    {
        int friendId = json.value("friendId").toInt();
        int userId = json.value("userId").toInt();
        QString msg = QString("用户发起拒绝好友申请请求");
        LOG_INFO(msg);
        respone = checkRejectFriendApply(friendId, userId);
    }
    else if(type == "updatedUserInfo")
    {
        int userId = json["userId"].toVariant().toInt();
        QString username = json["username"].toString();
        QByteArray profile = QByteArray::fromBase64(json["profile"].toString().toUtf8());
        QString msg = QString("用户发起了修改用户信息请求");
        LOG_INFO(msg);
        respone = checkUpdatedUserInfo(userId, username, profile);
    }
    else if(type == "deleteUser")
    {
        int userId = json.value("userId").toInt();
        int friendId = json.value("friendId").toInt();
        QString msg = QString("用户发起删除好友请求，删除用户Id：%1").arg(friendId);
        LOG_INFO(msg);
        respone = checkDeleteUser(userId, friendId);
    }
    else if(type == "newMessage")
    {
        int chatId = json.value("chatId").toInt();
        int sendId = json.value("sendId").toInt();
        int type = json.value("MsgType").toInt();
        QString content = json.value("content").toString();
        QString msg = QString("用户发起发送信息请求，发送给的用户Id：%1").arg(chatId);
        LOG_INFO(msg);
        respone = checkNewMessage(chatId, sendId, type, content);
    }
    QPointer<ChatSocket> guard(this);
    if (guard)
    {
        QMetaObject::invokeMethod(guard, "responeHandle", Qt::QueuedConnection, Q_ARG(QByteArray, respone));
    }
}

QByteArray ChatSocket::checkLogin(const QString &user, const QString &pwd)
{
    QJsonObject json;
    QString msg;
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery query1(db);
    qWarning() << "user:" << user;
    qWarning() << "pwd:" << pwd;
    query1.prepare("SELECT password, salt, id, username, profile FROM user WHERE username = :user OR phonenumber = :user");
    query1.bindValue(":user", user);
    if(!query1.exec() || !query1.next())
    {
        msg = QString("用户名不存在，用户%1请求登录失败!").arg(user).arg(query1.lastError().text());
        LOG_WARN(msg);
        json = {{"type","login"}, {"status","fail"}, {"reason","登录失败，用户名不存在！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    QByteArray dbPwd = query1.value(0).toByteArray();
    QByteArray salt = query1.value(1).toByteArray();
    if(!verifyPassword(pwd, salt, dbPwd))
    {
        msg = QString("密码错误，用户%1请求登录失败!").arg(user).arg(query1.lastError().text());
        LOG_WARN(msg);
        json = {{"type","login"}, {"status","fail"}, {"reason","登录失败，密码错误！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    // 生成用户token
    int userId = query1.value(2).toInt();
    QString username = query1.value(3).toString();
    QByteArray profile = query1.value(4).toByteArray();
    QString token = QString::fromUtf8(createToken(username));
    qint64 expireMs = QDateTime::currentMSecsSinceEpoch() + 7LL * 24 * 3600 * 1000;
    QString expire = QDateTime::fromMSecsSinceEpoch(expireMs).toString("yyyy-MM-dd hh:mm:ss.zzz");
    QSqlQuery query2(db);
    query2.prepare("UPDATE user SET token = :token, token_expire = :expire WHERE username = :username");
    query2.bindValue(":token", token);
    query2.bindValue(":expire", expire);
    query2.bindValue(":username", username);
    if (!query2.exec())
    {
        msg = QString("用户%1登录时更新token失败: %2").arg(user).arg(query2.lastError().text());
        LOG_WARN(msg);
        json = {{"type", "login"}, {"status", "fail"}, {"reason", "登录失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    QPointer<ChatSocket> guard(this);
    if (guard)
    {
        QMetaObject::invokeMethod(guard, "sendUserOnline", Qt::QueuedConnection, Q_ARG(int, userId), Q_ARG(QString, username));
    }
    qWarning() << "token:" << token;
    json["type"] = "login";
    json["status"] = "ok";
    json["userId"] = userId;
    json["username"] = username;
    json["token"] = token;
    json["profile"] = QString(profile.toBase64());
    json["lastLoginTime"] = QDateTime::currentMSecsSinceEpoch();
    return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
}

QByteArray ChatSocket::checkTokenLogin(const QString &username, const QString &token)
{
    QJsonObject json;
    QString msg;
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery query1(db);
    query1.prepare("SELECT token_expire, id, profile FROM user WHERE username=:username AND token=:token");
    query1.bindValue(":username", username);
    query1.bindValue(":token", token);
    if(!query1.exec() || !query1.next())
    {
        msg = QString("token无效，用户%1请求token登录失败!").arg(username);
        LOG_WARN(msg);
        json = {{"type","login"}, {"status","fail"}, {"reason","token无效，请重新登录！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    QDateTime time = QDateTime::fromString(query1.value("token_expire").toString(),Qt::ISODateWithMs);
    time.setTimeZone(QTimeZone::LocalTime);
    qint64 expire = time.toMSecsSinceEpoch();
    qWarning() << query1.value("token_expire").toString();
    qWarning() << QDateTime::currentMSecsSinceEpoch();
    if (expire < QDateTime::currentMSecsSinceEpoch())
    {
        msg = QString("token已过期，用户%1请求token登录失败!").arg(username);
        LOG_WARN(msg);
        json = {{"type","login"},{"status","fail"},{"reason","token已过期，请重新登录！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    int userId = query1.value(1).toInt();
    QByteArray profile = query1.value(2).toByteArray();
    QString newToken = QString::fromUtf8(createToken(username));
    qint64 expireMs = QDateTime::currentMSecsSinceEpoch() + 7LL * 24 * 3600 * 1000;
    QString newExpire = QDateTime::fromMSecsSinceEpoch(expireMs).toString("yyyy-MM-dd hh:mm:ss.zzz");
    QSqlQuery query2(db);
    query2.prepare("UPDATE user SET token = :token, token_expire = :expire WHERE username = :username");
    query2.bindValue(":token", newToken);
    query2.bindValue(":expire", newExpire);
    query2.bindValue(":username", username);
    if (!query2.exec())
    {
        msg = QString("用户%1 token登录时更新token失败: %2").arg(username).arg(query2.lastError().text());
        LOG_WARN(msg);
        json = {{"type", "login"}, {"status", "fail"}, {"reason", "服务器内部错误，请重新登录！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    QPointer<ChatSocket> guard(this);
    if (guard)
    {
        QMetaObject::invokeMethod(guard, "sendUserOnline", Qt::QueuedConnection, Q_ARG(int, userId), Q_ARG(QString, username));
    }
    qWarning() << "token:" << newToken;
    json["type"] = "login";
    json["status"] = "ok";
    json["userId"] = userId;
    json["username"] = username;
    json["token"] = newToken;
    json["profile"] = QString(profile.toBase64());
    json["lastLoginTime"] = QDateTime::currentMSecsSinceEpoch();
    return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
}

QByteArray ChatSocket::checkRegister(const QString &username, const QString &phonenumber, const QString &pwd)
{
    QJsonObject json;
    QString msg;
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery query(db);
    //对密码进行加盐哈希
    QByteArray salt = createSalt(16);
    QByteArray hashPwd = createHash(pwd.toUtf8() + salt);
    // 载入系统默认头像
    QFile file(":/imgs/profile.svg");
    file.open(QIODevice::ReadOnly);
    QByteArray defaultProfile = file.readAll();
    query.prepare("INSERT INTO user (username, phonenumber, password, salt, profile) "
                    "VALUES (:username, :phonenumber, :password, :salt, :profile)");
    query.bindValue(":username", username);
    query.bindValue(":phonenumber", phonenumber);
    query.bindValue(":password", hashPwd);
    query.bindValue(":salt", salt);
    query.bindValue(":profile", defaultProfile);
    if(!query.exec())
    {
        msg = QString("用户连接%1请求注册账号%2失败，原因：%3").arg(descriptor).arg(username).arg(query.lastError().text());
        LOG_WARN(msg);
        json =  {{"type","register"},{"status","fail"},{"msg","注册失败, 请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    msg = QString("用户连接%1请求注册账号%2成功!").arg(descriptor).arg(username);
    LOG_INFO(msg);
    json = {{"type","register"},{"status","ok"},{"msg","注册成功！"}};
    return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
}

QByteArray ChatSocket::checkChangePassword(int userId, const QString &oldPwd, const QString &newPwd)
{
    QJsonObject json;
    QString msg;
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery query(db);
    query.prepare("SELECT password, salt FROM user WHERE id = :userId");
    query.bindValue(":userId", userId);
    if (!query.exec() || !query.next())
    {
        msg = QString("用户连接%1修改密码时，查询数据库失败: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","changePassword"},{"status","fail"},{"msg","用户查询不存在，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    QByteArray dbPwd = query.value(0).toByteArray();
    QByteArray salt = query.value(1).toByteArray();
    if (!verifyPassword(oldPwd, salt, dbPwd))
    {
        msg = QString("用户连接%1修改密码时，旧密码错误").arg(descriptor);
        LOG_WARN(msg);
        json = {{"type","changePassword"},{"status","fail"},{"msg","旧密码错误，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    QByteArray newHash = createHash(newPwd.toUtf8() + salt);
    QSqlQuery update(db);
    update.prepare("UPDATE user SET password = :pwd WHERE id = :userId");
    update.bindValue(":pwd", newHash);
    update.bindValue(":userId", userId);
    if (!update.exec())
    {
        msg = QString("用户连接%1修改密码时，更新数据库失败: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","changePassword"},{"status","fail"},{"msg","修改密码失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    if (update.numRowsAffected() == 0)
    {
        msg = QString("用户连接%1修改密码时，更新数据库失败: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","changePassword"},{"status","fail"},{"msg","修改密码失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    json = {{"type","changePassword"},{"status","ok"},{"msg","密码修改成功, 请重新登录！"}};
    return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
}

QByteArray ChatSocket::checkSearchUser(const QString &keyword)
{
    QJsonObject json;
    QString msg;
    QJsonArray userArray;
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery query(db);
    // 模糊匹配 username
    query.prepare("SELECT u.id,u.username,u.profile, CASE WHEN fr.friend_id IS NOT NULL THEN 1 ELSE 0 END AS isFriend,"
                    " COALESCE(req.status, -1) AS status, CASE WHEN req.from_user_id = :me THEN 1 WHEN req.to_user_id = :me THEN 0 ELSE -1 END AS isSelf"
                    " FROM user u LEFT JOIN friend_relation fr ON fr.user_id = :me AND fr.friend_id = u.id"
                    " LEFT JOIN friend_request req ON ((req.from_user_id = :me AND req.to_user_id = u.id) OR (req.from_user_id = u.id AND req.to_user_id = :me))"
                    " WHERE u.username LIKE :key AND u.id != :me LIMIT 20");
    //query.prepare( "SELECT id, username, profile FROM user WHERE username LIKE :key LIMIT 20");
    query.bindValue(":me", mUserId.load());
    query.bindValue(":key", "%" + keyword + "%");

    if (query.exec())
    {
        while (query.next())
        {
            QJsonObject userObj;
            userObj["userId"] = query.value(0).toInt();
            userObj["username"] = query.value(1).toString();
            QByteArray profile = query.value(2).toByteArray();
            userObj["profile"] = QString(profile.toBase64());
            userObj["isFriend"] = query.value("isFriend").toInt();
            userObj["applyStatus"] = query.value("status").toInt();
            userObj["isSelf"] = query.value("isSelf").toInt();
            userArray.append(userObj);
        }
    }
    else
    {
        qWarning() << "searchUser query failed:" << query.lastError().text();
        msg = QString("用户连接%1请求搜索用户，数据库查询失败: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
    }

    json = {{"type","searchUser"}, {"status","ok"}, {"userlist", userArray}};
    return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
}
QByteArray ChatSocket::checkAddUser(int userId, int friendId, const QString &leaveMsg)
{
    QJsonObject json;
    QString msg;
    qint64 time;
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery query(db);
    query.prepare("SELECT username, profile FROM user WHERE id = :userId");
    query.bindValue(":userId", userId);
    if(!query.exec() || !query.next())
    {
        QString msg = QString("添加好友时查询本用户信息失败，用户%1请求登录失败!").arg(userId).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","applyResultToSender"}, {"status","fail"}, {"msg","数据库错误,请重新发起好友申请！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    QString username = query.value(0).toString();
    QByteArray profile = query.value(1).toByteArray();
    // 检查是否已经是好友
    query.prepare("SELECT 1 FROM friend_relation WHERE user_id=:u AND friend_id=:f LIMIT 1");
    query.bindValue(":u", userId);
    query.bindValue(":f", friendId);
    if (!query.exec())
    {
        msg = QString("用户连接%1发起好友申请时数据库查找失败: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","applyResultToSender"},{"status","fail"},{"msg","数据库错误,请重新发起好友申请！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    if (query.next())
    {
        msg = QString("用户连接%1发起好友申请时检查出已经是好友！").arg(descriptor);
        LOG_WARN(msg);
        json = {{"type","applyResultToSender"},{"status","fail"},{"msg","你们已经是好友了！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    // 查询好友申请记录
    query.prepare("SELECT status, from_user_id FROM friend_request WHERE (from_user_id=:u AND to_user_id=:f) "
                  "OR (from_user_id=:f AND to_user_id=:u) ORDER BY id DESC LIMIT 1");
    query.bindValue(":u", userId);
    query.bindValue(":f", friendId);
    if (!query.exec())
    {
        msg = QString("用户连接%1发起好友申请时数据库查找失败: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","applyResultToSender"},{"status","fail"},{"msg","数据库错误,请重新发起好友申请！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    if (query.next())
    {
        int applyStatus = query.value("status").toInt();
        int from = query.value("from_user_id").toInt();
        // 已有未处理申请
        if (applyStatus == 0)
        {
            msg = QString("用户连接%1发起好友申请时已存在好友申请").arg(descriptor);
            LOG_WARN(msg);
            json = {{"type","applyResultToSender"},{"status","fail"},{"msg","已存在对该用户好友申请，请到好友申请栏确认！"}};
            return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
        }
        // 曾被拒绝
        if (applyStatus == 2 && from == userId)
        {
            QSqlQuery update(db);
            time = QDateTime::currentMSecsSinceEpoch();
            update.prepare("UPDATE friend_request SET status=0, leave_msg=:msg, handled_time=:time "
                            "WHERE from_user_id=:u AND to_user_id=:f");
            update.bindValue(":u", userId);
            update.bindValue(":f", friendId);
            update.bindValue(":msg", leaveMsg);
            update.bindValue(":time", time);
            update.exec();
            msg = QString("用户连接%1成功对用户：%2重新发送好友申请!").arg(descriptor).arg(username);
            LOG_WARN(msg);
            QMetaObject::invokeMethod(this, "sendAddUserRequest", Qt::QueuedConnection,
                                      Q_ARG(int, userId), Q_ARG(int, friendId), Q_ARG(QString, username),
                                      Q_ARG(QByteArray, profile), Q_ARG(QString, leaveMsg));
            json["type"] = "applyResultToSender";
            json["status"] = "ok";
            json["friendId"] = friendId;
            json["timestamp"] = time;
            json["msg"] = "已重新发送好友申请";
            return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
        }
    }
    // 插入新的申请
    QSqlQuery insert(db);
    time = QDateTime::currentMSecsSinceEpoch();
    insert.prepare("INSERT INTO friend_request (from_user_id, to_user_id, leave_msg, created_time, handled_time) "
                    "VALUES (:from, :to, :msg, :time1, :time2)");
    insert.bindValue(":from", userId);
    insert.bindValue(":to", friendId);
    insert.bindValue(":msg", leaveMsg);
    insert.bindValue(":time1", time);
    insert.bindValue(":time2", time);
    if (!insert.exec())
    {
        msg = QString("用户连接%1插入新的好友申请时数据库插入失败: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","applyResultToSender"},{"status","fail"},{"msg","数据库错误,请重新发起好友申请！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    msg = QString("用户连接%1成功对用户id为：%2的用户发送好友申请!").arg(descriptor).arg(friendId);
    LOG_WARN(msg);
    // 向对方发送好友申请信息
    QPointer<ChatSocket> guard(this);
    if (guard)
    {
        QMetaObject::invokeMethod(guard, "sendAddUserRequest", Qt::QueuedConnection,
                                  Q_ARG(int, userId), Q_ARG(int, friendId), Q_ARG(QString, username),
                                  Q_ARG(QByteArray, profile), Q_ARG(QString, leaveMsg));
    }
    // 向自己发送好友申请回复信息
    json["type"] = "applyResultToSender";
    json["status"] = "ok";
    json["friendId"] = friendId;
    json["timestamp"] = time;
    json["msg"] = "已成功发送好友申请！";
    return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
}

QByteArray ChatSocket::checkAcceptFriendApply(int friendId, int userId)
{
    QJsonObject json;
    QString msg;
    QString leaveMsg;
    QString replyMsg = "我们已成功添加为好友，现在可以开始聊天啦～";
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery query(db);

    if (!db.transaction())
    {
        msg = QString("用户连接%1通过好友申请时数据库事务开启失败: %2").arg(descriptor);
        LOG_WARN(msg);
        json = {{"type","acceptFriendApply"},{"status","fail"},{"msg","通过添加好友失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    //更新好友申请状态
    query.prepare("UPDATE friend_request SET status=1, handled_time=:time  WHERE from_user_id=:f AND to_user_id=:u");
    query.bindValue(":f", friendId);
    query.bindValue(":u", userId);
    query.bindValue(":time", QDateTime::currentMSecsSinceEpoch());
    if (!query.exec())
    {
        db.rollback();
        msg = QString("用户连接%1更新好友申请状态时数据库写入失败，原因: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","acceptFriendApply"},{"status","fail"},{"msg","通过添加好友失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    //插入好友关系
    query.prepare("INSERT INTO friend_relation(user_id,friend_id, created_time) VALUES(:u,:f,:time),(:f,:u,:time) ON DUPLICATE KEY UPDATE user_id=user_id");
    query.bindValue(":u", userId);
    query.bindValue(":f", friendId);
    query.bindValue(":time", QDateTime::currentMSecsSinceEpoch());
    if (!query.exec())
    {
        db.rollback();
        msg = QString("用户连接%1插入好友关系时数据库写入失败，原因: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","acceptFriendApply"},{"status","fail"},{"msg","通过添加好友失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    //查询好友申请留言
    query.prepare("SELECT leave_msg, created_time, handled_time FROM friend_request WHERE from_user_id=:f AND to_user_id=:u");
    query.bindValue(":f", friendId);
    query.bindValue(":u", userId);
    if (!query.exec() || !query.next())
    {
        db.rollback();
        msg = QString("用户连接%1查询好友申请时数据库查询失败，原因: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","acceptFriendApply"},{"status","fail"},{"msg","通过添加好友失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    leaveMsg = query.value(0).toString();
    qint64 timeMs1 =query.value(1).toLongLong();
    qint64 timeMs2 =query.value(2).toLongLong();
    //主视角消息
    query.prepare("INSERT INTO message (owner_user_id,chat_user_id,from_user_id,type,content,created_time) VALUES "
                    "(:ownerId,:chatId,:fromId1,:type,:msg1,:time1), (:ownerId,:chatId,:fromId2,:type,:msg2,:time2)");
    query.bindValue(":ownerId", userId);
    query.bindValue(":chatId", friendId);
    query.bindValue(":fromId1", friendId);
    query.bindValue(":fromId2", userId);
    query.bindValue(":msgType", 0);
    query.bindValue(":msg1", leaveMsg);
    query.bindValue(":msg2", replyMsg);
    query.bindValue(":time1", timeMs1);
    query.bindValue(":time2", timeMs2);
    if (!query.exec())
    {
        db.rollback();
        msg = QString("用户连接%1插入主视角好友通过消息时数据库写入失败，原因: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","acceptFriendApply"},{"status","fail"},{"msg","通过添加好友失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    //好友视角消息
    query.prepare("INSERT INTO message (owner_user_id,chat_user_id,from_user_id,type,content,created_time) VALUES "
                    "(:ownerId,:chatId,:fromId1,:type,:msg1,:time1), (:ownerId,:chatId,:fromId2,:type,:msg2,:time2)");
    query.bindValue(":ownerId", friendId);
    query.bindValue(":chatId", userId);
    query.bindValue(":fromId1", friendId);
    query.bindValue(":fromId2", userId);
    query.bindValue(":msgType", 0);
    query.bindValue(":msg1", leaveMsg);
    query.bindValue(":msg2", replyMsg);
    query.bindValue(":time1", timeMs1);
    query.bindValue(":time2", timeMs2);
    if (!query.exec())
    {
        db.rollback();
        msg = QString("用户连接%1插入好友视角好友通过消息时数据库写入失败，原因: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","acceptFriendApply"},{"status","fail"},{"msg","通过添加好友失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    //提交事务
    if (!db.commit())
    {
        db.rollback();
        msg = QString("用户连接%1通过好友申请时数据库提交失败").arg(descriptor);
        LOG_WARN(msg);
        json = {{"type","acceptFriendApply"},{"status","fail"},{"msg","通过添加好友失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    msg = QString("用户连接%1通过了用户id：%2的好友申请!").arg(descriptor).arg(friendId);
    LOG_WARN(msg);
    QPointer<ChatSocket> guard(this);
    if (guard)
    {
        QMetaObject::invokeMethod(guard, "sendAcceptFriendApply", Qt::QueuedConnection,
                                  Q_ARG(int, userId), Q_ARG(int, friendId), Q_ARG(QString, leaveMsg),
                                  Q_ARG(QString, replyMsg), Q_ARG(qint64, timeMs1), Q_ARG(qint64, timeMs2));
    }
    json["type"] = "acceptFriendApply";
    json["status"] = "ok";
    json["friendId"] = friendId;
    json["timestamp"] = timeMs2;
    json["msg"] = "已通过对方的好友申请！";
    return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
}

QByteArray ChatSocket::checkRejectFriendApply(int friendId, int userId)
{
    QString msg;
    QJsonObject json;
    qint64 time = QDateTime::currentMSecsSinceEpoch();
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery query(db);
    query.prepare("UPDATE friend_request SET status=2, handled_time=:time WHERE from_user_id=:friendId AND to_user_id=:userId");
    query.bindValue(":friendId", friendId);
    query.bindValue(":userId", userId);
    query.bindValue(":time", time);
    if (!query.exec())
    {
        msg = QString("用户连接%1拒绝好友申请消息时数据库写入失败，原因: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","rejectFriendApply"},{"status","fail"},{"msg","拒绝添加好友失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    if (query.numRowsAffected() == 0)
    {
        msg = QString("用户连接%1拒绝好友申请时未找到对应申请记录").arg(descriptor);
        LOG_WARN(msg);
        json = {{"type", "rejectFriendApply"}, {"status", "fail"}, {"msg", "未找到好友申请记录"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    msg = QString("用户连接%1拒绝了用户id：%2的好友申请!").arg(descriptor).arg(friendId);
    LOG_WARN(msg);
    QPointer<ChatSocket> guard(this);
    if (guard)
    {
        QMetaObject::invokeMethod(guard, "sendRejectFriendApply", Qt::QueuedConnection,
                                    Q_ARG(int, userId), Q_ARG(int, friendId), Q_ARG(qint64, time));
    }
    json["type"] = "rejectFriendApply";
    json["status"] = "ok";
    json["friendId"] = friendId;
    json["timestamp"] = time;
    json["msg"] = "已拒绝对方的好友申请！";
    return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
}

QByteArray ChatSocket::checkDeleteUser(int userId, int friendId)
{
    QString msg;
    QJsonObject json;
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery query(db);
    // 开启事务
    if (!db.transaction())
    {
        LOG_WARN("删除好友时开启事务失败");
        json = {{"type","deleteFriend"},{"status","fail"},{"msg","服务器错误，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    // 删除好友关系（双向）
    query.prepare("DELETE FROM friend_relation WHERE (user_id=:u AND friend_id=:f) OR (user_id=:f AND friend_id=:u)");
    query.bindValue(":u", userId);
    query.bindValue(":f", friendId);
    if (!query.exec())
    {
        db.rollback();
        LOG_WARN(QString("删除好友关系失败: %1").arg(query.lastError().text()));
        json = {{"type","deleteFriend"},{"status","fail"},{"msg","删除好友失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    if (query.numRowsAffected() == 0)
    {
        db.rollback();
        json = {{"type","deleteFriend"},{"status","fail"},{"msg","你们不是好友关系！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    // 删除好友申请记录
    query.prepare("DELETE FROM friend_request WHERE (from_user_id=:u AND to_user_id=:f) OR (from_user_id=:f AND to_user_id=:u)");
    query.bindValue(":u", userId);
    query.bindValue(":f", friendId);
    query.exec();
    if (!db.commit())
    {
        db.rollback();
        LOG_WARN("提交事务失败");
        json = {{"type","deleteFriend"},{"status","fail"},{"msg","删除好友失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }

    msg = QString("用户连接%1删除了用户id为：%2的好友!").arg(descriptor).arg(friendId);
    LOG_WARN(msg);
    QPointer<ChatSocket> guard(this);
    if (guard)
    {
        QMetaObject::invokeMethod(guard, "sendDeletedFriend", Qt::QueuedConnection,
                                  Q_ARG(int, userId), Q_ARG(int, friendId));
    }
    json = {{"type","deleteFriend"},{"status","ok"},{"friendId", friendId},{"msg","删除好友成功！"}};
    return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
}

QByteArray ChatSocket::checkUpdatedUserInfo(int userId, const QString &username, const QByteArray &profile)
{
    QJsonObject json;
    QString msg;
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery update(db);
    update.prepare("UPDATE user SET username = :username, profile = :profile WHERE id = :userId");
    update.bindValue(":username", username);
    update.bindValue(":profile", profile);
    update.bindValue(":userId", userId);
    if (!update.exec())
    {
        msg = QString("用户连接%1更新用户名时数据库更新出错，原因: %2").arg(descriptor).arg(update.lastError().text());
        LOG_WARN(msg);
        json = {{"type","userInfoUpdated"},{"status","fail"},{"msg","用户名更新失败,请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    if (update.numRowsAffected() == 0)
    {
        msg = QString("用户连接%1更新用户名时未找到用户").arg(descriptor);
        LOG_WARN(msg);
        json = {{"type","userInfoUpdated"},{"status","fail"},{"msg","用户不存在，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    QList<int> userIdList;
    QSqlQuery query(db);
    query.prepare("SELECT friend_id FROM friend_relation WHERE user_id = :userId");
    query.bindValue(":userId", userId);
    if (!query.exec())
    {
        msg = QString("用户连接%1广播用户信息更新时查询好友列表失败，原因: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        json = {{"type","userInfoUpdated"},{"status","fail"},{"msg","用户信息更新失败，请重试！"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    while(query.next())
    {
        userIdList.append(query.value(0).toInt());
    }
    // emit userInfoUpdatedBroadcast(userId, username, profile, userIdList);
    QPointer<ChatSocket> guard(this);
    if (guard)
    {
        QMetaObject::invokeMethod(guard, "sendUserInfoUpdated", Qt::QueuedConnection,
                                  Q_ARG(int, userId), Q_ARG(QString, username), Q_ARG(QByteArray, profile), Q_ARG(QList<int>, userIdList));
    }
    msg = QString("用户连接%1的用户信息修改成功!").arg(descriptor);
    LOG_WARN(msg);
    json["type"] = "userInfoUpdated";
    json["status"] = "ok";
    json["userId"] = userId;
    json["username"] = username;
    json["profile"] = QString(profile.toBase64());
    json["msg"] = "用户信息修改成功！";
    return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
}

QByteArray ChatSocket::checkNewMessage(int chatId, int sendId, int type, const QString &content)
{
    QString msg;
    QJsonObject json;
    qint64 time = QDateTime::currentMSecsSinceEpoch();
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery insert(db);
    // 主视角
    insert.prepare("INSERT INTO message (owner_user_id, chat_user_id, from_user_id, type, content, created_time) "
                   "VALUES (:ownerId, :chatId, :sendId, :type, :content, :time)");
    insert.bindValue(":ownerId", mUserId.load());
    insert.bindValue(":chatId", chatId);
    insert.bindValue(":sendId", sendId);
    insert.bindValue(":type", type);
    insert.bindValue(":content", content);
    insert.bindValue(":time", time);
    if (!insert.exec())
    {
        msg = QString("用户连接%1在会话id：%2发送信息时数据库写入失败: %3").arg(descriptor).arg(chatId).arg(insert.lastError().text());
        LOG_WARN(msg);
        json = {{"type", "newMessage"}, {"status", "fail"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }
    // 好友视角
    insert.prepare("INSERT INTO message (owner_user_id, chat_user_id, from_user_id, type, content, created_time) "
                   "VALUES (:ownerId, :chatId, :sendId, :type, :content, :time)");
    insert.bindValue(":ownerId", chatId);
    insert.bindValue(":chatId", mUserId.load());
    insert.bindValue(":sendId", mUserId.load());
    insert.bindValue(":type", type);
    insert.bindValue(":content", content);
    insert.bindValue(":time", time);
    if (!insert.exec())
    {
        msg = QString("用户连接%1在会话id：%2发送信息时数据库写入失败: %3").arg(descriptor).arg(chatId).arg(insert.lastError().text());
        LOG_WARN(msg);
        json = {{"type", "newMessage"}, {"status", "fail"}};
        return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    }

    msg = QString("用户连接%1已成功在会话id：%2发送信息!").arg(descriptor).arg(chatId);
    LOG_WARN(msg);
    QPointer<ChatSocket> guard(this);
    if (guard)
    {
        QMetaObject::invokeMethod(guard, "sendNewMessage", Qt::QueuedConnection,
                                  Q_ARG(int, chatId), Q_ARG(int, sendId), Q_ARG(int, type),
                                  Q_ARG(QString, content), Q_ARG(qint64, time));
    }
    json["type"] = "newMessage";
    json["status"] = "ok";
    json["chatId"] = chatId;
    json["sendId"] = sendId;
    json["msgType"] = type;
    json["content"] = content;
    json["timestamp"] = time;
    json["isSelf"] = true;
    return QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
}

void ChatSocket::pullFrinedList(int userId)
{
    QString msg;
    QJsonObject json;
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery query(db);

    query.prepare("SELECT fr.friend_id, u.username, u.profile "
                    "FROM friend_relation fr LEFT JOIN user u ON u.id = fr.friend_id "
                    "WHERE fr.user_id = :userId ORDER BY fr.created_time ASC");
    query.bindValue(":userId", userId);
    if (!query.exec())
    {
        msg = QString("用户连接%1离线拉取好友列表失败，原因: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        return;
    }
    QJsonArray friendArray;
    while(query.next())
    {
        QJsonObject obj;
        obj["userId"] = query.value(0).toInt();
        obj["username"] = query.value(1).toString();
        QByteArray profile = query.value(2).toByteArray();
        obj["profile"] = QString(profile.toBase64());
        friendArray.append(obj);
    }

    json["type"] = "pullFriendList";
    json["friends"] = friendArray;
    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    responeHandle(data);  // 直接在当前线程发送
}

void ChatSocket::pullFriendApply(int userId)
{
    QString msg;
    QJsonObject json;
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery query(db);
    // 查询所有与 userId 相关的申请（作为申请人或被申请人），按时间倒序
    query.prepare("SELECT fr.from_user_id, fr.to_user_id, fr.leave_msg, fr.status, "
        "fr.handled_time, u_from.username, u_from.profile, u_to.username, u_to.profile "
        "FROM friend_request fr LEFT JOIN user u_from ON u_from.id = fr.from_user_id "
        "LEFT JOIN user u_to ON u_to.id = fr.to_user_id "
        "WHERE fr.from_user_id = :userId OR fr.to_user_id = :userId "
        "ORDER BY fr.created_time ASC");
    query.bindValue(":userId", userId);
    if (!query.exec())
    {
        QString msg = QString("用户%1拉取好友申请记录失败: %2").arg(userId).arg(query.lastError().text());
        LOG_WARN(msg);
        return;
    }

    QJsonArray applyArray;
    while (query.next())
    {
        QJsonObject obj;

        if(query.value(0).toInt() == userId) // 来自自己
        {
            obj["friendId"] = query.value(1).toInt();
            obj["leaveMsg"] = query.value(2).toString();
            obj["applyStatus"] = query.value(3).toInt();
            obj["isFriend"] = (query.value(3).toInt() == 1) ? 1 : 0;
            obj["isSelf"] = 1;
            obj["timestamp"] = query.value(4).toLongLong();
            obj["friendUsername"] = query.value(7).toString();
            QByteArray friendProfile = query.value(8).toByteArray();
            obj["friendProfile"] = QString(friendProfile.toBase64());
        }
        else
        {
            obj["friendId"] = query.value(0).toInt();
            obj["leaveMsg"] = query.value(2).toString();
            obj["applyStatus"] = query.value(3).toInt();
            obj["isFriend"] = (query.value(3).toInt() == 1) ? 1 : 0;
            obj["isSelf"] = 0;
            obj["timestamp"] = query.value(4).toLongLong();
            obj["friendUsername"] = query.value(5).toString();
            QByteArray friendProfile = query.value(6).toByteArray();
            obj["friendProfile"] = QString(friendProfile.toBase64());

        }
        applyArray.append(obj);
    }

    json["type"] = "pullFriendApplyList";
    json["applies"] = applyArray;
    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    responeHandle(data);
}

void ChatSocket::pullOfflineMessage(int userId)
{
    QString msg;
    QJsonObject json;
    QSqlDatabase db = SqlConnPool::Instance().getConn();
    QSqlQuery query(db);

    query.prepare("SELECT m.chat_user_id, m.from_user_id, m.type, m.content, m.created_time "
                  "FROM message m WHERE m.owner_user_id = :ownerId "
                  "AND (m.chat_user_id IN (SELECT friend_id FROM friend_relation WHERE user_id = :ownerId)) "
                  "ORDER BY m.created_time ASC LIMIT 200");
    query.bindValue(":ownerId", userId);
    if (!query.exec())
    {
        msg = QString("用户连接%1离线拉取消息列表时数据库查询失败，原因: %2").arg(descriptor).arg(query.lastError().text());
        LOG_WARN(msg);
        return;
    }

    QJsonArray msgArray;
    while(query.next())
    {
        QJsonObject obj;
        obj["chatId"] = query.value(0).toInt();
        obj["sendId"] = query.value(1).toInt();
        obj["content"] = query.value(3).toString();
        obj["msgType"] = query.value(2).toInt();
        obj["timestamp"] = query.value(4).toLongLong();
        obj["isSelf"] = (query.value(1).toInt() == userId) ? true : false;
        msgArray.append(obj);
    }

    json["type"] = "newMessages";
    json["messages"] = msgArray;
    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    responeHandle(data);
}

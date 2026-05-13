#include "SqlConnPool.h"

SqlConnPool& SqlConnPool::Instance()
{
    static SqlConnPool instance;
    return instance;
}

void SqlConnPool::init(const QString &host, int port, const QString &dbname, const QString &user, const QString &password)
{
    QMutexLocker locker(&mutex);
    mHost = host;
    mPort = port;
    mDbname = dbname;
    mUser = user;
    mPassword = password;
}

QSqlDatabase SqlConnPool::getConn()
{
    // 如果当前线程已经有连接，直接返回（QThreadStorage 自动管理）
    if (threadConn.hasLocalData()) return *(threadConn.localData());
    // 否则第一次创建
    QString connName = QString("Conn_%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    QSqlDatabase* db = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL", connName));
    db->setHostName(mHost);
    db->setPort(mPort);
    db->setDatabaseName(mDbname);
    db->setUserName(mUser);
    db->setPassword(mPassword);
    if (!db->open())
    {
        qWarning() << "Failed to open MySQL:" << db->lastError().text();
    }
    // 存入线程局部存储
    threadConn.setLocalData(db);
    return *db;
}

void SqlConnPool::closeConn()
{
    threadConn.setLocalData(nullptr);
    mHost.clear();
    mUser.clear();
    mPassword.clear();
    mDbname.clear();
    mPort = 0;
}

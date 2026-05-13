#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#pragma once
#include <QSqlDatabase>
#include <QThread>
#include <QThreadStorage>
#include <QWaitCondition>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QMutex>
#include <QQueue>
#include <QString>
#include <QMap>
#include <QDebug>

class SqlConnPool
{
public:
    static SqlConnPool& Instance();
    void init(const QString& host, int port, const QString& dbname,
              const QString& user, const QString& password);
    QSqlDatabase getConn();
    void closeConn();
private:
    SqlConnPool() = default;
    ~SqlConnPool() = default;
    Q_DISABLE_COPY(SqlConnPool)

    QString mHost;
    int mPort;
    QString mDbname;
    QString mUser;
    QString mPassword;
    QThreadStorage<QSqlDatabase*> threadConn;
    QMutex mutex;
};

#endif // SQLCONNPOOL_H

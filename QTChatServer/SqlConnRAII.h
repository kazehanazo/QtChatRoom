#ifndef SQLCONNRAII_H
#define SQLCONNRAII_H

#pragma once
#include "SqlConnPool.h"

class SqlConnRAII
{
public:
    SqlConnRAII(QSqlDatabase& db, SqlConnPool* pool)
    {
        Q_ASSERT(pool);
        db = pool->getConn();
        mDB = db;
        mPool = pool;
    }
    ~SqlConnRAII();
private:
    QSqlDatabase mDB;
    SqlConnPool* mPool;
};

#endif // SQLCONNRAII_H

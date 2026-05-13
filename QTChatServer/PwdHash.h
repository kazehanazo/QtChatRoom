#ifndef PWDHASH_H
#define PWDHASH_H

#include <QString>

class PwdHash
{
public:
    static bool initLibsodium();
    static QString hashPassword(const QString& pwd);
    static bool verifyPassword(const QString& pwd, const QString& hashPwd);
};

#endif // PWDHASH_H

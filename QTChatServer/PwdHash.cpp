#include "PwdHash.h"

bool PwdHash::initLibsodium()
{
    if (sodium_init() < 0)
    {
        qWarning() << "libsodium init failed！";
        return false;
    }
    qDebug() << "libsodium init success!";
    return true;
}

QString PwdHash::hashPassword(const QString &pwd)
{
    char hashPwd[crypto_pwhash_STRBYTES];
    QByteArray pwdBytes = pwd.toUtf8();

    if (crypto_pwhash_str(
            hashPwd,
            pwdBytes.constData(),
            pwdBytes.size(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
    {
        qWarning() << "Password hashing failed (out of memory)";
        return QString();
    }
    return QString(hashPwd);  // 返回包含盐和参数的完整hash字符串
}

bool PwdHash::verifyPassword(const QString &pwd, const QString &hashPwd)
{
    QByteArray pwdBytes = password.toUtf8();
    QByteArray hashBytes = hashPwd.toUtf8();
    if (crypto_pwhash_str_verify(hashBytes.constData(), pwdBytes.constData(), pwdBytes.size()) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

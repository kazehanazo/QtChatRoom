#ifndef HASHPASSWORD_H
#define HASHPASSWORD_H

#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QByteArray>
#include <QString>
#include <QDebug>

class HashPassword
{
public:
    QByteArray createSalt(int length)
    {

    }
    QByteArray createHash(const QByteArray &pwd)
    {

    }
    bool verifyPassword()
    {

    }

};

#endif // HASHPASSWORD_H

#ifndef USERSEARCHMODEL_H
#define USERSEARCHMODEL_H

#pragma once

#include <QWidget>
#include <QAbstractListModel>
#include <QList>

#include "UserInfoSDK.h"

class UserSearchModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit UserSearchModel(QObject *parent = nullptr);

    void setUsers(const QList<UserInfo> &users);
    UserInfo getUser(int row) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    QList<UserInfo> mUsers;
};

#endif // USERSEARCHWIDGET_H

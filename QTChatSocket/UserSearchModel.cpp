#include "UserSearchModel.h"

UserSearchModel::UserSearchModel(QObject *parent): QAbstractListModel(parent) {}

void UserSearchModel::setUsers(const QList<UserInfo> &users)
{
    beginResetModel();
    mUsers = users;
    endResetModel();
}

UserInfo UserSearchModel::getUser(int row) const
{
    return (row >= 0 && row < mUsers.size()) ? mUsers[row] : UserInfo{};
}

int UserSearchModel::rowCount(const QModelIndex &parent) const
{
    return mUsers.size();
}

QVariant UserSearchModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return {};
    const UserInfo &user = mUsers.at(index.row());
    if (role == Qt::DisplayRole)
    {
        return user.username;
    }
    if (role == Qt::UserRole)
    {
        return QVariant::fromValue(user);
    }
    return {};
}

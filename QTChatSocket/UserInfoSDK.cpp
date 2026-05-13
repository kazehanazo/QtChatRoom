#include "UserInfoSDK.h"

UserInfoSDK& UserInfoSDK::instance()
{
    static UserInfoSDK instance;
    return instance;
}

UserInfoSDK::UserInfoSDK(QObject* parent)
    : QObject(parent)
{

}

QPixmap UserInfoSDK::makeProfile(const QPixmap &src, int width, int height)
{
    //创建图像
    QPixmap pix(src);
    pix = pix.scaled(width, height, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    // 创建目标图像的透明背景
    QPixmap dest(width, height);
    dest.fill(Qt::transparent);
    // 开始绘制圆形裁剪
    QPainter painter(&dest);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, width, height);
    painter.setClipPath(path);
    painter.drawPixmap((width - pix.width()) / 2, (height - pix.height()) / 2,pix);
    return dest;
}

QSharedPointer<const UserInfo> UserInfoSDK::getUser(int userId) const
{
    auto it = mUserMap.constFind(userId);
    if (it == mUserMap.constEnd())
    {
        qWarning() << "[UserInfoSDK] user not found:" << userId;
        return {};
    }
    return it.value(); // 自动转成 const
}

QList<QSharedPointer<const UserInfo> > UserInfoSDK::getFriendList() const
{
    QList<QSharedPointer<const UserInfo>> friends;
    for (auto it = mUserMap.constBegin(); it != mUserMap.constEnd(); ++it)
    {
        if (it.value()->isFriend == 1)
        {
            friends.append(it.value());  // 智能指针自动转换
        }
    }
    return friends;
}

void UserInfoSDK::clearAll()
{
    mUserMap.clear();
}

void UserInfoSDK::updateUser(const UserInfo &info)
{
    auto it = mUserMap.find(info.userId);
    if (it == mUserMap.end())
    {
        // 不存在就全部创建
        mUserMap[info.userId] = QSharedPointer<UserInfo>::create(info);
        emit userInfoUpdated(info.userId);
        return;
    }
    // 已存在：增量更新（按字段更改）
    UserInfo *u = it.value().data();
    // 仅在传入的数据有效时更新（你可以自己定义“有效”标准）
    if (!info.username.isEmpty())
    {
        u->username = info.username;
    }
    if (!info.profile.isEmpty())
    {
        u->profile = info.profile;
    }
    // 如果是好友搜索返回的状态
    if (info.isFriend != -1)
    {
        u->isFriend = info.isFriend;
    }
    if (info.applyStatus != -1)
    {
        u->applyStatus = info.applyStatus;
    }
    if (info.isSelf != -1)
    {
        u->isSelf = info.isSelf;
    }
    emit userInfoUpdated(info.userId);
}




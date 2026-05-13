#include "UserInfoBubbleWidget.h"

UserInfoBubbleWidget::UserInfoBubbleWidget(int userId, QWidget *parent): QWidget{parent}, mUserId(userId)
{
    setAttribute(Qt::WA_TranslucentBackground);
    info = UserInfoSDK::instance().getUser(mUserId);
    if (info)
    {
        mProfile.loadFromData(info->profile);
    }
    else
    {
        qWarning() << "FriendApplyBubbleWidget: user not found:" << mUserId;
        return;
    }
    connect(&UserInfoSDK::instance(), &UserInfoSDK::userInfoUpdated, this, &UserInfoBubbleWidget::onUserInfoUpdated);
    initBubble();
}

void UserInfoBubbleWidget::initBubble()
{
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 5, 10, 5);

    // 头像
    mProfileLabel = new QLabel;
    mProfileLabel->setFixedSize(32,32);
    mProfileLabel->setScaledContents(true);
    mProfileLabel->setPixmap(UserInfoSDK::instance().makeProfile(mProfile, 30, 30));

    // 中间区域
    auto centerLayout = new QVBoxLayout;
    centerLayout->setSpacing(4);
    mUsernameLabel = new QLabel;
    mUsernameLabel->setStyleSheet("font-size:14px;font-weight:500;");
    mUsernameLabel->setText(info->username);
    centerLayout->addWidget(mUsernameLabel);
    mainLayout->addWidget(mProfileLabel);
    mainLayout->addLayout(centerLayout);

    setStyleSheet("UserInfoBubbleWidget:hover{background:#f5f5f5;}");
}

void UserInfoBubbleWidget::onUserInfoUpdated(int userId)
{
    if (userId != mUserId) return;
    auto info = UserInfoSDK::instance().getUser(userId);
    if (!info) return;
    mUsernameLabel->setText(info->username);
    mProfile.loadFromData(info->profile);
    QPixmap profile = UserInfoSDK::instance().makeProfile(mProfile, 24, 24);
    mProfileLabel->setPixmap(profile);
}

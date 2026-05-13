#include "FriendApplyBubbleWidget.h"

FriendApplyBubbleWidget::FriendApplyBubbleWidget(const FriendApplyData &data, QWidget *parent): QWidget{parent}, mData(data)
{
    setAttribute(Qt::WA_TranslucentBackground);
    info = UserInfoSDK::instance().getUser(mData.friendId);
    if (info)
    {
        mProfile.loadFromData(info->profile);
    }
    else
    {
        qWarning() << "FriendApplyBubbleWidget: user not found:" << mData.friendId;
        return;
    }
    connect(&UserInfoSDK::instance(), &UserInfoSDK::userInfoUpdated, this, &FriendApplyBubbleWidget::onUserInfoUpdated);
    initBubble();
}

void FriendApplyBubbleWidget::initBubble()
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
    mMsgLabel = new QLabel;
    mMsgLabel->setStyleSheet("font-size:12px;color:gray;");
    mMsgLabel->setWordWrap(false);
    mMsgLabel->setFixedWidth(80);
    mMsgLabel->setFixedHeight(16);
    centerLayout->addWidget(mUsernameLabel);
    centerLayout->addWidget(mMsgLabel);

    // 右侧
    auto rightLayout = new QVBoxLayout;
    rightLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    mTimeLabel = new QLabel;
    mTimeLabel->setStyleSheet("font-size:11px;color:#999;");
    QDateTime dt = QDateTime::fromSecsSinceEpoch(mData.timestamp);
    mTimeLabel->setText(dt.toString("hh:mm"));
    rightLayout->addWidget(mTimeLabel);

    mainLayout->addWidget(mProfileLabel);
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(rightLayout);

    setStyleSheet("FriendApplyBubbleWidget:hover{background:#f5f5f5;}");
    refreshStatus();
}

void FriendApplyBubbleWidget::updateBubble(const FriendApplyData &data)
{
    mData = data;
    QDateTime dt = QDateTime::fromSecsSinceEpoch(mData.timestamp);
    mTimeLabel->setText(dt.toString("hh:mm"));
    refreshStatus();
}

void FriendApplyBubbleWidget::onUserInfoUpdated(int userId)
{
    if (userId != mData.friendId) return;
    auto info = UserInfoSDK::instance().getUser(userId);
    if (!info) return;
    mUsernameLabel->setText(info->username);
    mProfile.loadFromData(info->profile);
    QPixmap profile = UserInfoSDK::instance().makeProfile(mProfile, 24, 24);
    mProfileLabel->setPixmap(profile);
}

void FriendApplyBubbleWidget::refreshStatus()
{
    if (mData.status == FriendApplyStatus::Pending)
    {
        if (info->isSelf)
        {
            mMsgLabel->setText("等待验证");
        }
        else
        {
            QFontMetrics fm(mMsgLabel->font());
            QString elidedText = fm.elidedText(mData.leaveMsg, Qt::ElideRight, mMsgLabel->width());
            mMsgLabel->setText(elidedText);
        }
    }
    else if (mData.status == FriendApplyStatus::Accepted)
    {
        mMsgLabel->setText("已添加");
    }
    else if (mData.status == FriendApplyStatus::Rejected)
    {
        mMsgLabel->setText("已拒绝");
    }
}

#include "ChatBubbleWidget.h"

ChatBubbleWidget::ChatBubbleWidget(const ChatData& data, QWidget *parent): QWidget{parent}, mData(data)
{
    setAttribute(Qt::WA_TranslucentBackground);
    info = UserInfoSDK::instance().getUser(mData.chatId);
    if (info)
    {
        mProfile.loadFromData(info->profile);
    }
    else
    {
        qWarning() << "ChatBubbleWidget: user not found:" << mData.chatId;
    }
    connect(&UserInfoSDK::instance(), &UserInfoSDK::userInfoUpdated, this, &ChatBubbleWidget::onUserInfoUpdated);
    initBubble();
    qWarning() << "ChatBubbleWidget:" << mData.lastTimestamp;
}

void ChatBubbleWidget::initBubble()
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
    mMsgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mMsgLabel->setFixedWidth(80);
    mMsgLabel->setFixedHeight(16);
    //设置文本过长时省略文本
    if(mData.lastMessage.type == MessageType::Text)
    {
        QFontMetrics fm(mMsgLabel->font());
        QString elidedText = fm.elidedText(mData.lastMessage.content, Qt::ElideRight, mMsgLabel->width());
        mMsgLabel->setText(elidedText);
        centerLayout->addWidget(mUsernameLabel);
        centerLayout->addWidget(mMsgLabel);
    }
    else if(mData.lastMessage.type == MessageType::Image)
    {
        mMsgLabel->setText("图片");
        centerLayout->addWidget(mUsernameLabel);
        centerLayout->addWidget(mMsgLabel);
    }

    // 右侧
    auto rightLayout = new QVBoxLayout;
    rightLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);

    mTimeLabel = new QLabel;
    mTimeLabel->setStyleSheet("font-size:11px;color:#999;");
    QDateTime dt = QDateTime::fromSecsSinceEpoch(mData.lastTimestamp);
    mTimeLabel->setText(dt.toString("hh:mm"));

    mUnreadLabel = new QLabel;
    mUnreadLabel->setFixedSize(18, 18);
    mUnreadLabel->setAlignment(Qt::AlignCenter);
    mUnreadLabel->setStyleSheet("background:red;color:white;border-radius:9px;font-size:11px;");
    mUnreadLabel->hide();
    if (mData.unreadCount > 0)
    {
        mUnreadLabel->show();
        mUnreadLabel->setText(QString::number(mData.unreadCount));
    }

    rightLayout->addWidget(mTimeLabel);
    rightLayout->addSpacing(8);
    rightLayout->addWidget(mUnreadLabel, 0, Qt::AlignRight);

    mainLayout->addWidget(mProfileLabel);
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(rightLayout);

    setStyleSheet("ChatBubbleWidget:hover{background:#f5f5f5;}");
}

void ChatBubbleWidget::updateBubble(const ChatData &data)
{
    mData = data;
    if(mData.lastMessage.type == MessageType::Text) // 更新最近一条信息
    {
        QFontMetrics fm(mMsgLabel->font());
        QString elidedText = fm.elidedText(mData.lastMessage.content, Qt::ElideRight, mMsgLabel->width());
        mMsgLabel->setText(elidedText);
    }
    else if(mData.lastMessage.type == MessageType::Image)
    {
        mMsgLabel->setText("图片");
    }
    QDateTime dt = QDateTime::fromSecsSinceEpoch(mData.lastTimestamp); // 更新最新消息发送时间
    mTimeLabel->setText(dt.toString("hh:mm"));
    if (mData.unreadCount > 0) // 更新未读消息红点图标
    {
        mUnreadLabel->show();
        mUnreadLabel->setText(QString::number(mData.unreadCount));
    }
    else
    {
        mUnreadLabel->hide();
    }
}

void ChatBubbleWidget::onUserInfoUpdated(int userId)
{
    if (userId != mData.chatId) return;
    auto info = UserInfoSDK::instance().getUser(userId);
    if (!info) return;
    mUsernameLabel->setText(info->username);
    mProfile.loadFromData(info->profile);
    QPixmap profile = UserInfoSDK::instance().makeProfile(mProfile, 24, 24);
    mProfileLabel->setPixmap(profile);
}

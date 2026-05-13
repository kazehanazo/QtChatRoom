#include "MessageBubbleWidget.h"

MessageBubbleWidget::MessageBubbleWidget(const MessageData& data, QWidget *parent): QWidget{parent}, mData(data)
{
    setAttribute(Qt::WA_TranslucentBackground);
    info = UserInfoSDK::instance().getUser(mData.sendId);
    if (info)
    {
        mProfile.loadFromData(info->profile);
    }
    else
    {
        qWarning() << "ChatBubbleWidget: user not found:" << mData.sendId;
    }
    connect(&UserInfoSDK::instance(), &UserInfoSDK::userInfoUpdated, this, &MessageBubbleWidget::onUserInfoUpdated);
    initBubble();
}

void MessageBubbleWidget::initBubble()
{

    //创建聊天气泡主布局
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2);
    // 创建聊天信息布局
    auto msgLayout = new QHBoxLayout();
    msgLayout->setContentsMargins(15, 5, 15, 5);
    msgLayout->setSpacing(5);
    // 发送时间
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(mData.timestamp);
    QString sendTime = dt.toString("yyyy-MM-dd hh:mm");
    mTimeLabel = new QLabel(sendTime);
    mTimeLabel->setStyleSheet("font-size:10px; color:#999;");
    mTimeLabel->setAlignment(Qt::AlignCenter);
    mTimeLabel->setFixedHeight(16);
    // 头像
    mProfileLabel = new QLabel;
    mProfileLabel->setFixedSize(24,24);
    mProfileLabel->setScaledContents(true);
    mProfileLabel->setPixmap(UserInfoSDK::instance().makeProfile(mProfile, 22, 22));
    // 气泡
    mBubbleWidget = new QWidget;
    auto bubbleLayout = new QVBoxLayout(mBubbleWidget);
    bubbleLayout->setContentsMargins(10, 5, 10, 5);
    //根据内容类型设置不同的气泡内容
    if (mData.type == MessageType::Text)
    {
        QLabel* textLabel = new QLabel(mData.content);
        textLabel->setWordWrap(true);
        textLabel->setMaximumWidth(350);
        bubbleLayout->addWidget(textLabel);
    }
    else if(mData.type == MessageType::Image)
    {
        // 将 Base64 字符串解码为图片数据
        QByteArray imageData = QByteArray::fromBase64(mData.content.toLatin1());
        QPixmap pixmap;
        if (pixmap.loadFromData(imageData))
        {
            // 限制图片最大尺寸（例如宽度 200，高度自动，保持比例）
            QPixmap scaled = pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QLabel* imageLabel = new QLabel;
            imageLabel->setPixmap(scaled);
            imageLabel->setScaledContents(true);
            bubbleLayout->addWidget(imageLabel);
        } else
        {
            // 图片解码失败，显示错误提示
            QLabel* errorLabel = new QLabel("[图片加载失败]");
            bubbleLayout->addWidget(errorLabel);
        }
    }
    //判断是自己还是对方的气泡
    if (mData.isSelf)
    {
        mBubbleWidget->setStyleSheet("background-color:#ffffff;""border-radius:8px;");
        msgLayout->addStretch();
        msgLayout->addWidget(mBubbleWidget);
        msgLayout->addWidget(mProfileLabel);
    }
    else
    {
        mBubbleWidget->setStyleSheet("background-color:#8ed2ff;""border-radius:8px;");
        msgLayout->addWidget(mProfileLabel);
        msgLayout->addWidget(mBubbleWidget);
        msgLayout->addStretch();
    }
    mainLayout->addWidget(mTimeLabel, 0, Qt::AlignCenter);
    mainLayout->addLayout(msgLayout);
}

void MessageBubbleWidget::onUserInfoUpdated(int userId)
{
    if (userId != mData.sendId) return;
    auto info = UserInfoSDK::instance().getUser(userId);
    if (!info) return;
    mProfile.loadFromData(info->profile);
    QPixmap profile = UserInfoSDK::instance().makeProfile(mProfile, 24, 24);
    mProfileLabel->setPixmap(profile);
}

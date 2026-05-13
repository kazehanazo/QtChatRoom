#ifndef CHATBUBBLEWIDGET_H
#define CHATBUBBLEWIDGET_H

#pragma once

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QFontMetrics>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>

#include "MessageStore.h"
#include "UserInfoSDK.h"

class ChatBubbleWidget: public QWidget
{
    Q_OBJECT
public:
    explicit ChatBubbleWidget(const ChatData& data, QWidget *parent = nullptr);

    void initBubble();
    void updateBubble(const ChatData &data);

private slots:
    void onUserInfoUpdated(int userId);

private:
    ChatData mData;
    QSharedPointer<const UserInfo> info;
    QLabel* mProfileLabel = nullptr;
    QPixmap mProfile;
    QLabel* mUsernameLabel;
    QLabel* mMsgLabel;
    QLabel* mTimeLabel;
    QLabel* mUnreadLabel;
    QWidget* mBubbleWidget = nullptr;
    QVBoxLayout* mBubbleLayout = nullptr;

    int mRadius = 8;
    int mArrowSize = 6;
    int mMaxWidth = 360;
};

#endif // CHATBUBBLEWIDGET_H

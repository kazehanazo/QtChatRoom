#ifndef FRIENDAPPLYBUBBLEWIDGET_H
#define FRIENDAPPLYBUBBLEWIDGET_H

#pragma once

#include <QObject>
#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QFontMetrics>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>

#include "MessageStore.h"
#include "FriendApplyStore.h"
#include "UserInfoSDK.h"

class FriendApplyBubbleWidget: public QWidget
{
    Q_OBJECT

public:
    explicit FriendApplyBubbleWidget(const FriendApplyData& data, QWidget *parent = nullptr);
    void initBubble();
    void updateBubble(const FriendApplyData &data);

private slots:
    void onUserInfoUpdated(int userId);
    //void onStatusUpdated(const FriendApplyData& apply);

private:
    void refreshStatus();

    FriendApplyData mData;
    QSharedPointer<const UserInfo> info;
    QLabel* mProfileLabel = nullptr;
    QPixmap mProfile;
    QLabel* mUsernameLabel;
    QLabel* mMsgLabel;
    QLabel* mTimeLabel;
    QWidget* mBubbleWidget = nullptr;
    QVBoxLayout* mBubbleLayout = nullptr;

    int mRadius = 8;
    int mArrowSize = 6;
    int mMaxWidth = 360;
};

#endif // FRIENDAPPLYBUBBLEWIDGET_H

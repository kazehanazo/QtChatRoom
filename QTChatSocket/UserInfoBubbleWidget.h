#ifndef USERINFOBUBBLEWIDGET_H
#define USERINFOBUBBLEWIDGET_H

#pragma once

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QFontMetrics>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>

#include "MessageModel.h"
#include "UserInfoSDK.h"


class UserInfoBubbleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UserInfoBubbleWidget(int userId, QWidget *parent = nullptr);

    void initBubble();

private slots:
    void onUserInfoUpdated(int userId);

private:
    int mUserId;
    QSharedPointer<const UserInfo> info;
    QLabel* mProfileLabel = nullptr;
    QPixmap mProfile;
    QLabel* mUsernameLabel;
    QWidget* mBubbleWidget = nullptr;
    QVBoxLayout* mBubbleLayout = nullptr;

    int mRadius = 8;
    int mArrowSize = 6;
    int mMaxWidth = 360;
signals:
};

#endif // USERINFOBUBBLEWIDGET_H

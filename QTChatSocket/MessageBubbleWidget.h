#ifndef MESSAGEBUBBLEWIDGET_H
#define MESSAGEBUBBLEWIDGET_H

#pragma once

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>

#include "MessageModel.h"
#include "UserInfoSDK.h"

class MessageBubbleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MessageBubbleWidget(const MessageData& data, QWidget *parent = nullptr);

    void initBubble();

private slots:
    void onUserInfoUpdated(int userId);

private:
    MessageData mData;
    QSharedPointer<const UserInfo> info;
    QLabel* mTimeLabel = nullptr;
    QLabel* mProfileLabel = nullptr;
    QPixmap mProfile; 
    QWidget* mBubbleWidget = nullptr;
    QVBoxLayout* mBubbleLayout = nullptr;

    int mRadius = 8;
    int mArrowSize = 6;
    int mMaxWidth = 360;
};

#endif // CHATBUBBLEWIDGET_H

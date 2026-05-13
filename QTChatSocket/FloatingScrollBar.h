#ifndef FLOATINGSCROLLBAR_H
#define FLOATINGSCROLLBAR_H

#pragma once

#include <QObject>
#include <QEvent>
#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>

class FloatingScrollBar : public QObject
{
    Q_OBJECT
public:
    explicit FloatingScrollBar(QAbstractScrollArea* view);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void updateGeometry();
    void fadeIn();
    void fadeOut();

private:
    QAbstractScrollArea* mView = nullptr;
    QScrollBar* mScrollBar = nullptr;

    QGraphicsOpacityEffect* mOpacityEffect = nullptr;
    QPropertyAnimation* mFadeAnim = nullptr;
    QTimer* mHideTimer = nullptr;

    bool hasScrollBar = false;
};
#endif // FLOATINGSCROLLBAR_H

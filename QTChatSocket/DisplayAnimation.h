#ifndef DISPLAYANIMATION_H
#define DISPLAYANIMATION_H

#pragma once

#include <QObject>
#include <QWidget>
#include <QScreen>
#include <QRegion>
#include <QEvent>
#include <QGuiApplication>
#include <QGraphicsView>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>

class DisplayAnimation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int maskHeight READ maskHeight WRITE setMaskHeight NOTIFY maskHeightChanged)
    Q_PROPERTY(int maskWidth READ maskWidth WRITE setMaskWidth NOTIFY maskWidthChanged)

public:

    enum animationType{None, TopToBottom, BottomToTop, LeftToRight, RightToLeft};
    explicit DisplayAnimation(QWidget* parent);
    int maskHeight() const;
    void setMaskHeight(int height);
    int maskWidth() const;
    void setMaskWidth(int width);
    void enterAnimation(animationType type);
    void exitAnimation(animationType type, bool closeWindow = false, std::function<void()> onFinished = nullptr);
    void animationPlay(animationType type);

signals:
    void maskHeightChanged(int height);
    void maskWidthChanged(int width);

private:
    QWidget* w;
    QGraphicsOpacityEffect* effect;
    QPropertyAnimation* animation;
    QPropertyAnimation* fade;
    QParallelAnimationGroup *group;
    int mMaskHeight = 0;
    int mMaskWidth = 0;

};

#endif // DISPLAYANIMATION_H

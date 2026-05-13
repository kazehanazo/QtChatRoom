#include "DisplayAnimation.h"

DisplayAnimation::DisplayAnimation(QWidget *parent): QObject(parent), w(parent)
{
    effect = new QGraphicsOpacityEffect(w);
    w->setGraphicsEffect(effect);
}

int DisplayAnimation::maskHeight() const
{
    return mMaskHeight;
}

void DisplayAnimation::setMaskHeight(int height)
{
    if(mMaskHeight == height) return;
    mMaskHeight = height;
    emit maskWidthChanged(mMaskHeight);
    QRegion region(0, 0, w->width(), mMaskHeight);
    w->setMask(region);
    w->update();
}

int DisplayAnimation::maskWidth() const
{
    return mMaskWidth;
}

void DisplayAnimation::setMaskWidth(int width)
{
    if(mMaskWidth == width) return;
    mMaskWidth = width;
    emit maskWidthChanged(mMaskWidth);
    QRegion region(0, 0, mMaskWidth, w->height());
    w->setMask(region);
    w->update();
}

void DisplayAnimation::enterAnimation(animationType type)
{
    w->show();
    if(type != None) animationPlay(type);
    fade = new QPropertyAnimation(effect, "opacity");
    fade->setDuration(1000);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    fade->setEasingCurve(QEasingCurve::Linear);
    fade->start(QPropertyAnimation::DeleteWhenStopped);
}

void DisplayAnimation::exitAnimation(animationType type, bool closeWindow, std::function<void()> onFinished)
{
    if(type != None) animationPlay(type);
    fade = new QPropertyAnimation(effect, "opacity");
    fade->setDuration(1000);
    fade->setStartValue(1.0);
    fade->setEndValue(0.0);
    fade->setEasingCurve(QEasingCurve::Linear);
    connect(fade, &QPropertyAnimation::finished, w, [=]() {
        if (onFinished) onFinished();
        if (closeWindow)
        {
            w->close();
        }
        else
        {
            w->hide();
        }
    });
    fade->start(QPropertyAnimation::DeleteWhenStopped);
}

void DisplayAnimation::animationPlay(animationType type)
{
    if(type == TopToBottom)
    {
        animation = new QPropertyAnimation(this, "maskHeight");
        animation->setStartValue(0.0);
        animation->setEndValue(w->height());
    }
    else if(type == BottomToTop)
    {
        animation = new QPropertyAnimation(this, "maskHeight");
        animation->setStartValue(w->height());
        animation->setEndValue(0.0);
    }
    else if(type == LeftToRight)
    {
        animation = new QPropertyAnimation(this, "maskWidth");
        animation->setStartValue(0.0);
        animation->setEndValue(w->width());
    }
    else if(type == RightToLeft)
    {
        animation = new QPropertyAnimation(this, "maskWidth");
        animation->setStartValue(w->width());
        animation->setEndValue(0.0);
    }
    animation->setDuration(1000);
    animation->setEasingCurve(QEasingCurve::Linear);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

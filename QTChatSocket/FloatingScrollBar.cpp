#include "FloatingScrollBar.h"

FloatingScrollBar::FloatingScrollBar(QAbstractScrollArea *view): mView(view)
{
    // 创建悬浮滚动条
    mScrollBar = new QScrollBar(Qt::Vertical, view);
    mScrollBar->setParent(view);
    mScrollBar->raise();
    mScrollBar->hide();

    // 透明度效果
    mOpacityEffect = new QGraphicsOpacityEffect(mScrollBar);
    mOpacityEffect->setOpacity(0.0);
    mScrollBar->setGraphicsEffect(mOpacityEffect);

    // 淡入淡出动画
    mFadeAnim = new QPropertyAnimation(mOpacityEffect, "opacity", this);
    mFadeAnim->setDuration(150);

    // 延迟隐藏（微信是“松开一会才消失”）
    mHideTimer = new QTimer(this);
    mHideTimer->setSingleShot(true);
    mHideTimer->setInterval(300);
    connect(mHideTimer, &QTimer::timeout, this, &FloatingScrollBar::fadeOut);

    // 同步原生滚动条
    auto srcScrollBar = view->verticalScrollBar();

    connect(srcScrollBar, &QScrollBar::rangeChanged, this,[=](int min, int max) {
                mScrollBar->setRange(min, max);
                mScrollBar->setPageStep(srcScrollBar->pageStep());
                hasScrollBar = (max > 0);
                updateGeometry(); });

    connect(srcScrollBar, &QScrollBar::valueChanged, mScrollBar, &QScrollBar::setValue);
    connect(mScrollBar, &QScrollBar::valueChanged, srcScrollBar, &QScrollBar::setValue);

    // 事件监听
    view->viewport()->installEventFilter(this);
    mScrollBar->installEventFilter(this);
    updateGeometry();
}

bool FloatingScrollBar::eventFilter(QObject *obj, QEvent *event)
{
    if (!hasScrollBar) return QObject::eventFilter(obj, event);

    // viewport resize：贴边
    if (obj == mView->viewport() && event->type() == QEvent::Resize)
    {
        updateGeometry();
    }
    // hover 显示
    if (event->type() == QEvent::Enter)
    {
        fadeIn();
        mHideTimer->stop();
    }
    // hover 离开：延迟隐藏
    else if (event->type() == QEvent::Leave)
    {
        mHideTimer->start();
    }
    // 滚动时强制显示
    else if (event->type() == QEvent::Wheel && (obj == mView->viewport() || obj == mScrollBar))
    {
        fadeIn();
        mHideTimer->start();
    }

    return QObject::eventFilter(obj, event);
}

void FloatingScrollBar::updateGeometry()
{
    if (!mView || !mScrollBar) return;
    QRect r = mView->viewport()->rect();
    mScrollBar->setGeometry(r.width() - 8, 2, 6, r.height() - 4);
}

void FloatingScrollBar::fadeIn()
{
    if (!hasScrollBar) return;
    mScrollBar->show();
    mFadeAnim->stop();
    mFadeAnim->setEndValue(1.0);
    mFadeAnim->start();
}

void FloatingScrollBar::fadeOut()
{
    mFadeAnim->stop();
    mFadeAnim->setEndValue(0.0);
    mFadeAnim->start();
}

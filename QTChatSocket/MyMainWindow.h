#ifndef MYMAINWINDOW_H
#define MYMAINWINDOW_H

#pragma once

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QMovie>
#include <QLabel>
#include <QListWidget>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QScreen>
#include <QTcpSocket>
#include <QSettings>
#include <QPixmap>
#include <QBuffer>
#include <QFileDialog>
#include <QInputDialog>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QScrollBar>
#include <QCompleter>

#include "DisplayAnimation.h"
#include "FloatingScrollBar.h"
#include "SocketBusiness.h"
#include "MessageBubbleWidget.h"
#include "ChatBubbleWidget.h"
#include "UserInfoBubbleWidget.h"
#include "FriendApplyBubbleWidget.h"
#include "UserSearchModel.h"
#include "MessageStore.h"
#include "ChatStore.h"
#include "FriendApplyStore.h"
#include "UserInfoSDK.h"

#include "Login.h"
#include "Register.h"
#include "SettingsDialog.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MyMainWindow;
}
QT_END_NAMESPACE

class MyMainWindow : public QWidget
{
    Q_OBJECT

public:
    MyMainWindow(const UserInfo& info, QWidget *parent = nullptr);
    ~MyMainWindow();

protected:
    //void showEvent(QShowEvent *event) override;
    //void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent  *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onProfileToolButtonClicked();
    void onChatToolButtonClicked();
    void onFriendToolButtonClicked();
    void onAddToolButtonClicked();
    void onSettingsToolButtonClicked();
    void onChangePwd();
    void onLogout();
    void onMinToolButtonClicked();
    void onMaxToolButtonClicked();
    void onCloseToolButtonClicked();
    void onSearchPushButtonClicked();
    void onAddUserPushButtonClicked();
    void onSetProfilePushButtonClicked();
    void onSetUsernamePushButtonClicked();
    void onEmojiToolButtonClicked();
    void onPictureToolButtonClicked();
    void onSendPushButtonClicked();
    void onFriendApplyItemClicked(QListWidgetItem* item);
    void onConfirmPushButtonClicked();
    void onCancelPushButtonClicked();
    void onSendMsgToolButtonClicked();
    void onDeleteUserToolButtonClicked();
    void onFriendInfoItemClicked(QListWidgetItem* item);
    void onChatItemClicked(QListWidgetItem* item);

    void onchangePasswordSuccess(const QString& leaveMsg);
    void onchangePasswordFailed(const QString& leaveMsg);
    void onUserInfoUpdated(int userId);
    void onUpdatedUserInfoResult(const QString& leaveMsg);
    void onUpdateSendButtonState();
    void onSearchUserResult(const QList<UserInfo>& users);
    void onUserSelected(const QModelIndex &index);
    void onSendFriendApplyResult(const QString& leaveMsg);
    void onAcceptFriendApplyResult(const QString& leaveMsg);
    void onRejectFriendApplyResult(const QString& leaveMsg);
    void onFriendApplyUpdated(const FriendApplyData& data);
    void onFriendAdded(int friendId);
    void onChatUpdated(int chatId);
    void onMessageAdded(const MessageData &msg);
    void onDeleteFriendResult(const QString& leaveMsg);
    void onFriendDeleted(int friendId);

private:
    enum mouseRegion
    {
        uiCenter,
        uiLeft, uiRight,
        uiTop, uiBottom,
        uiTopLeft, uiTopRight,
        uiBottomLeft, uiBottomRight
    };
    Ui::MyMainWindow *ui;
    int mUserId;
    int mCurrentChatId;
    int mCurrentFriendId;
    SettingsDialog *mSettings;
    UserInfo searchUserInfo;
    bool isMax = false;
    QLabel* mRedPoint = nullptr;
    bool isFriendApplyPage = false;
    QPoint mDragPos;
    bool mDragging = false;
    QRect normalRect;
    mouseRegion mMouseRegion = uiCenter;
    int mMouseMargin = 3;
    QScrollBar* mScrollBar = nullptr;
    bool hasScrollBar = false;
    UserSearchModel *mUserSearchModel = nullptr;
    QCompleter *mUserCompleter = nullptr;
    mouseRegion getMouseRegion(const QPoint &pos);
    void updateMouseShape(const QPoint &pos);
    void resizeWindow(const QPoint &globalPos);
    void refreshFriendApplyList();   // 刷新好友申请列表
    void refreshFriendList();       // 刷新好友列表
    void refreshChatList();        // 刷新会话列表
};
#endif // MYMAINWINDOW_H

#include "MyMainWindow.h"
#include "./ui_MyMainWindow.h"

MyMainWindow::MyMainWindow(const UserInfo& info, QWidget *parent): QWidget(parent), ui(new Ui::MyMainWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_StyledBackground);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setMouseTracking(true);
    mUserId = info.userId;
    //侧工具栏信号槽
    connect(ui->profileToolButton, &QToolButton::clicked, this, &MyMainWindow::onProfileToolButtonClicked);
    connect(ui->chatToolButton, &QToolButton::clicked, this, &MyMainWindow::onChatToolButtonClicked);
    connect(ui->friendToolButton, &QToolButton::clicked, this, &MyMainWindow::onFriendToolButtonClicked);
    connect(ui->addToolButton, &QToolButton::clicked, this, &MyMainWindow::onAddToolButtonClicked);
    connect(ui->settingsToolButton, &QToolButton::clicked, this, &MyMainWindow::onSettingsToolButtonClicked);
    //底部工具栏信号槽
    connect(ui->minToolButton, &QToolButton::clicked, this, &MyMainWindow::onMinToolButtonClicked);
    connect(ui->maxToolButton, &QToolButton::clicked, this, &MyMainWindow::onMaxToolButtonClicked);
    connect(ui->closeToolButton, &QToolButton::clicked, this, &MyMainWindow::onCloseToolButtonClicked);
    //添加好友页面信号槽
    connect(ui->searchPushButton, &QPushButton::clicked, this, &MyMainWindow::onSearchPushButtonClicked);
    connect(ui->searchLineEdit, &QLineEdit::returnPressed,this, &MyMainWindow::onSearchPushButtonClicked);
    connect(ui->addUserPushButton, &QPushButton::clicked, this, &MyMainWindow::onAddUserPushButtonClicked);
    //处理添加好友页面信号槽
    connect(ui->friendApplyListWidget, &QListWidget::itemClicked, this, &MyMainWindow::onFriendApplyItemClicked);
    connect(ui->confirmPushButton, &QPushButton::clicked, this, &MyMainWindow::onConfirmPushButtonClicked);
    connect(ui->cancelPushButton, &QPushButton::clicked, this, &MyMainWindow::onCancelPushButtonClicked);
    //好友信息页面信号槽
    connect(ui->userListWidget, &QListWidget::itemClicked, this, &MyMainWindow::onFriendInfoItemClicked);
    connect(ui->sendMsgToolButton, &QToolButton::clicked, this, &MyMainWindow::onSendMsgToolButtonClicked);
    connect(ui->deleteUserToolButton, &QToolButton::clicked, this, &MyMainWindow::onDeleteUserToolButtonClicked);
    //主用户信息页面信号槽
    connect(ui->setProfilePushButton, &QPushButton::clicked, this, &MyMainWindow::onSetProfilePushButtonClicked);
    connect(ui->setUsernamePushButton, &QPushButton::clicked, this, &MyMainWindow::onSetUsernamePushButtonClicked);
    //信息交流页面信号槽
    connect(ui->chatListWidget, &QListWidget::itemClicked, this, &MyMainWindow::onChatItemClicked);
    connect(ui->emojiToolButton, &QToolButton::clicked, this, &MyMainWindow::onEmojiToolButtonClicked);
    connect(ui->pictureToolButton, &QToolButton::clicked, this, &MyMainWindow::onPictureToolButtonClicked);
    connect(ui->sendPushButton, &QPushButton::clicked, this, &MyMainWindow::onSendPushButtonClicked);
    connect(ui->sendTextEdit, &QTextEdit::textChanged,this, &MyMainWindow::onUpdateSendButtonState);
    //服务器信息回复信号槽
    connect(&SocketBusiness::instance(), &SocketBusiness::changePasswordSuccess, this, &MyMainWindow::onchangePasswordSuccess);
    connect(&SocketBusiness::instance(), &SocketBusiness::changePasswordFailed, this, &MyMainWindow::onchangePasswordFailed);
    connect(&UserInfoSDK::instance(), &UserInfoSDK::userInfoUpdated, this, &MyMainWindow::onUserInfoUpdated);
    connect(&SocketBusiness::instance(), &SocketBusiness::updatedUserInfoResult, this, &MyMainWindow::onUpdatedUserInfoResult);
    connect(&SocketBusiness::instance(), &SocketBusiness::searchUserResult, this, &MyMainWindow::onSearchUserResult);
    connect(&SocketBusiness::instance(), &SocketBusiness::sendFriendApplyResult, this, &MyMainWindow::onSendFriendApplyResult);
    connect(&SocketBusiness::instance(), &SocketBusiness::acceptFriendApplyResult, this, &MyMainWindow::onAcceptFriendApplyResult);
    connect(&SocketBusiness::instance(), &SocketBusiness::rejectFriendApplyResult, this, &MyMainWindow::onRejectFriendApplyResult);
    connect(&FriendApplyStore::instance(), &FriendApplyStore::friendApplyUpdated, this, &MyMainWindow::onFriendApplyUpdated);
    connect(&SocketBusiness::instance(), &SocketBusiness::friendAdded, this, &MyMainWindow::onFriendAdded);
    connect(&SocketBusiness::instance(), &SocketBusiness::deleteFriendResult, this, &MyMainWindow::onDeleteFriendResult);
    connect(&SocketBusiness::instance(), &SocketBusiness::friendDeleted, this, &MyMainWindow::onFriendDeleted);
    connect(&MessageStore::instance(), &MessageStore::messageAdded, this, &MyMainWindow::onMessageAdded);
    connect(&ChatStore::instance(), &ChatStore::chatUpdated, this, &MyMainWindow::onChatUpdated);

    UserInfoSDK::instance().updateUser(info);
    refreshFriendList();
    refreshFriendApplyList();
    refreshChatList();

    new FloatingScrollBar(ui->chatListWidget);
    new FloatingScrollBar(ui->userListWidget);
    new FloatingScrollBar(ui->msgListWidget);
    ui->leftStackedWidget->setCurrentWidget(ui->leftChatPage);
    ui->rightStackedWidget->setCurrentWidget(ui->homePage);

    mUserSearchModel = new UserSearchModel(this);
    mUserCompleter = new QCompleter(mUserSearchModel, this);
    mUserCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    mUserCompleter->setFilterMode(Qt::MatchContains);
    mUserCompleter->setCompletionRole(Qt::DisplayRole);
    mUserCompleter->setCompletionMode(QCompleter::PopupCompletion);
    ui->searchLineEdit->setCompleter(mUserCompleter);
    connect(mUserCompleter, QOverload<const QModelIndex &>::of(&QCompleter::activated), this, &MyMainWindow::onUserSelected);
}

MyMainWindow::~MyMainWindow()
{
    delete ui;
}

void MyMainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        mMouseRegion = getMouseRegion(event->pos());
        if(mMouseRegion != uiCenter)
        {
            event->accept();
            return;
        }
        mDragging = true;
        mDragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
    QWidget::mousePressEvent(event);
}

void MyMainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint globalPos = event->globalPosition().toPoint();
    if(mMouseRegion != uiCenter && (event->buttons()) & Qt::LeftButton)
    {
        resizeWindow(globalPos);
        return;
    }
    updateMouseShape(event->pos());
    if (mDragging && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPosition().toPoint() - mDragPos);
    }
    QWidget::mouseMoveEvent(event);
}

void MyMainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    mDragging = false;
    mMouseRegion = uiCenter;
    QWidget::mouseReleaseEvent(event);
}

void MyMainWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (mScrollBar)
    {
        mScrollBar->setGeometry(ui->msgListWidget->width() - 10, 2, 7, ui->msgListWidget->height() - 2);
    }
}

bool MyMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (!hasScrollBar)
    {
        return QWidget::eventFilter(obj, event);
    }
    if (obj == ui->msgListWidget->viewport() || obj == mScrollBar)
    {
        if (event->type() == QEvent::Enter)
        {
            mScrollBar->setVisible(true);
        }
        else if (event->type() == QEvent::Leave)
        {
            mScrollBar->setVisible(false);
        }
    }
    return QWidget::eventFilter(obj, event);
}

MyMainWindow::mouseRegion MyMainWindow::getMouseRegion(const QPoint &pos)
{
    //获取当前鼠标所在的区域
    bool onLeft = pos.x() <= mMouseMargin;
    bool onRight = pos.x() >= width() - mMouseMargin;
    bool onTop = pos.y() <= mMouseMargin;
    bool onBottom = pos.y() >= height() - mMouseMargin;
    if (onLeft && onTop) return uiTopLeft; //左上
    if (onRight && onTop) return uiTopRight; //右上
    if (onLeft && onBottom) return uiBottomLeft; //左下
    if (onRight && onBottom) return uiBottomRight; //左下
    if (onLeft) return uiLeft; //左
    if (onRight) return uiRight; //右
    if (onTop) return uiTop; //上
    if (onBottom) return uiBottom; //下
    return uiCenter; //中间
}

void MyMainWindow::updateMouseShape(const QPoint &pos)
{
    switch (getMouseRegion(pos))
    {
        case uiLeft:
            this->setCursor(Qt::SizeHorCursor);
            break;
        case uiRight:
            this->setCursor(Qt::SizeHorCursor);
            break;
        case uiTop:
            this->setCursor(Qt::SizeVerCursor);
            break;
        case uiBottom:
            this->setCursor(Qt::SizeVerCursor);
            break;
        case uiTopLeft:
            this->setCursor(Qt::SizeFDiagCursor);
            break;
        case uiBottomRight:
            this->setCursor(Qt::SizeFDiagCursor);
            break;
        case uiTopRight:
            this->setCursor(Qt::SizeBDiagCursor);
            break;
        case uiBottomLeft:
            this->setCursor(Qt::SizeBDiagCursor);
            break;
        default:
            this->setCursor(Qt::ArrowCursor);
            break;
    }
}

void MyMainWindow::resizeWindow(const QPoint &globalPos)
{
    QRect geo = this->geometry();
    int minW = minimumWidth();
    int minH = minimumHeight();
    switch (mMouseRegion)
    {
        case uiLeft:
            geo.setLeft(globalPos.x());
            break;
        case uiRight:
            geo.setRight(globalPos.x());
            break;
        case uiTop:
            geo.setTop(globalPos.y());
            break;
        case uiBottom:
            geo.setBottom(globalPos.y());
            break;
        case uiTopLeft:
            geo.setTop(globalPos.y());
            geo.setLeft(globalPos.x());
            break;
        case uiTopRight:
            geo.setTop(globalPos.y());
            geo.setRight(globalPos.x());
            break;
        case uiBottomLeft:
            geo.setBottom(globalPos.y());
            geo.setLeft(globalPos.x());
            break;
        case uiBottomRight:
            geo.setBottom(globalPos.y());
            geo.setRight(globalPos.x());
            break;
        default:
            break;
    }
    // 限制最小尺寸
    if (geo.width() < minW) geo.setWidth(minW);
    if (geo.height() < minH) geo.setHeight(minH);
    this->setGeometry(geo);
}

void MyMainWindow::refreshFriendList()
{
    auto friendList = UserInfoSDK::instance().getFriendList();
    ui->userListWidget->clear();
    for (const auto friendUser : friendList)
    {
        auto bubble = new UserInfoBubbleWidget(friendUser->userId);
        auto item = new QListWidgetItem(ui->userListWidget);
        item->setSizeHint(bubble->sizeHint());
        item->setData(Qt::UserRole, friendUser->userId);
        ui->userListWidget->addItem(item);
        ui->userListWidget->setItemWidget(item, bubble);
    }
    QTimer::singleShot(0, ui->userListWidget, [=]() {
        auto bar = ui->userListWidget->verticalScrollBar();
        bar->setValue(bar->maximum());
    });
}

void MyMainWindow::refreshFriendApplyList()
{
    auto applies =  FriendApplyStore::instance().getAllFriendApplies();
    for(const auto apply : applies)
    {
        onFriendApplyUpdated(apply);
    }
}

void MyMainWindow::refreshChatList()
{
    auto chats =  ChatStore::instance().getAllChat();
    for(const auto chat : chats)
    {
        onChatUpdated(chat.chatId);
    }
}

void MyMainWindow::onProfileToolButtonClicked()
{
    ui->leftStackedWidget->setCurrentWidget(ui->leftUserPage);
    ui->rightStackedWidget->setCurrentWidget(ui->userPage);
}

void MyMainWindow::onChatToolButtonClicked()
{
    ui->leftStackedWidget->setCurrentWidget(ui->leftChatPage);
    ui->rightStackedWidget->setCurrentWidget(ui->homePage);
}

void MyMainWindow::onFriendToolButtonClicked()
{
    ui->leftStackedWidget->setCurrentWidget(ui->leftUserPage);
    ui->rightStackedWidget->setCurrentWidget(ui->homePage);
}

void MyMainWindow::onAddToolButtonClicked()
{
    isFriendApplyPage = true;
    ui->leftStackedWidget->setCurrentWidget(ui->leftFriendApplyPage);
    ui->rightStackedWidget->setCurrentWidget(ui->homePage);
}

void MyMainWindow::onSettingsToolButtonClicked()
{
    mSettings = new SettingsDialog(this);
    connect(mSettings, &SettingsDialog::changePassword, this, &MyMainWindow::onChangePwd);
    connect(mSettings, &SettingsDialog::logout, this, &MyMainWindow::onLogout);
    mSettings->exec();
}

void MyMainWindow::onChangePwd()
{
    bool ok;
    QString oldPwd = QInputDialog::getText(this, "修改密码", "请输入旧密码:", QLineEdit::Password, "", &ok);
    if (!ok || oldPwd.isEmpty()) return;
    QString newPwd = QInputDialog::getText(this, "修改密码", "请输入新密码:", QLineEdit::Password, "", &ok);
    if (!ok || newPwd.isEmpty()) return;
    QString confirmPwd = QInputDialog::getText(this, "修改密码", "请确认新密码:", QLineEdit::Password, "", &ok);
    if (!ok || confirmPwd != newPwd)
    {
        QMessageBox::warning(this, "修改密码", "两次输入的新密码不一致！");
        return;
    }
    SocketBusiness::instance().sendChangePassword(mUserId, oldPwd, newPwd);
}

void MyMainWindow::onLogout()
{
    // 断开服务器连接
    SocketBusiness::instance().disconnectFromServer();
    // 清除所有本地存储的数据
    UserInfoSDK::instance().clearAll();
    MessageStore::instance().clearAll();
    ChatStore::instance().clearAll();
    FriendApplyStore::instance().clearAll();
    // 关闭主窗口
    mSettings->close();
    this->close();
    // 显示登录窗口
    Login *login = new Login();
    login->show();
}

void MyMainWindow::onMinToolButtonClicked()
{
    this->showMinimized();
}

void MyMainWindow::onMaxToolButtonClicked()
{
    if (isMax)
    {
        this->showNormal();
        this->setGeometry(normalRect);
        ui->maxToolButton->setToolTip("最大化");
        ui->maxToolButton->setIcon(QIcon(":/imgs/max1.svg"));
    }
    else
    {
        normalRect = this->geometry();
        this->showMaximized();
        ui->maxToolButton->setToolTip("恢复");
        ui->maxToolButton->setIcon(QIcon(":/imgs/max2.svg"));
    }
    isMax = !isMax;
    QTimer::singleShot(0, this, [=](){
        ui->msgListWidget->doItemsLayout();
        ui->msgListWidget->viewport()->update();
    });
}

void MyMainWindow::onCloseToolButtonClicked()
{
    this->close();
}

void MyMainWindow::onSearchPushButtonClicked()
{
    QString keyword = ui->searchLineEdit->text().trimmed();
    if (keyword.isEmpty()) return;
    SocketBusiness::instance().sendSearchUser(keyword);
}

void MyMainWindow::onAddUserPushButtonClicked()
{
    bool ok = false;
    qWarning() << "friendId" << searchUserInfo.userId;
    QString leaveMsg = QInputDialog::getText(
        this, "添加好友",
        QString("请输入给 %1 的留言（可选）").arg(searchUserInfo.username),
        QLineEdit::Normal, "", &ok);
    if (!ok) return;
    SocketBusiness::instance().sendAddUser(mUserId, searchUserInfo.userId, leaveMsg.trimmed());
}

void MyMainWindow::onSetProfilePushButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,"选择头像","","图片 (*.png *.jpg *.jpeg)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return;
    QByteArray newProfile = file.readAll();
    file.close();
    auto info = UserInfoSDK::instance().getUser(mUserId);
    SocketBusiness::instance().sendUpdatedUserInfo(mUserId, info->username, newProfile);
}

void MyMainWindow::onSetUsernamePushButtonClicked()
{
    bool ok;
    QString newName = QInputDialog::getText(this, "修改用户名", "请输入新用户名：", QLineEdit::Normal, ui->userPageUsernameLabel->text(), &ok);
    if (!ok || newName.trimmed().isEmpty()) return;
    auto info = UserInfoSDK::instance().getUser(mUserId);
    SocketBusiness::instance().sendUpdatedUserInfo(mUserId, newName, info->profile);
}

void MyMainWindow::onEmojiToolButtonClicked()
{
    QDialog *emojiDialog = new QDialog(this);
    emojiDialog->setWindowTitle("选择表情");
    emojiDialog->setFixedSize(400, 300);
    emojiDialog->setModal(true);
    QTabWidget *tabWidget = new QTabWidget(emojiDialog);
    // 定义几个类别
    struct EmojiCategory
    {
        QString name;
        QStringList emojis;
    };
    QList<EmojiCategory> categories = {
        {"😀 表情", {
                        "😀", "😃", "😄", "😁", "😆", "😅", "😂", "🤣", "😊", "😇",
                        "🙂", "🙃", "😉", "😌", "😍", "🥰", "😘", "😗", "😙", "😚",
                        "😋", "😛", "😜", "🤪", "😝", "🤑", "🤗", "🤭", "🤫", "🤔",
                        "🤐", "🤨", "😑", "😶", "😏", "😒", "🙄", "😬", "😮", "😯"
                    }},
        {"👍 手势", {
                        "👍", "👎", "👌", "✌️", "🤞", "🤟", "🤘", "🤙", "👈", "👉",
                        "👆", "👇", "🖕", "🖖", "🙏", "💪", "🦵", "🦶", "👂", "👃"
                    }},
        {"❤️ 物品", {
                       "❤️", "🧡", "💛", "💚", "💙", "💜", "🖤", "🤍", "🤎", "💔",
                       "❣️", "💕", "💞", "💓", "💗", "💖", "💘", "💝", "💟", "🎁",
                       "🎉", "🎊", "🎈", "✨", "🌟", "🌈", "☀️", "⭐", "🌙", "🔥"
                   }},
        {"🐶 动物", {
                        "🐶", "🐱", "🐭", "🐹", "🐰", "🦊", "🐻", "🐼", "🐨", "🐯",
                        "🦁", "🐮", "🐷", "🐸", "🐵", "🐔", "🐧", "🐦", "🐴", "🦋"
                    }}
    };
    for (const auto &category : categories)
    {
        QWidget *page = new QWidget();
        QGridLayout *layout = new QGridLayout(page);
        int col = 0, row = 0;
        for (const QString &emoji : category.emojis)
        {
            QPushButton *btn = new QPushButton(emoji);
            btn->setFixedSize(40, 40);
            btn->setFont(QFont("Segoe UI Emoji", 20));
            connect(btn, &QPushButton::clicked, this, [this, emoji, emojiDialog]() {
                    ui->sendTextEdit->insertPlainText(emoji);
                    emojiDialog->accept();});
            layout->addWidget(btn, row, col);
            col++;
            if (col >= 8)
            {
                col = 0;
                row++;
            }
        }
        tabWidget->addTab(page, category.name);
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(emojiDialog);
    mainLayout->addWidget(tabWidget);
    emojiDialog->setLayout(mainLayout);
    emojiDialog->exec();
    emojiDialog->deleteLater();
}

void MyMainWindow::onPictureToolButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择图片", "", "图片 (*.png *.jpg *.jpeg *.bmp)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "错误", "无法打开图片文件");
        return;
    }
    QByteArray imageData = file.readAll();
    file.close();
    // 限制图片大小
    const int MAX_IMAGE_SIZE = 7 * 1024 * 1024;
    if (imageData.size() > MAX_IMAGE_SIZE)
    {
        QMessageBox::warning(this, "错误", "图片不能超过7MB");
        return;
    }
    // 可选：压缩图片（使用QImage缩放）
    QImageReader reader(fileName);
    QImage image = reader.read();
    if (!image.isNull())
    {
        // 限制最大边长 800 像素
        if (image.width() > 400 || image.height() > 400)
        {
            image = image.scaled(400, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QBuffer buffer(&imageData);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "JPEG", 70);
        }
    }
    QString base64 = QString::fromLatin1(imageData.toBase64());
    SocketBusiness::instance().sendNewMessage(mCurrentChatId, mUserId, 1, base64);
}

void MyMainWindow::onSendPushButtonClicked()
{
    QString content = ui->sendTextEdit->toPlainText().trimmed();
    if (content.isEmpty()) return;
    ui->sendTextEdit->clear();
    SocketBusiness::instance().sendNewMessage(mCurrentChatId, mUserId, 0, content);
}

void MyMainWindow::onFriendApplyItemClicked(QListWidgetItem *item)
{
    int friendId = item->data(Qt::UserRole).toInt();
    auto friendApply = FriendApplyStore::instance().getFriendApply(friendId);
    if (!friendApply) return;
    auto info = UserInfoSDK::instance().getUser(friendId);
    QPixmap src;
    src.loadFromData(info->profile);
    if(info->applyStatus == 0)
    {
        ui->rightStackedWidget->setCurrentWidget(ui->comfirmPage);
        ui->comfirmPageUsernameLabel->setText(info->username);
        ui->comfirmPageProfileLabel->setPixmap(UserInfoSDK::instance().makeProfile(src, ui->comfirmPageProfileLabel->width() - 2, ui->comfirmPageProfileLabel->height() - 2));
        if(info->isSelf == 0)
        {
            ui->leaveMsgTextEdit->setText(friendApply->leaveMsg);
            ui->confirmPushButton->setVisible(true);
            ui->cancelPushButton->setVisible(true);
            ui->confirmPushButton->setProperty("friendId", friendId);
            ui->cancelPushButton->setProperty("friendId", friendId);
        }
        else if(info->isSelf == 1)
        {
            ui->leaveMsgTextEdit->setText("等待验证");
            ui->confirmPushButton->setVisible(false);
            ui->cancelPushButton->setVisible(false);
        }
    }
    else if(info->applyStatus == 1)
    {
        ui->friendToolButton->setChecked(true);
        mCurrentFriendId = friendId;
        ui->rightStackedWidget->setCurrentWidget(ui->friendPage);
        ui->friendPageUsernameLabel->setText(info->username);
        ui->friendPageProfileLabel->setPixmap(UserInfoSDK::instance().makeProfile(src, ui->friendPageProfileLabel->width() - 2, ui->friendPageProfileLabel->height() - 2));
    }
    else if(info->applyStatus == 2)
    {
        ui->rightStackedWidget->setCurrentWidget(ui->addPage);
        ui->addPageUsernameLabel->setText(info->username);
        ui->addPageProfileLabel->setPixmap(UserInfoSDK::instance().makeProfile(src, ui->addPageProfileLabel->width() - 2, ui->addPageProfileLabel->height() - 2));
    }
}

void MyMainWindow::onConfirmPushButtonClicked()
{
    int friendId = sender()->property("friendId").toInt();
    SocketBusiness::instance().sendAcceptFriendApply(friendId, mUserId);
    ui->rightStackedWidget->setCurrentWidget(ui->homePage);
}

void MyMainWindow::onCancelPushButtonClicked()
{
    int friendId = sender()->property("friendId").toInt();
    SocketBusiness::instance().sendRejectFriendApply(friendId, mUserId);
    ui->rightStackedWidget->setCurrentWidget(ui->homePage);
}

void MyMainWindow::onSendMsgToolButtonClicked()
{
    int friendId = mCurrentFriendId;
    if (friendId <= 0) return;
    auto info = UserInfoSDK::instance().getUser(friendId);
    if (!info)
    {
        qWarning() << "User info not found for friendId:" << friendId;
        return;
    }
    mCurrentChatId = friendId;
    // 获取会话，不存在就创建
    auto chat = ChatStore::instance().getChatData(mCurrentChatId);
    if (!chat)
    {
        ChatData newChat;
        newChat.chatId = friendId;
        ChatStore::instance().addChat(newChat);
        chat = ChatStore::instance().getChatData(mCurrentChatId);
    }
    // 切换到聊天页面
    ui->chatToolButton->setChecked(true);
    ui->leftStackedWidget->setCurrentWidget(ui->leftChatPage);
    ui->rightStackedWidget->setCurrentWidget(ui->msgPage);
    ui->firendUsernameLabel->setText(info->username);
    // 加载历史消息
    ui->msgListWidget->clear();
    QList<MessageData> messages = MessageStore::instance().getMessages(mCurrentChatId);
    // // 按时间正序排序
    // std::sort(messages.begin(), messages.end(),
    //           [](const MessageData& a, const MessageData& b) {
    //               return a.timestamp < b.timestamp;
    //           });
    for (const auto& msg : messages)
    {
        auto bubble = new MessageBubbleWidget(msg);
        auto item = new QListWidgetItem(ui->msgListWidget);
        item->setSizeHint(bubble->sizeHint());
        ui->msgListWidget->addItem(item);
        ui->msgListWidget->setItemWidget(item, bubble);
    }
    QTimer::singleShot(0, ui->msgListWidget, [=]() {
        ui->msgListWidget->scrollToBottom();
    });
    // 选中对应会话并清零未读计数
    for (int i = 0; i < ui->chatListWidget->count(); ++i)
    {
        auto item = ui->chatListWidget->item(i);
        if (item->data(Qt::UserRole).toInt() == mCurrentChatId)
        {
            ui->chatListWidget->setCurrentItem(item);
            ui->chatListWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
            if (chat && chat->unreadCount > 0)
            {
                chat->unreadCount = 0;
                auto bubble = qobject_cast<ChatBubbleWidget*>(ui->chatListWidget->itemWidget(item));
                if (bubble)
                {
                    bubble->updateBubble(*chat);
                }
            }
            break;
        }
    }
}

void MyMainWindow::onDeleteUserToolButtonClicked()
{
    int friendId = mCurrentFriendId;
    if (friendId <= 0) return;
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "删除好友",
        QString("确定要删除好友 %1 吗？").arg(ui->friendPageUsernameLabel->text()),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;
    SocketBusiness::instance().sendDeleteUser(mUserId, friendId);
}

void MyMainWindow::onFriendInfoItemClicked(QListWidgetItem *item)
{
    int friendId = item->data(Qt::UserRole).toInt();
    auto info = UserInfoSDK::instance().getUser(friendId);
    QPixmap src;
    src.loadFromData(info->profile);
    ui->rightStackedWidget->setCurrentWidget(ui->friendPage);
    ui->friendPageUsernameLabel->setText(info->username);
    ui->friendPageProfileLabel->setPixmap(UserInfoSDK::instance().makeProfile(src, ui->friendPageProfileLabel->width() - 2, ui->friendPageProfileLabel->height() - 2));
    mCurrentFriendId = friendId;
}

void MyMainWindow::onChatItemClicked(QListWidgetItem* item)
{
    int chatId = item->data(Qt::UserRole).toInt();
    auto chat = ChatStore::instance().getChatData(chatId);
    if (!chat) return;
    auto info = UserInfoSDK::instance().getUser(chatId);
    QPixmap src;
    src.loadFromData(info->profile);
    ui->rightStackedWidget->setCurrentWidget(ui->msgPage);
    ui->firendUsernameLabel->setText(info->username);
    ui->msgListWidget->clear();
    QList<MessageData> messages = MessageStore::instance().getMessages(chatId);
    // std::sort(messages.begin(), messages.end(), [](const MessageData& a, const MessageData& b) { return a.timestamp < b.timestamp; });
    for (const auto& msg : messages)
    {
        auto bubble = new MessageBubbleWidget(msg);
        QListWidgetItem* item = new QListWidgetItem(ui->msgListWidget);
        item->setSizeHint(bubble->sizeHint());
        ui->msgListWidget->addItem(item);
        ui->msgListWidget->setItemWidget(item, bubble);
    }
    QTimer::singleShot(0, ui->msgListWidget, [=]() {
        auto bar = ui->msgListWidget->verticalScrollBar();
        bar->setValue(bar->maximum());
    });
    if (chat->unreadCount > 0)
    {
        chat->unreadCount = 0;
        auto bubble = qobject_cast<ChatBubbleWidget*>(ui->chatListWidget->itemWidget(item));
        if (bubble)
        {
            bubble->updateBubble(*chat);
        }
    }
    mCurrentChatId = chatId;
}

void MyMainWindow::onchangePasswordSuccess(const QString& leaveMsg)
{
    // 清除本地 token
    QSettings settings("users.ini", QSettings::IniFormat);
    settings.beginGroup(QString("users/%1").arg(mUserId));
    settings.remove("token");
    settings.endGroup();
    // 提示用户重新登录
    QMessageBox::information(this, "修改密码", leaveMsg, QMessageBox::Ok);
    onLogout(); // 自动退出登录
}

void MyMainWindow::onchangePasswordFailed(const QString& leaveMsg)
{
    QMessageBox::information(this, "修改密码", leaveMsg, QMessageBox::Ok);
}

void MyMainWindow::onUserInfoUpdated(int userId)
{
    if (userId != mUserId) return;
    auto info = UserInfoSDK::instance().getUser(userId);
    if (!info) return;
    QPixmap src;
    src.loadFromData(info->profile);
    ui->mainUserLabel->setText(info->username);
    ui->userPageUsernameLabel->setText(info->username);
    ui->userPageProfileLabel->setPixmap(UserInfoSDK::instance().makeProfile(src, ui->userPageProfileLabel->width() - 2, ui->userPageProfileLabel->height() - 2));
    ui->profileToolButton->setIcon(QIcon(UserInfoSDK::instance().makeProfile(src, ui->profileToolButton->width() - 2, ui->profileToolButton->height() - 2)));
}

void MyMainWindow::onUpdatedUserInfoResult(const QString &leaveMsg)
{
    QMessageBox::information(this, "用户信息更新", leaveMsg, QMessageBox::Ok);
}

void MyMainWindow::onUpdateSendButtonState()
{
    bool hasText = !ui->sendTextEdit->toPlainText().trimmed().isEmpty();
    ui->sendPushButton->setEnabled(hasText);
}

void MyMainWindow::onSearchUserResult(const QList<UserInfo>& users)
{
    mUserSearchModel->setUsers(users);
    if (!users.isEmpty())
    {
        mUserCompleter->complete();
    }
}

void MyMainWindow::onUserSelected(const QModelIndex &index)
{
    searchUserInfo = index.data(Qt::UserRole).value<UserInfo>();
    qDebug() << index.data(Qt::UserRole).isValid();
    QPixmap src;
    src.loadFromData(searchUserInfo.profile);
    if(searchUserInfo.isFriend == 1)
    {
        ui->rightStackedWidget->setCurrentWidget(ui->friendPage);
        ui->friendPageProfileLabel->setPixmap(UserInfoSDK::instance().makeProfile(src, ui->friendPageProfileLabel->width() - 2, ui->friendPageProfileLabel->height() - 2));
        ui->friendPageUsernameLabel->setText(searchUserInfo.username);
    }
    else
    {
        if(searchUserInfo.applyStatus == -1 || searchUserInfo.applyStatus == 2) // 无申请或被拒绝过，显示申请好友界面
        {
            ui->rightStackedWidget->setCurrentWidget(ui->addPage);
            ui->addPageUsernameLabel->setText(searchUserInfo.username);
            ui->addPageProfileLabel->setPixmap(UserInfoSDK::instance().makeProfile(src, ui->addPageProfileLabel->width() - 2, ui->addPageProfileLabel->height() - 2));
        }
        else if(searchUserInfo.applyStatus == 0) // 存在未处理申请，显示处理好友申请界面
        {
            auto friendApply = FriendApplyStore::instance().getFriendApply(searchUserInfo.userId);
            if (!friendApply) return;
            ui->rightStackedWidget->setCurrentWidget(ui->comfirmPage);
            ui->comfirmPageProfileLabel->setPixmap(UserInfoSDK::instance().makeProfile(src, ui->comfirmPageProfileLabel->width() - 2, ui->comfirmPageProfileLabel->height() - 2));
            ui->comfirmPageUsernameLabel->setText(searchUserInfo.username);
            if(searchUserInfo.isSelf == 0)
            {
                ui->leaveMsgTextEdit->setText(friendApply->leaveMsg);
                ui->confirmPushButton->setVisible(true);
                ui->cancelPushButton->setVisible(true);
                ui->confirmPushButton->setProperty("friendId", searchUserInfo.userId);
                ui->cancelPushButton->setProperty("friendId", searchUserInfo.userId);
            }
            else if(searchUserInfo.isSelf == 1)
            {
                ui->leaveMsgTextEdit->setText("等待验证");
                ui->confirmPushButton->setVisible(false);
                ui->cancelPushButton->setVisible(false);
            }
        }
    }
}

void MyMainWindow::onSendFriendApplyResult(const QString &leaveMsg)
{
    QMessageBox::information(this, "发送好友申请", leaveMsg, QMessageBox::Ok);
}

void MyMainWindow::onAcceptFriendApplyResult(const QString &leaveMsg)
{
    QMessageBox::information(this, "通过好友申请", leaveMsg, QMessageBox::Ok);
}

void MyMainWindow::onRejectFriendApplyResult(const QString &leaveMsg)
{
    QMessageBox::information(this, "拒绝好友申请", leaveMsg, QMessageBox::Ok);
}

void MyMainWindow::onFriendApplyUpdated(const FriendApplyData &data)
{
    if(isFriendApplyPage)
    {
        mRedPoint = new QLabel(ui->addToolButton);
        mRedPoint->setFixedSize(8, 8);
        mRedPoint->setStyleSheet("background:red; border-radius:4px;");
        mRedPoint->move(ui->addToolButton->width() - 10, 4);
    }
    for (int i = 0; i < ui->friendApplyListWidget->count(); ++i)
    {
        auto item = ui->friendApplyListWidget->item(i);
        if (item->data(Qt::UserRole).toInt() == data.friendId)
        {
            auto bubble = qobject_cast<FriendApplyBubbleWidget*>(ui->friendApplyListWidget->itemWidget(item));

            if (bubble)
            {
                bubble->updateBubble(data);
                item->setSizeHint(bubble->sizeHint());
            }
            return;
        }
    }
    auto bubble = new FriendApplyBubbleWidget(data);
    auto item = new QListWidgetItem(ui->friendApplyListWidget);
    item->setSizeHint(bubble->sizeHint());
    item->setData(Qt::UserRole, data.friendId);
    ui->friendApplyListWidget->addItem(item);
    ui->friendApplyListWidget->setItemWidget(item, bubble);
    QTimer::singleShot(0, ui->chatListWidget, [=]() {
        auto bar = ui->chatListWidget->verticalScrollBar();
        bar->setValue(bar->maximum());
    });
}

void MyMainWindow::onFriendAdded(int friendId)
{
    auto info = UserInfoSDK::instance().getUser(friendId);
    auto bubble = new UserInfoBubbleWidget(info->userId);
    auto item = new QListWidgetItem(ui->userListWidget);
    item->setSizeHint(bubble->sizeHint());
    item->setData(Qt::UserRole, friendId);
    ui->userListWidget->addItem(item);
    ui->userListWidget->setItemWidget(item, bubble);
    QTimer::singleShot(0, ui->userListWidget, [=]() {
        auto bar = ui->chatListWidget->verticalScrollBar();
        bar->setValue(bar->maximum());
    });
}

void MyMainWindow::onChatUpdated(int chatId)
{
    // 若用户处于该chatId的会话界面，则直接让未读信息数为0
    auto chat = ChatStore::instance().getChatData(chatId);
    if (!chat) return;
    if(chat->chatId == mCurrentChatId && ui->leftStackedWidget->currentWidget() == ui->leftChatPage)
    {
        chat->unreadCount = 0;
    }
    // 已存在该chatId的消息，则更新item
    for (int i = 0; i < ui->chatListWidget->count(); ++i)
    {
        auto item = ui->chatListWidget->item(i);
        if (item->data(Qt::UserRole).toInt() == chat->chatId)
        {

            auto bubble = qobject_cast<ChatBubbleWidget*>(ui->chatListWidget->itemWidget(item));
            if (bubble)
            {
                bubble->updateBubble(*chat);
                item->setSizeHint(bubble->sizeHint());
            }
            return;
        }
    }
    // 不存在该chatId的消息，创建新item
    auto bubble = new ChatBubbleWidget(*chat);
    auto item = new QListWidgetItem(ui->chatListWidget);
    item->setSizeHint(bubble->sizeHint());
    item->setData(Qt::UserRole, chat->chatId);
    ui->chatListWidget->addItem(item);
    ui->chatListWidget->setItemWidget(item, bubble);
    ui->chatListWidget->setCurrentItem(item);
    ui->chatListWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
    QTimer::singleShot(0, ui->chatListWidget, [=]() {
        auto bar = ui->chatListWidget->verticalScrollBar();
        bar->setValue(bar->maximum());
    });
}

void MyMainWindow::onMessageAdded(const MessageData &msg)
{
    if (msg.chatId == mCurrentChatId && ui->rightStackedWidget->currentWidget() == ui->msgPage)
    {
        auto bubble = new MessageBubbleWidget(msg);
        auto item = new QListWidgetItem(ui->msgListWidget);
        item->setSizeHint(bubble->sizeHint());
        ui->msgListWidget->addItem(item);
        ui->msgListWidget->setItemWidget(item, bubble);
        ui->msgListWidget->scrollToBottom();
    }
}

void MyMainWindow::onDeleteFriendResult(const QString &leaveMsg)
{
    QMessageBox::information(this, "删除好友", leaveMsg, QMessageBox::Ok);
}

void MyMainWindow::onFriendDeleted(int friendId)
{
    UserInfo info;
    info.userId = friendId;
    info.isFriend = 0;
    UserInfoSDK::instance().updateUser(info);
    // 移除好友列表 UI 项
    for (int i = 0; i < ui->userListWidget->count(); ++i)
    {
        auto item = ui->userListWidget->item(i);
        if (item->data(Qt::UserRole).toInt() == friendId)
        {
            delete ui->userListWidget->takeItem(i);
            break;
        }
    }
    // 移除聊天列表 UI 项
    ChatStore::instance().removeChat(friendId);
    for (int i = 0; i < ui->chatListWidget->count(); ++i)
    {
        auto item = ui->chatListWidget->item(i);
        if (item->data(Qt::UserRole).toInt() == friendId)
        {
            delete ui->chatListWidget->takeItem(i);
            break;
        }
    }
    // 移除好友申请列表 UI 项
    FriendApplyStore::instance().removeFriendApply(friendId);
    for (int i = 0; i < ui->friendApplyListWidget->count(); ++i)
    {
        auto item = ui->friendApplyListWidget->item(i);
        if (item->data(Qt::UserRole).toInt() == friendId)
        {
            delete ui->friendApplyListWidget->takeItem(i);
            break;
        }
    }
    // 如果当前正在显示该好友的信息页面或聊天页面，返回主页
    if (ui->rightStackedWidget->currentWidget() == ui->friendPage || ui->rightStackedWidget->currentWidget() == ui->msgPage)
    {
        if (mCurrentFriendId == friendId || mCurrentChatId == friendId)
        {
            ui->rightStackedWidget->setCurrentWidget(ui->homePage);
        }
    }
}


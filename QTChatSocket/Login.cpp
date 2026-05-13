#include "Login.h"
#include "ui_Login.h"

Login::Login(QWidget *parent): QDialog(parent), ui(new Ui::Login)
{
    ui->setupUi(this);
    display = new DisplayAnimation(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    //加载登录用户栏
    loadUsers();
    //连接相关的登录信号槽
    connect(&SocketBusiness::instance(), &SocketBusiness::loginSuccess, this, &Login::onLoginSuccess);
    connect(&SocketBusiness::instance(), &SocketBusiness::loginFailed, this, &Login::onLoginFailed);
    connect(ui->loginButton, &QPushButton::clicked, this, &Login::onLoginButtonClicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &Login::onRegisterButtonClicked);
    connect(ui->closeToolButton, &QToolButton::clicked, this, &Login::onCloseToolButtonClicked);
    connect(ui->userComboBox->lineEdit(), &QLineEdit::textChanged, this, &Login::onUserEdited);
    connect(ui->pwdEdit, &QLineEdit::textEdited,this,[this](){ mTokenLoginEnabled = false; });
    //设置动态背景
    gifLabel = new QLabel(this);
    gifLabel->setScaledContents(true);
    gifLabel->setGeometry(this->rect());
    gifLabel->lower();
    movie = new QMovie(":/imgs/starBackground.gif");
    gifLabel->setMovie(movie);
    movie->setSpeed(120);
    movie->start();
}

Login::~Login()
{
    delete ui;
}

void Login::loadUsers()
{
    ui->userComboBox->lineEdit()->setPlaceholderText("请输入用户名或手机号");
    ui->userComboBox->lineEdit()->setAlignment(Qt::AlignCenter);
    ui->userComboBox->lineEdit()->setCursor(Qt::IBeamCursor);
    ui->userComboBox->lineEdit()->setFont(QFont("Microsoft YaHei UI", 11));
    ui->userComboBox->setInsertPolicy(QComboBox::NoInsert);
    ui->userCloseToolButton->raise();
    ui->userComboBox->clear();
    //加载user配置文件
    QSettings settings("users.ini", QSettings::IniFormat);
    settings.beginGroup("users");
    QStringList userIds = settings.childGroups();
    settings.endGroup();
    for (const QString &uid : userIds)
    {
        settings.beginGroup("users/" + uid);
        loginUserInfo info;
        info.userId = uid.toInt();
        info.username = settings.value("username").toString();
        info.token = settings.value("token").toString();
        info.profile = settings.value("profile").toByteArray();
        info.lastLoginTime = settings.value("lastLoginTime", 0).toLongLong();
        settings.endGroup();
        userList.append(info);
    }
    std::sort(userList.begin(), userList.end(), [](const loginUserInfo &a, const loginUserInfo &b) {
                return a.lastLoginTime > b.lastLoginTime;});
    // 加载到 userComboBox
    for (const loginUserInfo &info : userList)
    {
        ui->userComboBox->addItem(info.username, info.userId);
    }
    // 默认显示最近登录的用户
    if (!userList.isEmpty())
    {
        if(!userList.first().username.isEmpty()) ui->userComboBox->setCurrentText(userList.first().username);
        if(!userList.first().token.isEmpty()) ui->pwdEdit->setText(userList.first().token.left(11));
        if (!userList.first().profile.isEmpty())
        {
            QPixmap pix;
            pix.loadFromData(userList.first().profile);
            ui->profileLabel->setPixmap(pix.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}

void Login::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect sg = screen->availableGeometry();
    move(sg.center() - this->rect().center());
    display->enterAnimation(DisplayAnimation::TopToBottom);
}

void Login::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (gifLabel)
    {
        gifLabel->setGeometry(this->rect());
    }
}

void Login::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        mDragging = true;
        mDragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
    QDialog::mousePressEvent(event);
}

void Login::mouseMoveEvent(QMouseEvent *event)
{
    if (mDragging && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPosition().toPoint() - mDragPos);
    }
    QDialog::mouseMoveEvent(event);
}

void Login::mouseReleaseEvent(QMouseEvent *event)
{
    mDragging = false;
    QDialog::mouseReleaseEvent(event);
}

void Login::onLoginSuccess(const UserInfo& info)
{
    QMessageBox::information(this, "登录成功", "欢迎回来, " + info.username + "!");
    display->exitAnimation(DisplayAnimation::BottomToTop, true, [this, info](){
        MyMainWindow *w = new MyMainWindow(info);
        w->show();
    });
}

void Login::onLoginFailed(const QString& reason)
{
    QMessageBox::warning(this, "登录失败", reason);
    if(reason == "token已过期" || reason == "token无效")
    {
        // 如果是token登录失败，清除对应账号token
        int index = ui->userComboBox->currentIndex();
        if (index < 0) return;
        int userId = ui->userComboBox->itemData(index).toInt();
        QSettings settings("users.ini", QSettings::IniFormat);
        settings.beginGroup(QString("users/%1").arg(userId));
        settings.remove("token");
        settings.sync();
        settings.endGroup();
        mTokenLoginEnabled = false;
    }
    ui->pwdEdit->clear();
    ui->pwdEdit->setFocus();
}

void Login::onUserEdited(const QString &user)
{
    if (user.trimmed().isEmpty())
    {
        // 用户清空账号,重置 UI
        ui->userComboBox->setCurrentIndex(-1);
        ui->pwdEdit->clear();
        QPixmap pix(":/imgs/profile.svg");
        ui->profileLabel->setPixmap(pix.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        qDebug() << "账号被清空，ui被重置";
        return;
    }
    // 输入了内容,尝试匹配本地是否有此账号的历史记录
    for (int i = 0; i < ui->userComboBox->count(); ++i)
    {
        if (ui->userComboBox->itemText(i) == user)
        {
            ui->userComboBox->setCurrentIndex(i);
            int userId = ui->userComboBox->itemData(i).toInt();
            QSettings settings("users.ini", QSettings::IniFormat);
            settings.beginGroup(QString("users/%1").arg(userId));
            QByteArray profile = settings.value("profile").toByteArray();
            QString token = settings.value("token").toString();
            settings.endGroup();
            if (!profile.isEmpty())
            {
                QPixmap pix;
                pix.loadFromData(profile);
                ui->profileLabel->setPixmap(pix.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
            ui->pwdEdit->setText(token.left(11));
            return;
        }
    }
}

void Login::onLoginButtonClicked()
{
    QString user = ui->userComboBox->lineEdit()->text().trimmed();
    QString pwd = ui->pwdEdit->text().trimmed();
    if(user.isEmpty() || pwd.isEmpty())
    {
        QMessageBox::warning(this, "提示", "账号或密码不能为空！");
        qWarning()<< "账号或密码不能为空！";
        return;
    }
    int index = ui->userComboBox->currentIndex();
    int userId = -1;
    QString token;
    if (index >= 0)
    {
        userId = ui->userComboBox->itemData(index).toInt();
        QSettings settings("users.ini", QSettings::IniFormat);
        settings.beginGroup(QString("users/%1").arg(userId));
        token = settings.value("token").toString();
        settings.endGroup();
    }
    qWarning()<< token;
    if (mTokenLoginEnabled && !token.isEmpty())
    {
        SocketBusiness::instance().sendTokenLogin(user, token);// token 登录
    }
    else
    {
        SocketBusiness::instance().sendLogin(user, pwd);// 第一次：账号密码登录
    }
}

void Login::onRegisterButtonClicked()
{
    Register *regist = new Register(this);
    connect(regist, &Register::registerSuccess, this, &Login::show);
    connect(regist, &Register::backToLogin, this, &Login::show);
    display->exitAnimation(DisplayAnimation::RightToLeft, false, [regist](){regist->show();});
}

void Login::on_userCloseToolButton_clicked()
{
    ui->userComboBox->lineEdit()->clear();
}

void Login::onCloseToolButtonClicked()
{
    display->exitAnimation(DisplayAnimation::BottomToTop);
}




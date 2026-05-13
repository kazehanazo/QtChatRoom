#include "Register.h"
#include "ui_Register.h"

Register::Register(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Register)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    display = new DisplayAnimation(this);
    connect(&SocketBusiness::instance(),&SocketBusiness::registerSuccess,this,&Register::onRegisterSuccess);
    connect(&SocketBusiness::instance(),&SocketBusiness::registerFailed,this,&Register::onRegisterFailed);
    connect(ui->registerButton, &QPushButton::clicked, this, &Register::onRegisterButtonClicked);
    connect(ui->returnButton, &QPushButton::clicked, this, &Register::onReturnButtonClicked);
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

Register::~Register()
{
    delete ui;
}

void Register::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect sg = screen->availableGeometry();
    move(sg.center() - this->rect().center());
    display->enterAnimation(DisplayAnimation::LeftToRight);
}

void Register::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (gifLabel) gifLabel->setGeometry(this->rect());
}

void Register::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mDragging = true;
        mDragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
    QDialog::mousePressEvent(event);
}

void Register::mouseMoveEvent(QMouseEvent *event)
{
    if (mDragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - mDragPos);
    }
    QDialog::mouseMoveEvent(event);
}

void Register::mouseReleaseEvent(QMouseEvent *event)
{
    mDragging = false;
    QDialog::mouseReleaseEvent(event);
}

void Register::onRegisterSuccess()
{
    QMessageBox::information(this, "成功", "注册成功！");
    display->exitAnimation(DisplayAnimation::BottomToTop, true, [this](){ emit registerSuccess(); });
}

void Register::onRegisterFailed(const QString& reason)
{
    QMessageBox::warning(this, "错误", reason);
}
void Register::onRegisterButtonClicked()
{
    QString username = ui->userEdit->text().trimmed();
    QString phonenumber = ui->phoneNumberEdit->text().trimmed();
    QString password = ui->pwdEdit->text();
    QString repassword = ui->rePwdEdit->text();
    if(username.isEmpty())
    {
        QMessageBox::warning(this, "错误", "用户名不能为空！");
        return;
    }
    if(phonenumber.isEmpty())
    {
        QMessageBox::warning(this, "错误", "手机号不能为空！");
        return;
    }
    if(password.isEmpty())
    {
        QMessageBox::warning(this, "错误", "密码不能为空！");
        return;
    }
    if(password != repassword)
    {
        QMessageBox::warning(this, "错误", "两次密码输入不一致！");
        return;
    }
    SocketBusiness::instance().sendRegister(username, phonenumber, password);
}

void Register::onReturnButtonClicked()
{
    display->exitAnimation(DisplayAnimation::BottomToTop, true, [this](){ emit backToLogin(); });
}

void Register::on_closeToolButton_clicked()
{
    display->exitAnimation(DisplayAnimation::BottomToTop, true, [this](){ emit backToLogin(); });
}


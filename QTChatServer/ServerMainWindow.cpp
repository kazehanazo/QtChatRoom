#include "ServerMainWindow.h"
#include "./ui_ServerMainWindow.h"

ServerMainWindow::ServerMainWindow(QWidget *parent): QWidget(parent), ui(new Ui::ServerMainWindow)
{
    connect(&Log::Instance(), &Log::logMessage, this, &ServerMainWindow::appendMsg);
    connect(&server, &ChatServer::onUserOnline, this, &ServerMainWindow::onUserOnline);
    connect(&server, &ChatServer::onUserOffline, this, &ServerMainWindow::onUserOffline);
    ui->setupUi(this);
}

ServerMainWindow::~ServerMainWindow()
{
    server.close();
    delete ui;
}

void ServerMainWindow::closeEvent(QCloseEvent *event)
{
    // 关闭数据库连接池
    SqlConnPool::Instance().closeConn();
    // 关闭日志系统
    Log::Instance().closeLog();
    // 调用基类关闭事件，确保窗口正常关闭
    QWidget::closeEvent(event);
}
void ServerMainWindow::appendMsg(const QString &msg)
{
    ui->chatMsgListWidget->addItem(msg);
    ui->chatMsgListWidget->scrollToBottom();
}

void ServerMainWindow::onUserOnline(const QString &username)
{
    ui->userListWidget->addItem(username);
    QString num = QString::number(ui->userListWidget->count());
    ui->userNumLabel->setText(QString("当前在线人数：%1").arg(num));
}

void ServerMainWindow::onUserOffline(const QString &username)
{
    QList<QListWidgetItem*> items = ui->userListWidget->findItems(username, Qt::MatchExactly);
    if(!items.isEmpty())
    {
        delete items.first();
        QString num = QString::number(ui->userListWidget->count());
        ui->userNumLabel->setText(QString("当前在线人数：%1").arg(num));
    }
}

void ServerMainWindow::on_serverONButton_clicked()
{
    QString host = ui->serverHostLineEdit->text().trimmed();
    quint16 port = ui->serverPortLineEdit->text().toUShort();
    if (server.start(host, port))
    {
        ui->statusLEDLabel->setStyleSheet("background-color:green; border-radius: 8px; border:1px solid black;");
        ui->serverStatusLabel->setText("服务器开启");
    }
    else
    {
        ui->statusLEDLabel->setStyleSheet("background-color:red; border-radius: 8px; border:1px solid black;");
        ui->serverStatusLabel->setText("服务器关闭");
    }
}

void ServerMainWindow::on_serverOFFButton_clicked()
{
    server.stop();
    ui->statusLEDLabel->setStyleSheet("background-color:red; border-radius: 8px; border:1px solid black;");
    ui->serverStatusLabel->setText("服务器关闭");
}


#include "ServerMainWindow.h"
#include "ChatServer.h"
#include "SqlConnPool.h"
#include "SqlConnRAII.h"
#include "Log.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 请自行配置数据库设置
    // 依次填入数据库地址、端口、数据库名称、用户名、密码
    // SqlConnPool::Instance().init("host", 3306, "dbname", "user", "password");
    Log::Instance();
    ServerMainWindow w;
    w.show();
    return a.exec();
}

#include "MyMainWindow.h"
#include "Login.h"
#include "Register.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //MyMainWindow w;
    Login *login = new Login();
    login->show();

    return a.exec();
}

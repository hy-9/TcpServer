#include "tcpserver.h"
#include "opedb.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OpeDB::getInstance();

    TcpServer w;
    w.show();
    return a.exec();
}

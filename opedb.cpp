#include "opedb.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>

OpeDB::OpeDB(QObject *parent)
    : QObject{parent}
{
    m_db = new QSqlDatabase;
    *m_db = QSqlDatabase::addDatabase("QSQLITE");
    init();
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instace;
    return instace;
}

void OpeDB::init()
{
    m_db->setHostName("localhost");
    m_db->setDatabaseName("D:\\code\\qt\\TcpServer\\NetworkDisk.db");
    if (m_db->open()) {
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while (query.next()) {
            QString data = QString("%1,%2,%3")
                               .arg(query.value(0).toString())
                               .arg(query.value(1).toString())
                               .arg(query.value(2).toString());
            qDebug()<<data;
        }
    }else {
        QMessageBox::critical(NULL, "数据库连接", "连接失败");
    }

}

OpeDB::~OpeDB()
{

}

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if (NULL == name || NULL == pwd) {
        return false;
    }
    QString data =
        QString("insert into usrInfo(name, pwd) values(%1,%2)")
                       .arg(name).arg(pwd);
    qDebug()<<data;
    QSqlQuery query;
    return query.exec(data);
}

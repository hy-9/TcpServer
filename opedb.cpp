#include "opedb.h"
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>

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
        query.exec("update usrInfo set online=0");
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
        QString("insert into usrInfo(name, pwd) values(\'%1\',\'%2\')")
                       .arg(name).arg(pwd);
    qDebug()<< "注册操作：" <<data;
    QSqlQuery query;
    return query.exec(data);
}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if (NULL == name || NULL == pwd) {
        return false;
    }
    QString data =
        QString("select * from usrInfo where name=\'%1\' and pwd=\'%2\' and online=0")
            .arg(name).arg(pwd);
    qDebug()<< "登录操作：" <<data;
    QSqlQuery query;
    query.exec(data);
    if(query.next()){
        data = QString("update usrInfo set online=1 where name=\'%1\'")
                   .arg(name);
        QSqlQuery query;
        query.exec(data);
        return true;
    }else{return false;}

}

QStringList OpeDB::handleAllOnline()
{
    QStringList result;
    result.clear();
    QString data =
        QString("select name from usrInfo where online=1");
    qDebug()<< "查询所有用户操作：" <<data;
    QSqlQuery query;
    query.exec(data);
    while (query.next()) {
        result.append(query.value(0).toString());
    }
    return result;
}

void OpeDB::handleOffline(const char *name)
{
    if (NULL == name) {
        qDebug()<<"客户端退出错误：用户名为空";
        return;
    }
    QString data = QString("update usrInfo set online=0 where name=\'%1\'")
               .arg(name);
    QSqlQuery query;
    qDebug()<<"客户端退出"<<data;
    query.exec(data);
}

int OpeDB::handleUrs(const char *name)
{
    if (NULL == name) {
        qDebug()<<"查询错误：用户名为空";
        return 0;
    }
    QString data = QString("select online from usrInfo where name=\'%1\'")
                       .arg(name);
    QSqlQuery query;
    qDebug()<<"用户查询"<<data;
    query.exec(data);
    if(query.next()){
        return query.value(0).toInt();
    }else{
        return -1;
    }
}

int OpeDB::handleFriend(const char *senderName, const char *name)
{
    if (NULL == name || NULL == senderName) {
        qDebug()<<"添加好友错误：用户名为空";
        return -1;
    }
    QString data = QString("select * from friendInfo where id=(select id from usrInfo where name=\'%1\') "
                           "and friendId=(select id from usrInfo where name=\'%2\')")
                       .arg(senderName)
                       .arg(name);
    QSqlQuery query;
    qDebug()<<"好友查询"<<data;
    query.exec(data);
    if(query.next()){
        return 0;
    }else{
        return 1;
    }
}

bool OpeDB::addFriend(const char *senderName, const char *name)
{
    QString data =
        QString("insert into friendInfo(id, friendId) values(\'%1\',\'%2\')")
                       .arg(getId(senderName)).arg(getId(name));
    qDebug()<< "添加好友操作：" <<data;
    QSqlQuery query;
    query.exec(data);
    data =
        QString("insert into friendInfo(id, friendId) values(\'%1\',\'%2\')")
            .arg(getId(name)).arg(getId(senderName));
    return query.exec(data);
}

int OpeDB::getId(const char *name)
{
    if (NULL == name) {
        qDebug()<<"查询ID失败：用户名为空";
        return -1;
    }
    QString data =
        QString("select id from usrInfo where name=\'%1\'")
            .arg(name);
    qDebug()<< "查询ID操作：" <<data;
    QSqlQuery query;
    query.exec(data);
    if(query.next()){
        return query.value(0).toInt();
    }else{
        return -1;
    }
}

QString OpeDB::getName(int id)
{
    QString data =
        QString("select name from usrInfo where id=\'%1\'")
            .arg(id);
    qDebug()<< "查询用户名操作：" <<data;
    QSqlQuery query;
    query.exec(data);
    if(query.next()){
        return query.value(0).toString();
    }else{
        return "";
    }
}

QStringList OpeDB::getShowFrieng(const char *name)
{
    QStringList result;
    result.clear();
    int nameId = getId(name);
    QString data =
        QString("select friendId from friendInfo where id=\'%1\'")
            .arg(nameId);
    qDebug()<< "查询好友ID操作：" <<data;
    QSqlQuery query;
    query.exec(data);
    while (query.next()) {
        // QSqlQuery tem_query;
        // data =
        //     QString("select * from charInfo where senderId=\'%1\' and joinId=\'%2\' and read=1")
        //            .arg(query.value(0).toInt()).arg(nameId);
        // qDebug()<< "查询未读操作：" <<data;
        // tem_query.exec(data);
        // if (tem_query.first()) {
        //     result.append(getName(query.value(0).toInt())
        //                   +(QString)"(新信息)");
        // }else{
        result.append(getName(query.value(0).toInt()));
        // }
    }
    return result;
}

bool OpeDB::deleteFriend(const char *senderName, const char *name)
{
    QString data =
        QString("delete from friendInfo where id=\'%1\' and friendId=\'%2\'")
            .arg(getId(senderName)).arg(getId(name));
    qDebug()<< "删除好友操作：" <<data;
    QSqlQuery query;
    query.exec(data);
    data =
        QString("delete from friendInfo where id=\'%1\' and friendId=\'%2\'")
            .arg(getId(name)).arg(getId(senderName));
    return query.exec(data);
}

QStringList OpeDB::handleChar(const char *senderName, const char *name)
{
    QStringList result;
    result.clear();
    int senderNameId = getId(senderName);
    int nameId = getId(name);
    QString data =
        QString("select senderId,char,time from charInfo "
                           "where (senderId=\'%1\' and joinId=\'%2\') "
                           "or (senderId=\'%3\' and joinId=\'%4\')")
            .arg(nameId).arg(senderNameId).arg(senderNameId).arg(nameId);
    qDebug()<< "查询好友对话操作：" <<data;
    QSqlQuery query;
    query.exec(data);
    while (query.next()) {
        if (query.value(0).toInt()==senderNameId) {
            result.append(query.value(2).toString()+" "+
                          senderName+": "+
                          query.value(1).toString());
        }else{
            result.append(query.value(2).toString()+" "+
                          senderName+": "+
                          query.value(1).toString());
        }

    }
    setRead(nameId, senderNameId);
    return result;
}

void OpeDB::setRead(const int senderNameID, const int nameID)
{
    QString data = QString("update charInfo set read=0 where senderId=\'%1\' and joinId=\'%2\'")
                       .arg(senderNameID).arg(nameID);
    QSqlQuery query;
    qDebug()<<"设置已读"<<data;
    query.exec(data);
}

bool OpeDB::addChar(const char *senderName, const char *name, const char *mes)
{
    QString data =
        QString("insert into charInfo(senderId, joinId, char, time, read) values(\'%1\',\'%2\',\'%3\',\'%4\',1)")
            .arg(getId(senderName)).arg(getId(name)).arg(mes)
            .arg(QDateTime::currentDateTime().toString("yy-MM-dd hh:mm"));
    qDebug()<< "发送信息操作：" <<data;
    QSqlQuery query;
    return query.exec(data);
}

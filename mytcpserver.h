#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include <QThread>
class MyTcpSocket;
class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    MyTcpServer();

    static MyTcpServer &getInstance();
    void incomingConnection(qintptr socketDescriptor);
public slots:
    void deleteSocket(MyTcpSocket *mysocket);


private:
    QList<MyTcpSocket *> m_tcpSocketList;
};

#endif // MYTCPSERVER_H

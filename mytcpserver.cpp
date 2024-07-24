#include "mytcpserver.h"
#include "mytcpsocket.h"
#include <QDebug>


MyTcpServer::MyTcpServer() {

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "新的客户端连接";
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket, &MyTcpSocket::offline
            , this, &MyTcpServer::deleteSocket);
}

void MyTcpServer::deleteSocket(MyTcpSocket *mysocket)
{
    auto iter = m_tcpSocketList.begin();
    for (; iter != m_tcpSocketList.end(); ++iter) {
        if (*iter == mysocket) {
            (*iter) -> deleteLater();
            // delete *iter;
            *iter = NULL;
            m_tcpSocketList.erase(iter);
            break;
        }
    }
    qDebug()<<"当前存在"<<m_tcpSocketList.size()<<"个用户为{";
    foreach (auto var, m_tcpSocketList) {
        qDebug()<<var->getUser_name();
    }
    qDebug()<<"}";
}

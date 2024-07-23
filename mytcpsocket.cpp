#include "mytcpsocket.h"
#include "protocol.h"
#include "opedb.h"
#include <QDebug>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{

    connect(this, SIGNAL(readyRead())
            , this, SLOT(recvMsg()));
}

void MyTcpSocket::recvMsg()
{
    qDebug() << this->bytesAvailable();
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen, sizeof(uint));
    uint uiMsgLen = uiPDULen-sizeof(PDU);
    PDU *pdu =mkPDU(uiMsgLen);
    this->read((char*)pdu+sizeof(uint), uiPDULen-sizeof(uint));
    switch (pdu->uiMsgType) {
    case ENUM_MSG_TYPE_REGIST_REQUEST:
    {
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        strncpy(caPwd, pdu->caData+32, 32);
        qDebug()<<"用户名:"<<caName<<" 密码:"<<caPwd<<" 连接模式注册请求";
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
        if (OpeDB::getInstance().handleRegist(caName,caPwd)) {
            strcpy(respdu->caData,REGIST_OK);
        }else{
            strcpy(respdu->caData,REGIST_FAILED);
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    default:
        break;
    }

    free(pdu);
    pdu = NULL;
}

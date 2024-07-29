#include "mytcpsocket.h"
#include "protocol.h"
#include "opedb.h"
#include <QDebug>
#include <QDir>
#include <QStringList>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{

    connect(this, SIGNAL(readyRead())
            , this, SLOT(recvMsg()));
    connect(this, SIGNAL(disconnected())
            , this, SLOT(clientOffline()));
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
            QDir dir;
            qDebug()<< QString("./%1").arg(caName) <<"创建新文件夹：" << dir.mkdir(QString("./%1").arg(caName));
        }else{
            strcpy(respdu->caData,REGIST_FAILED);
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST:
    {
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        strncpy(caPwd, pdu->caData+32, 32);
        qDebug()<<"用户名:"<<caName<<" 密码:"<<caPwd<<" 连接模式登录请求";
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
        if (OpeDB::getInstance().handleLogin(caName,caPwd)) {
            strcpy(respdu->caData,LOGIN_OK);
            user_name = caName;
        }else{
            strcpy(respdu->caData,LOGIN_FAILED);
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
    {
        QStringList ret = OpeDB::getInstance().handleAllOnline();
        uint uiMsgLen = ret.size()*32;
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
        for (int i = 0; i < ret.size(); ++i) {
            memcpy((char *)respdu->caMsg+i*32
                   , ret.at(i).toStdString().c_str()
                   , ret.at(i).size());
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:{
        char caName[32] = {'\0'};
        memcpy(caName, pdu->caData, 32);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
        switch (OpeDB::getInstance().handleUrs(caName)) {
        case -1:
        {
            strcpy(respdu->caData, SEARCH_USR_NO);
            break;
        }
        case 0:
        {
            strcpy(respdu->caData, SEARCH_USR_OFFLINE);
            break;
        }
        case 1:
        {
            strcpy(respdu->caData, SEARCH_USR_ONLINE);
            break;
        }
        default:
            break;
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:{
        char caSenderName[32] = {'\0'};
        char caName[32] = {'\0'};
        memcpy(caSenderName, pdu->caData, 32);
        memcpy(caName, pdu->caData+32, 32);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        switch (OpeDB::getInstance().handleFriend(caSenderName, caName)) {
        case -1:
        {
            strcpy(respdu->caData, ADD_FRIEND_FAILED);
            break;
        }case 0:
        {
            strcpy(respdu->caData, ADD_FRIEND_EXIST);
            break;
        }case 1:
        {
            if(OpeDB::getInstance().addFriend(caSenderName, caName)){
                strcpy(respdu->caData, ADD_FRIEND_OK);
            }else{
                strcpy(respdu->caData, ADD_FRIEND_FAILED);
            }
            break;
        }
        default:
            break;
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_SHOW_FRIEND_REQUEST:{
        char caName[32] = {'\0'};
        memcpy(caName, pdu->caData, 32);
        QStringList ret = OpeDB::getInstance().getShowFrieng(caName);
        uint uiMsgLen = ret.size()*64;
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_SHOW_FRIEND_RESPOND;
        for (int i = 0; i < ret.size(); ++i) {
            memcpy((char *)respdu->caMsg+i*64
                   , ret.at(i).toStdString().c_str()
                   , ret.at(i).size());
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:{
        char caSenderName[32] = {'\0'};
        char caName[32] = {'\0'};
        memcpy(caSenderName, pdu->caData, 32);
        memcpy(caName, pdu->caData+32, 32);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
        if (OpeDB::getInstance().deleteFriend(caSenderName, caName)) {
            strcpy(respdu->caData, DELETE_FRIEND_OK);
        }else{
            strcpy(respdu->caData, DELETE_FRIEND_FAILED);
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_SHOW_CHAR_REQUEST:{
        char caSenderName[32] = {'\0'};
        char caName[32] = {'\0'};
        memcpy(caSenderName, pdu->caData, 32);
        memcpy(caName, pdu->caData+32, 32);
        QStringList ret = OpeDB::getInstance().handleChar(caSenderName, caName);
        uint uiMsgLen = ret.size()*64;
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_SHOW_CHAR_RESPOND;
        for (int i = 0; i < ret.size(); ++i) {
            memcpy((char *)respdu->caMsg+i*64
                   , ret.at(i).toStdString().c_str()
                   , ret.at(i).size());
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }case ENUM_MSG_TYPE_SENDER_CHAR_REQUEST:{
        char caSenderName[32] = {'\0'};
        char caName[32] = {'\0'};
        char caChar[64] = {'\0'};
        memcpy(caSenderName, pdu->caData, 32);
        memcpy(caName, pdu->caData+32, 32);
        memcpy(caChar, pdu->caMsg, pdu->uiMsgLen);

        qDebug()<<caChar;

        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_SENDER_CHAR_RESPOND;
        if (OpeDB::getInstance().addChar(caSenderName,caName,caChar)) {
            strcpy(respdu->caData, SENDER_CHAR_OK);
        }else {
            strcpy(respdu->caData, SENDER_CHAR_FAILED);
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }case ENUM_MSG_TYPE_CREAT_DIR_REQUEST:{
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_CREAT_DIR_RESPOND;
        QDir dir;
        QString strCurPath = QString("%1").arg((char *)(pdu->caMsg));
        if (dir.exists(strCurPath)) {
            QString strNewDir = QString("%1/%2")
                                    .arg((char *)(pdu->caMsg))
                                    .arg((char *)(pdu->caData+32));
            if (dir.exists(strNewDir)) {
                strcpy(respdu->caData, CREAT_DIR_PATH_REP);
            }else{
                qDebug()<< strNewDir <<"创建新文件夹：" << dir.mkdir(strNewDir);
                strcpy(respdu->caData, CREAT_DIR_OK);
            }
        }else{
            strcpy(respdu->caData, CREAT_DIR_PATH_NO);
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    default:
    {break;}
    }

    free(pdu);
    pdu = NULL;
}

void MyTcpSocket::clientOffline()
{
    OpeDB::getInstance().handleOffline(user_name.toStdString().c_str());
    emit offline(this);
}

QString MyTcpSocket::getUser_name() const
{
    return user_name;
}

#include "mytcpsocket.h"
#include "protocol.h"
#include "opedb.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QFileInfoList>
#include <QFileInfo>

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
    }case ENUM_MSG_TYPE_SHOW_FLIE_REQUEST:{
        FLIE *flie = NULL;
        QString strNewFlie = QString("%1").arg((char *)(pdu->caMsg));
        QDir dir(strNewFlie);
        QFileInfoList flieName = dir.entryInfoList();
        PDU *respdu = mkPDU((sizeof(FLIE))*(flieName.size()-2));
        respdu->uiMsgType = ENUM_MSG_TYPE_SHOW_FLIE_RESPOND;
        QString strFlieName;
        for (int var = 2; var < flieName.size(); ++var) {
            flie = (FLIE *)(respdu->caMsg)+var-2;
            strFlieName = flieName[var].fileName();
            strcpy(flie->flieName, strFlieName.toStdString().c_str());
            flie->flieSize = flieName[var].size()/1024;
            if (flieName[var].isFile()) {
                flie->isDir=0;
            }else{
                flie->isDir=1;
            }
        }


        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }case ENUM_MSG_TYPE_DELETE_FLIE_REQUEST:{
        char strFlie[32];
        memcpy(strFlie, pdu->caData, 32);
        QString strPath = QString("%1/%2")
                              .arg((char *)(pdu->caMsg)).arg(strFlie);
        QFileInfo flieInfo(strPath);
        bool validation = false;
        if (flieInfo.isFile()) {
            QDir dir;
            validation = dir.remove(strPath);
        }else if (flieInfo.isDir()) {
            QDir dir;
            dir.setPath(strPath);
            validation = dir.removeRecursively();
        }
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FLIE_RESPOND;
        if (validation) {
            strcpy(respdu->caData, DELETE_FLIE_OK);
        }else{
            strcpy(respdu->caData, DELETE_FLIE_FAILED);
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }case ENUM_MSG_TYPE_RENNAME_FLIE_REQUEST:{
        char strNewNameFlie[32];
        char strOldNameFlie[32];
        memcpy(strNewNameFlie, pdu->caData+32, 32);
        memcpy(strOldNameFlie, pdu->caData, 32);
        QString strNewPath = QString("%1/%2.%3")
                              .arg((char *)(pdu->caMsg)).arg(strNewNameFlie)
                                 .arg(QFileInfo(strOldNameFlie).suffix());
        QString strOldPath = QString("%1/%2")
                              .arg((char *)(pdu->caMsg)).arg(strOldNameFlie);
        QDir dir;
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_RENNAME_FLIE_RESPOND;
        if (dir.rename(strOldPath, strNewPath)) {
            strcpy(respdu->caData, RENNAME_FLIE_OK);
        }else{
            strcpy(respdu->caData, RENNAME_FLIE_FAILED);
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:{
        PDU *respdu;
        FLIE *flie = NULL;
        char strDir[32];
        memcpy(strDir, pdu->caData, 32);
        // qDebug()<<strDir;
        QString strNewFlie = QString("%1").arg((char *)(pdu->caMsg));
        // qDebug()<<strNewFlie;
        QDir existsDir;
        if (!existsDir.exists(strNewFlie)) {
            respdu = mkPDU(0);
            // qDebug()<<"c";
            strcpy(respdu->caData, DIR_INEXISTENCE);
            respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
        }else{
            strNewFlie = QString("%1/%2").arg((char *)(pdu->caMsg)).arg(strDir);
            // qDebug()<<strNewFlie;
            QFileInfo flieInfo(strNewFlie);
            if (flieInfo.isDir()) {
                QDir dir(strNewFlie);
                QFileInfoList flieName = dir.entryInfoList();
                respdu = mkPDU(strNewFlie.size()+1);
                strcpy(respdu->caData, DIR_ENTER_OK);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                strcpy((char *)respdu->caMsg, strNewFlie.toStdString().c_str());
            }else{
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                strcpy(respdu->caData, DIR_ENTER_FAILED);
            }
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:{
        char caFileName[32] = {'\0'};
        qint64 fileSize = 0;
        sscanf(pdu->caData, "%s %lld",caFileName, &fileSize);
        char *pPath = new char[pdu->uiMsgLen];
        memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
        QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
        delete []pPath;
        pPath = NULL;
        qDebug()<<"文件: "<<caFileName<<" ("<<fileSize/1024<<"kb)开始上传。";
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
        QDir dir;
        if (!dir.exists(strPath)) {
            strcpy(respdu->caData, FILE_CREAT_OK);
            upLoadFile *upF = new upLoadFile();
            upF->m_file.setFileName(strPath);
            upF->m_strPath = strPath;
            if (upF->m_file.open(QIODevice::WriteOnly)) {
                upF->m_iTotal = fileSize;
                upF->m_iRecved = 0;
            }
            m_upLoadFileList.append(upF);
        }else{
            strcpy(respdu->caData, FILE_CREAT_FAILED);
        }
        write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }case ENUM_MSG_TYPE_UPLOAD_DATA_REQUEST:{
        qDebug()<<"cs";
        char caFileName[32] = {'\0'};
        qint64 fileSize = 0;
        sscanf(pdu->caData, "%s %lld",caFileName, &fileSize);
        foreach (auto &vate, m_upLoadFileList) {
            // QString m_strfileName = vate->m_file.fileName();
            if(vate->m_iTotal == fileSize ){
                qDebug()<<"文件: "<<caFileName<<" ("<<fileSize/1024<<"kb)上传中。";
                char *Buff = new char[pdu->uiMsgLen];
                vate->m_size=pdu->uiMsgLen;
                memcpy(Buff ,pdu->caMsg, pdu->uiMsgLen);
                vate->pBuffer = Buff;
                vate->start();
                break;
            }
        }
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

MyTcpSocket &MyTcpSocket::getInstance()
{
    static MyTcpSocket instance;
    return instance;
}

upLoadFile::upLoadFile(QObject *parent) : QThread(parent)
{}

void upLoadFile::run()
{
    PDU *respdu = mkPDU(0);
    respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_DATA_RESPOND;
    m_file.open(QIODevice::WriteOnly);
    m_file.write(pBuffer);
    free(pBuffer);
    pBuffer = NULL;
    m_iRecved += m_size;
    if (m_iTotal == m_iRecved) {
        m_file.close();
        strcpy(respdu->caData, UPLOAD_DATA_OK);
        MyTcpSocket::getInstance().write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        quit();
    }else if (m_iTotal < m_iRecved) {
        m_file.close();
        strcpy(respdu->caData, UPLOAD_DATA_FAILED);
        MyTcpSocket::getInstance().write((char *)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        quit();
    }
}

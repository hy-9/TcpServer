#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QThread>
#include <QFile>
#include <QList>

class upLoadFile:public QThread
{
public:
    upLoadFile(QObject *parent = nullptr);
    QString m_strPath;
    char *pBuffer;
    QFile m_file;
    qint64 m_size;
    qint64 m_iTotal;
    qint64 m_iRecved;
protected:
    void run();
};

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT

signals:
    void offline(MyTcpSocket *mysocket);
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString getUser_name() const;
    static MyTcpSocket & getInstance();

public slots:
    void recvMsg();
    void clientOffline();

private:
    QString user_name;
    QList <upLoadFile *> m_upLoadFileList;

};

#endif // MYTCPSOCKET_H

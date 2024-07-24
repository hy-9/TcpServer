#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT

signals:
    void offline(MyTcpSocket *mysocket);
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString getUser_name() const;

public slots:
    void recvMsg();
    void clientOffline();

private:
    QString user_name;
};

#endif // MYTCPSOCKET_H

#ifndef OPEDB_H
#define OPEDB_H
#include <QObject>
class QSqlDatabase;
class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    static OpeDB& getInstance();
    void init();
    ~OpeDB();

    bool handleRegist(const char *name,const char *pwd);
    bool handleLogin(const char *name,const char *pwd);
    QStringList handleAllOnline();
    void handleOffline(const char *name);
    //查询用户，不存在-1 不在线0 在线1
    int handleUrs(const char *name);
    // 查询好友，无法添加-1 可以添加1 已存在0
    int handleFriend(const char *senderName,const char *name);
    bool addFriend(const char *senderName,const char *name);
    int getId(const char *name);
    QString getName(int id);
    QStringList getShowFrieng(const char *name);
    bool deleteFriend(const char *senderName,const char *name);
    QStringList handleChar(const char *senderName,const char *name);
    void setRead(const int senderNameID, const int nameID);
    bool addChar(const char *senderName,const char *name,const char *mes);

signals:

private:
    QSqlDatabase *m_db;
};

#endif // OPEDB_H

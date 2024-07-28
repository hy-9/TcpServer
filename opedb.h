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

signals:

private:
    QSqlDatabase *m_db;
};

#endif // OPEDB_H

#ifndef MAINSERVER_H
#define MAINSERVER_H
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QLinkedList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlResult>
#include <QCryptographicHash>
#include <QMutex>
#include <QDate>
#include <QTimer>
#include <acore.h>
#include <atcpserver.h>
#include <config.h>
#include "aipfunc.h"
extern ACore::ALog logs;
extern ACore::ASettings settings;
extern int MinThreadd;
extern int MaxThreadd,MaxCommandsInQuest;
extern int SRCMode;
extern bool isDebug,isHttpMode;
extern QSqlDatabase db;
struct MainClient : public validClient
{
    QString name;
    QString pass;
    int id,achived,banned;
    bool Hidden;
    QStringList permissions;
    QString RegIP,init,initV,TimeZone,ShowName,status,email,prefix,colored,real_name;
    QMap<int,int> msgmap;
    QList<int> allowrooms;
};

struct Room
{
    int id;
    QString name;
    int creater;
    QList<int> userslist;
};
class MainServer : public ATCPServer
{
    Q_OBJECT
public:
    MainServer();
    ~MainServer();
    void UseCommand(QByteArray hdata, validClient* lClient, ServerThread *thisThread) override;
    virtual validClient* NewValidClient();
    virtual void DelValidClient(validClient* h);
    QTimer* timer;
    QByteArray versionarr;
    QString ClientInConnectText;
public slots:
    void MonitorTimer();
    void clientConn(validClient* user);
protected:
    long long int GetedBytes;
    long long int GetedCmds;
    long long int sendedCmds;
};
void ReloadConfig();
void AdminLog(validClient* n,QString action,QString info);
extern MainServer* serverd;
#endif // MAINSERVER_H

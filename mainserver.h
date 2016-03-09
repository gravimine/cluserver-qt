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
#include <QTimer>
#include <acore.h>
#include <atcpserver.h>
#include <config.h>
#include "aipfunc.h"
extern ACore::ALog logs;
extern ACore::ASettings settings;
extern int MinThreadd;
extern int MaxThreadd;
extern int SRCMode;
extern bool isDebug;
extern QSqlDatabase db;
struct MainClient : public validClient
{
    QString name;
    QString pass;
    int id,achived;
    QStringList permissions;
    QString RegIP,init,initV,TimeZone,ShowName,status,email,prefix,colored,real_name;
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
    void UseCommand(QByteArray hdata, validClient* lClient, ServerThread *thisThread);
    virtual validClient* NewValidClient();
    virtual void DelValidClient(validClient* h);
    QTimer* timer;
public slots:
    void MonitorTimer();
protected:
    long int GetedBytes;
};
void ReloadConfig();
extern MainServer* serverd;
#endif // MAINSERVER_H

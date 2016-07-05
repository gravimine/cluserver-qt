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
#include "asettings.h"
#include "alog.h"
#include "maincommand.h"
extern ACore::ALog logs;
extern ACore::ASettings settings;
extern int MinThreadd;
extern int MaxThreadd,MaxCommandsInQuest;
extern int SRCMode;
extern bool isDebug,isHttpMode;
extern QSqlDatabase db;
class MainClient : public validClient
{
public:
    QString name;
    QString pass;
    int id,banned;
    bool Hidden;
    QStringList permissions;
    QString RegIP,init,initV,TimeZone,ShowName,status,email,prefix,colored,real_name;
    QMap<int,int> msgmap;
    QList<int> allowrooms;
    void clear();
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
    virtual void DelValidClient(validClient* h);
    bool addCommand(MainCommand* cmd);
    bool removeCommand(QString name);
    bool disableCommand(QString name);
    bool enableCommand(QString name);
    QTimer* timer;
    QByteArray versionarr;
    QString ClientInConnectText;
    QMap<QString,MainCommand*> commands;
    validClient* NewValidClient()
    {
        return (validClient*) new MainClient();
    }
    long long int GetedBytes;
    long long int GetedCmds;
    long long int sendedCmds;
public slots:
    void MonitorTimer();
    void clientConn(validClient* user);
};
void ReloadConfig();
void AdminLog(validClient* n,QString action,QString info);
extern MainServer* serverd;
#endif // MAINSERVER_H

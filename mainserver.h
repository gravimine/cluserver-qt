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
extern ACore::ALog logs;
extern ACore::ASettings settings;
extern int MinThreadd;
extern int MaxThreadd;
extern bool isDebug;
extern QSqlDatabase db;
struct MainClient : public validClient
{
    QString name;
    QString pass;
    int id;
    QStringList permissions;
};

class MainServer : public ATCPServer
{
    Q_OBJECT
public:
    MainServer();
    ~MainServer();
    void UseCommand(QByteArray hdata, validClient* lClient, int mClientID, ServerThread *thisThread);
    virtual validClient* NewValidClient();
    virtual void DelValidClient(validClient* h);
    QTimer* timer;
public slots:
    void MonitorTimer();
protected:
    int GetedBytes;
};
void ReloadConfig();
extern MainServer* serverd;
#endif // MAINSERVER_H

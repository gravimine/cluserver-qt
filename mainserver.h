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
extern ACore::ALog logs;
extern ACore::ASettings settings;
extern int MinThreadd;
extern int MaxThreadd;
extern QSqlDatabase db;
#define NO_PERMISSIONS_ERROR "<key>403</key>\n"
#define YES_REPLY "<key>200</key>\n"
#define THREAD_KILL_ERROR "<key>500</key>\n"
#define SERVER_STOP_REPLY "<key>505</key>\n"
#define HELLO_REPLY "<connect>200</connect>\n"
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
    void UseCommand(ArrayCommand sCommand, validClient* lClient, int mClientID, ServerThread *thisThread);
    virtual validClient* NewValidClient();
    virtual void DelValidClient(validClient* h);
    QTimer* timer;
public slots:
    void MonitorTimer();
protected:

};
void ReloadConfig();
extern MainServer serverd;
#endif // MAINSERVER_H

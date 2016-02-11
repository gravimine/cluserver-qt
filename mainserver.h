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
extern ACore::ALog logs;
extern ACore::ASettings settings;
extern int MinThread;
extern int MaxThread;
extern QSqlDatabase db;
#define NO_PERMISSIONS_ERROR "<key>403</key>\n"
#define YES_REPLY "<key>200</key>\n"
#define THREAD_KILL_ERROR "<key>500</key>\n"
#define SERVER_STOP_REPLY "<key>505</key>\n"
#define HELLO_REPLY "<connect>200</connect>\n"
struct newClient
{
    QTcpSocket* socket;
    QString data;
};
enum ClientState
{
    NoAuthState,
    AuthState,
    waitCloseInServer,
    WaitCliseInClient
};

struct validClient : public newClient
{
    QString name;
    QString pass;
    int id,numUsingCommands;
    ClientState state;
    QStringList permissions;
    bool isAuth,isUseCommand;
};
struct ArrayCommand
{
    QByteArray command;
    QTcpSocket* client;
    bool operator ==(ArrayCommand h)
    {
        if(client==h.client) return true;
        else return false;
    }
};
class ServerThread : public QThread
{
    Q_OBJECT
private:
    void run();
public:
    ServerThread();
    void UseCommand();
    QMutex mutexArray;
    QLinkedList<ArrayCommand> ArrayCommands;
    bool isStaticThread,isSleep;
    int currentIdThread,currentCommandID;
public slots:
    void NewCommand(int idThread);
};
class MainServer : public QObject
{
    Q_OBJECT
public:
    MainServer();
    ~MainServer();
    QList<validClient*> ClientsList;
    QList<ServerThread*> ThreadList;
    bool launch(int port);
    void sendToClient(QTcpSocket* socket, QString str);
    void sendToClient(int clientID, QString str);
    validClient *getClient(QTcpSocket* socket);
    int GetIDClient(QTcpSocket* socket);
    QTcpServer* serverd;
protected:
    QTimer* timer;
signals:
void signalCommand(int idThread);
public slots:
    void clientConnected();
    void clientDisconnect();
    void clientReadyRead();
    void MonitorTimer();
};
void ReloadConfig();
extern MainServer server;
#endif // MAINSERVER_H

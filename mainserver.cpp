#include "mainserver.h"
#include <QApplication>
ACore::ALog logs;
bool isDebug;
QSqlDatabase db;
int MinThread;
int MaxThread;
ACore::ASettings settings("settings.cfg",ACore::CfgFormat);
MainServer server;

MainServer::MainServer()
{
    serverd = new QTcpServer(this);
    timer = new QTimer();
    logs.SetCoutDebug(true);
    connect(serverd, SIGNAL(newConnection()),
                this,         SLOT(clientConnected())
               );
    connect(timer, SIGNAL(timeout()), this, SLOT(MonitorTimer()));

}
void MainServer::MonitorTimer()
{
    ACore::RecursionArray otchet;
    otchet["ClientsListSize"]=ClientsList.size();
    otchet["Threads"]=ThreadList.size();
    for(int i=0;i<ThreadList.size();i++)
    {
        if(ThreadList.value(i)->isSleep)
            otchet[QString::number(i)] = ThreadList.value(i)->ArrayCommands.size();
        else
            otchet[QString::number(i)] = ThreadList.value(i)->ArrayCommands.size()+1;
    }
    qDebug() << otchet.print();
}

MainServer::~MainServer()
{
    settings.SaveSettings();
    for(int i=0;i<ThreadList.size();i++)
    {
        ServerThread* thread = ThreadList[i];
        thread->exit(0);
        thread->deleteLater();
        ThreadList.removeAt(i);
    }
    for(int i=0;i<ClientsList.size();i++)
    {
        validClient* nClient = ClientsList[i];
        delete nClient;
        ClientsList.removeAt(i);
    }
    serverd->close();
    delete serverd;
}

bool MainServer::launch(int port)
{
    if(!serverd->listen(QHostAddress::Any,port))
    {
        logs << "Bad listen(): "+serverd->errorString();
        return false;
    }
    else {
        logs << "Server started";
        //Создание потоков
        for(int i=0;i<MinThread;i++)
        {
            ServerThread * newServerThread = new ServerThread();
            newServerThread->currentIdThread = i;
            newServerThread->isStaticThread = true;
            connect(this, SIGNAL(signalCommand(int)),
                        newServerThread , SLOT(NewCommand(int))
                       , Qt::QueuedConnection);
            newServerThread->start();
            ThreadList << newServerThread;
        }
        timer->start(1000);
        return true;
    }
}
void MainServer::clientConnected()
{
    QTcpSocket* pClientSocket = serverd->nextPendingConnection();
    connect(pClientSocket, SIGNAL(disconnected()),
                this , SLOT(clientDisconnect())
               );
        connect(pClientSocket, SIGNAL(readyRead()),
                this,          SLOT(clientReadyRead())
               );
    validClient* h = new validClient();
    h->socket = pClientSocket;
    h->isAuth = false;
    h->isUseCommand = false;
    h->numUsingCommands = 0;
    h->state = NoAuthState;
    ClientsList << h;
    logs << "Client connect "+pClientSocket->peerAddress().toString()+":"+QString::number(pClientSocket->peerPort());
    sendToClient(pClientSocket, "Server Response: Connected!\n");
}
void MainServer::clientDisconnect()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    for(int i=0;i<ThreadList.size();i++)
    {
        ArrayCommand t;
        t.client = socket;
        while(ThreadList[i]->ArrayCommands.removeOne(t)) {
        }
    }
    int mClientID = GetIDClient(socket);
    //delete ClientsList[mClientID];
    if(mClientID > 0 && mClientID < ClientsList.size()) {
    if(!ClientsList.value(mClientID)->isUseCommand){
    ClientsList.removeAt(mClientID);
    socket->deleteLater();}
    else
    {
        ClientsList[mClientID]->state = WaitCliseInClient;
    }}
    logs << "Client disconnect";
}
validClient* MainServer::getClient(QTcpSocket* socket)
{
    validClient* result;
    for(int i=0;i<ClientsList.size();i++)
    {
        if(ClientsList.value(i)->socket == socket) result = ClientsList[i];
    }
    return result;
}
int MainServer::GetIDClient(QTcpSocket* socket)
{
    int result;
    for(int i=0;i<ClientsList.size();i++)
    {
        if(ClientsList.value(i)->socket == socket) result = i;
    }
    return result;
}
void ReloadConfig()
{
    isDebug = settings["Debug"].toBool();
    MinThread = settings["MinThread"].toInt();
    MaxThread = settings["MaxThread"].toInt();
}

void MainServer::clientReadyRead()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    QByteArray data = socket->readAll();
    ArrayCommand cmd;
    cmd.client = socket;
    cmd.command = data;
    int ThreadID  = MaxThread+1;
    int MinCommands=INT_MAX;
    for(int i=0;i<ThreadList.size();i++)
    {
        if(ThreadList.value(i)->ArrayCommands.size()==0)
        {
            MinCommands=0;
            ThreadID = i;
            break;
        }
        else if(ThreadList.value(i)->ArrayCommands.size()<MinCommands)
        {
            MinCommands=ThreadList.value(i)->ArrayCommands.size();
            ThreadID = i;
        }
    }
    ThreadList.value(ThreadID)->mutexArray.lock();
    ThreadList.value(ThreadID)->ArrayCommands << cmd;
    ThreadList.value(ThreadID)->mutexArray.unlock();
    if(MinCommands==0) signalCommand(ThreadID);
    /*ClientsList[mClientID].data+=data;
    QString realData = ClientsList[mClientID].data;
    if(realData.mid(realData.size()-2) != "\n\n")
    {
        return;
    }
    ClientsList[mClientID].data.clear();*/
}
void MainServer::sendToClient(QTcpSocket* socket, QString str)
{
    socket->write(str.toUtf8());
}
void MainServer::sendToClient(int clientID, QString str)
{
    validClient* n = ClientsList.value(clientID);
    if(n->state != WaitCliseInClient)
    ClientsList.value(clientID)->socket->write(str.toUtf8());
}
ServerThread::ServerThread()
{
    isSleep = true;
    moveToThread(this);
}

void ServerThread::run()
{
    if(!isStaticThread) UseCommand();
    else exec();
}
void ServerThread::NewCommand(int idThread)
{
    if(idThread == currentIdThread)
    {
        UseCommand();
        if(!ArrayCommands.isEmpty()) UseCommand();
        else isSleep=true;
    }
}
void ServerThread::UseCommand()
{
    isSleep=false;
    if(ArrayCommands.isEmpty())
    {
        return;
    }
    mutexArray.lock();
    ArrayCommand sCommand = ArrayCommands.takeFirst();
    mutexArray.unlock();
    QString data = QString::fromUtf8( sCommand.command );
    int mClientID= server.GetIDClient(sCommand.client);
    data = data.replace("\r\n","\n");
    logs << data;
    validClient* nClient = server.ClientsList.value(mClientID);
    nClient->isUseCommand = true;
    nClient->numUsingCommands++;
    ACore::RecursionArray ReplyMap;
    ReplyMap.fromHTMLTegsFormat(data);
    if(isDebug) qDebug() << ReplyMap.print();
    QString cmd=ReplyMap["type"].toString();
    if(cmd=="set")
    {
        if(nClient->isAuth){
            if(nClient->permissions.indexOf("ADM"))
        settings[ReplyMap["name"].toString()]=ReplyMap["value"].toString();
        }
        else server.sendToClient(mClientID, NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="reload")
    {
        if(nClient->isAuth){
            if(nClient->permissions.indexOf("ADM"))
        ReloadConfig();
        }
        else server.sendToClient(mClientID, NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="close")
    {
        sCommand.client->close();
    }
    else if(cmd=="stop") //ОПАСТНО!!
    {
        if(nClient->isAuth){
            if(nClient->permissions.indexOf("ADM")){
                for(int i=0;i<server.ClientsList.size();i++) {server.ClientsList.value(i)->socket->write(SERVER_STOP_REPLY);
        server.ClientsList.value(i)->socket->waitForBytesWritten(1000);}
        qApp->quit();}}
        else server.sendToClient(mClientID, NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="sleep")
    {
        sleep(20);
        server.sendToClient(mClientID, "SLEEP OK\n");
    }
    else if(cmd=="monitor")
    {
        ACore::RecursionArray otchet;
        otchet["ClientsListSize"]=server.ClientsList.size();
        otchet["Threads"]=server.ThreadList.size();
        for(int i=0;i<server.ThreadList.size();i++) otchet[QString::number(i)] = server.ThreadList.value(i)->ArrayCommands.size();
        server.sendToClient(mClientID, otchet.toHTMLTegsFormat());
    }
    else if(cmd=="auth")
    {
        QSqlQuery sqlquery;

        if (!sqlquery.exec(QString("SELECT * FROM users WHERE name = '%1' && pass = md5('%2');").arg(ReplyMap["login"].toString()).arg(ReplyMap["pass"].toString()))) {
                qDebug() << "[auth]Query stopped: "+sqlquery.lastError().text();
            }
        else
        {
            sqlquery.next();
            validClient* newClient = new validClient();
            newClient->isAuth=true;
            newClient->name=ReplyMap["login"].toString();
            newClient->pass=ReplyMap["pass"].toString();
            newClient->permissions=sqlquery.value("group").toString().split(", ");
            newClient->socket=sCommand.client;
            newClient->isUseCommand = false;
            newClient->state = AuthState;
            newClient->numUsingCommands = nClient->numUsingCommands;
            newClient->id=sqlquery.value("id").toInt();
            delete server.ClientsList[mClientID];
            server.ClientsList.removeAt(mClientID);
            server.ClientsList << newClient;
        }
    }
    nClient->isUseCommand = false;
    nClient->numUsingCommands--;
    if(nClient->state == WaitCliseInClient && nClient->numUsingCommands == 0)
    {
        server.ClientsList.removeAt(mClientID);
        sCommand.client->deleteLater();
    }
}

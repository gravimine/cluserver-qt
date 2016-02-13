#include "mainserver.h"
#include <QCoreApplication>
ACore::ALog logs;
bool isDebug;
QSqlDatabase db;
int MinThreadd;
int MaxThreadd;
int MaxCommandsInQuest;
ACore::ASettings settings("settings.cfg",ACore::CfgFormat);
MainServer* serverd;
void ReloadConfig()
{
    isDebug = settings["Debug"].toBool();
    MinThreadd = settings["MinThread"].toInt();
    MaxThreadd = settings["MaxThread"].toInt();
    MaxCommandsInQuest =settings["MaxCommandsInQuest"].toInt();
}
validClient* MainServer::NewValidClient()
{
    return (validClient*) new MainClient();
}
void MainServer::DelValidClient(validClient* h)
{
    delete (MainClient*)h;
}
void MainServer::MonitorTimer()
{
    ACore::RecursionArray otchet;
    otchet["ClientsListSize"]=ClientsList.size();
    otchet["Threads"]=ThreadList.size();
    otchet["GetBytes"]=QString::number( GetedBytes );
    for(int i=0;i<ThreadList.size();i++)
    {
        if(ThreadList.value(i)->isSleep)
            otchet[QString::number(i)] = ThreadList.value(i)->ArrayCommands.size();
        else
            otchet[QString::number(i)] = ThreadList.value(i)->ArrayCommands.size()+1;
    }
    qDebug() << otchet.print();
}
MainServer::MainServer()
{
    timer = new QTimer();
    GetedBytes = 0;
    connect(timer, SIGNAL(timeout()), this, SLOT(MonitorTimer()));

}

MainServer::~MainServer()
{
    delete timer;
}

void MainServer::UseCommand(QByteArray hdata, validClient* lClient, int mClientID, ServerThread* thisThread)
{
    QStringList dataList;
    GetedBytes +=hdata.size();
    if(MaxCommandsInQuest>0) {
        dataList = QString::fromUtf8( hdata ).split("\n\n");
        if(dataList.size()>MaxCommandsInQuest) thisThread->sendToClient(mClientID, BAD_REQUEST_REPLY);
    }
    else dataList << QString::fromUtf8( hdata );
    for(int lstd=0;lstd<dataList.size();lstd++){

    MainClient* nClient = (MainClient*) lClient;
    QString data = dataList.value(lstd);
    data = data.replace("\r","");
    data = data.replace("\n","");
    ACore::RecursionArray ReplyMap;
    ReplyMap.fromPostGetFormat(data);
    if(isDebug) qDebug() << ReplyMap.print();
    QString cmd=ReplyMap["type"].toString();
    if(cmd=="set")
    {
        if(nClient->isAuth){
            if(nClient->permissions.indexOf("ADM"))
        settings[ReplyMap["name"].toString()]=ReplyMap["value"].toString();
        }
        else thisThread->sendToClient(mClientID, NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="reload")
    {
        if(nClient->isAuth){
            if(nClient->permissions.contains("ADM"))
        ReloadConfig();
        }
        else thisThread->sendToClient(mClientID, NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="savesettings")
    {
        if(nClient->isAuth){
            if(nClient->permissions.contains("ADM"))
        settings.SaveSettings();
            else thisThread->sendToClient(mClientID, YES_REPLY);
        }
        else thisThread->sendToClient(mClientID, NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="close")
    {
        CloseClient(mClientID);
    }
    else if(cmd=="stop") //ОПАСТНО!!
    {
        if(nClient->isAuth){
            if(nClient->permissions.contains("ADM")){
                for(int i=0;i<ClientsList.size();i++) {ClientsList.value(i)->socket->write(SERVER_STOP_REPLY);
        ClientsList.value(i)->socket->waitForBytesWritten(1000);}
        qApp->quit();}}
        else thisThread->sendToClient(mClientID, NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="sleep")
    {
        ACore::Sleeper::sleep(20);
        thisThread->sendToClient(mClientID, "SLEEP OK\n");
    }
    else if(cmd=="infosu")
    {
        ACore::RecursionArray result;
        result["isAuth"]=nClient->isAuth;
        result["Name"]=nClient->name;
        result["id"]=nClient->id;
        if(nClient->isAuth){
            if(nClient->permissions.contains("ADM")){
                result["AccountType"]="Admin";
            }
            else result["AccountType"]="User";
        }
        thisThread->sendToClient(mClientID, result.toHTMLTegsFormat());
    }
    else if(cmd=="su")
    {
        if(SUPERUSER_AVALIBLE){
        MainClient* newClient = new MainClient();
        newClient->isAuth=true;
        newClient->name=SUPERUSER_LOGIN;
        newClient->pass=SUPERUSER_PASS;
        QString str = SUPERUSER_PERMISSIONS;
        newClient->permissions=str.split(", ");
        newClient->socket=nClient->socket;
        newClient->isUseCommand = false;
        newClient->state = AuthState;
        newClient->numUsingCommands = nClient->numUsingCommands;
        newClient->id=SUPERUSER_ID;
        delete ClientsList[mClientID];
        ClientsList.removeAt(mClientID);
        ClientsList << (validClient*) newClient;
        qDebug() << "SUPERUSER ENTERED";
        thisThread->sendToClient(mClientID, YES_REPLY);}
        else
        thisThread->sendToClient(mClientID, NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="auth")
    {
        QSqlQuery sqlquery;

        if (!sqlquery.exec(QString("SELECT * FROM users WHERE name = '%1' && pass = md5('%2');").arg(ReplyMap["login"].toString()).arg(ReplyMap["pass"].toString()))) {
                qDebug() << "[auth]Query stopped: "+sqlquery.lastError().text();
            }
        else
        {
            if(sqlquery.size()<1)
            {
                thisThread->sendToClient(mClientID, AUTH_ERROR);
                return;
            }
            sqlquery.next();
            MainClient* newClient = new MainClient();
            newClient->isAuth=true;
            newClient->name=ReplyMap["login"].toString();
            newClient->pass=ReplyMap["pass"].toString();
            QString str = sqlquery.value("group").toString();
            newClient->permissions=str.split(", ");
            newClient->socket=nClient->socket;
            newClient->isUseCommand = false;
            newClient->state = AuthState;
            newClient->numUsingCommands = nClient->numUsingCommands;
            newClient->id=sqlquery.value("id").toInt();
            delete ClientsList[mClientID];
            ClientsList.removeAt(mClientID);
            ClientsList << (validClient*) newClient;
            thisThread->sendToClient(mClientID, YES_REPLY);
        }
    }
    }
}

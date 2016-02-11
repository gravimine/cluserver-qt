#include "mainserver.h"
#include <QApplication>
ACore::ALog logs;
bool isDebug;
QSqlDatabase db;
int MinThreadd;
int MaxThreadd;
ACore::ASettings settings("settings.cfg",ACore::CfgFormat);
MainServer serverd;
void ReloadConfig()
{
    isDebug = settings["Debug"].toBool();
    MinThreadd = settings["MinThread"].toInt();
    MaxThreadd = settings["MaxThread"].toInt();
}
validClient* MainServer::NewValidClient()
{
    return (validClient*) new MainClient();
}
void MainServer::DelValidClient(validClient* h)
{
    delete (MainClient*)h;
}

void MainServer::UseCommand(ArrayCommand sCommand, validClient* lClient,int mClientID)
{
    MainClient* nClient = (MainClient*) lClient;
    QString data = QString::fromUtf8( sCommand.command );
    data = data.replace("\r\n","\n");
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
        else sendToClient(mClientID, NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="reload")
    {
        if(nClient->isAuth){
            if(nClient->permissions.indexOf("ADM"))
        ReloadConfig();
        }
        else sendToClient(mClientID, NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="close")
    {
        sCommand.client->close();
    }
    else if(cmd=="stop") //ОПАСТНО!!
    {
        if(nClient->isAuth){
            if(nClient->permissions.indexOf("ADM")){
                for(int i=0;i<ClientsList.size();i++) {ClientsList.value(i)->socket->write(SERVER_STOP_REPLY);
        ClientsList.value(i)->socket->waitForBytesWritten(1000);}
        qApp->quit();}}
        else sendToClient(mClientID, NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="sleep")
    {
        ACore::Sleeper::sleep(20);
        sendToClient(mClientID, "SLEEP OK\n");
    }
    else if(cmd=="monitor")
    {
        ACore::RecursionArray otchet;
        otchet["ClientsListSize"]=ClientsList.size();
        otchet["Threads"]=ThreadList.size();
        for(int i=0;i<ThreadList.size();i++) otchet[QString::number(i)] = ThreadList.value(i)->ArrayCommands.size();
        sendToClient(mClientID, otchet.toHTMLTegsFormat());
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
            MainClient* newClient = new MainClient();
            newClient->isAuth=true;
            newClient->name=ReplyMap["login"].toString();
            newClient->pass=ReplyMap["pass"].toString();
            newClient->permissions=sqlquery.value("group").toString().split(", ");
            newClient->socket=sCommand.client;
            newClient->isUseCommand = false;
            newClient->state = AuthState;
            newClient->numUsingCommands = nClient->numUsingCommands;
            newClient->id=sqlquery.value("id").toInt();
            delete ClientsList[mClientID];
            ClientsList.removeAt(mClientID);
            ClientsList << (validClient*) newClient;
        }
    }
}

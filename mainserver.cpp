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
using namespace ACore;
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
    RecursionArray otchet;
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
    //if(isDebug) logs << hdata;
    if(MaxCommandsInQuest>0) {
        dataList = QString::fromUtf8( hdata ).split("\n\n");
        if(dataList.size()>MaxCommandsInQuest) SEND_CLIENT( BAD_REQUEST_REPLY);
    }
    else dataList << QString::fromUtf8( hdata );
    for(int lstd=0;lstd<dataList.size();lstd++){

    MainClient* nClient = (MainClient*) lClient;
    QString data = dataList.value(lstd);
    //data = data.replace("\r","");
    //data = data.replace("\n","");
    RecursionArray ReplyMap;
    ReplyMap.fromPostGetFormat(data);
    //if(isDebug) qDebug() << ReplyMap.print();
    QString cmd=ReplyMap["type"].toString();
    if(cmd=="set")
    {
        if(nClient->isAuth){
            if(IS_ADMIN)
        settings[ReplyMap["name"].toString()]=ReplyMap["value"].toString();
            else SEND_CLIENT(NO_PERMISSIONS_ERROR);
        }
        else SEND_CLIENT(NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="reload")
    {
        if(nClient->isAuth){
            if(IS_ADMIN)
            {
                settings.LoadSettings();
                ReloadConfig();
            }
            else SEND_CLIENT(NO_PERMISSIONS_ERROR);
        }
        else SEND_CLIENT(NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="savesettings")
    {
        if(nClient->isAuth){
            if(IS_ADMIN){
            settings.SaveSettings();
            SEND_CLIENT(YES_REPLY);}
            else SEND_CLIENT(NO_PERMISSIONS_ERROR);
        }
        else SEND_CLIENT(NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="close")
    {
        CloseClient(mClientID);
    }
    else if(cmd=="test")
    {
        SEND_CLIENT(YES_REPLY);
    }
    else if(cmd=="stop") //ОПАСТНО!!
    {
        if(nClient->isAuth){
            if(IS_ADMIN){
                for(int i=0;i<ClientsList.size();i++) {ClientsList.value(i)->socket->write(SERVER_STOP_REPLY);
                ClientsList.value(i)->socket->waitForBytesWritten(1000);}
                qApp->quit();
            }
            else SEND_CLIENT(NO_PERMISSIONS_ERROR);
        }
        else SEND_CLIENT(NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="killclients")
    {
        if(nClient->isAuth){
            if(IS_ADMIN){
                for(int i=0;i<ClientsList.size();i++) {ClientsList.value(i)->socket->write(THREAD_KILL_ERROR);
                ClientsList.value(i)->socket->waitForBytesWritten(1000); CloseClient(i);}
            }
            else SEND_CLIENT(NO_PERMISSIONS_ERROR);
        }
        else SEND_CLIENT(NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="rooms")
    {
        if(nClient->isAuth){
            QSqlQuery sqlquery;
            sqlquery.prepare("SELECT * from rooms");
            if(!sqlquery.exec()) {qDebug() << "[rooms]Query stopped: "+sqlquery.lastError().text();
            SEND_CLIENT( SQL_ERROR );}
            else
            {
                RecursionArray replymap;
                for(int i=0;i<sqlquery.size();i++){
                    sqlquery.next();
                    RecursionArray reply;
                reply["id"]=sqlquery.value("id").toInt();
                reply["name"]=sqlquery.value("nameTextRoom").toString();
                reply["idUserCreat"]=sqlquery.value("idUserCreat").toInt();
                reply["users"]=sqlquery.value("idsUsers").toString();
                replymap[QString::number(i)]=reply;}
                SEND_CLIENT ( replymap.toHTMLTegsFormat());
            }

        }
        else SEND_CLIENT(NO_PERMISSIONS_ERROR);
    }
    else if(cmd=="sleep")
    {
        if(nClient->isAuth){
            if(IS_ADMIN){
                ACore::Sleeper::msleep(ReplyMap["n"].toString().toInt());
                SEND_CLIENT(YES_REPLY);
            }
            else SEND_CLIENT(NO_PERMISSIONS_ERROR);
        }
        else SEND_CLIENT(NO_PERMISSIONS_ERROR);

    }
    else if(cmd=="info")
    {
        if(!nClient->isAuth)
        {
            SEND_CLIENT(NO_PERMISSIONS_ERROR);
            return;
        }
        RecursionArray result2;
        if(ReplyMap["id"].toString()=="?"){
            RecursionArray result;
            result["isAuth"]=nClient->isAuth;
            result["real_name"]=nClient->real_name;
            result["id"]=nClient->id;
            result["RegIP"]=nClient->RegIP;
            result["init"]=nClient->init;
            result["initV"]=nClient->initV;
            result["TimeZone"]=nClient->TimeZone;
            result["colored"]=nClient->colored;
            result["prefix"]=nClient->prefix;
            result["status"]=nClient->status;
            if(IS_ADMIN)
            {
                result["group"]="ADM, acc";
            }
            else result["group"]="acc";
            result2["0"]=result;
        }
        else
        {
            QSqlQuery sqlquery;

            QStringList clientlist = ReplyMap["id"].toString().split(",");
            if(clientlist.size()<1)
            {
                SEND_CLIENT(BAD_REQUEST_REPLY);
                return;
            }
            QString reply="SELECT * from users where ";
            for(int i=0;i<clientlist.size();i++)
            {
                reply+="id = "+clientlist.value(i);
                if(i!=clientlist.size()-1) reply+=" || ";
            }
            if (!sqlquery.exec(reply)) {
                    qDebug() << "[info]Query stopped: "+sqlquery.lastError().text();
                    SEND_CLIENT ( SQL_ERROR );
                }
            else
            {
                if(sqlquery.size()<1)
                {
                    SEND_CLIENT(BAD_REQUEST_REPLY);
                    return;
                }
                for(int i=0;i<sqlquery.size();i++)
                {
                    RecursionArray result;
                    result["isAuth"]=nClient->isAuth;
                    result["real_name"]=nClient->real_name;
                    result["id"]=nClient->id;
                    result["RegIP"]=nClient->RegIP;
                    result["init"]=nClient->init;
                    result["initV"]=nClient->initV;
                    result["TimeZone"]=nClient->TimeZone;
                    result["colored"]=nClient->colored;
                    result["prefix"]=nClient->prefix;
                    result["status"]=nClient->status;
                    if(IS_ADMIN)
                    {
                        result["group"]="ADM, acc";
                    }
                    else result["group"]="acc";
                    result2[QString::number(i)]=result;
                }
            }
        }
        SEND_CLIENT(result2.toHTMLTegsFormat());
    }
    else if(cmd=="sendsrc")
    {
        if(!nClient->isAuth)
        {
            SEND_CLIENT(NO_PERMISSIONS_ERROR);
            return;
        }
        QString host = ReplyMap["to"].toString();
        if(host.isEmpty()) SEND_CLIENT(BAD_REQUEST_REPLY);
        else
        {
            QTcpSocket sock;
            QStringList spl=host.split(":");
            sock.connectToHost(spl.value(0),spl.value(1).toInt());
            sock.waitForConnected(200);

            QString cmdd=ReplyMap["cmd"].toString();
            if(cmdd=="ping")
            {
                sock.write(QString("type=srcget&cmd=ping&sender="+nClient->name+"@"+settings["Host"].toString()+":"+settings["Port"].toString()).toUtf8());
                sock.waitForBytesWritten(200);
                sock.disconnectFromHost();
            }
        }
    }
    else if(cmd=="srcget")
    {
        if(ReplyMap["cmd"]=="ping")
            qDebug() << hdata;
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
        logs << "Client logged in superuser!";
        SEND_CLIENT(YES_REPLY);}
        else
        SEND_CLIENT(NO_PERMISSIONS_ERROR);
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
                SEND_CLIENT(AUTH_ERROR);
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
            newClient->init=sqlquery.value("init").toString();
            newClient->initV=sqlquery.value("initV").toString();
            newClient->status=sqlquery.value("status").toString();
            newClient->real_name=sqlquery.value("real_name").toString();
            newClient->prefix=sqlquery.value("prefix").toString();
            newClient->email=sqlquery.value("email").toString();
            newClient->TimeZone=sqlquery.value("TimeZone").toString();
            newClient->colored=sqlquery.value("colored").toString();
            newClient->RegIP=sqlquery.value("IP_REG").toString();
            newClient->achived=sqlquery.value("HAID").toInt();
            delete ClientsList[mClientID];
            ClientsList.removeAt(mClientID);
            ClientsList << (validClient*) newClient;
            logs << newClient->name +" auth";
            SEND_CLIENT(YES_REPLY);
        }
    }
    }
}

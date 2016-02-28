#include "mainserver.h"
#include <QCoreApplication>
ACore::ALog logs;
bool isDebug;
QSqlDatabase db;
int MinThreadd;
int MaxThreadd;
int SRCMode;
int MaxCommandsInQuest;
ACore::ASettings settings("settings.cfg",ACore::CfgFormat);
MainServer* serverd;
using namespace ACore;
void ReloadConfig()
{
    isDebug = settings["Debug"].toBool();
    SRCMode = settings["AllowSRC"].toInt();
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

void MainServer::UseCommand(QByteArray hdata, validClient* lClient, QLinkedList<validClient*>::iterator mClientID, ServerThread* thisThread)
{
    MainClient* nClient = (MainClient*) lClient;
    //data = data.replace("\r","");
    //data = data.replace("\n","");
    RecursionArray ReplyMap;
    ReplyMap.fromPostGetFormat(hdata);
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
        CloseClient(lClient);
    }
    else if(cmd=="test")
    {
        SEND_CLIENT(YES_REPLY);
    }
    else if(cmd=="stop") //ОПАСТНО!!
    {
        if(nClient->isAuth){
            if(IS_ADMIN){
                for(QLinkedList<validClient*>::iterator i=ClientsList.begin();i!=ClientsList.end();i++)
                {
                    (*i)->socket->write(SERVER_STOP_REPLY);
                    (*i)->socket->waitForBytesWritten(1000);
                }
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
                for(QLinkedList<validClient*>::iterator i=ClientsList.begin();i!=ClientsList.end();i++)
                {
                    (*i)->socket->write(THREAD_KILL_ERROR);
                    (*i)->socket->waitForBytesWritten(1000); CloseClient((*i));
                }
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
                reply["nameTextRoom"]=sqlquery.value("nameTextRoom").toString();
                reply["idUserCreat"]=sqlquery.value("idUserCreat").toInt();
                reply["idsUsers"]=sqlquery.value("idsUsers").toString();
                replymap[QString::number(i)]=reply;}
                RecursionArray tmpmap;
                tmpmap["arg"]=replymap;
                tmpmap["key"]=403;
                SEND_CLIENT ( tmpmap.toHTMLTegsFormat());
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
            result["timeZone"]=nClient->TimeZone;
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
                    result["timeZone"]=nClient->TimeZone;
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
        RecursionArray tmpmap;
        tmpmap["arg"]=result2;
        tmpmap["key"]=403;
        SEND_CLIENT(tmpmap.toHTMLTegsFormat());
    }
    else if(cmd=="sendsrc")
    {
        if(!nClient->isAuth || SRCMode == 0 || SRCMode == 5)
        {
            SEND_CLIENT(NO_PERMISSIONS_ERROR);
            return;
        }
        if(SRCMode == 3 || SRCMode == 1)
        {
            if(!IS_ADMIN)
            {
                SEND_CLIENT(NO_PERMISSIONS_ERROR);
                return;
            }
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
    else if(cmd=="onlineUsersRoom")
    {
        if(!nClient->isAuth)
        {
            SEND_CLIENT(NO_PERMISSIONS_ERROR);
            return;
        }
        QStringList idsList = ReplyMap["room"].toString().split("/");
        if(idsList.size()<1)
        {
            SEND_CLIENT(BAD_REQUEST_REPLY);
        }
        RecursionArray result;
                for(int i=0;i<idsList.size();i++)
                {
                    bool isActive=false;
                    for(QLinkedList<validClient*>::iterator j=ClientsList.begin();j!=ClientsList.end();j++)
                    {
                        MainClient* s = (MainClient*)(*j);
                        if(s->id==idsList.value(i).toInt()){isActive=true;
                        }
                    }
                    if(isActive)
                    {
                        RecursionArray result2;
                        result2["userOnlineStatus"]=403;
                        result2["key"]=403;
                        result2["userId"]=idsList.value(i);
                        result[QString::number(i)]=result2;
                    }
                    else
                    {
                        RecursionArray result2;
                        result2["key"]=402;
                        result2["userId"]=idsList.value(i);
                        result[QString::number(i)]=result2;
                    }
                }
        RecursionArray super;
        super["key"]=403;
        super["arg"]=result;
        qDebug() << super.print();
        SEND_CLIENT(super.toHTMLTegsFormat());
    }
    else if(cmd=="srcget")
    {
        if(SRCMode == 0 || SRCMode == 1 || SRCMode == 2) return;
        if(ReplyMap["cmd"]=="ping")
            qDebug() << hdata;
    }
    else if(cmd=="su")
    {
        if(SUPERUSER_AVALIBLE){
        nClient->isAuth=true;
        nClient->name=SUPERUSER_LOGIN;
        nClient->pass=SUPERUSER_PASS;
        QString str = SUPERUSER_PERMISSIONS;
        nClient->permissions=str.split(", ");
        nClient->socket=nClient->socket;
        nClient->isUseCommand = false;
        nClient->state = AuthState;
        nClient->numUsingCommands = nClient->numUsingCommands;
        nClient->id=SUPERUSER_ID;
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
            nClient->isAuth=true;
            nClient->name=ReplyMap["login"].toString();
            nClient->pass=ReplyMap["pass"].toString();
            QString str = sqlquery.value("group").toString();
            nClient->permissions=str.split(", ");
            nClient->socket=nClient->socket;
            nClient->isUseCommand = false;
            nClient->state = AuthState;
            nClient->numUsingCommands = nClient->numUsingCommands;
            nClient->id=sqlquery.value("id").toInt();
            nClient->init=sqlquery.value("init").toString();
            nClient->initV=sqlquery.value("initV").toString();
            nClient->status=sqlquery.value("status").toString();
            nClient->real_name=sqlquery.value("nickname").toString();
            nClient->prefix=sqlquery.value("prefix").toString();
            nClient->email=sqlquery.value("email").toString();
            nClient->TimeZone=sqlquery.value("TimeZone").toString();
            nClient->colored=sqlquery.value("colored").toString();
            nClient->RegIP=sqlquery.value("IP_REG").toString();
            nClient->achived=sqlquery.value("HAID").toInt();
            logs << nClient->name +" auth";
            SEND_CLIENT(YES_REPLY);
        }
    }
}

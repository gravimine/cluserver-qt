#include "mainserver.h"
#include <QCoreApplication>

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
    otchet["GetCmds"]=QString::number( GetedCmds );
    otchet["sendCmds"]=QString::number( sendedCmds );
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
    GetedBytes = 0LL;
    sendedCmds = 0LL;
    GetedCmds = 0LL;
    connect(timer, &QTimer::timeout, this, &MainServer::MonitorTimer);
    connect(this, &MainServer::signalClientConnected, this, &MainServer::clientConn);
}
void MainServer::clientConn(validClient* user)
{
    qDebug() << "Client connect "+user->socket->peerAddress().toString()+":"+QString::number(user->socket->peerPort());
    sendToClient(user, ClientInConnectText.toUtf8());
}

MainServer::~MainServer()
{
    delete timer;
}

void MainServer::UseCommand(QByteArray hdata, validClient* lClient, ServerThread* thisThread)
{
    MainClient* nClient = (MainClient*) lClient;
    //data = data.replace("\r","");
    //data = data.replace("\n","");
    RecursionArray ReplyMap;

    ReplyMap.fromPost(QString::fromUtf8( hdata ));
    //if(isDebug) qDebug() << ReplyMap.print();
    QString cmd=ReplyMap["type"].toString();
    GetedCmds++;
    GetedBytes+=hdata.size();
    if(cmd=="deleteRoom")
    {
        QSqlQuery sql,sql2;
        if(!sql2.exec("SELECT * FROM rooms WHERE idUserCreat = "+QString::number(nClient->id)+" && `id` = "+ReplyMap["nameTextRoom"].toString().toInt()))
        {
            logs << "[deleteRoom]Query stopped: "+sql2.lastError().text();
           SEND_CLIENT( SQL_ERROR );
        }
        else{
            if(sql2.size()<1)
            {
                SEND_CLIENT(NO_PERMISSIONS_ERROR);
                return;
            }
        }
        if(!sql.exec("DELETE FROM rooms WHERE `id` = "+ReplyMap["nameTextRoom"].toString().toInt()))
        {
            logs << "[addRoom]Query stopped: "+sql.lastError().text();
           SEND_CLIENT( SQL_ERROR );
        }
        else
        {
            SEND_CLIENT( YES_REPLY );
        }
    }
    else if(cmd=="addRoom")
    {
        if(!nClient->isAuth)
        {
            SEND_CLIENT(NO_PERMISSIONS_ERROR);
            return;
        }
        QString RoomName = ReplyMap["nameTextRoom"].toString().replace("'","\\'").replace("\\","\\\\");
        QSqlQuery sql,sql2;
        if(!sql.exec("INSERT INTO rooms (`nameTextRoom`,  `idUserCreat`) VALUES ( '"+RoomName+"', '"+QString::number(nClient->id)+"')"))
        {
            logs << "[addRoom]Query stopped: "+sql.lastError().text();
           SEND_CLIENT( SQL_ERROR );
        }
        else
        {
            sql.next();
            if(!sql2.exec("SELECT * from rooms WHERE nameTextRoom = "+RoomName)) {
                logs << "[auth]Query stopped: "+sql2.lastError().text();
                SEND_CLIENT(SQL_ERROR);
                return;
            }
            else
            {
                sql2.next();
                nClient->allowrooms << sql2.value("id").toInt();
            }
            SEND_CLIENT( YES_REPLY );
        }
    }
    else if(cmd=="close")
    {
        CloseClient(lClient);
    }
    else if(cmd=="rooms")
    {
        if(!nClient->isAuth){
        SEND_CLIENT(NO_PERMISSIONS_ERROR);
        return;}
            QSqlQuery sqlquery;
            sqlquery.prepare("SELECT * from rooms");
            if(!sqlquery.exec()) {logs << "[rooms]Query stopped: "+sqlquery.lastError().text();
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
                tmpmap["key"]=YES_REPLY_INT;
                SEND_CLIENT ( tmpmap.toHtml().toUtf8());
            }


    }
    else if(cmd=="version")
    {
        SEND_CLIENT(versionarr);
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
            QStringList clientlist = ReplyMap["id"].toString().split("/");
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
                    sqlquery.next();
                    RecursionArray result;
                    result["real_name"]=sqlquery.value("real_name").toString();
                    result["id"]=sqlquery.value("id").toInt();
                    result["init"]=sqlquery.value("init").toString();
                    result["initV"]=sqlquery.value("initV").toString();
                    result["timeZone"]=sqlquery.value("timeZone").toString();
                    result["colored"]=sqlquery.value("colored").toString();
                    result["prefix"]=sqlquery.value("prefix").toString();
                    result["status"]=sqlquery.value("status").toString();
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
        tmpmap["key"]=YES_REPLY_INT;
        SEND_CLIENT(tmpmap.toHtml().toUtf8());
    }
    else if(cmd=="sendmsg")
    {
        if(!nClient->isAuth)
        {
            SEND_CLIENT(NO_PERMISSIONS_ERROR);
            return;
        }
        QList<QString> keys = ReplyMap.keys();
        QList<QString> keys2;
        for(int i=0;i<keys.size();i++)
        {
            QStringList tmp = keys.value(i).split("_");
            if(tmp.size() != 3) continue;
            if(!nClient->allowrooms.contains(tmp.value(1).toInt())) continue;
            if(tmp.value(0).isEmpty() || tmp.value(1).isEmpty() || tmp.value(2).isEmpty() || ReplyMap[keys.value(i)].toString().isEmpty()) continue;
            keys2<< keys.value(i);
        }
        if(keys2.size() < 1)
        {
            SEND_CLIENT(BAD_REQUEST_REPLY);
            return;
        }
        QSqlQuery sqlquery;
        QString reply = " INSERT INTO private (`idR`, `type`, `textMessage`, `idOt`, `dateMessage`, `commandMessage`) VALUES ";
        for(int i=0;i<keys2.size();i++)
        {
            QStringList tmp = keys2.value(i).split("_");

            if(tmp.value(0)!="c")
                reply+=" ( '"+tmp.value(1)+"', 'pm', '"+ReplyMap[keys2.value(i)].toString().replace("'","\\'").replace("\\","\\\\")+"', '"+QString::number(nClient->id)+"', CURRENT_TIMESTAMP, '' )";
            else
                reply+=" ( '"+tmp.value(1)+"', 'pm', '', '"+QString::number(nClient->id)+"', CURRENT_TIMESTAMP, '"+ReplyMap[keys2.value(i)].toString().replace("'","\\'").replace("\\","\\\\")+"' )";
            if(i!=keys2.size()-1) reply+=",";
        }
        logs << reply;
        if (!sqlquery.exec(reply)) {
                logs << "[sendmsg]Query stopped: "+sqlquery.lastError().text();
                SEND_CLIENT ( SQL_ERROR );
            }
        else
        {
            SEND_CLIENT(YES_REPLY);
        }
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
        if(host.isEmpty()) {SEND_CLIENT(BAD_REQUEST_REPLY);}
        else
        {
            QTcpSocket sock;
            QStringList spl=host.split(":");
            QString splhost = host.remove(host.size()-spl.value(spl.size()-1).size()-1,spl.value(spl.size()-1).size()+1);
            sock.connectToHost(splhost,spl.value(spl.size()-1).toInt());
            sock.waitForConnected(200);

            QString cmdd=ReplyMap["cmd"].toString();
            if(cmdd=="echo")
            {
                sock.write(QString("type=srcget&cmd=echo&sender="+nClient->name+"@"+serverd->serverAddress().toString()+":"+QString::number( serverd->serverPort() )).toUtf8());
                sock.waitForBytesWritten(200);
                sock.disconnectFromHost();
            }
            else if(cmdd=="ping")
            {
                QTime time = QTime::currentTime();
                time.start();
                sock.waitForReadyRead(500);
                QString server_hello = sock.readAll();
                if(server_hello.isEmpty()) {SEND_CLIENT("Timeout");}
                else
                {
                    QString result = QString(YES_REPLY)+"<time>"+QString::number( time.elapsed() )+"</time><server>"+server_hello+"</server>";
                    SEND_CLIENT(result.toLocal8Bit());
                }
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
                        result2["userOnlineStatus"]=YES_REPLY_INT;
                        result2["key"]=YES_REPLY_INT;
                        result2["userId"]=idsList.value(i);
                        result[QString::number(i)]=result2;
                    }
                    else
                    {
                        RecursionArray result2;
                        result2["key"]=BAD_REQUEST_REPLY_INT;
                        result2["userId"]=idsList.value(i);
                        result[QString::number(i)]=result2;
                    }
                }
        RecursionArray super;
        super["key"]=YES_REPLY_INT;
        super["arg"]=result;
        SEND_CLIENT(super.toHtml().toUtf8());
    }
    else if(cmd=="srcget")
    {
        if(SRCMode == 0 || SRCMode == 1 || SRCMode == 2) return;
        if(ReplyMap["cmd"]=="echo")
            qDebug() << hdata;
        else if(ReplyMap["cmd"]=="ping")
        {
            SEND_CLIENT(YES_REPLY);
        }
    }
    else if(cmd=="msglist")
    {
        if(!nClient->isAuth)
        {
            SEND_CLIENT(NO_PERMISSIONS_ERROR);
            return;
        }
        QSqlQuery sqlquery;
        QStringList clientlist = ReplyMap["room"].toString().split("/");
        if(clientlist.size()<1)
        {
            SEND_CLIENT(BAD_REQUEST_REPLY);
            return;
        }
        QString reply;
        if(ReplyMap["new"].toString()!="true")
        {
        for(int i=0;i<clientlist.size();i++)
        {
            if(!clientlist.value(i).isEmpty()){reply+="( SELECT * FROM private WHERE idR = "+clientlist.value(i)+" ORDER BY id DESC  LIMIT 20 )";
            if(i!=clientlist.size()-1) reply+=" UNION ";}
            else
                clientlist.removeAt(i);
            nClient->msgmap[clientlist.value(i).toInt()] = 0;
        }
        reply+=" ORDER BY id ASC";
        }
        else
        {
            for(int i=0;i<clientlist.size();i++)
            {
                if(!clientlist.value(i).isEmpty()){reply+="( SELECT * FROM private WHERE idR = "+clientlist.value(i)+"  && id > "+QString::number( nClient->msgmap[clientlist.value(i).toInt()] )+" )";
                if(i!=clientlist.size()-1) reply+=" UNION ";}
                else
                    clientlist.removeAt(i);
            }
        }

            if(!sqlquery.exec(reply))
            {
                logs << "[msglist]Query stopped: "+sqlquery.lastError().text();
                SEND_CLIENT ( SQL_ERROR );
                return;
            }
            else
            {
                QMap<int,RecursionArray> replymap;
                for(int i=0;i<sqlquery.size();i++)
                {
                    sqlquery.next();
                    int idRoom = sqlquery.value("idR").toInt();
                    int idUser = sqlquery.value("idOt").toInt();
                    int idLS = sqlquery.value("id").toInt();
                    QString msgList = sqlquery.value("textMessage").toString();
                    QString date = sqlquery.value("dateMessage").toString();
                    QString cmdMessage = sqlquery.value("commandMessage").toString();
                    RecursionArray valuemap;
                    if(!msgList.isEmpty())valuemap["textMessage"] = msgList;
                    else valuemap["commandMessage"] = cmdMessage;
                    valuemap["dateMessage"] = date.replace("T"," ");
                    valuemap["idUser"] = idUser;
                    valuemap["id"] = idLS;
                    if(nClient->msgmap[idRoom] < idLS) nClient->msgmap[idRoom] = idLS;
                    replymap[idRoom][QString::number(replymap[idRoom].size())]=valuemap;
                }
                RecursionArray resultmap;
                QList<int> keys = replymap.keys();
                for(int i=0;i<keys.size();i++)
                {
                    replymap[keys.value(i)]["id"] = keys.value(i);

                    if(nClient->allowrooms.contains(keys.value(i))) {
                        replymap[keys.value(i)]["key"] = YES_REPLY_INT;
                        resultmap[QString::number(i)] = replymap[keys.value(i)];}
                    else
                        replymap[keys.value(i)]["key"] = BAD_REQUEST_REPLY_INT;
                }
                if(keys.size() < clientlist.size())
                {
                    for(int i=0;i<clientlist.size();i++)
                    {
                        if(replymap[clientlist.value(i).toInt()].size() == 0)
                        {
                            replymap[clientlist.value(i).toInt()]["id"] = clientlist.value(i).toInt();
                            replymap[clientlist.value(i).toInt()]["key"] = BAD_REQUEST_REPLY_INT;
                            resultmap[QString::number(i)] = replymap[clientlist.value(i).toInt()];
                        }
                    }
                }
                RecursionArray super;
                super["key"]=YES_REPLY_INT;
                super["arg"]=resultmap;
                qDebug() << super.print();
                SEND_CLIENT(super.toHtml().toUtf8());
            }
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
        nClient->state = AuthState;
        nClient->id=SUPERUSER_ID;
        logs << "Client "+nClient->socket->peerAddress().toString()+" logged in superuser!";
        SEND_CLIENT(YES_REPLY);}
        else
        {
            SEND_CLIENT(NO_PERMISSIONS_ERROR);
        }
    }
    else if(cmd=="admpanel")
    {
        if(!nClient->isAuth)
        {
            SEND_CLIENT(NO_PERMISSIONS_ERROR);
            return;
        }
        if(!IS_ADMIN)
        {
            SEND_CLIENT(NO_PERMISSIONS_ERROR);
            return;
        }
        QString str = ReplyMap["action"].toString();
        if(str=="reguser")
        {
            QString userlogin = ReplyMap["login"].toString();
            QString userpass = QCryptographicHash::hash((ReplyMap["pass"].toString().toUtf8()),QCryptographicHash::Md5).toHex();
            QString usergroup = ReplyMap["group"].toString();
            QString username = ReplyMap["name"].toString();
            QString useremail = ReplyMap["email"].toString();
            if(userlogin.isEmpty() || userpass.isEmpty() || usergroup.isEmpty() | username.isEmpty())
            {
                SEND_CLIENT(BAD_REQUEST_REPLY);
                return;
            }
            QSqlQuery sqlquery;
            QString sqlrequest = "INSERT INTO users (`name`,  `pass`, `group`, `real_name`, `init`, `TimeZone`, `email`) VALUES ( '"+userlogin+"', '"+userpass+"', '"+usergroup+"', '"+username+"', 'Console', 'Moscow', '"+useremail+"')";
            qDebug() << sqlrequest;
            if (!sqlquery.exec(sqlrequest)) {
                    logs << "[admpanel.reguser]Query stopped: "+sqlquery.lastError().text();
                    SEND_CLIENT(SQL_ERROR);
                    return;
            }
            else
            {
                SEND_CLIENT(YES_REPLY);
            }
        }
        else if(str=="stop") //ОПАСТНО!!
        {
           for(QLinkedList<validClient*>::iterator i=ClientsList.begin();i!=ClientsList.end();i++)
           {
               (*i)->socket->write(SERVER_STOP_REPLY);
               (*i)->socket->waitForBytesWritten(1000);
           }
           qApp->quit();
        }
        else if(str=="killclients")
        {
            for(QLinkedList<validClient*>::iterator i=ClientsList.begin();i!=ClientsList.end();i++)
            {
               (*i)->socket->write(THREAD_KILL_ERROR);
               (*i)->socket->waitForBytesWritten(1000); CloseClient((*i));
            }
        }
        else if(str=="savesettings")
        {
            settings.SaveSettings();
            SEND_CLIENT(YES_REPLY);
        }
        else if(str=="set")
        {
            settings[ReplyMap["name"].toString()]=ReplyMap["value"].toString();
            SEND_CLIENT(YES_REPLY);
        }
        else if(str=="reload")
        {
            settings.LoadSettings();
            ReloadConfig();
            SEND_CLIENT(YES_REPLY);
        }
        else if(str=="sleep")
        {
            ACore::Sleeper::msleep(ReplyMap["n"].toString().toInt());
            SEND_CLIENT(YES_REPLY);
        }
        else
        {
            SEND_CLIENT(BAD_REQUEST_REPLY);
            return;
        }
    }
    else if(cmd=="auth")
    {
        QSqlQuery sqlquery,sql2;

        if (!sqlquery.exec(QString("SELECT * FROM users WHERE ( name = '%1' || email = '%1' ) && pass = md5('%2');").arg(ReplyMap["login"].toString()).arg(ReplyMap["pass"].toString()))) {
                logs << "[auth]Query stopped: "+sqlquery.lastError().text();
                SEND_CLIENT(SQL_ERROR);
                return;
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
            nClient->state = AuthState;
            nClient->id=sqlquery.value("id").toInt();
            nClient->init=sqlquery.value("init").toString();
            nClient->initV=sqlquery.value("initV").toString();
            nClient->status=sqlquery.value("status").toString();
            nClient->real_name=sqlquery.value("real_name").toString();
            nClient->prefix=sqlquery.value("prefix").toString();
            nClient->email=sqlquery.value("email").toString();
            nClient->TimeZone=sqlquery.value("TimeZone").toString();
            nClient->colored=sqlquery.value("colored").toString();
            nClient->RegIP=sqlquery.value("IP_REG").toString();
            nClient->achived=sqlquery.value("HAID").toInt();
            logs << nClient->name +" auth";
            SEND_CLIENT(YES_REPLY);
            if(!sql2.exec("SELECT * from rooms")) {
                logs << "[auth]Query stopped: "+sqlquery.lastError().text();
                SEND_CLIENT(SQL_ERROR);
                return;
            }
            else
            {
                for(int i=0;i<sql2.size();i++)
                {
                    sql2.next();
                    if(sql2.value("idsUsers").toString().split(", ").contains(QString::number(nClient->id)) || sql2.value("idUserCreat").toInt() == nClient->id)
                        nClient->allowrooms << sql2.value("id").toInt();
                }
            }
        }
    }
}

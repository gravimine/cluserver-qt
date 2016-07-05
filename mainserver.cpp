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
    qDebug() << otchet.toArcan();
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
void MainClient::clear()
{
    isAuth=false;
    name.clear();
    pass.clear();
    permissions.clear();
    state = NoAuthState;
    id=0;
    init.clear();
    banned=0;
    Hidden=0;
    initV.clear();
    status.clear();
    real_name.clear();
    prefix.clear();
    email.clear();
    TimeZone.clear();
    colored.clear();
    RegIP.clear();
}

void AdminLog(MainClient* lClient,QString action,QString info)
{
    QSqlQuery sql2;
    if(!sql2.exec("INSERT INTO admlogs ( `login`, `action`, `info` ) VALUES ( '"+lClient->name+"', '"+action+"', '"+info+"', )"))
    {
        logs << "[AdmLog]Query stopped: "+sql2.lastError().text();
    }
}
bool MainServer::addCommand(MainCommand* cmd)
{
    if(commands.contains(cmd->cmd_name)) return false;
    else
    {
        commands[cmd->cmd_name]=cmd;
        if(cmd->cmd_isActive)
            logs << "Комманда "+cmd->cmd_name+" активна";
        else
            logs << "Комманда "+cmd->cmd_name+" неактивна";
        return true;
    }

}

bool MainServer::removeCommand(QString name)
{
    for(auto i=commands.begin();i!=commands.end();++i)
    {
        if(i.key()==name)
        {
            delete i.value();
            commands.erase(i);
            return true;
        }
    }
    return false;
}
bool MainServer::disableCommand(QString name)
{
    for(auto i=commands.begin();i!=commands.end();++i)
    {
        if(i.key()==name)
        {
            i.value()->cmd_isActive=false;
            return true;
        }
    }
    return false;
}
bool MainServer::enableCommand(QString name)
{
    for(auto i=commands.begin();i!=commands.end();++i)
    {
        if(i.key()==name)
        {
            i.value()->cmd_isActive=true;
            return true;
        }
    }
    return false;
}
void MainServer::UseCommand(QByteArray hdata, validClient* lClient, ServerThread* thisThread)
{
    MainClient* nClient = (MainClient*) lClient;
    //data = data.replace("\r","");
    //data = data.replace("\n","");
    RecursionArray ReplyMap;
    ReplyMap.fromArcan(hdata);
    //if(isDebug) qDebug() << ReplyMap.print();
    QString cmd=ReplyMap["type"].toString();
    GetedCmds++;
    GetedBytes+=hdata.size();
    if(!commands.isEmpty())
    {
        MainCommand* maincmd=commands.value(cmd,nullptr);
        if(maincmd!=nullptr)
        {
            if(maincmd->cmd_isActive==true)
            {
                if(maincmd->cmd_authlevel==0)
                    maincmd->exec(&ReplyMap,nClient,thisThread);
                else if(maincmd->cmd_authlevel>=1)
                {
                    if(!IS_AUTH)
                    {
                        SEND_CLIENT(NO_PERMISSIONS_ERROR);
                        return;
                    }
                }
                else if(maincmd->cmd_authlevel==2)
                {
                    if(!IS_MODERATOR)
                    {
                        SEND_CLIENT(NO_PERMISSIONS_ERROR);
                        return;
                    }
                }
                else if(maincmd->cmd_authlevel==3)
                {
                    if(!IS_ADMIN)
                    {
                        SEND_CLIENT(NO_PERMISSIONS_ERROR);
                        return;
                    }
                }
                else if(maincmd->cmd_authlevel==4)
                {
                    if(!IS_SUPERUSER)
                    {
                        SEND_CLIENT(NO_PERMISSIONS_ERROR);
                        return;
                    }
                }
                else if(maincmd->cmd_authlevel==5)
                {
                    if(!(nClient->permissions.contains(maincmd->cmd_requestgroup)))
                    {
                        SEND_CLIENT(NO_PERMISSIONS_ERROR);
                        return;
                    }
                }
                maincmd->exec(&ReplyMap,nClient,thisThread);
            }
        }
    }
    else if(cmd=="deleteRoom")
    {
        QSqlQuery sql,sql2;
        sql2.prepare("SELECT * FROM rooms WHERE idUserCreat = ? && `id` = ?");
        sql2.addBindValue(nClient->id);
        sql2.addBindValue(ReplyMap["nameTextRoom"].toString());
        if(!sql2.exec())
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
        sql.prepare("DELETE FROM rooms WHERE `id` = ");
        sql.addBindValue(ReplyMap["nameTextRoom"].toString());
        if(!sql.exec())
        {
            logs << "[deleteRoom]Query stopped: "+sql.lastError().text();
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
        QString RoomName = ReplyMap["nameTextRoom"].toString();
        QSqlQuery sql;
        sql.prepare("INSERT INTO rooms (`nameTextRoom`,  `idUserCreat`) VALUES ( ?, ?)");
        sql.addBindValue(RoomName);
        sql.addBindValue(nClient->id);

        if(!sql.exec())
        {
            logs << "[addRoom]Query stopped: "+sql.lastError().text();
           SEND_CLIENT( SQL_ERROR );
        }
        else
        {
            nClient->allowrooms << sql.lastInsertId().toInt();
            SEND_CLIENT( YES_REPLY );
        }
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
                SEND_CLIENT ( tmpmap.toArcan().toUtf8());
            }


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
        SEND_CLIENT(tmpmap.toArcan().toUtf8());
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
                reply+=" ( ?, 'pm', ?, ?, CURRENT_TIMESTAMP, '' )";
            else
                reply+=" ( ?, 'pm', '', ?, CURRENT_TIMESTAMP, ? )";
            if(i!=keys2.size()-1) reply+=",";
        }
        sqlquery.prepare(reply);
        for(int i=0;i<keys2.size();i++)
        {
            QStringList tmp = keys2.value(i).split("_");

            if(tmp.value(0)!="c")
            {
                sqlquery.addBindValue(tmp.value(1));
                sqlquery.addBindValue(ReplyMap[keys2.value(i)].toString());
                sqlquery.addBindValue(nClient->id);
            }
            else
            {
                sqlquery.addBindValue(tmp.value(1));
                sqlquery.addBindValue(nClient->id);
                sqlquery.addBindValue(ReplyMap[keys2.value(i)].toString());
            }
        }
        logs << reply;
        if (!sqlquery.exec()) {
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
                        if(s->id==idsList.value(i).toInt()){if(s->Hidden == false)isActive=true;
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
        SEND_CLIENT(super.toArcan().toUtf8());
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
                SEND_CLIENT(super.toArcan().toUtf8());
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
        nClient->banned=0;
        nClient->Hidden=true;
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
            QString username = ReplyMap["name"].toString();
            QString useremail = ReplyMap["email"].toString();
            if(userlogin.isEmpty() || userpass.isEmpty() || username.isEmpty() || useremail.isEmpty())
            {
                SEND_CLIENT(BAD_REQUEST_REPLY);
                return;
            }
            QSqlQuery sqlquery;
            QString sqlrequest = "INSERT INTO users (`name`,  `pass`, `group`, `real_name`, `init`, `TimeZone`, `email`) VALUES ( ?, ?, 'acc', ?, 'Console', 'Moscow', ?)";
            sqlquery.prepare(sqlrequest);
            sqlquery.addBindValue(userlogin);
            sqlquery.addBindValue(userpass);
            sqlquery.addBindValue(username);
            sqlquery.addBindValue(useremail);
            if (!sqlquery.exec()) {
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
            if(!IS_SUPERUSER)
            {
                SEND_CLIENT(NO_PERMISSIONS_ERROR);
                return;
            }
           for(QLinkedList<validClient*>::iterator i=ClientsList.begin();i!=ClientsList.end();i++)
           {
               (*i)->socket->write(SERVER_STOP_REPLY);
               (*i)->socket->waitForBytesWritten(1000);
           }
           qApp->quit();
        }
        else if(str=="serverinfo") //ОПАСТНО!!
        {
            if(!IS_SUPERUSER)
            {
                SEND_CLIENT(NO_PERMISSIONS_ERROR);
                return;
            }
            RecursionArray otchet;
            otchet["key"]=YES_REPLY_INT;
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
            SEND_CLIENT(otchet.toArcan().toUtf8());
        }
        else if(str=="setpermissions") //ОПАСТНО!!
        {
            if(!IS_SUPERUSER)
            {
                SEND_CLIENT(NO_PERMISSIONS_ERROR);
                return;
            }
            QString userlogin = ReplyMap["login"].toString();
            QString usergroup = ReplyMap["group"].toString();
            if(userlogin.isEmpty() || usergroup.isEmpty())
            {
                SEND_CLIENT(BAD_REQUEST_REPLY);
                return;
            }
            QSqlQuery sqlquery;
            sqlquery.prepare("UPDATE `users` SET `group` = ? WHERE `name` = ?");
            sqlquery.addBindValue(usergroup);
            sqlquery.addBindValue(userlogin);
            if(!sqlquery.exec())
            {
                logs << "[admpanel.setpermissions]Query stopped: "+sqlquery.lastError().text();
                SEND_CLIENT(SQL_ERROR);
                return;
            }
            else
            {
                SEND_CLIENT(YES_REPLY);
            }
        }
        else if(str=="killclients")
        {
            for(QLinkedList<validClient*>::iterator i=ClientsList.begin();i!=ClientsList.end();i++)
            {
               (*i)->socket->write(KICK_REPLY);
               (*i)->socket->waitForBytesWritten(1000); CloseClient((*i));
            }
        }
        else if(str=="clients")
        {
            RecursionArray returner;
            int index=0;
            for(QLinkedList<validClient*>::iterator i=ClientsList.begin();i!=ClientsList.end();++i)
            {
               MainClient* client = (MainClient*)(*i);
               if(client->isAuth)
               {
                   QString perm;
                   for(int j=0;j<client->permissions.size();j++)
                   {
                       perm+=client->permissions.value(j)+", ";
                   }
                   RecursionArray arr{
                       {"ip",client->socket->peerAddress().toString()},
                       {"port",(int)client->socket->peerPort()},
                       {"login",client->name},
                       {"group",perm}
                   };
                   returner[QString::number(index)]=arr;
                   index++;
               }
               else
               {
                   RecursionArray arr{
                       {"ip",client->socket->peerAddress().toString()},
                       {"port",(int)client->socket->peerPort()}
                   };
                   returner[QString::number(index)]=arr;
                   index++;
               }
            }
            SEND_CLIENT(returner.toArcan().toUtf8());
        }
        else if(str=="savesettings")
        {
            settings.SaveSettings();
            SEND_CLIENT(YES_REPLY);
        }
        else if(cmd=="deleteRoom")
        {
            QSqlQuery sql;
            QString idroom=ReplyMap["id"].toString();
            if(idroom.isEmpty())
            {
                SEND_CLIENT(BAD_REQUEST_REPLY);
                return;
            }
            sql.prepare("DELETE FROM rooms WHERE `id` = ?");
            sql.addBindValue(idroom.toInt());
            if(!sql.exec())
            {
                logs << "[admpanel.deleteRoom]Query stopped: "+sql.lastError().text();
               SEND_CLIENT( SQL_ERROR );
            }
            else
            {
                SEND_CLIENT( YES_REPLY );
            }
        }
        else if(str=="set")
        {
            if(!IS_SUPERUSER)
            {
                SEND_CLIENT(NO_PERMISSIONS_ERROR);
                return;
            }
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
    else if(cmd=="modpanel")
    {
        if(!nClient->isAuth)
        {
            SEND_CLIENT(NO_PERMISSIONS_ERROR);
            return;
        }
        if(!IS_MODERATOR && !IS_ADMIN)
        {
            SEND_CLIENT(NO_PERMISSIONS_ERROR);
            return;
        }
        QString str = ReplyMap["action"].toString();
        if(str=="ban")
        {
            QString userid = ReplyMap["login"].toString();
            QString reason = ReplyMap["reason"].toString();
            QString hours = ReplyMap["time"].toString();
            QString bantype;
            if(userid.isEmpty() || reason.isEmpty())
            {
                SEND_CLIENT(BAD_REQUEST_REPLY);
                return;
            }
            QSqlQuery sql1,sql2;
            if(hours.isEmpty())
            {
                sql1.prepare("INSERT INTO banlist ( `userid`, `adminid`, `reason`, `permanet` ) VALUES ( ?, ?,?, '1' )");
                sql1.addBindValue(userid);
                sql1.addBindValue(QString::number(nClient->id));
                sql1.addBindValue(reason);
                bantype="permanet";
            }
            else
            {
                QDateTime dat = QDateTime::currentDateTime();
                dat=dat.addSecs((int)(hours.toDouble()*(double)(60*60)));
                sql1.prepare("INSERT INTO banlist ( `userid`, `adminid`, `reason`, `permanet`,`unbandata` ) VALUES ( ?, ?, ?, '0', CURRENT_TIMESTAMP + INTERVAL "+QString::number(hours.toInt())+" HOUR )");
                sql1.addBindValue(userid);
                sql1.addBindValue(QString::number(nClient->id));
                sql1.addBindValue(reason);
                bantype=dat.toString("yyyy-MM-ddThh:mm:ss");
            }
            if (!sql1.exec()) {
                    logs << "[moderpanel.ban]Query stopped: "+sql1.lastError().text();
                    SEND_CLIENT(SQL_ERROR);
                    return;
            }
            else
            {
                if (!sql2.exec("UPDATE users SET `baned` = "+QString::number(sql1.lastInsertId().toInt())+" WHERE `name` = '"+userid+"'")) {
                        logs << "[moderpanel.ban ]Query stopped: "+sql2.lastError().text();
                        SEND_CLIENT(SQL_ERROR);
                        return;
                }
                else
                {
                    SEND_CLIENT(YES_REPLY);
                    for(auto i=ClientsList.begin();i!=ClientsList.end();++i)
                    {
                        if(((MainClient*)(*i))->name == userid)
                        {
                            ACore::RecursionArray RecArr{
                                {"key",BANNED_REPLY_INT},
                                {"reason",reason},
                                {"unban",bantype}
                            };
                            SEND_CLIENT_R((*i),RecArr.toArcan().toLocal8Bit());
                            ((MainClient*)(*i))->clear();
                        }
                    }
                }
            }
        }
        else if(str=="unban")
        {
            QString userid = ReplyMap["login"].toString();
            if(userid.isEmpty())
            {
                SEND_CLIENT(BAD_REQUEST_REPLY);
                return;
            }
            QSqlQuery sql2;
            sql2.prepare("UPDATE users SET `baned` = 0 WHERE `name` = ?");
            sql2.addBindValue(userid);
            if (!sql2.exec()) {
                        logs << "[moderpanel.unban]Query stopped: "+sql2.lastError().text();
                        SEND_CLIENT(SQL_ERROR);
                        return;
                }
                else
                {
                    SEND_CLIENT(YES_REPLY);
                }
        }
        else if(str=="kickip")
        {
            QString ip = ReplyMap["ip"].toString();

            for(auto i=ClientsList.begin();i!=ClientsList.end();++i)
            {
                if((*i)->socket->peerAddress().toString() == ip)
                {
                    CloseClient((*i));
                    SEND_CLIENT(YES_REPLY);
                }
            }
        }
    }
    else
    {
        SEND_CLIENT(BAD_REQUEST_REPLY);
    }
}

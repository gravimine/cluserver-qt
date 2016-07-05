#include "cmd_auth.h"
#include "cmdheader.h"
cmd_auth::cmd_auth()
{
    cmd_isActive=true;
    cmd_name="auth";
}

bool cmd_auth::exec(ACore::RecursionArray* arr,MainClient* nClient,ServerThread *thisThread)
{
    ACore::RecursionArray ReplyMap = *arr;
    QSqlQuery sqlquery,sql2;
    sqlquery.prepare(QString("SELECT * FROM users WHERE ( name = ? || email = ? ) && pass = md5(?);"));
    QString userlogin = ReplyMap["login"].toString();
    QString userpass = ReplyMap["pass"].toString();
    sqlquery.addBindValue(userlogin);
    sqlquery.addBindValue(userlogin);
    sqlquery.addBindValue(userpass);
    qDebug() << sqlquery.executedQuery();
    if (!sqlquery.exec()) {
            logs << "[auth]Query stopped: "+sqlquery.lastError().text();
            SEND_CLIENT(SQL_ERROR);
            return false;
        }
    else
    {

        if(!sqlquery.next())
        {
            SEND_CLIENT(AUTH_ERROR);
            return false;
        }
        nClient->isAuth=true;
        nClient->name=userlogin;
        nClient->pass=userpass;
        QString str = sqlquery.value("group").toString();
        nClient->permissions=str.split(", ");
        nClient->state = AuthState;
        nClient->id=sqlquery.value("id").toInt();
        nClient->init=sqlquery.value("init").toString();
        nClient->banned=sqlquery.value("baned").toInt();
        nClient->Hidden=sqlquery.value("hidden").toBool();
        nClient->initV=sqlquery.value("initV").toString();
        nClient->status=sqlquery.value("status").toString();
        nClient->real_name=sqlquery.value("real_name").toString();
        nClient->prefix=sqlquery.value("prefix").toString();
        nClient->email=sqlquery.value("email").toString();
        nClient->TimeZone=sqlquery.value("TimeZone").toString();
        nClient->colored=sqlquery.value("colored").toString();
        nClient->RegIP=sqlquery.value("IP_REG").toString();
        if(nClient->banned>0)
        {
            sql2.prepare("SELECT * from banlist WHERE `id` = ?");
            sql2.addBindValue(nClient->banned);
            if(!sql2.exec()) {
                logs << "[auth]Query stopped: "+sqlquery.lastError().text();
                SEND_CLIENT(SQL_ERROR);
                return false;
            }
            else
            {
                sql2.next();
                QString reason = sql2.value("reason").toString();
                QString bantype;
                if(sql2.value("permanet").toBool()==true) {
                    bantype="permanet";
                    ACore::RecursionArray RecArr{
                        {"key",BANNED_REPLY_INT},
                        {"reason",reason},
                        {"unban",bantype}
                    };
                    SEND_CLIENT(RecArr.toArcan().toLocal8Bit());
                    logs << nClient->name +" auth reply. He/She banned. reason: "+reason;
                    return true;
                }
                else
                {
                    bantype=sql2.value("unbandata").toString();
                    if(QDateTime::currentDateTime() > QDateTime::fromString(bantype,"yyyy-MM-ddThh:mm:ss"))
                    {
                        if(!sql2.exec("UPDATE `users` SET `baned` = '0' WHERE `id` = "+QString::number(nClient->id))) {
                            logs << "[auth]Query stopped: "+sqlquery.lastError().text();
                            SEND_CLIENT(SQL_ERROR);
                            return false;
                        }
                        else
                        {
                            logs << nClient->name +" auto unbaned";
                        }
                    }
                    else
                    {
                        ACore::RecursionArray RecArr{
                            {"key",BANNED_REPLY_INT},
                            {"reason",reason},
                            {"unban",bantype}
                        };
                        SEND_CLIENT(RecArr.toArcan().toLocal8Bit());
                        logs << nClient->name +" auth reply. He/She banned. reason: "+reason;
                        return true;
                    }
                }

            }
        }
        logs << nClient->name +" auth";
        SEND_CLIENT(YES_REPLY);
        sql2.prepare("SELECT * from rooms");
        if(!sql2.exec()) {
            logs << "[auth]Query stopped: "+sqlquery.lastError().text();
            SEND_CLIENT(SQL_ERROR);
            return false;
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
    return true;
}

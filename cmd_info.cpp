#include "cmd_info.h"
#include "cmdheader.h"
using namespace ACore;
cmd_info::cmd_info()
{
    cmd_isActive=true;
    cmd_name="info";
    cmd_authlevel=1;
}
bool cmd_info::exec(ACore::RecursionArray* arr,MainClient* nClient,ServerThread *thisThread)
{
    RecursionArray result2;
    ACore::RecursionArray ReplyMap = *arr;
    if(ReplyMap["id"].toString()=="?"){
        RecursionArray result;
        result["isAuth"]=nClient->isAuth;
        result["real_name"]=nClient->real_name;
        result["id"]=nClient->id;
        result["RegIP"]=nClient->RegIP;
        result["init"]=nClient->init;
        thisThread->msleep(111);
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
            return false;
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
                return false;
            }
        else
        {
            if(sqlquery.size()<1)
            {
                SEND_CLIENT(BAD_REQUEST_REPLY);
                return false;
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
    return true;
}

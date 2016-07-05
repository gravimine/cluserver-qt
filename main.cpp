#include <QCoreApplication>
#include <mainserver.h>
#include "aipfunc.h"
#include "atests.h"
#include "openssl/evp.h"
#include "openssl/md5.h"
#include <string>
#include <map>
#include <iostream>
#include "commandlauncher.h"
//#include "apermissions.h"
AClusterKeys clusterkeys;
ACore::ALog logs;
bool isDebug,isHttpMode;
QSqlDatabase db;
int MinThreadd;
int MaxThreadd;
int SRCMode;
int MaxCommandsInQuest;
ACore::ASettings settings("settings.cfg",ACore::CfgFormat);
MainServer* serverd;
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    bool sn=false;
    ACore::GetBytes(&sn,1);
    //printBin<QString>("\n");
    //return 0;
    serverd = new MainServer();
    settings["Debug"]=false;
    settings["HttpMode"]=true;
    settings["MinThread"]=8;
    settings["MaxThread"]=10;
    settings["AllowSRC"]=3;
    logs.setfile("serverlog.log");
    //0 - Запрещен
    //1 - Только администраторам(отправка)
    //2 - только отправка(всем)
    //3 - Прием, и отправка только администраторам
    //4 - Прием и отправка
    //5 - Только прием
    ACore::RecursionArray tester;
    tester.fromArcan("test@1[1]beta[tester]integer@I[12]каралоооЛо[ss]");
    qDebug() << tester.print();
    //return 0;
    settings.setAutoSave(true);
    settings["Port"]=6592;
    settings["Host"]=QString("192.168.1.43");
    settings["ServerName"]=QString("localhostServer");
    settings["ServerInfo"]=QString("Еще один сервер.");
    settings["MySqlHost"]=QString("localhost");
    settings["MySqlPort"]=3306;
    settings["MySqlUser"]=QString("Qt5");
    settings["MySqlDriver"]=QString("QMYSQL");
    settings["MySqlDatabase"]=QString("s1user");
    settings["MySqlPass"]=QString("passwd");
    settings["MaxCommandsInQuest"]=12;
    settings.LoadSettings();
    qDebug() << settings.print();
    ReloadConfig();
    logs.setcoutdebug(true);
    serverd->MinThread = MinThreadd;
    serverd->MaxThread = MaxThreadd;
    isHttpMode = settings["HttpMode"].toBool();
    //Парс стандартного сообщения при подключении клиента
    ACore::RecursionArray arr{
    {"key",YES_REPLY_INT},
    {"serverVersion",SERVER_VERSION},
    {"aslib",ACORE_VERSION},
    {"qt",QT_VERSION_STR},
    {"serverName",settings["ServerName"].toString()},
    {"DBDriver",settings["MySqlDriver"].toString()},
    {"info",settings["ServerInfo"].toString()}}
    ;
    serverd->versionarr = arr.toArcan().toUtf8();
    qDebug() << "Инициализация комманд...";
    CommandLaunch();
    qDebug() << "Комманды инициализированны. Всего: "+QString::number(serverd->commands.size())+" комманд";
    serverd->ClientInConnectText = ACore::RecursionArray{{"status","OK"}}.toArcan();
    QString thishost = settings["Host"].toString();
    int thisport = settings["Port"].toInt();
    if(a.arguments().size()>2)
    {
        thishost = a.arguments().value(1);
        thisport = a.arguments().value(2).toInt();
    }
    if(thisport<= 0 || thisport > 65535){
        logs << "Fatal: port "+QString::number(thisport)+" invalid!";
        delete serverd;
        return 0;
    }
    //END
    if(serverd->launch( QHostAddress(thishost )  ,thisport))
    {
        qDebug() << "Host: " + thishost + " Port: " + QString::number(thisport);
        serverd->timer->start(1000);
        db=QSqlDatabase::addDatabase(settings["MySqlDriver"].toString());
        db.setDatabaseName(settings["MySqlDatabase"].toString());
        db.setHostName(settings["MySqlHost"].toString());
        db.setPassword(settings["MySqlPass"].toString());
        db.setUserName(settings["MySqlUser"].toString());
        db.setPort(settings["MySqlPort"].toInt());
        if(!db.open())
            logs << "Mysql connect .. failed. Error: "+db.lastError().text();
        else
            logs << "Mysql connected .. done";
        a.exec();
        logs.savelog();
        logs << "Close Database";
        db.close();
        logs << "Exit";
        logs.savelog();
    }
    return 0;
}

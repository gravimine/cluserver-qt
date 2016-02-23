#include <QCoreApplication>
#include <mainserver.h>
#include "aipfunc.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    serverd = new MainServer();
    settings["Debug"]=false;
    settings["MinThread"]=3;
    settings["MaxThread"]=7;
    settings["AllowSRC"]=3;
    //0 - Запрещен
    //1 - Только администраторам(отправка)
    //2 - только отправка(всем)
    //3 - Прием, и отправка только администраторам
    //4 - Прием и отправка
    //5 - Только прием
    settings["Port"]=6592;
    settings["Host"]="192.168.1.43";
    settings["ServerName"]="localhostServer";
    settings["ServerInfo"]="Еще один сервер.";
    settings["MySqlHost"]="localhost";
    settings["MySqlPort"]=3306;
    settings["MySqlUser"]="Qt5";
    settings["MySqlDriver"]="QMYSQL";
    settings["MySqlDatabase"]="s1user";
    settings["MySqlPass"]="passwd";
    settings["MaxCommandsInQuest"]=12;
    settings.LoadSettings();
    ReloadConfig();
    qDebug() << settings.print();
    logs.SetCoutDebug(true);
    serverd->MinThread = MinThreadd;
    serverd->MaxThread = MaxThreadd;
    //Парс стандартного сообщения при подключении клиента
    ACore::RecursionArray arr;
    arr["version"]=SERVER_VERSION;
    arr["engine"]=SERVER_ENGINE;
    arr["name"]=settings["ServerName"].toString();
    arr["info"]=settings["ServerInfo"].toString();
    serverd->ClientInConnectText = arr.toHTMLTegsFormat();
    QString thishost = settings["Host"].toString();
    int thisport = settings["Port"].toInt();
    if(a.arguments().size()>2)
    {
        thishost = a.arguments().value(1);
        thisport = a.arguments().value(2).toInt();
        if(thisport<0 || thisport > 65535) logs << "Warning: port unvalid!";
    }
    //END
    if(serverd->launch( QHostAddress(thishost )  ,thisport))
    {
        qDebug() << "Host: " + thishost + " Port: " + QString::number(thisport);
        if(isDebug) serverd->timer->start(1000);
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

    }
    delete serverd;
    settings.SaveSettings();
    return 0;
    /*QSqlDatabase test=QSqlDatabase::addDatabase("QMYSQL");
    test.setDatabaseName("test");
    test.setHostName("localhost");
    test.setPassword("passwd");
    test.setUserName("Qt");
    if(!test.open()) qDebug() << "Connect not open "+test.lastError().text();
    QSqlQuery sqlquery;
    if(!sqlquery.exec("SELECT * FROM `users` ")) qDebug() << "Query error: "+sqlquery.lastError().text();
    while (sqlquery.next()) qDebug() << sqlquery.value("name").toString();*/

}

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
    //END
    if(serverd->launch( QHostAddress(settings["Host"].toString() )  ,settings["Port"].toInt()))
    {
        qDebug() << "Host: " + settings["Host"].toString() + " Port: " + QString::number(settings["Port"].toInt());
        if(isDebug) serverd->timer->start(1000);
        db=QSqlDatabase::addDatabase("QMYSQL");
        db.setDatabaseName("s1user");
        db.setHostName("localhost");
        db.setPassword("passwd");
        db.setUserName("Qt5");
        if(!db.open())
            qDebug() << "Connect not open "+db.lastError().text();
        qDebug() << "Mysql connected";
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

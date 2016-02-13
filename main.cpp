#include <QCoreApplication>
#include <mainserver.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    serverd = new MainServer();
    settings["Debug"]=false;
    settings["MinThread"]=3;
    settings["MaxThread"]=7;
    settings["Port"]=6592;
    settings["Host"]="127.0.0.1";
    settings["MaxCommandsInQuest"]=12;
    settings.LoadSettings();
    ReloadConfig();
    qDebug() << settings.print();
    serverd->MinThread = MinThreadd;
    serverd->MaxThread = MaxThreadd;
    if(!serverd->launch(settings["Host"].toString(),settings["Port"].toInt())) return 0;
    else {
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

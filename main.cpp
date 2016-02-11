#include <QApplication>
#include <mainserver.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    settings["Debug"]=false;
    settings["MinThread"]=3;
    settings["MaxThread"]=7;
    settings.LoadSettings();
    ReloadConfig();
    if(!server.launch(12345)) return 0;
    else {
        db=QSqlDatabase::addDatabase("QMYSQL");
        db.setDatabaseName("s1user");
        db.setHostName("localhost");
        db.setPassword("passwd");
        db.setUserName("Qt5");
        if(!db.open()) qDebug() << "Connect not open "+db.lastError().text();
        logs << "Mysql connected";
        return a.exec();
    }
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

#ifndef CONFIG
#define CONFIG
//Администратор
#define SUPERUSER_LOGIN "root"
#define SUPERUSER_PASS "12345"
#define SUPERUSER_PERMISSIONS "ADM, acc, ban"
#define SUPERUSER_ID INT_MAX-1
#define SUPERUSER_AVALIBLE true
//Ответы сервера
#define NO_PERMISSIONS_ERROR "<key>402</key><error>NoPermissions</error>"
#define YES_REPLY "<key>403</key>"
#define THREAD_KILL_ERROR "<key>500</key>"
#define SERVER_STOP_REPLY "<key>505</key>"
#define BAD_REQUEST_REPLY "<key>402</key>"
#define SQL_ERROR "<key>501</key>"
#define AUTH_ERROR "<key>402</key><error>NoFoundLoginAndPass</error>"
//Не трогать
#define SEND_CLIENT(n) lClient->numUsingCommands++; thisThread->sendToClient(lClient, n)
#define IS_ADMIN (nClient->permissions.contains("ADM"))
#define IS_AUTH (nClient->isAuth)
//Версии
#define SERVER_VERSION "1.0.4"
#define SERVER_ENGINE "Qt5"
#endif // CONFIG


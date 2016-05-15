#ifndef CONFIG
#define CONFIG
//Администратор
#define SUPERUSER_LOGIN "root"
#define SUPERUSER_PASS "12345"
#define SUPERUSER_PERMISSIONS "ADM, acc, MODER, SUPERUSER"
#define SUPERUSER_ID INT_MAX-1
#define SUPERUSER_AVALIBLE nClient->socket->peerAddress().toString() == serverd->serverAddress().toString()
//Ответы сервера
#define NO_PERMISSIONS_ERROR "<key>402</key><error>NoPermissions</error>"
#define YES_REPLY "<key>403</key>"
#define BANNED_REPLY "<key>402</key><ban>You banned</ban>"
#define THREAD_KILL_ERROR "<key>500</key>"
#define SERVER_STOP_REPLY "<key>505</key>"
#define BAD_REQUEST_REPLY "<key>402</key><error>BadRequest</error>"
#define SQL_ERROR "<key>501</key>"
#define AUTH_ERROR "<key>402</key><error>NoFoundLoginAndPass</error>"


#define NO_PERMISSIONS_ERROR_INT 402
#define YES_REPLY_INT 403
#define BANNED_REPLY_INT 402
#define THREAD_KILL_ERROR_INT 500
#define SERVER_STOP_REPLY_INT 505
#define BAD_REQUEST_REPLY_INT 402
#define SQL_ERROR_INT 501
#define AUTH_ERROR_INT 402
//Не трогать
#define SEND_CLIENT(n) lClient->numUsingCommands++; sendedCmds++; thisThread->sendToClient(lClient, n)
#define IS_ADMIN (nClient->permissions.contains("ADM"))
#define IS_MODERATOR (nClient->permissions.contains("MODER"))
#define IS_SUPERUSER (nClient->permissions.contains("SUPERUSER"))
#define IS_AUTH (nClient->isAuth)
//Версии
#define SERVER_VERSION "1.0.7"
#define SERVER_ENGINE "Qt5"
#endif // CONFIG


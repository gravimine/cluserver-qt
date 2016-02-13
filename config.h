#ifndef CONFIG
#define CONFIG
//Администратор
#define SUPERUSER_LOGIN "root"
#define SUPERUSER_PASS "12345"
#define SUPERUSER_PERMISSIONS "ADM, acc, ban"
#define SUPERUSER_ID 1
#define SUPERUSER_AVALIBLE true
//Ответы сервера
#define NO_PERMISSIONS_ERROR "<key>403</key><error>NoPermissions</error>\n"
#define YES_REPLY "<key>200</key>\n"
#define THREAD_KILL_ERROR "<key>500</key>\n"
#define SERVER_STOP_REPLY "<key>505</key>\n"
#define BAD_REQUEST_REPLY "<key>402</key>\n"
#define HELLO_REPLY "<connect>200</connect>\n"
#define AUTH_ERROR "<key>403</key><error>NoFoundLoginAndPass</error>\n"

#endif // CONFIG


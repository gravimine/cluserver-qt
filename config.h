#ifndef CONFIG
#define CONFIG
#include "aclusterkeys.h"
//Администратор
#define SUPERUSER_LOGIN "root"
#define SUPERUSER_PASS "12345"
#define SUPERUSER_PERMISSIONS "ADM, acc, MODER, SUPERUSER"
#define SUPERUSER_ID INT_MAX-1
#define SUPERUSER_AVALIBLE nClient->socket->peerAddress().toString() == serverd->serverAddress().toString()
//Ответы сервера
extern AClusterKeys clusterkeys;
//NPE
#define NO_PERMISSIONS_ERROR clusterkeys.NPE
#define NO_PERMISSIONS_ERROR_INT clusterkeys.iNPE
 //YR
#define YES_REPLY clusterkeys.ROK
#define YES_REPLY_INT clusterkeys.iROK
 //BR
#define BANNED_REPLY clusterkeys.BANE
#define BANNED_REPLY_INT clusterkeys.iBANE
 //KR
#define KICK_REPLY clusterkeys.KR
#define KICK_REPLY_INT clusterkeys.iKR
 //SSR
#define SERVER_STOP_REPLY clusterkeys.ISE
#define SERVER_STOP_REPLY_INT clusterkeys.iISE
 //BRR
#define BAD_REQUEST_REPLY clusterkeys.RBAD
#define BAD_REQUEST_REPLY_INT clusterkeys.iRBAD
 //NFR
#define NOT_FOUND_REPLY clusterkeys.NFR
#define NOT_FOUND_REPLY_INT clusterkeys.iNFR
 //SQLE
#define SQL_ERROR clusterkeys.SQLE
#define SQL_ERROR_INT clusterkeys.iSQLE
 //AUE
#define AUTH_ERROR clusterkeys.BAR
#define AUTH_ERROR_INT clusterkeys.iBAR
//EVR
#define EVENT_REPLY clusterkeys.EVENTR
#define EVENT_REPLY_INT clusterkeys.iEVENTR







//Не трогать
#define SEND_CLIENT(n) sendedCmds++; thisThread->sendToClient(lClient, n)
#define SEND_CLIENT_R(zclient, n) zclient->numUsingCommands++; sendedCmds++; thisThread->sendToClient(zclient, n)
#define IS_ADMIN (nClient->permissions.contains("ADM"))
#define IS_MODERATOR (nClient->permissions.contains("MODER"))
#define IS_SUPERUSER (nClient->permissions.contains("SUPERUSER"))
#define IS_AUTH (nClient->isAuth)
//Версии
#define SERVER_VERSION "1.0.7"
#define SERVER_ENGINE "Qt5"
#endif // CONFIG


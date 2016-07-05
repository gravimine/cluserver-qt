#ifndef CMDHEADER
#define CMDHEADER
#include "config.h"
#undef SEND_CLIENT
#undef SEND_CLIENT_R
#define SEND_CLIENT(n) serverd->sendedCmds++; thisThread->sendToClient(nClient, n)
#define SEND_CLIENT_R(zclient, n) zclient->numUsingCommands++; serverd->sendedCmds++; thisThread->sendToClient(zclient, n)
#endif // CMDHEADER


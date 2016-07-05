#include "cmd_version.h"
#include "cmdheader.h"
cmd_version::cmd_version()
{
    cmd_isActive=true;
    cmd_name="version";
}

bool cmd_version::exec(ACore::RecursionArray* arr,MainClient* nClient,ServerThread *thisThread)
{
    SEND_CLIENT(serverd->versionarr);
    return true;
}

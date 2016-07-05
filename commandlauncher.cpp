#include "commandlauncher.h"
#include "mainserver.h"
#include "cmd_test.h"
#include "cmd_version.h"
#include "cmd_auth.h"
void CommandLaunch()
{
    cmd_test* testcmd = new cmd_test();
    cmd_version* versioncmd = new cmd_version();
    cmd_auth* authcmd = new cmd_auth();
    serverd->addCommand( testcmd );
    serverd->addCommand( versioncmd );
    serverd->addCommand( authcmd );
}

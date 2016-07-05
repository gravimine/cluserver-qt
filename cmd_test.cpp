#include "cmd_test.h"
#include "cmdheader.h"
cmd_test::cmd_test()
{
    cmd_isActive=true;
    cmd_name="test";
}

bool cmd_test::exec(ACore::RecursionArray* arr,MainClient* nClient,ServerThread *thisThread)
{
    SEND_CLIENT(YES_REPLY);
    return true;
}

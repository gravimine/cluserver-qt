#include "maincommand.h"
#include "mainserver.h"
MainCommand::MainCommand()
{
    cmd_isActive=false;
    cmd_authlevel=0;
}

bool MainCommand::exec(ACore::RecursionArray* arr,MainClient* nClient,ServerThread *thisThread)
{
    qDebug() << "Bad Command: "+arr->toArcan();
    thisThread->sendToClient(nClient, BAD_REQUEST_REPLY);
    return true;
}

MainCommand::~MainCommand()
{

}
bool MainCommand::operator ==(MainCommand h)
{
    if(h.cmd_name==cmd_name) return true;
    else return false;
}
bool MainCommand::operator ==(MainCommand* h)
{
    if(h->cmd_name==cmd_name) return true;
    else return false;
}

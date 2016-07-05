#ifndef CMD_AUTH_H
#define CMD_AUTH_H
#include "mainserver.h"

class cmd_auth : public MainCommand
{
public:
    cmd_auth();
    virtual bool exec(ACore::RecursionArray* arr,MainClient* nClient,ServerThread *thisThread);
};

#endif // CMD_AUTH_H

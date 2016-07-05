#ifndef CMD_VERSION_H
#define CMD_VERSION_H
#include "mainserver.h"

class cmd_version : public MainCommand
{
public:
    cmd_version();
    virtual bool exec(ACore::RecursionArray* arr,MainClient* nClient,ServerThread *thisThread);
};

#endif // CMD_VERSION_H

#ifndef CMD_INFO_H
#define CMD_INFO_H
#include "mainserver.h"

class cmd_info : public MainCommand
{
public:
    cmd_info();
    virtual bool exec(ACore::RecursionArray* arr,MainClient* nClient,ServerThread *thisThread);
};

#endif // CMD_INFO_H

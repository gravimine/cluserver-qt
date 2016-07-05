#ifndef CMD_TEST_H
#define CMD_TEST_H
#include "mainserver.h"

class cmd_test : public MainCommand
{
public:
    cmd_test();
    virtual bool exec(ACore::RecursionArray* arr,MainClient* nClient,ServerThread *thisThread);
};

#endif // CMD_TEST_H

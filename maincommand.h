#ifndef MAINCOMMAND_H
#define MAINCOMMAND_H
#include <QString>
#include "recursionarray.h"
class MainClient;
class ServerThread;
class MainCommand
{
public:
    bool cmd_isActive;
    QString cmd_name,cmd_requestgroup;
    int cmd_authlevel;
    //0 - без авторизации
    //1 - с авторизацией
    //2 - модераторам
    //3 - администраторам
    //4 - суперюзерам
    //5 - особые
    MainCommand();
    bool operator ==(MainCommand h);
    bool operator ==(MainCommand* h);
    virtual ~MainCommand();
    virtual bool exec(ACore::RecursionArray* arr,MainClient* nClient,ServerThread *thisThread);
};

#endif // MAINCOMMAND_H

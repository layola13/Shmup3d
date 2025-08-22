#include "world.h"
#include "netchannel.h"

#include "commands.h"

int NET_Init(void){return 0;}
void NET_Free(void){}
void NET_Setup(void){}
void NET_Receive(void){}
void NET_Send(void){}
char NET_IsRunning(void){return 0;}
void NET_OnNextLevelLoad(void){}
void Net_SendDie(command_t* command){}
uint NET_GetDropedPackets(void){return 0;}
char NET_IsInitialized(void) {return 0;}
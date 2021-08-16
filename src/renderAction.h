#ifndef RENDERACTION_H
#define RENDERACTION_H

#include "Audio.h"
#include <Arduino.h>

#define CODE_GetPositionInfo 0
#define CODE_Play 1
#define CODE_Pause 2
#define CODE_SetAVTransportURI 3
#define CODE_Seek 4
#define CODE_GetVolume 5
#define CODE_SetVolume 6

void playerInit();
void playerLoop();

char* playerGetPosition();
char* playerPlay();
char* playerPause();
char* playerSetAVTransportURI(String xml);
int playerSeek(String xml);
char* playerGetVolume();
char* playerSetVolume(String xml);

int parseActionHeader(String action);

#endif
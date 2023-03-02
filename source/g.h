#ifndef G_H
#define G_H

#define NUM_ROOMS 16

#include <exec/types.h>

#if !defined(X_CHAR)
typedef char CHAR;
#define X_CHAR
#endif

#define GAME_PATH(FN) "Work:murder/game/" FN



extern CHAR* gText;

BOOL LoadRooms();
VOID FreeRooms();

#endif

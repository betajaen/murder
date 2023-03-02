#include "x.h"
#include "g.h"
#include "when.inc"

typedef UWORD* Actions;

typedef struct Condition {
    UWORD kind;
    UWORD numArgs;
    UWORD args[8];
} Condition;

typedef struct When {
    Condition cond;
    UWORD     actionsLen;
    Actions   actions;
} When;

typedef struct Room {
    UWORD num;
    UWORD name;
    UWORD numWhens;
    When* whens;
} Room;

Room* gRooms[NUM_ROOMS] = { NULL };

static VOID FreeWhen(When* wn) {
    if (wn->actions != NULL) {
        xFreeVec(wn->actions);
    }
}

static VOID FreeRoom(Room* rm) {
    for(UWORD i=0;i < rm->numWhens;i++) {
        FreeWhen(&rm->whens[i]);
    }
    xFreeVec(rm->whens);
}

VOID FreeRooms() {
    for(UWORD i=0;i < NUM_ROOMS;i++) {
        if (gRooms[i] != NULL) {
            FreeRoom(gRooms[i]);
            gRooms[i] = NULL;
        }
    }
}

static BOOL LoadCondition(Condition* cn) {
    cn->kind = xReadUWord(1);
    cn->numArgs = xReadUWord(1);

    if (cn->numArgs >= 8) {
        return FALSE;
    }

    for(UWORD i=0;i < cn->numArgs;i++) {
        cn->args[i] = xReadUWord(i);
    }

    return TRUE;
}

static BOOL LoadWhen(When* wn) {
    if (LoadCondition(&wn->cond) == FALSE) {
        return FALSE;
    }

    wn->actionsLen = xReadUWord(1);
    wn->actions = NULL;

    if (wn->actionsLen == 0)
        return TRUE;

    wn->actions = xAllocVec(sizeof(UWORD) * wn->actionsLen);
    xReadMem(1, wn->actions, sizeof(UWORD) * wn->actionsLen);

    return TRUE;
}

static Room* LoadRoom() {
    Room* rm = xAllocVec(sizeof(Room));
    rm->num = xReadUWord(1);
    rm->name = xReadUWord(1);
    rm->numWhens = xReadUWord(1);
    rm->whens = xAllocVec(rm->numWhens * sizeof(When));
    for(UWORD i=0;i < rm->numWhens;i++) {
        if (LoadWhen(&rm->whens[i]) == FALSE) {
            xFreeVec(rm->whens);
            xFreeVec(rm);
            return NULL;
        }
    }
    return rm;
}

BOOL LoadRooms() {
    if (xReadFile(1, GAME_PATH("room")) == FALSE) {
        return FALSE;
    }


    UWORD numRooms = xReadUWord(1);

    if (numRooms >= NUM_ROOMS) {
        xPrintStr("Bad number of rooms\n");
        goto close_rooms;
    }

    for(UWORD i=0;i < numRooms;i++) {
        Room* rm = LoadRoom();

        if (rm == NULL) {
            goto close_rooms;
        }

        // TODO check to see if num is not empty!
        gRooms[rm->num] = rm;
    }

close_rooms:
    xCloseFile(1);
    return TRUE;
}

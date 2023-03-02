#include "compiler.h"
#include <stdio.h>
#include <string.h>
#include ACTION_INCLUDE_PATH
#include WHEN_INCLUDE_PATH

/*
NAME study

WHEN SAID(Hello This)
    cmd
    cmd
    cmd
END

WHEN ENTER()
    cmd
    cmd
    cmd
END

WHEN EXIT()
    cmd
    cmd
    cmd
END

WHEN RAND(4)
    cmd
    cmd
    cmd
END

WHEN TIMER(40 timerName)

END
*/

enum Syntax {
    SY_When  = 1,
    SY_Name  = 2,
    SY_End   = 3,
    SY_Enter = 4,
    SY_Exit  = 5,
    SY_Said  = 6,
    SY_Rand  = 7,
};

static KeyWord sSyntax[] = {
    { SY_When,  0, "WHEN"  },
    { SY_Name,  0, "NAME"  },
    { SY_End,   0, "END"   },
    { SY_Enter, 0, "ENTER" },
    { SY_Exit,  0, "EXIT"  },
    { SY_Said,  0, "SAID"  },

    { 0,0,0}
};


#define T UWORD
#define FNAME Action
#define VNAME Actions_t
#include VEC_INCLUDE_PATH

union ul_to_us {
    UWORD uw[2];
    ULONG ul;
};

VOID Actions_pushULONG(Actions_t* a, ULONG val) {
    union ul_to_us xx;
    xx.ul = val;

    Action_pushInto(a, &xx.uw[0]);
    Action_pushInto(a, &xx.uw[1]);
}

typedef struct Condition {
    UWORD kind;
    UWORD numArgs;
    UWORD args[8];
} Condition;

typedef struct When {
    Condition cond;
    Actions_t actions;
} When;

#define T When
#define FNAME When
#define VNAME Whens_t
#define GC

VOID When_move(When* dst, When* src) {
    dst->cond = src->cond;
    Action_moveVec(&dst->actions, &src->actions);
}

VOID When_free(When* self) {
    Action_delete(&self->actions);
}

#include VEC_INCLUDE_PATH

typedef struct Room {
    UWORD   num;
    UWORD   name;
    Whens_t whens;
} Room;

#define T Room
#define FNAME Room
#define VNAME Rooms_t
#define GC    

Room* rm = NULL;
When* wn = NULL;

VOID Room_move(Room* dst, Room* src) {
    dst->num  = src->num;
    dst->name = src->name;
    When_moveVec(&dst->whens, &src->whens);
}

VOID Room_free(Room* self) {
    When_delete(&self->whens);
}

#include VEC_INCLUDE_PATH
static Rooms_t sRooms;


//
//
//  Loading
//
//


static VOID PushUWord(UWORD v) {
    Action_pushInto(&wn->actions, &v);
}

static VOID PushWord(WORD v) {
    Action_pushInto(&wn->actions, &v);
}

static VOID PushULong(ULONG v) {
    union ul_to_us xx;
    xx.ul = v;

    Action_pushInto(&wn->actions, &xx.uw[0]);
    Action_pushInto(&wn->actions, &xx.uw[1]);
}

static BOOL VisitCall() {

    ActionSyntax* ac = FindActionSyntax(TokHash);

    if (ac == NULL) {
        printf("> Syntax Error. Unknown call %s\n", TokText);
        return FALSE;
    }

    UWORD argIdx = 0;

    PushUWord(ac->num); // This wrong when a friend is changed.

    BOOL skipRead = FALSE;
    UWORD tok = 0;

    while(TRUE) {

        UWORD type = ac->args[argIdx];

        if (type == 0) {

            break;
        }

        if (skipRead == FALSE) {
            tok = NextTok();
            skipRead = FALSE;
        }
       

        UWORD needTok  = 0;
        UWORD wordSize = 2;

        switch(type) {
            default:
                printf("Syntax Error unknown arg type! %c", (char) type);
                return FALSE;
            case 'x':
                needTok  = TOK_SYMBOL;
                break;
            case 'u':
            case 'i':
                needTok  = TOK_NUM;
                break;
            case 's':
                needTok  = TOK_STR;
                break;
            case 'L':
                needTok  = TOK_NUM;
                wordSize = 4;
                break;
        }

        if (tok != needTok) {
            if (ac->friend != 0) {
                printf("Finding friend %u", ac->friend);
                ac = FindActionSyntaxByNum(ac->friend);
                if (ac != NULL) {
                    skipRead = TRUE;
                    continue;
                }
            }

            printf("> Syntax Error. Incorect argument for %c", (char) type);
            return FALSE;
        }

        switch(type) {
            case 'x': {
                UWORD global = FindGlobal(TokHash);

                if (global == 0) {
                    printf("> Syntax Error. Could not find global %s\n", TokText);
                    return FALSE;
                }

                PushUWord(global);
            }
            break;
            case 's': {
                ULONG offset = PushTextFromToken();
                PushULong(offset);
            }
            break;
            case 'u':
                PushUWord(TokNum);
            break;
            case 'i':
                PushWord(TokNum);
            break;
            case 'L':
                PushULong((LONG) TokNum);
            break;
        }

        argIdx++;
    }


    return TRUE;
}

static BOOL VisitCalls() {
    while(TRUE) {
        ULONG tok = MustNextTok(TOK_SYMBOL);

        if (tok == TOK_EOF) {
            printf("> Syntax Error. Missing END on WHEN\n");
            return FALSE;
        }

        UWORD kw = FindKeyWordNum(&sSyntax[0], TokHash);

        if (kw == SY_End) {
            return TRUE;
        }

        if (VisitCall() == FALSE)
            return FALSE;
    }
}

static BOOL VisitName() {
    MustNextTok(TOK_SYMBOL);
    rm->name = TokHash;
    return TRUE;
}

static BOOL VisitWhen() {
    ULONG tok = MustNextTok(TOK_SYMBOL);
    UWORD kw = FindKeyWordNum(&sSyntax[0], TokHash);

    wn = When_push(&rm->whens);
    Action_new(&wn->actions);

    switch(kw) {
        default:
            printf("> Syntax Error: Unknown When Condition %s\n", TokText);
            return FALSE;
        case SY_Enter:
        {
            MustNextTok(TOK_OPEN);
            MustNextTok(TOK_CLOSE);

            wn->cond.kind = WHEN_Enter;
            wn->cond.numArgs = 0;
        }
        break;
        case SY_Exit:
        {
            MustNextTok(TOK_OPEN);
            MustNextTok(TOK_CLOSE);
                                     
            wn->cond.kind = WHEN_Exit;
            wn->cond.numArgs = 0;
        }
        break;
    }

    return VisitCalls();
}

static VOID PrintRoom(Room* room) {
    printf("Room.Num  = %u\n", room->num);
    printf("Room.Name = %u\n", room->name);
}

static BOOL VisitRoom(UWORD num) {

    rm = Room_push(&sRooms);
    rm->num = num;
    rm->name = 0;
    When_new(&rm->whens);

    while(TRUE) {

        ULONG tok = NextTok();

        if (tok == TOK_EOF) {
            printf("End of Room File\n");
            break;
        }

        if (tok != TOK_SYMBOL) {
            printf("> Syntax Error: Expected Keyword. Got %lu\n", tok);
            return FALSE;
        }

        UWORD syntax = FindKeyWordNum(&sSyntax[0], TokHash);

        switch(syntax) {
            default:
                printf("> Syntax Error: %s\n", TokText);
                return FALSE;
            case SY_When:
                if (VisitWhen() == FALSE)
                    return FALSE;
                continue;
            case SY_Name:
                if (VisitName() == FALSE)
                    return FALSE;
                continue;
        }
    }

    return TRUE;
}

static char RoomPath[256] = { 0 };


static void SaveRooms();

void ReadRooms() {
 
    Room_new(&sRooms);
    Room_setCapacity(&sRooms, 24);

    HashActionSyntax();
    HashKeyWords(&sSyntax[0]);

    for(UWORD i=1;i <= MAX_ROOMS;i++) {
        sprintf(RoomPath, "%s%u", ROOMS_SRC_PATH, i);

        if (OpenTokFile(RoomPath) == FALSE) {
            printf("Could not Room open %s", RoomPath);
            break;
        }

        if (VisitRoom(i) == FALSE) {
            CloseTokFile();
            break;
        }

        CloseTokFile();
    }
    SaveRooms();


    printf("ROOMS COMPLETED\n");
    Room_delete(&sRooms);
}


//
//
//
// Saving
//
//
FILE* sRoomFile = NULL;

static void SaveUWORD(UWORD v) {
    fwrite(&v, sizeof(v), 1, sRoomFile);
}

static void SaveWORD(WORD v) {
    fwrite(&v, sizeof(v), 1, sRoomFile);
}

static void SaveULONG(ULONG v) {
    fwrite(&v, sizeof(v), 1, sRoomFile);
}

static void SaveCondition(Condition* cn) {
    SaveUWORD(cn->kind);
    SaveUWORD(cn->numArgs);
    for(UWORD i=0;i < cn->numArgs;i++) {
        SaveUWORD(cn->args[i]);
    }
}

static void SaveWhen(When* wn) {
    SaveCondition(&wn->cond);
    SaveUWORD(wn->actions.size);
    fwrite(&wn->actions.items[0], wn->actions.size * sizeof(UWORD), 1, sRoomFile);
}

static void SaveRoom(Room* rm, UWORD num) {        
   
    SaveUWORD(rm->num);
    SaveUWORD(rm->name);
    SaveUWORD((UWORD) rm->whens.size);

    for(UWORD i=0;i < rm->whens.size;i++) {
        SaveWhen(&rm->whens.items[i]);
    }
                        
}

static void SaveRooms() {
    sRoomFile = fopen(ROOMS_DST_PATH, "wb");

    SaveUWORD(sRooms.size);

    for(UWORD i=0;i < sRooms.size;i++) {
        SaveRoom(&sRooms.items[i], i+1);
    }

    fclose(sRoomFile);

}

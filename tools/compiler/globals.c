#include <exec/types.h>
#include <stdio.h>

#include "compiler.h"

typedef struct Global {
    UWORD num;
    UWORD hash;
    char  text[32];
} Global;

#define T Global
#define VNAME GlobalsT
#define FNAME Globals
#include VEC_INCLUDE_PATH

static GlobalsT sGlobals;

void InitGlobals() {
    Globals_new(&sGlobals);
    Globals_setCapacity(&sGlobals, MAX_GLOBALS);
}

void PrintGlobals() {
    for(ULONG i=0;i < sGlobals.size;i++) {
        Global* g = &sGlobals.items[i];
        printf("%lu %lu %s\n", (ULONG) g->num, (ULONG) g->hash, g->text);
    }
}

void AddGlobal(CONST_STRPTR text, ULONG len, UWORD hash) {
    if (len > 31) {
        printf("Global %s is to long in length to use!\n", text);
        return;
    }

    Global* g = Globals_push(&sGlobals);
    g->num = sGlobals.size;
    g->hash = hash;
    CopyMem(text, g->text, len);
}

UWORD FindGlobal(UWORD hash) {
    for(ULONG i=0;i < sGlobals.size;i++) {
        Global* g = &sGlobals.items[i];
        if (g->hash == hash) {
            return g->num;
        }
    }
    return 0;
}

void ReadGlobals() {
    InitGlobals();
    OpenTokFile(GLOBALS_PATH);
    while(TRUE) {
        ULONG tok = NextTok();
        if (tok == TOK_EOF)
            break;

        if (tok != TOK_SYMBOL) {
            printf("Syntax Error reading global\n Expected symbol\n");
            break;
        }

        AddGlobal(TokText, TokTextLen, TokHash);
    }
    CloseTokFile();

    PrintGlobals();
}


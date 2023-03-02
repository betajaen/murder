#include <exec/types.h>

#include "compiler.h"

extern void ReadGlobals();
extern void ReadVocab();
extern void SaveVocab();
extern void ReadRooms();
extern void SaveRooms();

UWORD HashString(CONST_STRPTR str) {
    ULONG hash = 0;

    while(*str != 0) {
        char ch = *str++;
        if (ch >= 'A' && ch <= 'Z') {
            ch += 32;
        }
        hash += ch;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    UWORD r = (hash ^ (hash >> 16)) & 0xFFFF;

    return r;
}

int main() {
    InitText();
    PushText("EVEN", 4);
    PushText("ODD", 3);
    PushText("STRING", 6);
    ReadGlobals();
    ReadVocab();
    SaveVocab();
    ReadRooms();
    SaveText();
}



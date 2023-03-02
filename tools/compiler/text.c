#include <proto/exec.h>
#include "compiler.h"
#include "stdio.h"

static char* sText = NULL;
static ULONG size = 0;
static ULONG capacity = 0;

VOID InitText() {
    capacity = 8192;
    size = 0;
    sText = AllocVec(capacity, MEMF_CLEAR);
}

VOID SaveText() {
    FILE* f = fopen(TEXT_DST_PATH, "wb");
    fwrite(sText, size, 1, f);
    fclose(f);

    FreeVec(sText);
}

ULONG PushText(CONST_STRPTR text, UWORD length) {
    ULONG nextSize = size + length + 1;

    if (nextSize & 1 == 1) {
        nextSize++;
    }

    if(nextSize > capacity) {
        ULONG newCapacity = capacity;
        while(nextSize < newCapacity) {
            newCapacity *= 2;
        }

        char* newText = AllocVec(newCapacity, MEMF_CLEAR);
        CopyMem(sText, newText, size);
        FreeVec(sText);

        sText = newText;
    }

    CopyMem(text, &sText[size], length);
    ULONG offset = size;
    size = nextSize;

    return offset;
}

ULONG PushTextFromToken() {
    return PushText(TokText, TokTextLen);
}

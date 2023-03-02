#include "x.h"
#include "g.h"

CHAR* gText = NULL;

static BOOL ReadText() {

    if (xReadFile(1, GAME_PATH("text")) == FALSE) {
        xPrintStr("Could not open text file!");
        return FALSE;
    }

    ULONG textLen = xFileLength(1);

    if (gText != NULL) {
        xFreeVec(gText);
    }

    gText = xAllocVec(textLen);

    xPrintFmt("Text Data Len = %lu", textLen);

    xReadMem(1, gText, textLen);

    xCloseFile(1);
    return TRUE;
}

static VOID FreeText() {
    if (gText != NULL) {
        xFreeVec(gText);
    }
}

VOID gEntry() {

    if (ReadText() == FALSE) {
        return;
    }

    if (xOpenFont(GAME_PATH("BitPotion.font"), 9) == FALSE) {
        xPrintStr("Could not open font!");
        FreeText();
        return;
    }

    if (xOpenScreen(320, 240, 5) == FALSE) {
        xPrintStr("Could not open screen!");
        xCloseFont();
        FreeText();
        return;
    }

    xUseFont();

    if (LoadRooms() == FALSE) {
        xPrintStr("Could not open rooms file!");
        xCloseScreen();
        xCloseFont();
        FreeText();
        return;
    }

    xLoadImg(GAME_PATH("bedroom1.img"), 2, 1);
    xUsePalette(1);
    xCopyBitMap(2);

    xMove(20,20);
    xDrawMode(1, 4, 0);
    //xWriteStr("Hello World!");
    xWriteStr(gText);
    xPrintStr(gText);
    /*
    xDrawMode(1, 5, 3);
    xBox(9,9,301,101);
    xDrawMode(2, 6, 8);
    xBar(10,10,300,100);
    */

    xPrintStr("** Started");


    while(xWaitEvent()) {
        xPrintFmt("Evt %ld %ld", (ULONG) xEventKind, (ULONG) xEventCode);
        if (xEventKind == xEK_KEY) {
            break;
        }
    }

    FreeRooms();
    xPrintStr("** Stopped");
    xCloseScreen();
    xCloseFont();
    FreeText();
}

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>

#include "x.h"
#include "SDI/includes/SDI_stdarg.h"


static const ULONG PutChar = 0x16c04e75;
static const ULONG LenChar = 0x52934e75;

//
//
// SHARED CONSTS
//
//

static struct Screen*   SCR = NULL;
static struct Window*   WIN = NULL;
static struct RastPort* RP  = NULL;
static struct ViewPort* VP  = NULL;
static struct TextFont* FNT = NULL;
static struct BitMap*   BM  = NULL;

static ULONG  xWidth  = 0;
static ULONG  xHeight = 0;
static ULONG  xDepth  = 0;

static char sStrBuf[1024] = { 0 };


//
//
//   MEMORY
//
//

VOID* xAllocVec(ULONG size) {
    return AllocVec(size, MEMF_CLEAR);
}

VOID xFreeVec(VOID* mem) {
    FreeVec(mem);
}

VOID xCopyMem(VOID* dst, VOID* src, ULONG size) {
    CopyMem(dst, src, size);
}

VOID xZeroMem(void* dst, ULONG size) {
    BYTE* data = (BYTE*) dst;
    while(size--) {
        *data++ = 0;
    }
}

ULONG xLen(CONST_STRPTR str) {
    ULONG count = 0;
    if (str != NULL) {
        while(*str++ != 0) {
            count++;
        }
    }
    return count;
}

//
//
//     DEBUG
//
//

VOID xPrintStr(CONST_STRPTR text) {
    Printf("%s\n", text);
}

VOID xPrintFmt(CONST_STRPTR fmt, ...) {
    VA_LIST args;
    VA_START(args, fmt);
    RawDoFmt(fmt, VA_ARG(args, void*), (void(*)()) &PutChar, sStrBuf);
    VA_END(args);

    Printf("%s\n", sStrBuf);
}

//
//     BITMAP
//
//

typedef struct xBitMap {
    struct BitMap bm;
    UWORD         num;
    UWORD         width;
    UWORD         height;
    UWORD         colours;
    UWORD         depth;
    ULONG         ps;
} xBitMap;

void xBitMap_move(xBitMap* dst, xBitMap* src) {
    CopyMem(dst, src, sizeof(xBitMap));
    xZeroMem(src, sizeof(xBitMap));
}

void xBitMap_free(xBitMap* self) {
    if (self->num != 0) {
        for(ULONG i=0;i < self->depth;i++) {
            PLANEPTR plane = self->bm.Planes[i];
            if (plane != NULL) {
                FreeRaster(plane, self->width, self->height);
            }
        }
        xZeroMem(self, sizeof(xBitMap));
    }
}

#define T  xBitMap
#define GC
#define VNAME xBitMaps
#define FNAME xBMS

#include "vector.inc"

static xBitMaps BMS;


xBitMap* xGetBitMap(UWORD num) {
    if (num == 0 || num > NUM_BITMAPS) {
        return NULL;
    }
    return &BMS.items[num - 1];
}

BOOL xCreateBitMap(UWORD num, UWORD w, UWORD h, UWORD d) {
    xBitMap* bm = xGetBitMap(num);
    if (bm == NULL) {
        xPrintStr("BitMap out of bounds!!");
        return FALSE;
    }

    if (bm->num != 0) {
        xPrintStr("BitMap is in use!!");
        return FALSE;
    }

    InitBitMap(&bm->bm, d, w, h);
    bm->width = w;
    bm->height = h;
    bm->depth = d;
    bm->colours = 1 << d;
    bm->ps = (w >> 3) * h;
    bm->num = num;

    for(UWORD i=0;i < d;i++) {
        PLANEPTR plane = (PLANEPTR) AllocRaster(w, h);
        if (plane == NULL) {
            xPrintStr("Could not allocate BitMap raster!!");
            return FALSE;
        }

        bm->bm.Planes[i] = plane;
        BltClear(plane, bm->ps, 1);
    }

    return TRUE;
}

VOID xFreeBitMap(UWORD num) {
    xBitMap* bm = xGetBitMap(num);
    if (bm == NULL) {
        xPrintStr("Could not free an empty bitmap!!");
        return;
    }
    if (bm->num == 0) {
        xPrintStr("Could not free an empty bitmap!!");
        return;
    }
    xBitMap_free(bm);
}

VOID xCopyBitMap(UWORD num) {
    xBitMap* bm = xGetBitMap(num);
    if (bm == NULL || bm->num == 0) {
        xPrintStr("Could not copy an empty bitmap!!");
        return;
    }

    BltBitMapRastPort(
        &bm->bm,
        0, 0,
        RP,
        0, 0,
        bm->width, bm->height,
        0xC0
    );
}

//
//
//     PALETTE
//
//

typedef struct xPalette {
    #if IS_ECS
        UWORD data[64];
    #else
        ULONG data[2 + (256 * 3)];
    #endif
    UWORD num;
    UWORD cols;
} xPalette;

#define T  xPalette
#define VNAME xPalettes
#define FNAME xPALS

#include "vector.inc"

static xPalettes PALS;

xPalette* xGetPalette(UWORD num) {
    if (num == 0 || num > NUM_PALETTES) {
        return NULL;
    }
    return &PALS.items[num - 1];
}

BOOL xCreatePalette(UWORD num, UWORD count) {
    xPalette* pal = xGetPalette(num);
    if (pal == NULL) {
        xPrintStr("Palette out of bounds!!");
        return FALSE;
    }

    if (pal->num != 0) {
        xPrintStr("Palette is in use!!");
        return FALSE;
    }

    pal->cols = count;
    pal->num = num;

    return TRUE;
}

VOID xFreePalette(UWORD num) {
    xPalette* pal = xGetPalette(num);
    if (pal == NULL) {
        xPrintStr("Could not free an empty palette!!");
        return;
    }
    if (pal->num == 0) {
        xPrintStr("Could not free an empty palette!!");
        return;
    }
    pal->num = 0;
    pal->cols = 0;
}

VOID xUsePalette(UWORD num) {
    xPalette* pal = xGetPalette(num);
    if (pal == NULL || pal->num == 0) {
        xPrintStr("Could not use an empty palette!");
        return;
    }

    #if IS_ECS
        LoadRGB4(VP, pal->cols, &pal->data[0]);
    #else
        LoadRGB32(VP, &pal->data[0]);
    #endif
}


//
//
//     SCREEN AND EVENT HANDLING
//
//

BOOL xOpenScreen(UWORD w, UWORD h, UWORD d) {
    xWidth  = w;
    xHeight = h;
    xDepth  = d;

    xCreateBitMap(1, w, h, d);
    BM = &(xGetBitMap(1)->bm);

    xCreatePalette(1, 1 << d);

    struct NewScreen scr;
    scr.LeftEdge = 0;
    scr.TopEdge = 0;
    scr.Width = w;
    scr.Height = h;
    scr.Depth = d;
    scr.ViewModes = SPRITES;
    scr.BlockPen = 1;
    scr.DetailPen = 0;
    scr.Type = CUSTOMSCREEN | CUSTOMBITMAP | SCREENQUIET;
    scr.Font = NULL;
    scr.DefaultTitle = NULL;
    scr.Gadgets = NULL;
    scr.CustomBitMap = BM;

    SCR = OpenScreen(&scr);

    if (SCR == NULL) {
        xFreeBitMap(1);
        BM = NULL;
        return FALSE;
    }

    struct NewWindow win;
    win.LeftEdge = 0;
    win.TopEdge = 0;
    win.DetailPen = 0;
    win.BlockPen = 0;
    win.Title = NULL;
    win.Width = w;
    win.Height = h;
    win.IDCMPFlags = IDCMP_VANILLAKEY | IDCMP_REFRESHWINDOW;
    win.Flags = WFLG_ACTIVATE |
                WFLG_OTHER_REFRESH |
                WFLG_RMBTRAP |
                WFLG_BORDERLESS |
                WFLG_NOCAREREFRESH |
                WFLG_GIMMEZEROZERO;
    win.Screen = SCR;
    win.Type = CUSTOMSCREEN;
    win.FirstGadget = NULL;
    win.CheckMark = NULL;
    win.BitMap = NULL;

    WIN = OpenWindow(&win);

    if (WIN == NULL) {
        CloseScreen(SCR);
        SCR = NULL;
        xFreeBitMap(1);
        BM = NULL;
        return FALSE;
    }

    RP = WIN->RPort;
    VP = &SCR->ViewPort;

    xUsePalette(1);

    return TRUE;
}

VOID xCloseScreen() {

    if (WIN != NULL) {
        CloseWindow(WIN);
        WIN = NULL;
    }

    if (SCR != NULL) {
        CloseScreen(SCR);
        SCR = NULL;
    }

    if (BM != NULL) {
        xFreeBitMap(1);
        BM = NULL;
    }
}

UWORD xEventKind = 0;
UWORD xEventCode = 0;

BOOL xWaitEvent() {
    ULONG bit = 1L << WIN->UserPort->mp_SigBit;
    ULONG signals = Wait(bit);
    if (signals & bit) {
        struct IntuiMessage* msg;
        ULONG code, class;

        xEventKind = xEK_NONE;
        xEventCode = 0;

        while(NULL != (msg = (struct IntuiMessage*) GetMsg(WIN->UserPort))) {
            class = msg->Class;
            code = msg->Code;

            ReplyMsg((struct Message*) msg);

            switch(class) {
                case IDCMP_VANILLAKEY:
                    xEventKind = xEK_KEY;
                    xEventCode = code;
                break;
                case IDCMP_REFRESHWINDOW:                    
                    RefreshWindowFrame(WIN);
                break;
            }
        }
    }
    else {
        xEventKind = xEK_NONE;
        xEventCode = 0;
    }

    return TRUE;
}

//
//
//  FONTS
//
//
BOOL xOpenFont(CONST_STRPTR path, UWORD size) {
    struct TextAttr ta = {
        (STRPTR) path,
        size,
        0,
        NULL
    };


    FNT = OpenDiskFont(&ta);
    return FNT != NULL;
}

VOID xCloseFont() {
    CloseFont(FNT);
    FNT = NULL;
}

VOID xUseFont() {
    if (SCR != NULL && FNT != NULL) {
        SetFont(RP, FNT);
    }
}


//
//
// Drawing
//
//

VOID xCls(UBYTE pen) {
    SetRast(RP, pen);
}

VOID xMove(WORD x, WORD y) {
    Move(RP, x, y);
}

VOID xDraw(WORD x, WORD y) {
    Draw(RP, x, y);
}

VOID xJam1() {
    SetDrMd(RP, JAM1);
}

VOID xJam2() {
    SetDrMd(RP, JAM2);
}

VOID xDrawMode(UBYTE jam, UBYTE aPen, UBYTE bPen) {
    SetDrMd(RP, JAM1 - 1 + jam);
    SetAPen(RP, aPen);
    SetBPen(RP, bPen);
}

VOID xWriteStr(CONST_STRPTR text) {
    ULONG len = xLen(text);
    Text(RP, text, len);
}

VOID xWriteFmt(CONST_STRPTR fmt, ...) {

    VA_LIST args;
    VA_START(args, fmt);
    RawDoFmt(fmt, VA_ARG(args, void*), (void(*)()) &PutChar, sStrBuf);
    VA_END(args);

    ULONG len = xLen(sStrBuf);
    Text(RP, sStrBuf, len);
}

VOID xBox(WORD x0, WORD y0, WORD x1, WORD y1) {
    Move(RP, x0, y0);
    Draw(RP, x1, y0);
    Draw(RP, x1, y1);
    Draw(RP, x0, y1);
    Draw(RP, x0, y0);
}

VOID xBar(WORD x0, WORD y0, WORD x1, WORD y1) {
    RectFill(RP, x0, y0, x1, y1);
}

//
//
//     FILES
//
//

static BPTR FILES[1+NUM_FILES] = { 0 };
static ULONG FILES_LEN[1+NUM_FILES] = { 0 };

BOOL xReadFile(UWORD num, CONST_STRPTR path) {
    if (FILES[num] != 0) {
        xCloseFile(num);
    }

    BPTR f = Open(path, MODE_OLDFILE);

    if (f == 0)
        return FALSE;

    FILES[num] = f;
    Seek(f, 0, OFFSET_END);
    LONG b = Seek(f, 0, OFFSET_BEGINNING);
    FILES_LEN[num] = b;

    return TRUE;
}

BOOL xWriteFile(UWORD num, CONST_STRPTR path) {
    BPTR f = Open(path, MODE_NEWFILE);
    FILES_LEN[num] = 0;
    FILES[num] = f;
    return f != 0;
}

VOID  xCloseFile(UWORD num) {
    BPTR f = FILES[num];
    if (f != 0) {
        Close(f);
        FILES[num] = 0;
        FILES_LEN[num] = 0;
    }
}

ULONG xFileLength(UWORD num) {
    return FILES_LEN[num];
}


UBYTE xReadUByte(UWORD num) {
    BPTR f = FILES[num];
    UBYTE v = 0;

    if (f != 0) {
        Read(f, &v, 1);
    }

    return v;
}

UWORD xReadUWord(UWORD num) {
    BPTR f = FILES[num];
    UWORD v = 0;

    if (f != 0) {
        Read(f, &v, 2);
    }

    return v;
}

ULONG xReadULong(UWORD num) {
    BPTR f = FILES[num];
    ULONG v = 0;

    if (f != 0) {
        Read(f, &v, 4);
    }

    return v;
}

WORD  xReadWord(UWORD num) {
    BPTR f = FILES[num];
    WORD v = 0;

    if (f != 0) {
        Read(f, &v, 2);
    }

    return v;
}

LONG xReadLong(UWORD num)  {
    BPTR f = FILES[num];
    LONG v = 0;

    if (f != 0) {
        Read(f, &v, 4);
    }

    return v;
}

VOID xReadMem(UWORD num, APTR value, ULONG length) {
    BPTR f = FILES[num];
    if (f != 0) {
        Read(f, value, length);
    }
}

VOID xSkipFile(UWORD num, LONG offset) {
    BPTR f = FILES[num];
    if (f != 0) {
        Seek(f, offset, OFFSET_CURRENT);
    }
}

VOID xSeekFile(UWORD num, ULONG offset) {
    BPTR f = FILES[num];
    if (f != 0) {
        Seek(f, offset, OFFSET_BEGINNING);
    }
}

//
//
//      Assets
//
//
VOID xLoadImg(CONST_STRPTR path, UWORD bitmap, UWORD palette) {

    if (xReadFile(0, path) == FALSE) {
        xPrintStr("Could not open file!!");
        return;
    }

    UWORD w = xReadUWord(0);
    UWORD h = xReadUWord(0);
    UWORD d = xReadUWord(0);
    UWORD cols = 1 << d;
    ULONG ps = (w >> 3) * h;

    ULONG ecsPalSize = sizeof(UWORD) * cols;
    ULONG agaPalSize = sizeof(ULONG) * cols * 3 + sizeof(ULONG) * 2;

    if (palette != 0) {

        xPalette* pal = xGetPalette(palette);
        pal->num = palette;
        pal->cols = cols;

        xPrintFmt("Pal %xd", pal);

        #if IS_ECS
            xReadMem(0, &pal->data[0], ecsPalSize);
            xSkipFile(0, agaPalSize);
        #else
            xSkipFile(0, ecsPalSize);
            xReadMem(0, &pal->data[0], agaPalSize);
        #endif
    }
    else {
        xSkipFile(0, ecsPalSize + agaPalSize);
    }

    if (bitmap != 0 && xCreateBitMap(bitmap, w, h, d)) {
        xBitMap* bm = xGetBitMap(bitmap);

        for(UWORD i=0;i < d;i++) {
            xReadMem(0, bm->bm.Planes[i], ps);
        }
    }

    xCloseFile(0);
}


//
//
//
//      Entry
//
//
//
//
static VOID xInit() {
    xBMS_new(&BMS);
    xBMS_setCapacity(&BMS, NUM_BITMAPS);

    for(UWORD i=0;i <= NUM_BITMAPS;i++) {
        xBitMap* bm = xBMS_push(&BMS); /* bm ignored */
    }

    xPALS_new(&PALS);
    xPALS_setCapacity(&PALS, NUM_PALETTES);

    for(UWORD i=0;i <= NUM_PALETTES;i++) {
        xPalette* pal = xPALS_push(&PALS);
    }
}

static VOID xShutdown() {

    xCloseScreen();

    xPALS_delete(&PALS);
    xBMS_delete(&BMS);

    if (FNT) {
        xCloseFont();
    }
}


int main(int argc, char** argv) {

    xInit();
    gEntry();
    xShutdown();

    return 0;
}


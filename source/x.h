#ifndef X_H
#define X_H

#include <exec/types.h>

#define NUM_BITMAPS  4
#define NUM_PALETTES 2
#define NUM_FILES    2

#define _X_CAT(X,Y) X##Y
#define X_CAT(X,Y) _X_CAT(X,Y)

#define NONE 0

typedef UWORD Handle;

#if !defined(X_CHAR)
typedef char CHAR;
#define X_CHAR
#endif

// Game

extern VOID  gEntry();

// Memory
extern VOID* xAllocVec(ULONG size);
extern VOID  xFreeVec(VOID* mem);
extern VOID  xCopyMem(VOID* dst, VOID* src, ULONG size);
extern ULONG xLen(CONST_STRPTR str);

//
extern VOID  xPrintStr(CONST_STRPTR text);
extern VOID  xPrintFmt(CONST_STRPTR fmt, ...);

// Screen
extern BOOL  xOpenScreen(UWORD w, UWORD h, UWORD d);
extern VOID  xCloseScreen();
extern BOOL  xWaitEvent();

#define xEK_NONE  0
#define xEK_KEY   1

extern UWORD xEventKind;
extern UWORD xEventCode;

// BitMap
extern BOOL xCreateBitMap(UWORD num, UWORD w, UWORD h, UWORD d);
extern VOID xFreeBitMap(UWORD num);
extern VOID xCopyBitMap(UWORD num);
extern VOID xUsePalette(UWORD num);

// Drawing
extern VOID xCls(UBYTE pen);
extern VOID xMove(WORD x, WORD y);
extern VOID xDraw(WORD x, WORD y);
extern VOID xJam1();
extern VOID xJam2();
extern VOID xDrawMode(UBYTE jam, UBYTE aPen, UBYTE bPen);
extern VOID xWriteStr(CONST_STRPTR text);
extern VOID xWriteFmt(CONST_STRPTR fmt, ...);

// FONT

extern BOOL xOpenFont(CONST_STRPTR path, UWORD size);
extern VOID xCloseFont();
extern VOID xUseFont();

// File
extern BOOL xReadFile(UWORD num, CONST_STRPTR path);
extern BOOL xWriteFile(UWORD num, CONST_STRPTR path);
extern VOID  xCloseFile(UWORD num);
extern ULONG xFileLength(UWORD num);
extern UBYTE xReadUByte(UWORD num);
extern UWORD xReadUWord(UWORD num);
extern ULONG xReadULong(UWORD num);
extern WORD  xReadWord(UWORD num);
extern LONG  xReadLong(UWORD num);
extern VOID  xReadMem(UWORD num, APTR value, ULONG length);
extern VOID  xSkipFile(UWORD num, LONG offset);
extern VOID  xSeekFile(UWORD num, ULONG offset);

// Assets
extern VOID  xLoadImg(CONST_STRPTR path, UWORD bitmap, UWORD palette);

#endif


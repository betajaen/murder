#ifndef COMPILER_H
#define COMPILER_H

#include <exec/types.h>
#include <proto/exec.h>

#define COMPILER

//
//
// Paths
//
//

#define _X_CAT(X,Y) X##Y
#define X_CAT(X,Y) _X_CAT(X,Y)

#define BASE_PATH "work:murder/"

#define VOCAB_SRC_PATH   BASE_PATH "data/vocab"
#define VOCAB_DST_PATH   BASE_PATH "source/vocab.inc"
#define GLOBALS_PATH     BASE_PATH "data/globals"
#define VEC_INCLUDE_PATH "work:murder/source/vector.inc"
#define ACTION_INCLUDE_PATH "work:murder/source/action.inc"
#define WHEN_INCLUDE_PATH "work:murder/source/when.inc"
#define TEXT_DST_PATH    BASE_PATH "game/text"
#define ROOMS_SRC_PATH   BASE_PATH "data/room"
#define ROOMS_DST_PATH   BASE_PATH "game/room"

//
//
//
#define xAllocVec(S) AllocVec(S, MEMF_CLEAR)
#define xFreeVec FreeVec
#define xCopyMem CopyMem


//
//
// Globals
//
//


#define MAX_GLOBALS 256

extern UWORD FindGlobal(UWORD hash);

// Hash

extern UWORD HashString(CONST_STRPTR str);

// Keywords

typedef struct KeyWord {
    UWORD num;
    UWORD hash;
    CONST_STRPTR text;
} KeyWord;

extern void HashKeyWords(KeyWord* keyWords);
extern UWORD FindKeyWordNum(KeyWord* keyWords, UWORD hash);
extern VOID PrintKeyWords(KeyWord* keyWords);

// Vocab

extern BOOL FindVocab(UWORD hash, UWORD* out_vocab);

// Tokenizer

enum TOKS
{
    TOK_EOF,
    TOK_OPEN,     // (
    TOK_CLOSE,    // )
    TOK_COMMA,    // ,
    TOK_NUM,      // 1234 (WORD)
    TOK_STR,      // "Hello"
    TOK_SYMBOL,   // hello
    TOK_SECTION,  // :hello

    TOK_COUNT
};

extern BOOL OpenTokFile(CONST_STRPTR path);
extern void CloseTokFile();
extern UWORD NextTok();
extern UWORD MustNextTok(UWORD tok);
extern UWORD Tok;
extern LONG  TokNum;
extern UWORD TokHash;
extern char  TokText[4096];
extern UWORD TokTextLen;

// Rooms

#define MAX_ROOMS 1

// Text
extern VOID InitText();
extern VOID SaveText();
extern ULONG PushTextFromToken();
extern ULONG PushText(CONST_STRPTR text, UWORD length);

#endif

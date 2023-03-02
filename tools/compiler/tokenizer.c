#include <proto/dos.h>
#include "stdio.h"
#include "compiler.h"


static BPTR TokFile = 0;
CONST_STRPTR TokFileName = NULL;
UWORD Tok = 0;
LONG  TokNum = 0;
UWORD TokHash = 0;
char  TokText[4096] = { 0 };
UWORD TokTextLen = 0;
UWORD TokLineNum = 1;
UWORD MaxTokenRead = 0;

CONST_STRPTR TokNames[] = {
    "EOF",
    "(",
    ")",
    ",",
    "NUM",
    "STR",
    "SYMBOL",
    "SECTION"
};

void HashKeyWords(KeyWord* keyWords) {
    while(keyWords->text != NULL) {
        keyWords->hash = HashString(keyWords->text);
        keyWords++;
    }
}

UWORD FindKeyWordNum(KeyWord* keyWords, UWORD hash) {
    while(keyWords->text != NULL) {
        if (keyWords->hash == hash) {
            return keyWords->num;
        }
        keyWords++;
    }
    return 0;
}

VOID PrintKeyWords(KeyWord* keyWords) {
    while(keyWords->text != NULL) {
        printf("Num=%u Hash=%u Text=%s\n", keyWords->num, keyWords->hash, keyWords->text);
        keyWords++;
    }
}

BOOL OpenTokFile(CONST_STRPTR path) {
    if (TokFile != 0) {
        return FALSE;
    }

    TokFileName = path;
    printf("Open Tok File\n");
    TokFile = Open(TokFileName, MODE_OLDFILE);
    TokLineNum = 1;
    MaxTokenRead = 0;

    return TokFile != 0;
}

void CloseTokFile() {
    if (TokFile != 0) {
        printf("Close Tok File\n");
        Close(TokFile);
        TokFile = 0;
    }
}

static LONG ReadChar() {
    return FGetC(TokFile);
}

static VOID SkipWhitespace() {

    while(TRUE) {
        LONG ch = ReadChar();
        if (ch == -1)
            return;

        if (ch == '\n')
            TokLineNum++;

        if (ch < 33)
            continue;

        break;
    }

    UnGetC(TokFile, -1);
}

static VOID ReadNum(BOOL negative) {

    LONG num = 0;

    while(TRUE) {
        LONG ch = ReadChar();

        if (ch == -1)
            break;

        if (ch >= '0' && ch <= '9') {
            num *= 10;
            num += ch - '0';
            continue;
        }

        UnGetC(TokFile, -1);
        break;
    }

    if (negative) {
        num = -num;
    }

    TokNum = num;
}

static VOID ReadSymbol() {
    TokTextLen = 0;
    TokText[0] = '\0';
    TokHash = 0;

    ULONG hash = 0;

    while(TokTextLen < sizeof(TokText)) {

        LONG ch = ReadChar();

        if (ch == -1)
            break;

        if (ch >= 'a' && ch <= 'z')
            ch -= 32;

        if (ch >= 'A' && ch <= 'Z' ||
            ch >= '0' && ch <= '9' ||
            ch == '_')
        {
            TokText[TokTextLen++] = ch & 0xFF;
            TokText[TokTextLen] = '\0';

            if (ch >= 'A' && ch <= 'Z') {
                ch += 32;
            }

            hash += ch;
            hash += (hash << 10);
            hash ^= (hash >> 6);

            continue;
        }

        UnGetC(TokFile, -1);
        break;
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    TokHash = (hash ^ (hash >> 16)) & 0xFFFF;
}

static VOID ReadString() {
    TokTextLen = 0;
    TokText[0] = '\0';
    TokHash = 0;

    ULONG hash = 0;

    while(TokTextLen < sizeof(TokText)) {
        LONG ch = ReadChar();

        if (ch == -1)
            break;

        if (ch == '"')
            break;


        // Escape
        if (ch == '\\') // backslash
        {
            ch = ReadChar();
            if (ch == -1)
                break;

            switch(ch) {
                default:
                    // Invalid char sequence
                    continue;
                case '\\':  // Backslash
                case '"':   // Quote
                    break;
                case 'n':
                    ch = '\n'; // \n  Newline
                break;
            }
        }

        TokText[TokTextLen++] = ch;
        TokText[TokTextLen] = 0;

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

    TokHash = (hash ^ (hash >> 16)) & 0xFFFF;
}

UWORD NextTok() {

    MaxTokenRead++;

    Tok = TOK_EOF;
    TokHash = 0;
    TokNum = 0;
    TokText[0] = 0;
    TokTextLen = 0;


    if (MaxTokenRead > 4096) {
        printf("TOKEN LOOP PREVENTION EXIT\n");
        Tok = TOK_EOF;
        Exit(-1);
        return TOK_EOF;
    }

    if (TokFile == 0) {
        Tok = TOK_EOF;   
        return Tok;
    }

    SkipWhitespace();

    LONG ch = ReadChar();

    if (ch == -1) {
        Tok = TOK_EOF;
        return Tok;
    }

    // open
    if (ch == '(') {
        Tok = TOK_OPEN;
        return Tok;
    }

    // close
    if (ch == ')') {
        Tok = TOK_CLOSE;
        return Tok;
    }

    // comma
    if (ch == ',') {
        Tok = TOK_COMMA;
        return Tok;
    }

    // sections
    if (ch == ':') {
        ReadSymbol();
        Tok = TOK_SECTION;
        return Tok;
    }

    // symbols
    if (ch >= 'A' && ch <= 'Z' ||
        ch >= 'a' && ch <= 'z' ||
        ch == '_')
    {
        UnGetC(TokFile, -1);
        ReadSymbol();
        Tok = TOK_SYMBOL;
        return Tok;
    }

    // negative num
    if (ch == '-') {
        ReadNum(TRUE);
        Tok = TOK_NUM;
        return Tok;
    }

    // postive num
    if (ch == '+') {
        ReadNum(FALSE);
        Tok = TOK_NUM;
        return Tok;
    }

    // postive num
    if (ch >= '0' && ch <= '9') {
        UnGetC(TokFile, -1);
        ReadNum(FALSE);
        Tok = TOK_NUM;
        return Tok;
    }

    // strs
    if (ch == '"') {
        ReadString();
        Tok = TOK_STR;
        return Tok;
    }

    return TOK_EOF;
}

UWORD MustNextTok(UWORD token) {
    UWORD tok = NextTok();

    if (tok != token) {
        printf("> Syntax Error at %s:%lu\n", TokFileName, (ULONG) TokLineNum);
        printf("  Expected '%s'\n", TokNames[token]);
        printf("  Got '%s'", TokNames[tok]);

        switch(tok)
        {
            case TOK_NUM:
                printf(" %ld", TokNum);
                break;
            case TOK_SYMBOL:
            case TOK_SECTION:
            case TOK_STR:
                printf(" %s", TokText);
                break;
        }

        printf("\n");
        Exit(-1);
    }

    return tok;
}

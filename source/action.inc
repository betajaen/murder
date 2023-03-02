// Action Schema

// Types:
//
//  .x  -- Global        (USHORT)
//  .u  -- Unsigned Word (USHORT)
//  .i  -- Signed Word   (SHORT)
//  .L  -- Unsigned long (ULONG)
//  .s  -- String        (ULONG offset)
//
// Action IDs are always USHORT
//
// String are provided as offsets (ULONG)
//  The format is:
//
//  LEN.u CHARS... NULL-BYTE/NULL-BYTE2 (Rounding)
//
//
// All actions are rounded to two bytes in length

enum ACTION {

    AC_NONE,

    AC_SET,      // VarSet(VAR.x, VAL.i)
    AC_ADD,      // VarAdd(VAR.x, VAL.i)
    AC_SUB,      // VarSub(VAR.x, VAL.i)
    AC_CPY,      // VarCpy(DST.x, SRC.x)

    AC_EX_EQI,   // ExitIfI(CND_EQ, LHS.x, RHS.i)
    AC_EX_NEI,   // ExitIfI(CND_NE, LHS.x, RHS.i)
    AC_EX_GTI,   // ExitIfI(CND_GT, LHS.x, RHS.i)
    AC_EX_LTI,   // ExitIfI(CND_LT, LHS.x, RHS.i)
    AC_EX_GEI,   // ExitIfI(CND_GE, LHS.x, RHS.i)
    AC_EX_LEI,   // ExitIfI(CND_LE, LHS.x, RHS.i)

    AC_EX_EQX,   // ExitIfX(CND_EQ, LHS.x, RHS.x)
    AC_EX_NEX,   // ExitIfX(CND_NE, LHS.x, RHS.x)
    AC_EX_GTX,   // ExitIfX(CND_GT, LHS.x, RHS.x)
    AC_EX_LTX,   // ExitIfX(CND_LT, LHS.x, RHS.x)
    AC_EX_GEX,   // ExitIfX(CND_GE, LHS.x, RHS.x)
    AC_EX_LEX,   // ExitIfX(CND_LE, LHS.x, RHS.x)


    AC_SAY,      // Say(MSG.s)
    AC_ASK,      // VarSet(VAR.u, Ask(MSG.s, YES.s, NO.s))

    AC_SET_BG,   // SetBg(NAME.s)
    AC_CLR_BG,   // ClrBg()

    AC_SET_FG,   // SetFg(NUM.u, NAME.s)
    AC_CLR_FG,   // ClrFg(NUM.u)


    AC_COUNT,    // COUNT
};

#if defined(COMPILER)

typedef struct ActionSyntax {
    UWORD num;
    CONST_STRPTR text;
    CONST_STRPTR args;
    UWORD friend;
    UWORD hash;
} ActionSyntax;

static ActionSyntax kActionSyntax[] = {

//  NAME        TEXT    ARGS     FRIEND     HASH
  { AC_SET,    "SET",   "xi",    AC_NONE,   0 },
  { AC_ADD,    "ADD",   "xi",    AC_NONE,   0 },
  { AC_SUB,    "SUB",   "xi",    AC_NONE,   0 },
  { AC_CPY,    "CPY",   "xx",    AC_NONE,   0 },

  { AC_EX_EQI, "EQ",    "xi",    AC_EX_EQX, 0 },
  { AC_EX_NEI, "NE",    "xi",    AC_EX_NEX, 0 },
  { AC_EX_GTI, "GE",    "xi",    AC_EX_LTX, 0 },
  { AC_EX_LTI, "LT",    "xi",    AC_EX_GTX, 0 },
  { AC_EX_GEI, "GE",    "xi",    AC_EX_GEX, 0 },
  { AC_EX_LEI, "LE",    "xi",    AC_EX_LEX, 0 },

  { AC_EX_EQX, "0",     "xx",    AC_NONE,   0 },
  { AC_EX_NEX, "0",     "xx",    AC_NONE,   0 },
  { AC_EX_GTX, "0",     "xx",    AC_NONE,   0 },
  { AC_EX_LTX, "0",     "xx",    AC_NONE,   0 },
  { AC_EX_GEX, "0",     "xx",    AC_NONE,   0 },
  { AC_EX_LEX, "0",     "xx",    AC_NONE,   0 },

  { AC_SAY,    "SAY",   "s",     AC_NONE,   0 },
  { AC_ASK,    "ASK",   "xsss",  AC_NONE,   0 },

  { AC_SET_BG, "SET_BG", "s",     AC_NONE,   0 },
  { AC_CLR_BG, "CLR_BG", "",      AC_NONE,   0 },
  { AC_SET_FG, "SET_FG", "us",    AC_NONE,   0 },
  { AC_CLR_FG, "CLR_FG", "u",     AC_NONE,   0 },

  { 0, 0, 0, 0, 0 }
};

static inline VOID HashActionSyntax() {
    ActionSyntax* syntax = &kActionSyntax[0];

    while(syntax->text != NULL) {
        syntax->hash = HashString(syntax->text);
        syntax++;
    }
}

static inline ActionSyntax* FindActionSyntax(UWORD hash) {
    ActionSyntax* syntax = &kActionSyntax[0];

    while(syntax->text != NULL) {
        if (syntax->hash == hash) {
            return syntax;
        }
        syntax++;
    }

    return NULL;
}

static inline ActionSyntax* FindActionSyntaxByNum(UWORD num) {
    ActionSyntax* syntax = &kActionSyntax[0];

    while(syntax->text != NULL) {
        if (syntax->num == num) {
            return syntax;
        }
        syntax++;
    }
}

#else

extern VOID VarSet(UWORD var, WORD val);
extern VOID VarAdd(UWORD var, WORD val);
extern VOID VarSub(UWORD var, WORD val);
extern VOID VarCpy(UWORD dst, UWORD src);

enum VarCondition {
    CND_EQ,
    CND_NEQ,
    CND_GT,
    CND_LT,
    CND_GE,
    CND_LE,
};

extern VOID ExitIfI(UWORD cond, UWORD lhs, UWORD rhs);
extern VOID ExitIfX(UWORD cond, UWORD lhs, WORD rhs);

extern VOID Say(ULONG offset);
extern UWORD Ask(ULONG msg, ULONG yes, ULONG no);

extern VOID SetBg(ULONG name);
extern VOID ClrBg();

extern VOID SetFg(UWORD num, ULONG name);
extern VOID ClrFg(UWORD num);

#endif

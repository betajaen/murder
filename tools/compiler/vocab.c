#include "compiler.h"
#include <stdio.h>

static KeyWord kKeyWords[] = {
    { 1, 0, "PREPOSITION" },  // above, near, on
    { 2, 0, "ARTICLE" },      // the, a, an
    { 3, 0, "ADJECTIVE" },    // red, shiny
    { 4, 0, "NOUN" },         // body, staircase
    { 5, 0, "VERB" },         // take, look
    { 0, 0, NULL }
};

typedef struct Vocab {
    UWORD kind;
    UWORD variants[16];
} Vocab;

#define T Vocab
#define VNAME VocabsT
#define FNAME Vocabs
#include VEC_INCLUDE_PATH

VocabsT sVocabs;

BOOL FindVocab(UWORD hash, UWORD* out_Vocab) {
    Vocab* v = &sVocabs.items[0];
    for(UWORD i=0;i < sVocabs.size;i++) {
        for(UWORD j=0;j < 16;j++) {
            if (v->variants[j] == 0) {
                break;
            }
            if (v->variants[j] == hash) {
                *out_Vocab = v->variants[0];
                return TRUE;
            }
        }
        v++;
    }
    return TRUE;
}

void SaveVocab() {
    // TODO: Sort
    FILE* f = fopen(VOCAB_DST_PATH, "wb");
    fprintf(f, "UWORD kVocabs[] = {\n");
    Vocab* v = &sVocabs.items[0];
    for(UWORD i=0;i < sVocabs.size;i++) {
        for(UWORD j=0;j < 16;j++) {
            if (v->variants[j] == 0) {
                break;
            }
            fprintf(f, "\t0x%04X, 0x%04X,\n", v->variants[j], v->variants[0]);
        }
        v++;
    }
    fprintf(f, "};\n");
    fclose(f);
}

void ReadVocab() {

    HashKeyWords(kKeyWords);

    Vocabs_new(&sVocabs);
    Vocabs_setCapacity(&sVocabs, 64);
    UWORD kind = -1;

    if (OpenTokFile(VOCAB_SRC_PATH) == FALSE) {
        printf("Could not open vocab source file\n> %s\n", VOCAB_SRC_PATH);
        return;
    }

    Vocab* vocab = NULL;
    UWORD variantIdx = 1;

    while(TRUE)
    {
        ULONG tok = NextTok();
        if (tok == TOK_EOF)
            break;

        // :type
        // vocab()
        // vocab(variant1 variant variantN)

        if (tok == TOK_SECTION) {
            vocab = NULL;
            variantIdx = 1;
            kind = FindKeyWordNum(kKeyWords, TokHash);

            if (kind == 0) {
                printf("Invalid vocab type %s %u\n", TokText, TokHash);
                break;
            }
            continue;
        }

        if (tok == TOK_OPEN) {
            if (vocab == NULL) {
                printf("Vocab syntax error. ( in the wrong place\n");
                break;
            }
            continue;
        }

        if (tok == TOK_CLOSE) {
            if (vocab == NULL) {
                printf("Vocab syntax error. ) in the wrong place\n");
            }
            vocab = NULL;
            variantIdx = 1;
            continue;
        }

        if (tok != TOK_SYMBOL) {
            printf("Vocab syntax error. Expect symbol\n");
            break;
        }

        if (vocab != NULL) {
            vocab->variants[variantIdx++] = TokHash;
        }
        else {
            vocab = Vocabs_push(&sVocabs);
            vocab->kind = kind;
            vocab->variants[0] = TokHash;
            variantIdx = 1;
        }



    }

    CloseTokFile();
}

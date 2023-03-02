typedef struct VNAME {
    T* items;
    ULONG size;
    ULONG capacity;
} VNAME;

static inline VOID X_CAT(FNAME, _clear)(VNAME* self) {
    #if defined(GC)
    for(ULONG i=0;i < self->size;i++) {
        T* thing = &self->items[i];
        X_CAT(T, _free)(thing);
    }
    #endif
    self->size = 0;
}

static inline VOID X_CAT(FNAME, _new)(VNAME* self) {
    self->items = NULL;
    self->size  = 0;
    self->capacity = 0;
}

static inline VOID X_CAT(FNAME, _delete)(VNAME* self) {
    X_CAT(FNAME, _clear)(self);
    if (self->items) {
        xFreeVec(self->items);
    }
    self->capacity = 0;
}

static inline VOID X_CAT(FNAME, _setCapacity)(VNAME* self, ULONG count) {
    if (self->items != NULL) {
        FreeVec(self->items);
    }

    self->items = (T*) AllocVec(count * sizeof(T), MEMF_CLEAR);
    self->size  = 0;
    self->capacity = count;
}

static inline VOID X_CAT(FNAME, _grow)(VNAME* self) {
    ULONG newSize = self->capacity << 1;
    if (newSize == 0) {
        newSize = 32;
    }

    T* newItems = (T*) xAllocVec(newSize * sizeof(T));
    T* items = self->items;

    if (items != NULL && self->size != 0) {
        #if defined(GC)
            for(ULONG i=0;i < self->size;i++) {
               X_CAT(T, _move)(&newItems[i], &items[i]);
            }
        #else
            xCopyMem(items, newItems, self->size * sizeof(T));
        #endif
    }

    if (items != NULL) {
       xFreeVec(items);
    }

    self->items = newItems;
}

static inline T* X_CAT(FNAME, _push)(VNAME* self) {
    if (self->size == self->capacity) {
        X_CAT(FNAME, _grow)(self);
    }

    T* item = &self->items[self->size];
    self->size++;

    return item;
}

static inline VOID X_CAT(FNAME, _pushInto)(VNAME* self, T* item) {
    if (self->size == self->capacity) {
        X_CAT(FNAME, _grow)(self);
    }

    T* dst = &self->items[self->size];
    #if defined(GC)
        X_CAT(T, _move)(dst, item);
    #else
        *dst = *item;
    #endif

    self->size++;
}

static inline VOID X_CAT(FNAME, _moveVec)(VNAME* dst, VNAME* src) {
    dst->items = src->items;
    dst->size  = src->size;
    dst->capacity = src->capacity;

    src->items = NULL;
    src->size  = 0;
    src->capacity = 0;
}

#undef T
#undef VNAME
#undef FNAME
#ifdef GC
#undef GC
#endif

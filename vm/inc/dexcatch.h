
#ifndef LIBDEX_DEXCATCH_H_
#define LIBDEX_DEXCATCH_H_

#include "Dexfile.h"
#include "Leb128.h"

typedef struct DexCatchHandler_S  DexCatchHandler;
typedef struct DexCatchIterator_S DexCatchIterator;

/*
 * Catch handler entry, used while iterating over catch_handler_items.
 */
struct DexCatchHandler_S {
    u4          typeIdx;    /* type index of the caught exception type */
    u4          address;    /* handler address */
};

/*
 * Iterator over catch handler data. This structure should be treated as
 * opaque.
 */
struct DexCatchIterator_S {
    const u1* pEncodedData;
    bool_t catchesAll;
    u4 countRemaining;
    DexCatchHandler handler;
};



/* Get the first handler offset for the given DexCode.
 * It's not 0 because the handlers list is prefixed with its size
 * (in entries) as a uleb128. */
u4 dexGetFirstHandlerOffset(const DexCode* pCode);

/* Get count of handler lists for the given DexCode. */
u4 dexGetHandlersSize(const DexCode* pCode);


/* Initialize a DexCatchIterator to emptiness. This mostly exists to
 * squelch innocuous warnings. */
void dexCatchIteratorClear(DexCatchIterator* pIterator);

/* Initialize a DexCatchIterator with a direct pointer to encoded handlers. */
void dexCatchIteratorInitToPointer(DexCatchIterator* pIterator,
    const u1* pEncodedData);

/* Initialize a DexCatchIterator to a particular handler offset. */
void dexCatchIteratorInit(DexCatchIterator* pIterator,
    const DexCode* pCode, u4 offset);

/* Get the next item from a DexCatchIterator. Returns NULL if at end. */
DexCatchHandler* dexCatchIteratorNext(DexCatchIterator* pIterator);

/* Get the handler offset just past the end of the one just iterated over.
 * This ends the iteration if it wasn't already. */
u4 dexCatchIteratorGetEndOffset(DexCatchIterator* pIterator,
    const DexCode* pCode);

/* Helper for dexFindCatchHandler(). Do not call directly. */
int dexFindCatchHandlerOffset0(u2 triesSize, const DexTry* pTries,
        u4 address);

/* Find the handler associated with a given address, if any.
 * Initializes the given iterator and returns true if a match is
 * found. Returns false if there is no applicable handler. */
bool_t dexFindCatchHandler(DexCatchIterator *pIterator,const DexCode* pCode, u4 address);

#endif  // LIBDEX_DEXCATCH_H_

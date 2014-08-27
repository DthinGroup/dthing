
#include <dexcatch.h>

/* Initialize a DexCatchIterator to emptiness. This mostly exists to
 * squelch innocuous warnings. */
void dexCatchIteratorClear(DexCatchIterator* pIterator) {
    pIterator->pEncodedData = NULL;
    pIterator->catchesAll = FALSE;
    pIterator->countRemaining = 0;
    pIterator->handler.typeIdx = 0;
    pIterator->handler.address = 0;
}

/* Initialize a DexCatchIterator with a direct pointer to encoded handlers. */
void dexCatchIteratorInitToPointer(DexCatchIterator* pIterator,
    const u1* pEncodedData)
{
    s4 count = readSignedLeb128(&pEncodedData);

    if (count <= 0) {
        pIterator->catchesAll = TRUE;
        count = -count;
    } else {
        pIterator->catchesAll = FALSE;
    }

    pIterator->pEncodedData = pEncodedData;
    pIterator->countRemaining = count;
}

/* Initialize a DexCatchIterator to a particular handler offset. */
void dexCatchIteratorInit(DexCatchIterator* pIterator,
    const DexCode* pCode, u4 offset)
{
    dexCatchIteratorInitToPointer(pIterator,
            dexGetCatchHandlerData(pCode) + offset);
}

/* Get the next item from a DexCatchIterator. Returns NULL if at end. */
DexCatchHandler* dexCatchIteratorNext(DexCatchIterator* pIterator) {
    if (pIterator->countRemaining == 0) {
        if (! pIterator->catchesAll) {
            return NULL;
        }

        pIterator->catchesAll = FALSE;
        pIterator->handler.typeIdx = kDexNoIndex;
    } else {
        u4 typeIdx = readUnsignedLeb128(&pIterator->pEncodedData);
        pIterator->handler.typeIdx = typeIdx;
        pIterator->countRemaining--;
    }

    pIterator->handler.address = readUnsignedLeb128(&pIterator->pEncodedData);
    return &pIterator->handler;
}

/* Find the handler associated with a given address, if any.
 * Initializes the given iterator and returns true if a match is
 * found. Returns false if there is no applicable handler. */
bool_t dexFindCatchHandler(DexCatchIterator *pIterator,const DexCode* pCode, u4 address) {
    u2 triesSize = pCode->triesSize;
    int offset = -1;

    // Short-circuit the overwhelmingly common cases.
    switch (triesSize) {
        case 0: {
            break;
        }
        case 1:
        {
            const DexTry* tries = dexGetTries(pCode);
            u4 start = tries[0].startAddr;
            u4 end =0;

            if (address < start) {
                break;
            }

            end = start + tries[0].insnCount;

            if (address >= end) {
                break;
            }

            offset = tries[0].handlerOff;
        }
        break;

        default:
        {
            offset = dexFindCatchHandlerOffset0(triesSize, dexGetTries(pCode),
                    address);
        }
        break;
    }

    if (offset < 0) {
        dexCatchIteratorClear(pIterator); // This squelches warnings.
        return FALSE;
    } else {
        dexCatchIteratorInit(pIterator, pCode, offset);
        return TRUE;
    }
}


/* Get the first handler offset for the given DexCode.
 * It's not 0 because the handlers list is prefixed with its size
 * (in entries) as a uleb128. */
u4 dexGetFirstHandlerOffset(const DexCode* pCode) {
    const u1* baseData;
    const u1* data;
    if (pCode->triesSize == 0) {
        return 0;
    }

    baseData = dexGetCatchHandlerData(pCode);
    data = baseData;

    readUnsignedLeb128(&data);

    return data - baseData;
}

/* Get count of handler lists for the given DexCode. */
u4 dexGetHandlersSize(const DexCode* pCode) {
    const u1* data;
    if (pCode->triesSize == 0) {
        return 0;
    }

    data = dexGetCatchHandlerData(pCode);

    return readUnsignedLeb128(&data);
}

/* Helper for dexFindCatchHandlerOffset(), which does an actual search
 * in the tries table. Returns -1 if there is no applicable handler. */
int dexFindCatchHandlerOffset0(u2 triesSize, const DexTry* pTries,
        u4 address) {
    // Note: Signed type is important for max and min.
    int min = 0;
    int max = triesSize - 1;

    while (max >= min) {
        int guess = (min + max) >> 1;
        const DexTry* pTry = &pTries[guess];
        u4 start = pTry->startAddr;
        u4 end =0;

        if (address < start) {
            max = guess - 1;
            continue;
        }

        end = start + pTry->insnCount;

        if (address >= end) {
            min = guess + 1;
            continue;
        }

        // We have a winner!
        return (int) pTry->handlerOff;
    }

    // No match.
    return -1;
}

/* Get the handler offset just past the end of the one just iterated over.
 * This ends the iteration if it wasn't already. */
u4 dexCatchIteratorGetEndOffset(DexCatchIterator* pIterator,
        const DexCode* pCode) {
    while (dexCatchIteratorNext(pIterator) != NULL) /* empty */ ;

    return (u4) (pIterator->pEncodedData - dexGetCatchHandlerData(pCode));
}

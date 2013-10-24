#include <dthing.h>
#include "interpStack.h"
#include "Object.h"


vbool dvmIsBreakFrame(const u4* fp)
{
    return SAVEAREA_FROM_FP(fp)->method == NULL;
}

/*
 * Get the calling frame.  Pass in the current fp.
 *
 * Skip "break" frames and reflection invoke frames.
 */
void* dvmGetCallerFP(const void* curFrame)
{
    void* caller = SAVEAREA_FROM_FP(curFrame)->prevFrame;
    StackSaveArea* saveArea;

retry:
    if (dvmIsBreakFrame(caller)) {
        /* pop up one more */
        caller = SAVEAREA_FROM_FP(caller)->prevFrame;
        if (caller == NULL)
            return NULL;        /* hit the top */

        /*
         * If we got here by java.lang.reflect.Method.invoke(), we don't
         * want to return Method's class loader.  Shift up one and try
         * again.
         */
        saveArea = SAVEAREA_FROM_FP(caller);
        if (dvmIsReflectionMethod(saveArea->method)) {
            caller = saveArea->prevFrame;
            assert(caller != NULL);
            goto retry;
        }
    }

    return caller;
}


/*
 * Get the caller's caller's class.  Pass in the current fp.
 *
 * This is used by e.g. java.lang.Class, which wants to know about the
 * class loader of the method that called it.
 */
ClassObject* dvmGetCaller2Class(const void* curFrame)
{
    void* caller = SAVEAREA_FROM_FP(curFrame)->prevFrame;
    void* callerCaller;

    /* at the top? */
    if (dvmIsBreakFrame(caller) && SAVEAREA_FROM_FP(caller)->prevFrame == NULL)
        return NULL;

    /* go one more */
    callerCaller = dvmGetCallerFP(caller);
    if (callerCaller == NULL)
        return NULL;

    return SAVEAREA_FROM_FP(callerCaller)->method->clazz;
}


vbool dvmPushInterpFrame(Thread* self, const Method* method)
{
	StackSaveArea* saveBlock = NULL;
    StackSaveArea* breakSaveBlock = NULL;
    int stackReq;
    u1* stackPtr = NULL;

#ifdef __CANCEL_NOTE__
    assert(!dvmIsNativeMethod(method));
    assert(!dvmIsAbstractMethod(method));
#endif
    stackReq = method->registersSize * 4        // params + locals
                + sizeof(StackSaveArea) * 2     // break frame + regular frame
                + method->outsSize * 4;         // args to other methods

    if (self->interpSave.curFrame != NULL)
        stackPtr = (u1*) SAVEAREA_FROM_FP(self->interpSave.curFrame);
    else
        stackPtr = self->interpStackStart;

    if (stackPtr - stackReq < self->interpStackEnd) 
	{
        /* not enough space */
#ifdef __CANCEL_NOTE__
        LOGW("Stack overflow on call to interp "
             "(req=%d top=%p cur=%p size=%d %s.%s)",
            stackReq, self->interpStackStart, self->interpSave.curFrame,
            self->interpStackSize, method->clazz->descriptor, method->name);
        dvmHandleStackOverflow(self, method);
        assert(dvmCheckException(self));
#endif
        return false;
    }

    /*
     * Shift the stack pointer down, leaving space for the function's
     * args/registers and save area.
     */
    stackPtr -= sizeof(StackSaveArea);
    breakSaveBlock = (StackSaveArea*)stackPtr;
    stackPtr -= method->registersSize * 4 + sizeof(StackSaveArea);
    saveBlock = (StackSaveArea*) stackPtr;

#if !defined(NDEBUG) && !defined(PAD_SAVE_AREA)
    /* debug -- memset the new stack, unless we want valgrind's help */
    memset(stackPtr - (method->outsSize*4), 0xaf, stackReq);
#endif
#ifdef EASY_GDB
    breakSaveBlock->prevSave =
       (StackSaveArea*)FP_FROM_SAVEAREA(self->interpSave.curFrame);
    saveBlock->prevSave = breakSaveBlock;
#endif

    breakSaveBlock->prevFrame = self->interpSave.curFrame;
    breakSaveBlock->savedPc = NULL;             // not required
    breakSaveBlock->xtra.localRefCookie = 0;    // not required
    breakSaveBlock->method = NULL;
    saveBlock->prevFrame = FP_FROM_SAVEAREA(breakSaveBlock);
    saveBlock->savedPc = NULL;                  // not required
    saveBlock->xtra.currentPc = NULL;           // not required?
    saveBlock->method = method;

    self->interpSave.curFrame = FP_FROM_SAVEAREA(saveBlock);

    return true;
}

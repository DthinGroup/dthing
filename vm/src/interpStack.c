#include "interpStack.h"
#include "Object.h"


vbool dvmIsBreakFrame(const u4* fp)
{
    return SAVEAREA_FROM_FP(fp)->method == NULL;
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
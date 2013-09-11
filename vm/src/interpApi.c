

#include <interpApi.h>
#include <interpState.h>




void dvmInterpretEntry(Thread * self)
{
    InterpSaveState interpSaveState;
    ExecutionSubModes savedSubModes;
	Method * method;
	JValue * pResult;
    /*
     * Save interpreter state from previous activation, linking
     * new to last.
     */
    interpSaveState = self->interpSave;
    self->interpSave.prev = &interpSaveState;
    /*
     * Strip out and save any flags that should not be inherited by
     * nested interpreter activation.
     */
    savedSubModes = (ExecutionSubModes)(
              self->interpBreak.ctl.subMode & LOCAL_SUBMODE);
    if (savedSubModes != kSubModeNormal) {
#if  __NIX__
        dvmDisableSubMode(self, savedSubModes);
#endif
    }


#if defined(WITH_TRACKREF_CHECKS)
    self->interpSave.debugTrackedRefStart =
        dvmReferenceTableEntries(&self->internalLocalRefTable);
#endif
    self->debugIsMethodEntry = true;


    /*
     * method must be set in advance
     */
	method = self->interpSave.method;
    self->interpSave.curFrame = (u4*) self->interpSave.curFrame;
    self->interpSave.pc = method->insns;

    assert(!dvmIsNativeMethod(method));

#if __NIX__
    /*
     * Make sure the class is ready to go.  Shouldn't be possible to get
     * here otherwise.
     */
    if (method->clazz->status < CLASS_INITIALIZING ||
        method->clazz->status == CLASS_ERROR)
    {
        dvmDumpThread(self, false);
        dvmAbort();
    }
#endif

    // Call the interpreter
    dvmInterpretPortable(self);

    *pResult = self->interpSave.retval;

    /* Restore interpreter state from previous activation */
    self->interpSave = interpSaveState;

#if __NIX__
    if (savedSubModes != kSubModeNormal) 
	{
        dvmEnableSubMode(self, savedSubModes);
    }	
#endif
}

#include <vm_common.h>
#include <interpApi.h>
#include <interpState.h>
#include <dthread.h>
#include <kni.h>
#include <Object.h>


void dvmInterpretEntry(Thread * self,JValue *pResult)
{
    InterpSaveState interpSaveState;
    ExecutionSubModes savedSubModes;
	Method * method;
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
	if(self->bInterpFirst)
	{
		method = (Method *)self->entryMthd;
		self->interpSave.method = method;
		self->interpSave.pc = method->insns;
	}
	else
	{
		method = (struct Method *)self->interpSave.method;
		//self->interpSave.pc = method->insns;
	}	
    self->interpSave.curFrame = (u4*) self->interpSave.curFrame;
    

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
    //self->interpSave = interpSaveState;

#if __NIX__
    if (savedSubModes != kSubModeNormal) 
	{
        dvmEnableSubMode(self, savedSubModes);
    }	
#endif
}

#define SUPPORT_MAX_PARAMS  10

void dvmInterpretMakeNativeCall(const u4* args, JValue* pResult, const Method* method, Thread* self)
{
	KniFunc nativefunc = NULL;
	Object * thisClz = NULL;   //if static: class ptr;if non static :instance this ptr
	u4 argvs[SUPPORT_MAX_PARAMS] = {0};
	int idx = 0;
	int offset = 0;

	if(dvmIsStaticMethod(method))
	{
		/*nix: maybe error!*/
		thisClz = (Object*) method->clazz;
	}
	else
	{
		thisClz = ((Object*) args)->clazz ;  //the first param of Fp is "this ptr"
		offset++;
	}
	argvs[idx++] = (u4) thisClz;

	/*prase other params to argvs*/
	{
        const char* shorty = &method->shorty[1];        /* skip return type */
        while (*shorty != '\0') 
		{
#if 1
			argvs[idx] = (u4) args[offset];
			idx++;
			offset++;
			shorty++;
#else			
            switch (*shorty++) 
			{
            case 'L':
                printf("  local %d: 0x%08x", idx, args[offset]);
				argvs[idx] = (u4) args[offset];
				idx++;
                break;
            case 'D':
            case 'J':
                //idx++;
                break;
            default:
                /* Z B C S I -- do nothing */
                break;
            }
            //idx++;
			offset++;
#endif
        }
    }

	nativefunc = Kni_findFuncPtr(method);
	DVM_ASSERT(nativefunc != NULL);
	nativefunc(argvs,pResult);

}

/**/
void dvmCallClinitMethod(const Method* method, Object* obj)
{
	JValue pResult;
	dthread_fill_ghost(method,obj);
	dvmInterpretEntry(ghostThread,&pResult);
}
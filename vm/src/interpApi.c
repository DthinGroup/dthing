
#include <vm_common.h>
#include <interpApi.h>
#include <interpState.h>
#include <interpOpcode.h>
#include <dthread.h>
#include <kni.h>
#include <Object.h>

#include <stdarg.h>
#ifdef ARCH_ARM_SPD
#include <cafstdlib.h>
#endif

#ifdef DVM_LOG
#undef DVM_LOG
#endif

#define DVM_LOG		DVMTraceInf

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


u4 dvmFloatToU4(float val) {
    union { float in; u4 out; } conv;
    conv.in = val;
    return conv.out;
}

void dvmCallInitMethod(const Method* method, Object* obj, ...)
{
    va_list args;
    va_start(args, obj);
    {
        const char* desc = &(method->shorty[1]); // [0] is the return type.
        int verifyCount = 0; //for debug
    	JValue pResult;
        u4* ins;

        /* "ins" for new frame start at frame pointer plus locals */
        ins = ((u4*)ghostThread->interpSave.curFrame) +
               (method->registersSize - method->insSize);

        /* put "this" pointer into in0 if appropriate */
        if (!dvmIsStaticMethod(method))
        {
            *ins++ = (u4) obj;
            verifyCount++;
        }

        while (*desc != '\0')
        {
            switch (*(desc++)) {
                case 'D': case 'J': {
                    u8 val = va_arg(args, u8);
                    CRTL_memcpy(ins, &val, 8);       // EABI prevents direct store
                    ins += 2;
                    verifyCount += 2;
                    break;
                }
                case 'F': {
                    /* floats were normalized to doubles; convert back */
                    float f = (float) va_arg(args, double);
                    *ins++ = dvmFloatToU4(f);
                    verifyCount++;
                    break;
                }
                case 'L': {     /* 'shorty' descr uses L for all refs, incl array */
                    void* argObj = va_arg(args, void*);
                    *ins++ = (u4) argObj;
                    verifyCount++;
                    break;
                }
                default: {
                    /* Z B C S I -- all passed as 32-bit integers */
                    *ins++ = va_arg(args, u4);
                    verifyCount++;
                    break;
                }
            }
        }

        //dvmDumpThreadStack(dvmThreadSelf());

    	dthread_fill_ghost(method,obj);
    	dvmInterpretEntry(ghostThread,&pResult);
    }
    va_end(args);
}


/*
 * Copy data for a fill-array-data instruction.  On a little-endian machine
 * we can just do a memcpy(), on a big-endian system we have work to do.
 *
 * The trick here is that dexopt has byte-swapped each code unit, which is
 * exactly what we want for short/char data.  For byte data we need to undo
 * the swap, and for 4- or 8-byte values we need to swap pieces within
 * each word.
 */
static void copySwappedArrayData(void* dest, const u2* src, u4 size, u2 width)
{
#ifdef DVM_BIG_ENDIAN
    int i;

    switch (width) {
    case 1:
        /* un-swap pairs of bytes as we go */
        for (i = (size-1) & ~1; i >= 0; i -= 2) {
            ((u1*)dest)[i] = ((u1*)src)[i+1];
            ((u1*)dest)[i+1] = ((u1*)src)[i];
        }
        /*
         * "src" is padded to end on a two-byte boundary, but we don't want to
         * assume "dest" is, so we handle odd length specially.
         */
        if ((size & 1) != 0) {
            ((u1*)dest)[size-1] = ((u1*)src)[size];
        }
        break;
    case 2:
        /* already swapped correctly */
        CRTL_memcpy(dest, src, size*width);
        break;
    case 4:
        /* swap word halves */
        for (i = 0; i < (int) size; i++) {
            ((u4*)dest)[i] = (src[(i << 1) + 1] << 16) | src[i << 1];
        }
        break;
    case 8:
        /* swap word halves and words */
        for (i = 0; i < (int) (size << 1); i += 2) {
            ((int*)dest)[i] = (src[(i << 1) + 3] << 16) | src[(i << 1) + 2];
            ((int*)dest)[i+1] = (src[(i << 1) + 1] << 16) | src[i << 1];
        }
        break;
    default:
        DVMTraceErr("Unexpected width %d in copySwappedArrayData", width);
        dvmAbort();
        break;
    }
#else
    CRTL_memcpy(dest, src, size*width);
#endif
}


/*
 * Fill the array with predefined constant values.
 *
 * Returns true if job is completed, otherwise false to indicate that
 * an exception has been thrown.
 */
bool_t dvmInterpHandleFillArrayData(ArrayObject* arrayObj, const u2* arrayData)
{
    u2 width;
    u4 size;

    if (arrayObj == NULL) {
        dvmThrowNullPointerException(NULL);
        return FALSE;
    }
    //assert (!IS_CLASS_FLAG_SET(((Object *)arrayObj)->clazz,
    //                           CLASS_ISOBJECTARRAY));

    /*
     * Array data table format:
     *  ushort ident = 0x0300   magic value
     *  ushort width            width of each element in the table
     *  uint   size             number of elements in the table
     *  ubyte  data[size*width] table of data values (may contain a single-byte
     *                          padding at the end)
     *
     * Total size is 4+(width * size + 1)/2 16-bit code units.
     */
    if (arrayData[0] != kArrayDataSignature) {
        dvmThrowInternalError("bad array data magic");
        return FALSE;
    }

    width = arrayData[1];
    size = arrayData[2] | (((u4)arrayData[3]) << 16);

    if (size > arrayObj->length) {
        dvmThrowArrayIndexOutOfBoundsException(arrayObj->length, size);
        return FALSE;
    }
    copySwappedArrayData(arrayObj->contents, &arrayData[4], size, width);
    return TRUE;
}

/*
 * Construct an s4 from two consecutive half-words of switch data.
 * This needs to check endianness because the DEX optimizer only swaps
 * half-words in instruction stream.
 *
 * "switchData" must be 32-bit aligned.
 */
#if __BYTE_ORDER == __LITTLE_ENDIAN
static  s4 s4FromSwitchData(const void* switchData) 
{
    return *(s4*) switchData;
}
#else
static  s4 s4FromSwitchData(const void* switchData) {
    u2* data = switchData;
    return data[0] | (((s4) data[1]) << 16);
}
#endif


/*
 * Switch table and array data signatures are a code unit consisting
 * of "NOP" (0x00) in the low-order byte and a non-zero identifying
 * code in the high-order byte. (A true NOP is 0x0000.)
 */
#define kPackedSwitchSignature  0x0100
#define kSparseSwitchSignature  0x0200
#define kArrayDataSignature     0x0300

s4 dvmInterpHandlePackedSwitch(const u2* switchData, s4 testVal)
{
	const int kInstrLen = 3;
    u2 size;
    s4 firstKey;
    const s4* entries;

	DVM_LOG(">>>call dvmInterpHandlePackedSwitch!\n");
    /*
     * Packed switch data format:
     *  ushort ident = 0x0100   magic value
     *  ushort size             number of entries in the table
     *  int first_key           first (and lowest) switch case value
     *  int targets[size]       branch targets, relative to switch opcode
     *
     * Total size is (4+size*2) 16-bit code units.
     */
    if (*switchData++ != kPackedSwitchSignature) 
	{
        /* should have been caught by verifier */
        dvmThrowInternalError("bad packed switch magic");
        return kInstrLen;
    }

    size = *switchData++;
    DVM_ASSERT(size > 0);

    firstKey = *switchData++;
    firstKey |= (*switchData++) << 16;

    if (testVal < firstKey || testVal >= firstKey + size) 
	{
        DVM_LOG("Value %d not found in switch (%d-%d)",testVal, firstKey, firstKey+size-1);
        return kInstrLen;
    }

    /* The entries are guaranteed to be aligned on a 32-bit boundary;
     * we can treat them as a native int array.
     */
    entries = (const s4*) switchData;
    DVM_ASSERT(((u4)entries & 0x3) == 0);

    DVM_ASSERT(testVal - firstKey >= 0 && testVal - firstKey < size);
    DVM_LOG("Value %d found in slot %d (goto 0x%02x)",testVal, testVal - firstKey,s4FromSwitchData(&entries[testVal - firstKey]));
    return s4FromSwitchData(&entries[testVal - firstKey]);

}

/*
 * Find the matching case.  Returns the offset to the handler instructions.
 *
 * Returns 3 if we don't find a match (it's the size of the sparse-switch
 * instruction).
 */
s4 dvmInterpHandleSparseSwitch(const u2* switchData, s4 testVal)
{
    int lo = 0;
    int hi;
    const int kInstrLen = 3;
    u2 size;
    const s4* keys;
    const s4* entries;

    /*
     * Sparse switch data format:
     *  ushort ident = 0x0200   magic value
     *  ushort size             number of entries in the table; > 0
     *  int keys[size]          keys, sorted low-to-high; 32-bit aligned
     *  int targets[size]       branch targets, relative to switch opcode
     *
     * Total size is (2+size*4) 16-bit code units.
     */

    if (*switchData++ != kSparseSwitchSignature) {
        /* should have been caught by verifier */
        dvmThrowInternalError("bad sparse switch magic");
        return kInstrLen;
    }

    size = *switchData++;
    assert(size > 0);

    /* The keys are guaranteed to be aligned on a 32-bit boundary;
     * we can treat them as a native int array.
     */
    keys = (const s4*) switchData;
    assert(((u4)keys & 0x3) == 0);

    /* The entries are guaranteed to be aligned on a 32-bit boundary;
     * we can treat them as a native int array.
     */
    entries = keys + size;
    assert(((u4)entries & 0x3) == 0);

    /*
     * Binary-search through the array of keys, which are guaranteed to
     * be sorted low-to-high.
     */
    hi = size - 1;
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;

        s4 foundVal = s4FromSwitchData(&keys[mid]);
        if (testVal < foundVal) {
            hi = mid - 1;
        } else if (testVal > foundVal) {
            lo = mid + 1;
        } else {
        
            DVM_LOG("Value %d found in entry %d (goto 0x%02x)",
                testVal, mid, s4FromSwitchData(&entries[mid]));
            return s4FromSwitchData(&entries[mid]);
        }
    }

    DVM_LOG("Value %d not found in switch", testVal);
    return kInstrLen;
}

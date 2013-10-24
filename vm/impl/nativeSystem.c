/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/09/22 $
 * Last modified:	$Date: 2013/09/22 $
 * Version:         $ID: nativeSystem.c#1
 */

/**
 * The implementation of java.lang.System.
 */

#include <nativeSystem.h>
#include <typecheck.h>
#include <array.h>
#include <exception.h>
/* porting layer apis */
#include <opl_core.h>


static void move16(void* dest, const void* src, size_t n)
{
    //assert((((uintptr_t) dest | (uintptr_t) src | n) & 0x01) == 0);

    uint16_t* d = (uint16_t*) dest;
    const uint16_t* s = (uint16_t*) src;

    n /= sizeof(uint16_t);

    if (d < s) {
        /* copy forward */
        while (n--) {
            *d++ = *s++;
        }
    } else {
        /* copy backward */
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
}

static void move32(void* dest, const void* src, size_t n)
{
    //assert((((uintptr_t) dest | (uintptr_t) src | n) & 0x03) == 0);

    uint32_t* d = (uint32_t*) dest;
    const uint32_t* s = (uint32_t*) src;

    n /= sizeof(uint32_t);

    if (d < s) {
        /* copy forward */
        while (n--) {
            *d++ = *s++;
        }
    } else {
        /* copy backward */
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
}

/* see nativeSystem.h */
void Java_java_lang_System_arrayCopy(const u4* args, JValue* pResult)
{
    ArrayObject* srcArray = (ArrayObject*) args[1];
    int srcPos = args[2];
    ArrayObject* dstArray = (ArrayObject*) args[3];
    int dstPos = args[4];
    int length = args[5];

    ClassObject* srcClass;
    ClassObject* dstClass;
    char         srcType;
    char         dstType;
    bool_t       srcPrim;
    bool_t       dstPrim;

    /* Check for null pointers. */
    if (srcArray == NULL) {
        dvmThrowNullPointerException("src == null");
        RETURN_VOID();
    }
    if (dstArray == NULL) {
        dvmThrowNullPointerException("dst == null");
        RETURN_VOID();
    }

    /* Make sure source and destination are arrays. */
    if (!dvmIsArray(srcArray)) {
        dvmThrowArrayStoreExceptionNotArray(((Object*)srcArray)->clazz, "source");
        RETURN_VOID();
    }
    if (!dvmIsArray(dstArray)) {
        dvmThrowArrayStoreExceptionNotArray(((Object*)dstArray)->clazz, "destination");
        RETURN_VOID();
    }

    /* avoid int overflow */
    if (srcPos < 0 || dstPos < 0 || length < 0 ||
        srcPos > (int) srcArray->length - length ||
        dstPos > (int) dstArray->length - length)
    {
        dvmThrowException("Ljava/lang/ArrayIndexOutOfBoundsException;", NULL);
        RETURN_VOID();
    }

    srcClass = srcArray->obj.clazz;
    dstClass = dstArray->obj.clazz;
    srcType = srcClass->descriptor[1];
    dstType = dstClass->descriptor[1];

    /*
     * If one of the arrays holds a primitive type, the other array must
     * hold the same type.
     */
    srcPrim = (srcType != '[' && srcType != 'L');
    dstPrim = (dstType != '[' && dstType != 'L');
    if (srcPrim || dstPrim) {
        if (srcPrim != dstPrim || srcType != dstType) {
            dvmThrowException("Ljava/lang/ArrayStoreException;", NULL);
            RETURN_VOID();
        }

        DVMTraceInf("arraycopy prim[%c] dst=%p %d src=%p %d len=%d\n",
            srcType, dstArray->contents, dstPos,
            srcArray->contents, srcPos, length);

        switch (srcType) {
        case 'B':
        case 'Z':
            /* 1 byte per element */
            CRTL_memmove((u1*) dstArray->contents + dstPos,
                (const u1*) srcArray->contents + srcPos,
                length);
            break;
        case 'C':
        case 'S':
            /* 2 bytes per element */
            move16((u1*) dstArray->contents + dstPos * 2,
                (const u1*) srcArray->contents + srcPos * 2,
                length * 2);
            break;
        case 'F':
        case 'I':
            /* 4 bytes per element */
            move32((u1*) dstArray->contents + dstPos * 4,
                (const u1*) srcArray->contents + srcPos * 4,
                length * 4);
            break;
        case 'D':
        case 'J':
            /*
             * 8 bytes per element.  We don't need to guarantee atomicity
             * of the entire 64-bit word, so we can use the 32-bit copier.
             */
            move32((u1*) dstArray->contents + dstPos * 8,
                (const u1*) srcArray->contents + srcPos * 8,
                length * 8);
            break;
        default:        /* illegal array type */
            DVMTraceErr("Weird array type '%s'", srcClass->descriptor);
            dvmAbort();
        }
    } else {
        /*
         * Neither class is primitive.  See if elements in "src" are instances
         * of elements in "dst" (e.g. copy String to String or String to
         * Object).
         */
        const int width = sizeof(Object*);

        if (srcClass->arrayDim == dstClass->arrayDim &&
            dvmInstanceof(srcClass, dstClass))
        {
            /*
             * "dst" can hold "src"; copy the whole thing.
             */
            DVMTraceInf("arraycopy ref dst=%p %d src=%p %d len=%d\n",
                dstArray->contents, dstPos * width,
                srcArray->contents, srcPos * width,
                length * width);
            move32((u1*)dstArray->contents + dstPos * width,
                (const u1*)srcArray->contents + srcPos * width,
                length * width);
            dvmWriteBarrierArray(dstArray, dstPos, dstPos+length);
        } else {
            /*
             * The arrays are not fundamentally compatible.  However, we
             * may still be able to do this if the destination object is
             * compatible (e.g. copy Object[] to String[], but the Object
             * being copied is actually a String).  We need to copy elements
             * one by one until something goes wrong.
             *
             * Because of overlapping moves, what we really want to do
             * is compare the types and count up how many we can move,
             * then call move32() to shift the actual data.  If we just
             * start from the front we could do a smear rather than a move.
             */
            Object** srcObj;
            int copyCount;
            ClassObject*   clazz = NULL;

            srcObj = ((Object**)(void*)srcArray->contents) + srcPos;

            if (length > 0 && srcObj[0] != NULL)
            {
                clazz = srcObj[0]->clazz;
                if (!dvmCanPutArrayElement(clazz, dstClass))
                    clazz = NULL;
            }

            for (copyCount = 0; copyCount < length; copyCount++)
            {
                if (srcObj[copyCount] != NULL &&
                    srcObj[copyCount]->clazz != clazz &&
                    !dvmCanPutArrayElement(srcObj[copyCount]->clazz, dstClass))
                {
                    /* can't put this element into the array */
                    break;
                }
            }

            DVMTraceInf("arraycopy iref dst=%p %d src=%p %d count=%d of %d\n",
                dstArray->contents, dstPos * width,
                srcArray->contents, srcPos * width,
                copyCount, length);
            move32((u1*)dstArray->contents + dstPos * width,
                (const u1*)srcArray->contents + srcPos * width,
                copyCount * width);
            dvmWriteBarrierArray(dstArray, 0, copyCount);
            if (copyCount != length) {
                dvmThrowException("Ljava/lang/ArrayStoreException;", NULL);
                RETURN_VOID();
            }
        }
    }

    RETURN_VOID();

}

/* see nativeSystem.h */
void Java_java_lang_System_currentTimeMillis(const u4* args, JValue* pResult)
{
    int64_t curTime = OPL_core_getCurrentTimeMillis();

    RETURN_LONG(curTime);    
}

/* see nativeSystem.h */
void Java_java_lang_System_identityHashCode(const u4* args, JValue* pResult)
{
    u4      iHashCode = 0;
    Object* thisPtr = (Object*) args[1];
    
    iHashCode = (u4)thisPtr;
    
    RETURN_INT(iHashCode);
}

/* see nativeSystem.h */
void Java_com_yarlungsoft_util_SystemPrintSteam_write(const u4* args, JValue* pResult)
{
    int ch = (int)args[1];
    OPL_core_logChar(ch);
    RETURN_VOID();
}

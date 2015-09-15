/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * Exception handling.
 */

#include <dthing.h>
#include <dthread.h>
#include <exception.h>
#include <gc.h>
#include <utfstring.h>
#include <interpApi.h>
#include <dexcatch.h>
#include <dvmdex.h>

/*
Notes on Exception Handling

We have one fairly sticky issue to deal with: creating the exception stack
trace.  The trouble is that we need the current value of the program
counter for the method now being executed, but that's only held in a local
variable or hardware register in the main interpreter loop.

The exception mechanism requires that the current stack trace be associated
with a Throwable at the time the Throwable is constructed.  The construction
may or may not be associated with a throw.  We have three situations to
consider:

 (1) A Throwable is created with a "new Throwable" statement in the
     application code, for immediate or deferred use with a "throw" statement.
 (2) The VM throws an exception from within the interpreter core, e.g.
     after an integer divide-by-zero.
 (3) The VM throws an exception from somewhere deeper down, e.g. while
     trying to link a class.

We need to have the current value for the PC, which means that for
situation (3) the interpreter loop must copy it to an externally-accessible
location before handling any opcode that could cause the VM to throw
an exception.  We can't store it globally, because the various threads
would trample each other.  We can't store it in the Thread structure,
because it'll get overwritten as soon as the Throwable constructor starts
executing.  It needs to go on the stack, but our stack frames hold the
caller's *saved* PC, not the current PC.

Situation #1 doesn't require special handling.  Situation #2 could be dealt
with by passing the PC into the exception creation function.  The trick
is to solve situation #3 in a way that adds minimal overhead to common
operations.  Making it more costly to throw an exception is acceptable.

There are a few ways to deal with this:

 (a) Change "savedPc" to "currentPc" in the stack frame.  All of the
     stack logic gets offset by one frame.  The current PC is written
     to the current stack frame when necessary.
 (b) Write the current PC into the current stack frame, but without
     replacing "savedPc".  The JNI local refs pointer, which is only
     used for native code, can be overloaded to save space.
 (c) In dvmThrowException(), push an extra stack frame on, with the
     current PC in it.  The current PC is written into the Thread struct
     when necessary, and copied out when the VM throws.
 (d) Before doing something that might throw an exception, push a
     temporary frame on with the saved PC in it.

Solution (a) is the simplest, but breaks Dalvik's goal of mingling native
and interpreted stacks.

Solution (b) retains the simplicity of (a) without rearranging the stack,
but now in some cases we're storing the PC twice, which feels wrong.

Solution (c) usually works, because we push the saved PC onto the stack
before the Throwable construction can overwrite the copy in Thread.  One
way solution (c) could break is:
 - Interpreter saves the PC
 - Execute some bytecode, which runs successfully (and alters the saved PC)
 - Throw an exception before re-saving the PC (i.e in the same opcode)
This is a risk for anything that could cause <clinit> to execute, e.g.
executing a static method or accessing a static field.  Attemping to access
a field that doesn't exist in a class that does exist might cause this.
It may be possible to simply bracket the dvmCallMethod*() functions to
save/restore it.

Solution (d) incurs additional overhead, but may have other benefits (e.g.
it's easy to find the stack frames that should be removed before storage
in the Throwable).

Current plan is option (b), because it's simple, fast, and doesn't change
the way the stack works.
*/


/*
 * Return a newly-allocated string for the internal-form class name for
 * the given type descriptor. That is, the initial "L" and final ";" (if
 * any) have been removed.
 */
static char* dvmDescriptorToName(const char* str)
{
    if (str[0] == 'L') {
        size_t length = CRTL_strlen(str) - 1;
        char* newStr = (char*)CRTL_malloc(length + 1);

        if (newStr == NULL) {
            return NULL;
        }
		CRTL_memset(newStr, 0, length +1);
        CRTL_memcpy(newStr, str + 1, length);
        return newStr;
    }

    return CRTL_strdup(str);
}
static void dvmThrowExceptionFmtV(ClassObject* exceptionClass,
    const char* fmt, va_list args)
{
    char msgBuf[512];

    vsnprintf(msgBuf, sizeof(msgBuf), fmt, args);
    dvmThrowChainedException(exceptionClass, msgBuf, NULL);
}

static void dvmThrowExceptionFmt(ClassObject* exceptionClass,
    const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    dvmThrowExceptionFmtV(exceptionClass, fmt, args);
    va_end(args);
}

static void throwTypeError(ClassObject* exceptionClass, const char* fmt,
    ClassObject* actual, ClassObject* desired)
{   
	char * actual_str = dvmDescriptorToName(actual->descriptor);
	char * desired_str = dvmDescriptorToName(desired->descriptor);
    dvmThrowExceptionFmt(exceptionClass, fmt, actual_str, desired_str);
}

/*
 * Returns "true" if an exception is pending.  Use this if you have a
 * "self" pointer.
 */
bool_t dvmCheckException(Thread* self)
{
    return (self->exception != NULL);
}


/*
 * Find and return an exception constructor method that can take the
 * indicated parameters, or return NULL if no such constructor exists.
 */
static Method* findExceptionInitMethod(ClassObject* excepClass, bool_t hasMessage, bool_t hasCause)
{
    if (hasMessage) {
        Method* result;

        if (hasCause) {
            result = dvmFindDirectMethodByDescriptor(
                    excepClass, "<init>",
                    "(Ljava/lang/String;Ljava/lang/Throwable;)V");
        } else {
            result = dvmFindDirectMethodByDescriptor(
                    excepClass, "<init>", "(Ljava/lang/String;)V");
        }

        if (result != NULL) {
            return result;
        }

        if (hasCause) {
            return dvmFindDirectMethodByDescriptor(
                    excepClass, "<init>",
                    "(Ljava/lang/Object;Ljava/lang/Throwable;)V");
        } else {
            return dvmFindDirectMethodByDescriptor(
                    excepClass, "<init>", "(Ljava/lang/Object;)V");
        }
    } else if (hasCause) {
        return dvmFindDirectMethodByDescriptor(
                excepClass, "<init>", "(Ljava/lang/Throwable;)V");
    } else {
        return dvmFindDirectMethodByDescriptor(excepClass, "<init>", "()V");
    }
}


/*
 * Initialize an exception with an appropriate constructor.
 *
 * "exception" is the exception object to initialize.
 * Either or both of "msg" and "cause" may be null.
 * "self" is dvmThreadSelf(), passed in so we don't have to look it up again.
 *
 * If the process of initializing the exception causes another
 * exception (e.g., OutOfMemoryError) to be thrown, return an error
 * and leave self->exception intact.
 */
static bool_t initException(Object* exception, const char* msg, Object* cause, Thread* self)
{
    InitKind initExcept = kInitUnknown;
    Method* initMethod = NULL;
    ClassObject* excepClass = exception->clazz;
    ClassObject* exThrowable = NULL;
    StringObject* msgStr = NULL;
    bool_t result = FALSE;
    bool_t needInitCause = FALSE;

    /* if we have a message, create a String */
    if (msg == NULL)
        msgStr = NULL;
    else {
        msgStr = dvmCreateStringFromCstr(msg);
        if (msgStr == NULL) {
            DVMTraceWar("Could not allocate message string \"%s\" while "
                    "throwing internal exception (%s)",
                    msg, excepClass->descriptor);
            goto bail;
        }
    }

    if (cause != NULL) {
        exThrowable = dvmFindSystemClassNoInit("Ljava/lang/Throwable;");
        if (exThrowable == NULL || !dvmInstanceof(cause->clazz, exThrowable)) {
            DVMTraceErr("Tried to init exception with cause '%s'",
                cause->clazz->descriptor);
            dvmAbort();
        }
    }

    /*
     * The Throwable class has four public constructors:
     *  (1) Throwable()
     *  (2) Throwable(String message)
     *  (3) Throwable(String message, Throwable cause)  (added in 1.4)
     *  (4) Throwable(Throwable cause)                  (added in 1.4)
     *
     * The first two are part of the original design, and most exception
     * classes should support them.  The third prototype was used by
     * individual exceptions. e.g. ClassNotFoundException added it in 1.2.
     * The general "cause" mechanism was added in 1.4.  Some classes,
     * such as IllegalArgumentException, initially supported the first
     * two, but added the second two in a later release.
     *
     * Exceptions may be picky about how their "cause" field is initialized.
     * If you call ClassNotFoundException(String), it may choose to
     * initialize its "cause" field to null.  Doing so prevents future
     * calls to Throwable.initCause().
     *
     * So, if "cause" is not NULL, we need to look for a constructor that
     * takes a throwable.  If we can't find one, we fall back on calling
     * #1/#2 and making a separate call to initCause().  Passing a null ref
     * for "message" into Throwable(String, Throwable) is allowed, but we
     * prefer to use the Throwable-only version because it has different
     * behavior.
     *
     * java.lang.TypeNotPresentException is a strange case -- it has #3 but
     * not #2.  (Some might argue that the constructor is actually not #3,
     * because it doesn't take the message string as an argument, but it
     * has the same effect and we can work with it here.)
     *
     * java.lang.AssertionError is also a strange case -- it has a
     * constructor that takes an Object, but not one that takes a String.
     * There may be other cases like this, as well, so we generally look
     * for an Object-taking constructor if we can't find one that takes
     * a String.
     */
    if (cause == NULL) {
        if (msgStr == NULL) {
            initMethod = findExceptionInitMethod(excepClass, FALSE, FALSE);
            initExcept = kInitNoarg;
        } else {
            initMethod = findExceptionInitMethod(excepClass, TRUE, FALSE);
            if (initMethod != NULL) {
                initExcept = kInitMsg;
            } else {
                /* no #2, try #3 */
                initMethod = findExceptionInitMethod(excepClass, TRUE, TRUE);
                if (initMethod != NULL) {
                    initExcept = kInitMsgThrow;
                }
            }
        }
    } else {
        if (msgStr == NULL) {
            initMethod = findExceptionInitMethod(excepClass, FALSE, TRUE);
            if (initMethod != NULL) {
                initExcept = kInitThrow;
            } else {
                initMethod = findExceptionInitMethod(excepClass, FALSE, FALSE);
                initExcept = kInitNoarg;
                needInitCause = true;
            }
        } else {
            initMethod = findExceptionInitMethod(excepClass, TRUE, TRUE);
            if (initMethod != NULL) {
                initExcept = kInitMsgThrow;
            } else {
                initMethod = findExceptionInitMethod(excepClass, TRUE, FALSE);
                initExcept = kInitMsg;
                needInitCause = TRUE;
            }
        }
    }

    if (initMethod == NULL) {
        ClassObject* exRuntimeException = dvmFindSystemClassNoInit("Ljava/lang/RuntimeException;");
        /*
         * We can't find the desired constructor.  This can happen if a
         * subclass of java/lang/Throwable doesn't define an expected
         * constructor, e.g. it doesn't provide one that takes a string
         * when a message has been provided.
         */
        DVMTraceWar("WARNING: exception class '%s' missing constructor "
            "(msg='%s' kind=%d)",
            excepClass->descriptor, msg, initExcept);
        //assert(strcmp(excepClass->descriptor,
        //              "Ljava/lang/RuntimeException;") != 0);
        dvmThrowChainedException(exRuntimeException, "re-throw on exception class missing constructor", NULL);
        goto bail;
    }

    /*
     * Call the constructor with the appropriate arguments.
     */

    switch (initExcept) {
    case kInitNoarg:
        DVMTraceInf("+++ exc noarg (ic=%d)", needInitCause);
        dvmCallInitMethod(initMethod, exception);
        break;
    case kInitMsg:
        DVMTraceInf("+++ exc msg (ic=%d)", needInitCause);
        dvmCallInitMethod(initMethod, exception, msgStr);
        break;
    case kInitThrow:
        DVMTraceInf("+++ exc throw");
        //assert(!needInitCause);
        dvmCallInitMethod(initMethod, exception, cause);
        break;
    case kInitMsgThrow:
        DVMTraceInf("+++ exc msg+throw");
        //dvmCallInitMethod(!needInitCause);
        dvmCallInitMethod(initMethod, exception, msgStr, cause);
        break;
    default:
        //assert(false);
        goto bail;
    }

    /*
     * It's possible the constructor has thrown an exception.  If so, we
     * return an error and let our caller deal with it.
     */
    if (self->exception != NULL) {
        DVMTraceWar("Exception thrown (%s) while throwing internal exception (%s)",
            self->exception->clazz->descriptor, exception->clazz->descriptor);
        goto bail;
    }

    /*
     * If this exception was caused by another exception, and we weren't
     * able to find a cause-setting constructor, set the "cause" field
     * with an explicit call.
     */
    if (needInitCause) {
        Method* initCause;
        initCause = dvmFindVirtualMethodHierByDescriptor(excepClass, "initCause",
            "(Ljava/lang/Throwable;)Ljava/lang/Throwable;");
        if (initCause != NULL) {
            dvmCallInitMethod(initCause, exception, cause);
            if (self->exception != NULL) {
                /* initCause() threw an exception; return an error and
                 * let the caller deal with it.
                 */
                DVMTraceWar("Exception thrown (%s) during initCause() "
                        "of internal exception (%s)",
                        self->exception->clazz->descriptor,
                        exception->clazz->descriptor);
                goto bail;
            }
        } else {
            DVMTraceWar("WARNING: couldn't find initCause in '%s'",
                excepClass->descriptor);
        }
    }


    result = TRUE;

bail:
    return result;
}

void dvmThrowChainedException(ClassObject* excepClass, const char* msg, Object* cause)
{
    Thread* self = dthread_currentThread();
    Object* exception;

    if (excepClass == NULL) {
        /*
         * The exception class was passed in as NULL. This might happen
         * early on in VM initialization. There's nothing better to do
         * than just log the message as an error and abort.
         */
        DVMTraceErr("Fatal error: %s", msg);
        dvmAbort();
    }

    /* make sure the exception is initialized */
    if (!dvmIsClassInitialized(excepClass) && !dvmInitClass(excepClass)) {
        DVMTraceErr("ERROR: unable to initialize exception class '%s'",
            excepClass->descriptor);
        if (CRTL_strcmp(excepClass->descriptor, "Ljava/lang/VirtualMachineError;") == 0)
            dvmAbort();
        return;
    }

    exception = dvmAllocObject(excepClass, ALLOC_DEFAULT);
    if (exception == NULL) {
        /*
         * We're in a lot of trouble.  We might be in the process of
         * throwing an out-of-memory exception, in which case the
         * pre-allocated object will have been thrown when our object alloc
         * failed.  So long as there's an exception raised, return and
         * allow the system to try to recover.  If not, something is broken
         * and we need to bail out.
         */
        if (dvmCheckException(self))
            goto bail;
        DVMTraceErr("FATAL: unable to allocate exception '%s' '%s'",
            excepClass->descriptor, msg != NULL ? msg : "(no msg)");
        dvmAbort();
    }

    /*
     * Init the exception.
     */
    if (!initException(exception, msg, cause, self)) {
        /*
         * Whoops.  If we can't initialize the exception, we can't use
         * it.  If there's an exception already set, the constructor
         * probably threw an OutOfMemoryError.
         */
        if (!dvmCheckException(self)) {
            /*
             * We're required to throw something, so we just
             * throw the pre-constructed internal error.
             */
            self->exception = gDvm.internalErrorObj;
        }
        goto bail;
    }

    self->exception = exception;

bail:
    return;
}

/*
 * Create a "stock instance" of an exception class.
 */
static Object* createStockException(const char* descriptor, const char* msg)
{
    StringObject* msgStr = NULL;
    ClassObject* clazz;
    Method* init;
    Object* obj;

    /* find class, initialize if necessary */
    clazz = dvmFindClass(descriptor);
    if (clazz == NULL) {
        DVMTraceErr("Unable to find %s", descriptor);
        return NULL;
    }

    init = dvmFindDirectMethodByDescriptor(clazz, "<init>",
            "(Ljava/lang/String;)V");
    if (init == NULL) {
        DVMTraceErr("Unable to find String-arg constructor for %s", descriptor);
        return NULL;
    }

    obj = dvmAllocObject(clazz, ALLOC_DEFAULT);
    if (obj == NULL)
        return NULL;

    if (msg == NULL) {
        msgStr = NULL;
    } else {
        msgStr = dvmCreateStringFromCstr(msg);
        if (msgStr == NULL) {
            DVMTraceWar("Could not allocate message string \"%s\"", msg);
            return NULL;
        }
    }

    dvmCallInitMethod(init, obj, msgStr);
    return obj;
}

/*
 * Create some "stock" exceptions.  These can be thrown when the system is
 * too screwed up to allocate and initialize anything, or when we don't
 * need a meaningful stack trace.
 *
 * We can't do this during the initial startup because we need to execute
 * the constructors.
 */
bool_t dvmCreateStockExceptions()
{
    /*
     * Pre-allocate some throwables.  These need to be explicitly added
     * to the GC's root set (see dvmHeapMarkRootSet()).
     */
    gDvm.outOfMemoryObj = createStockException("Ljava/lang/OutOfMemoryError;",
        "[memory exhausted]");
    if (gDvm.outOfMemoryObj != NULL) {
        dvmGCAddToGlobalRef(gDvm.outOfMemoryObj);
    }

    gDvm.internalErrorObj = createStockException("Ljava/lang/InternalError;",
        "[pre-allocated]");
    if (gDvm.internalErrorObj != NULL) {
        dvmGCAddToGlobalRef(gDvm.internalErrorObj);
    }
    
    gDvm.noClassDefFoundErrorObj =
        createStockException("Ljava/lang/NoClassDefFoundError;", "[generic]");
    if (gDvm.noClassDefFoundErrorObj != NULL) {
        dvmGCAddToGlobalRef(gDvm.noClassDefFoundErrorObj);
    }

    if (gDvm.outOfMemoryObj == NULL || gDvm.internalErrorObj == NULL ||
        gDvm.noClassDefFoundErrorObj == NULL)
    {
        DVMTraceWar("Unable to create stock exceptions");
        return FALSE;
    }

    return TRUE;
}


void dvmThrowException(const char* exceptionDescriptor, const char* msg)
{
    ClassObject* exception = NULL;

    exception = dvmFindSystemClassNoInit(exceptionDescriptor);
    if (exception == NULL)
    {
        DVMTraceErr("dvmThrowException - Error: load exception(%s) failure\n", exceptionDescriptor);
        //goto fatal Error?
    }

    dvmThrowChainedException(exception, msg, NULL);
}


void dvmThrowVirtualMachineErrorWithClassMessage(const char* descriptor)
{
    ClassObject* virtualMachineError = NULL;
    char*        message = NULL;
    
    virtualMachineError = dvmFindSystemClassNoInit("Ljava/lang/VirtualMachineError;");
    if (virtualMachineError == NULL)
    {
        DVMTraceErr("dvmThrowVirtualMachineErrorWithClassMessage - Error: load virtualMachineError failure\n");
        //goto fatal Error;
    }
    
    message = dvmDescriptorToName(descriptor);
    
    dvmThrowChainedException(virtualMachineError, message, NULL);

    CRTL_free(message);
}

void dvmThrowNullPointerException(const char* msg)
{
    ClassObject* exNullPointerException = NULL;

    exNullPointerException = dvmFindSystemClassNoInit("Ljava/lang/NullPointerException;");
    if (exNullPointerException == NULL)
    {
        DVMTraceErr("dvmThrowVirtualMachineErrorWithClassMessage - Error: load virtualMachineError failure\n");
        //goto fatal Error;
    }
    dvmThrowChainedException(exNullPointerException, msg, NULL);
}


void dvmThrowArrayStoreExceptionNotArray(ClassObject* actual, const char* label)
{
    UNUSED(actual);
    UNUSED(label);
    dvmThrowException("Ljava/lang/ArrayStoreException;", NULL);
}

/**
 * Throw an InstantiationException in the current thread, with
 * the human-readable form of the given class as the detail message,
 * with optional extra detail appended to the message.
 */
void dvmThrowInstantiationException(ClassObject* clazz, const char* extraDetail)
{
    UNUSED(clazz);
    dvmThrowException("Ljava/lang/InstantiationException;", extraDetail);
}


/**
 * Throw an AbstractMethodError in the current thread, with the given detail
 * message.
 */
void dvmThrowIllegalAccessException(const char* msg)
{
    dvmThrowException("Ljava/lang/IllegalAccessException;", msg);
}

/*
 * Search the method's list of exceptions for a match.
 *
 * Returns the offset of the catch block on success, or -1 on failure.
 */
static int findCatchInMethod(Thread* self, const Method* method, int relPc,
    ClassObject* excepClass)
{
	DvmDex* pDvmDex = NULL;
	const DexCode* pCode = NULL;
	DexCatchIterator iterator;
    /*
     * Need to clear the exception before entry.  Otherwise, dvmResolveClass
     * might think somebody threw an exception while it was loading a class.
     */
    assert(!dvmCheckException(self));
    assert(!dvmIsNativeMethod(method));

	pDvmDex = method->clazz->pDvmDex;
	pCode = dvmGetMethodCode(method);
    

    if (dexFindCatchHandler(&iterator, pCode, relPc)) {
        for (;;) {
            DexCatchHandler* handler = dexCatchIteratorNext(&iterator);
			ClassObject* throwable = NULL;
            if (handler == NULL) {
                break;
            }

            if (handler->typeIdx == kDexNoIndex) {
                /* catch-all */
                DVMTraceInf("Match on catch-all block at 0x%02x in %s.%s for %s",
                        relPc, method->clazz->descriptor,
                        method->name, excepClass->descriptor);
                return handler->address;
            }

            throwable = dvmDexGetResolvedClass(pDvmDex, handler->typeIdx);
            if (throwable == NULL) {
                /*
                 * TODO: this behaves badly if we run off the stack
                 * while trying to throw an exception.  The problem is
                 * that, if we're in a class loaded by a class loader,
                 * the call to dvmResolveClass has to ask the class
                 * loader for help resolving any previously-unresolved
                 * classes.  If this particular class loader hasn't
                 * resolved StackOverflowError, it will call into
                 * interpreted code, and blow up.
                 *
                 * We currently replace the previous exception with
                 * the StackOverflowError, which means they won't be
                 * catching it *unless* they explicitly catch
                 * StackOverflowError, in which case we'll be unable
                 * to resolve the class referred to by the "catch"
                 * block.
                 *
                 * We end up getting a huge pile of warnings if we do
                 * a simple synthetic test, because this method gets
                 * called on every stack frame up the tree, and it
                 * fails every time.
                 *
                 * This eventually bails out, effectively becoming an
                 * uncatchable exception, so other than the flurry of
                 * warnings it's not really a problem.  Still, we could
                 * probably handle this better.
                 */
                throwable = dvmResolveClass(method->clazz, handler->typeIdx,
                    true);
                if (throwable == NULL) {
                    /*
                     * We couldn't find the exception they wanted in
                     * our class files (or, perhaps, the stack blew up
                     * while we were querying a class loader). Cough
                     * up a warning, then move on to the next entry.
                     * Keep the exception status clear.
                     */
                    DVMTraceWar("Could not resolve class ref'ed in exception "
                            "catch list (class index %d, exception %s)",
                            handler->typeIdx,
                            (self->exception != NULL) ?
                            self->exception->clazz->descriptor : "(none)");
                    dvmClearException(self);
                    continue;
                }
            }

            //LOGD("ADDR MATCH, check %s instanceof %s",
            //    excepClass->descriptor, pEntry->excepClass->descriptor);

            if (dvmInstanceof(excepClass, throwable)) {
                DVMTraceInf("Match on catch block at 0x%02x in %s.%s for %s",
                        relPc, method->clazz->descriptor,
                        method->name, excepClass->descriptor);
                return handler->address;
            }
        }
    }

    DVMTraceInf("No matching catch block at 0x%02x in %s for %s",
        relPc, method->name, excepClass->descriptor);
    return -1;
}



/*
 * Find a matching "catch" block.  "pc" is the relative PC within the
 * current method, indicating the offset from the start in 16-bit units.
 *
 * Returns the offset to the catch block, or -1 if we run up against a
 * break frame without finding anything.
 *
 * The class resolution stuff we have to do while evaluating the "catch"
 * blocks could cause an exception.  The caller should clear the exception
 * before calling here and restore it after.
 *
 * Sets *newFrame to the frame pointer of the frame with the catch block.
 * If "scanOnly" is false, self->interpSave.curFrame is also set to this value.
 */
int dvmFindCatchBlock(Thread* self, int relPc, Object* exception,
    vbool scanOnly, void** newFrame)
{
    u4* fp = self->interpSave.curFrame;
    int catchAddr = -1;

    assert(!dvmCheckException(self));

    while (true) {
        StackSaveArea* saveArea = SAVEAREA_FROM_FP(fp);
        catchAddr = findCatchInMethod(self, saveArea->method, relPc,
                        exception->clazz);
        if (catchAddr >= 0)
            break;

        /*
         * Normally we'd check for ACC_SYNCHRONIZED methods and unlock
         * them as we unroll.  Dalvik uses what amount to generated
         * "finally" blocks to take care of this for us.
         */

        /* output method profiling info */
        if (!scanOnly) {
            //TRACE_METHOD_UNROLL(self, saveArea->method);
        }

        /*
         * Move up one frame.  If the next thing up is a break frame,
         * break out now so we're left unrolled to the last method frame.
         * We need to point there so we can roll up the JNI local refs
         * if this was a native method.
         */
        assert(saveArea->prevFrame != NULL);
        if (dvmIsBreakFrame((u4*)saveArea->prevFrame)) {
            if (!scanOnly)
                break;      // bail with catchAddr == -1

            /*
             * We're scanning for the debugger.  It needs to know if this
             * exception is going to be caught or not, and we need to figure
             * out if it will be caught *ever* not just between the current
             * position and the next break frame.  We can't tell what native
             * code is going to do, so we assume it never catches exceptions.
             *
             * Start by finding an interpreted code frame.
             */
            fp = saveArea->prevFrame;           // this is the break frame
            saveArea = SAVEAREA_FROM_FP(fp);
            fp = saveArea->prevFrame;           // this may be a good one
            while (fp != NULL) {
                if (!dvmIsBreakFrame((u4*)fp)) {
                    saveArea = SAVEAREA_FROM_FP(fp);
                    if (!dvmIsNativeMethod(saveArea->method))
                        break;
                }

                fp = SAVEAREA_FROM_FP(fp)->prevFrame;
            }
            if (fp == NULL)
                break;      // bail with catchAddr == -1

            /*
             * Now fp points to the "good" frame.  When the interp code
             * invoked the native code, it saved a copy of its current PC
             * into xtra.currentPc.  Pull it out of there.
             */
            relPc =
                saveArea->xtra.currentPc - SAVEAREA_FROM_FP(fp)->method->insns;
        } else {
            fp = saveArea->prevFrame;

            /* savedPc in was-current frame goes with method in now-current */
            relPc = saveArea->savedPc - SAVEAREA_FROM_FP(fp)->method->insns;
        }
    }

    if (!scanOnly)
        self->interpSave.curFrame = fp;

    /*
     * The class resolution in findCatchInMethod() could cause an exception.
     * Clear it to be safe.
     */
    self->exception = NULL;

    *newFrame = fp;
    return catchAddr;
}

void dvmThrowArrayStoreExceptionIncompatibleElement(ClassObject* objectType,
        ClassObject* arrayType)
{
	ClassObject* arrayStoreException;
	DVMTraceWar(">>>call dvmThrowArrayStoreExceptionIncompatibleElement!\n");
	
	arrayStoreException = dvmFindSystemClassNoInit("Ljava/lang/ArrayStoreException;");
	throwTypeError(arrayStoreException, "%s cannot be stored in an array of type %s", objectType, arrayType);
}

void dvmThrowArithmeticException(const char* msg) 
{
	DVMTraceWar(">>>call dvmThrowArithmeticException!\n");
	dvmThrowException("Ljava/lang/ArithmeticException;", msg);
}
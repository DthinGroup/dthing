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
        char* newStr = (char*)CRTL_malloc(length);

        if (newStr == NULL) {
            return NULL;
        }

        CRTL_memcpy(newStr, str + 1, length);
        return newStr;
    }

    return CRTL_strdup(str);
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

    gDvm.internalErrorObj = createStockException("Ljava/lang/InternalError;",
        "[pre-allocated]");

    gDvm.noClassDefFoundErrorObj =
        createStockException("Ljava/lang/NoClassDefFoundError;", "[generic]");

    if (gDvm.outOfMemoryObj == NULL || gDvm.internalErrorObj == NULL ||
        gDvm.noClassDefFoundErrorObj == NULL)
    {
        DVMTraceWar("Unable to create stock exceptions");
        return FALSE;
    }

    return TRUE;
}



void dvmThrowVirtualMachineErrorWithClassMessage(const char* descriptor)
{
    ClassObject* virtualMachineError = NULL;
    char*        message = NULL;
    
    virtualMachineError = dvmFindSystemClassNoInit("Ljava/lang/VirtualMachineError;");
    if (virtualMachineError == NULL)
    {
        //goto fatal Error;
        DVMTraceErr("dvmThrowVirtualMachineErrorWithClassMessage - Error: load virtualMachineError failure\n");
    }
    
    message = dvmDescriptorToName(descriptor);
    
    dvmThrowChainedException(virtualMachineError, message, NULL);

    CRTL_free(message);
}

/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/25 $
 * Last modified:	$Date: 2013/07/01 $
 * Version:         $ID: init.c#1
 */

/**
 * VM lifeclycle control.
 */


#include <std_global.h>
#include <dthing.h>
#include <opl_mm.h>
#include <opl_file.h>
//#include <vmTime.h>

#include <class.h>
#include <gc.h>


GLOBAL DVMGlobal gDvm;

/**
 * Initialize native environment, such as file system,
 * graphics and so on.
 * This API is called before vm lifecycle.
 */
LOCAL void DVM_native_init()
{
    file_startup();
}


/**
 * finalize native environment, such as file system,
 * graphics and so on.
 * This API is called after vm lifecycle is end.
 * And it must be matched DVM_native_init;
 */
LOCAL void DVM_native_final()
{
    file_shutdown();
}


/**
 * VM lifecycle initialization, including
 * file, mm or other native modules used during entire time 
 * of VM running.
 *
 * This API is called on start of VM lifecycle.
 */
LOCAL void DVM_lifecycle_init()
{
    DVM_mm_initialize();
    dvmGCStartup();

    dvmClassStartup();

    vmtime_init();
    Schd_InitThreadLists();

    dvmCreateStockExceptions();
	//dthread_init();
}

/**
 * Finalise native modules to ensure all VM relevant resources are
 * freed while VM is going to shutdown status.
 *
 * This API is called on end of vm lifecycle.
 */
LOCAL void DVM_lifecycle_final()
{
    Schd_FinalThreadLists();
    vmtime_term();
    dvmClassShutdown();

    dvmGCShutdown();
    DVM_mm_finalize();
}


/*
 * Process an argument vector full of options.  Unlike standard C programs,
 * argv[0] does not contain the name of the program.
 *
 * If "ignoreUnrecognized" is set, we ignore options starting with "-X" or "_"
 * that we don't recognize.  Otherwise, we return with an error as soon as
 * we see anything we can't identify.
 *
 * Returns 0 on success, -1 on failure, and 1 for the special case of
 * "-version" where we want to stop without showing an error message.
 */
static uint8_t** processOptions(int32_t argc, const uint8_t* const argv[], int32_t* newArgc)
{
    int32_t   i, j;
    uint8_t** newArgv = NULL;

    DVMTraceDbg("VM options (%d):\n", argc);
    for (i = 0; i < argc; i++) {
        DVMTraceDbg("  %d: '%s'", i, argv[i]);
    }

    newArgv = (uint8_t**)CRTL_malloc(argc * sizeof(uint8_t*));
    if (newArgv == NULL)
    {
        DVMTraceErr("processOptions: Not enough memory\n");
        return -1;
    }

    for (i = 0, j = 0; i < argc; i++)
    {
        if (CRTL_strcmp(argv[i], "-classpath") == 0 || CRTL_strcmp(argv[i], "-cp") == 0)
        {
            /* set classpath */
            if (i == argc-1)
            {
                DVMTraceErr("Missing classpath path list\n");
                return -1;
            }
            gDvm.classPathStr = CRTL_strdup(argv[++i]);
            DVMTraceDbg("-classpath/-cp  classPathStr (%s)\n", gDvm.classPathStr);
        } 
        else if (CRTL_strncmp(argv[i], "-run", sizeof("-run") - 1) == 0)
        {
            /* set application path */
            const char* path = argv[++i];

            if (*path == '\0')
            {
                DVMTraceErr("Missing applet path list\n");
                return -1;
            }
            gDvm.appPathStr = CRTL_strdup(path);
            newArgv[j++] = "-run";
            newArgv[j++] = argv[++i];
            DVMTraceDbg("applet appPathStr (%s)\n", gDvm.appPathStr);
        }
        else if (CRTL_strncmp(argv[i], "-D", 2) == 0)
        {
            /* Properties are handled in managed code. We just check syntax. */
            if ((char*)CRTL_strchr(argv[i], '=') == NULL)
            {
                DVMTraceErr("Bad system property setting: \"%s\"\n", argv[i]);
                return -1;
            }
            //TODO: how to handle global definitions here!!!!!!!!!!
            DVMTraceErr(" have not handled system properties yet! ");
        }
        else
        {
            DVMTraceErr( "Unrecognized option '%s'\n", argv[i]);
            newArgv[j++] = argv[i];
        }
    }
    *newArgc = j;
    return newArgv;
}


/**
 * Main method of VM entry. 
 * The API will not exit until VM is goging to shutdown status.
 */
int32_t DVM_main(int32_t argc, int8_t * argv[])
{
    uint8_t** newArgv;
    int32_t   newArgc;
	/* Find main class */
	ClassObject* mainClass = NULL;
	Method*      startMeth = NULL;

    /* dummy thread class */
    ClassObject* dummyThreadCls = NULL;
    Object*      dummyThreadObj = NULL;

    DVM_native_init();

	//ret = sizeof(ArrayObject);

    newArgv = processOptions(argc, argv, &newArgc);
    if (newArgv == NULL)
    {
        DVMTraceDbg("DVM_main: no extra arguments\n");
    }

    DVM_lifecycle_init();

#if 0
   /**
    * Test code area.
    */
    /* =================================== */
    dvmFindClass("Lcom/yarlungsoft/main/Main;");
    dvmFindClass("Ljava/lang/String;");
    dvmFindClass("Ljava/lang/Object;");
    /* =================================== */
#endif

	/* Find main class */
	//mainClass = dvmFindClass("Ljava/net/SocketUdpTest;");
    mainClass = dvmFindClass("Lcom/yarlungsoft/ams/Main;");
    //mainClass = dvmFindClass("Lcom/yarlungsoft/print/printTest;");
	/* Find Entry function method */
	//startMeth = dvmGetStaticMethodID(mainClass, "main", "()V");
	startMeth = dvmGetStaticMethodID(mainClass, "main", "([Ljava/lang/String;)V");
    dummyThreadCls = dvmFindClass("Ljava/lang/Thread;");
    dummyThreadObj = dvmAllocObject(dummyThreadCls, 0);
    
    if (newArgc == 0)
    {
	    dthread_create(startMeth, dummyThreadObj);
    }
    else if (newArgc > 0)
    {
        int i;
        ClassObject* strCls = NULL;
        ArrayObject* params = NULL; 

        strCls = dvmFindClass("[Ljava/lang/String;");
        params = dvmAllocArrayByClass(strCls, newArgc, 0);
        for (i = 0; i < newArgc; i++)
        {
            StringObject* strObj = NewStringUTF(newArgv[i]);
            dvmSetObjectArrayElement(params, i, strObj);
        }

        dthread_create_params(startMeth, dummyThreadObj, params);
    }

    //entry interpret;
    Schd_SCHEDULER();

    DVM_lifecycle_final();
    DVM_native_final();
	return 0;
}

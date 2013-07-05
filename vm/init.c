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
#include <opl_mm.h>


/**
 * VM lifecycle initialization, including
 * file, mm or other native modules used during entire time 
 * of VM running.
 *
 * This API is called on start of VM lifecycle.
 */
LOCAL void DVM_lifecycle_initial()
{
	DVM_mm_initialize();
}

/**
 * Finalise native modules to ensure all VM relevant resources are
 * freed while VM is going to shutdown status.
 *
 * This API is called on end of vm lifecycle.
 */
LOCAL void DVM_lifecycle_final()
{
	DVM_mm_initialize();
}


/**
 * Main method of VM entry. 
 * The API will not exit until VM is goging to shutdown status.
 */
int32_t DVM_main(int32_t argc, int8_t * argv[])
{
	DVM_lifecycle_initial();


	//enter interpret;


	DVM_lifecycle_final();


	return 0;
}

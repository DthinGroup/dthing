#ifndef __DVM_JARPARSER_H__
#define __DVM_JARPARSER_H__

#include <std_global.h>

#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************************/
/*          jar parsing pre-definitions                                              */
/*************************************************************************************/

/**
 * open the jar by jar file name;
 * @param jarfn, jar file name in ucs2 encoding.
 * @return the identifier of opened jar file which will be used in releveant APIs.
 */
int32_t  openJar(const uint16_t* jarfn);

/**
 * Get the uncompress data from the jar by fixed local file name.
 * @handle, the identifier which is returned by openJar.
 * @confn, the local file name which was compressed in jar package.
 * @dataLen, is a output data which saved the uncompress data size in bytes.
 * @return return the uncompress data of fixed local file.
 * NOTE: 
 *      the returned memory pointer must be freed by callee API.
 */
uint8_t* getJarContentByFileName(int32_t handle, uint8_t* confn, int32_t* dataLen);

/**
 * Closed jar file by identifier which is return ed openJar.
 * Note: this API must be matched with openJar.
 * @handle, the identifier.
 */
void closeJar(int32_t handle);
/*************************************************************************************/
/*          jar parsing                                                              */
/*************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

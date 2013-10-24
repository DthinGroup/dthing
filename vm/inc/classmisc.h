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


#ifndef __CLASS_MISC_H__
#define __CLASS_MISC_H__

#include <dthing.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Find a class by name, initializing it if requested.
 */
ClassObject* dvmFindClassByName(StringObject* nameObj);


/*
 * Return a newly-allocated string for the type descriptor
 * corresponding to the "dot version" of the given class name. That
 * is, non-array names are surrounded by "L" and ";", and all
 * occurrences of '.' are changed to '/'.
 */
char* dvmDotToDescriptor(const char* str);

/*
 * Return a newly-allocated string for the "dot version" of the class
 * name for the given type descriptor. That is, The initial "L" and
 * final ";" (if any) have been removed and all occurrences of '/'
 * have been changed to '.'.
 *
 * "Dot version" names are used in the class loading machinery.
 * See also dvmHumanReadableDescriptor.
 */
char* dvmDescriptorToDot(const char* str);

#ifdef __cplusplus
}
#endif

#endif //__CLASS_MISC_H__
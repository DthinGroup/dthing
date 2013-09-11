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
 * Check access to fields and methods.
 */
#ifndef DALVIK_OO_ACCESSCHECK_H_
#define DALVIK_OO_ACCESSCHECK_H_

#include <dthing.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Determine whether the "accessFrom" class is allowed to get at "clazz".
 */
bool_t dvmCheckClassAccess(const ClassObject* accessFrom, const ClassObject* clazz);

/*
 * Determine whether the "accessFrom" class is allowed to get at "method".
 */
bool_t dvmCheckMethodAccess(const ClassObject* accessFrom, const Method* method);

/*
 * Determine whether the "accessFrom" class is allowed to get at "field".
 */
bool_t dvmCheckFieldAccess(const ClassObject* accessFrom, const Field* field);

/*
 * Returns "true" if the two classes are in the same runtime package.
 */
bool_t dvmInSamePackage(const ClassObject* class1, const ClassObject* class2);


#ifdef __cplusplus
}
#endif

#endif  // DALVIK_OO_ACCESSCHECK_H_

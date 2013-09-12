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
 * instanceof, checkcast, etc.
 */
#ifndef DALVIK_OO_TYPECHECK_H_
#define DALVIK_OO_TYPECHECK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Determine whether "instance" is an instance of "clazz".
 *
 * Returns 0 (false) if not, 1 (true) if so.
 */
int dvmInstanceof(const ClassObject* instance, const ClassObject* clazz);

/*
 * Determine whether a class implements an interface.
 *
 * Returns 0 (false) if not, 1 (true) if so.
 */
int dvmImplements(const ClassObject* clazz, const ClassObject* interface);

/*
 * Determine whether or not we can store an object into an array, based
 * on the classes of the two.
 *
 * Returns 0 (false) if not, 1 (true) if so.
 */
bool_t dvmCanPutArrayElement(const ClassObject* elemClass, const ClassObject* arrayClass);

#ifdef __cplusplus
}
#endif

#endif  // DALVIK_OO_TYPECHECK_H_

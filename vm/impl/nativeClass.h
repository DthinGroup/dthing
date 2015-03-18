/**
 * Copyright (C) 2013-2014 YarlungSoft. All Rights Reserved.
 *
 * Created:         $Date: 2013/10/17 $
 * Last modified:   $Date: 2014/04/03 $
 * Version:         $ID: nativeClass.h#1
 */

#include <dthing.h>
#include <kni.h>

/* Header for class java.lang.Class */

#ifndef __NATIVE_CLASS_H__
#define __NATIVE_CLASS_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Class:     java_lang_Class
 * Method:    forName
 * Signature: (Ljava/lang/String;)Ljava/lang/Class;
 *
 * Returns the <code>Class</code> object associated with the class
 * with the given string name.
 * Given the fully-qualified name for a class or interface, this
 * method attempts to locate, load and link the class.  If it
 * succeeds, returns the Class object representing the class.  If
 * it fails, the method throws a ClassNotFoundException.
 * <p>
 * For example, the following code fragment returns the runtime
 * <code>Class</code> descriptor for the class named
 * <code>java.lang.Thread</code>:
 * <ul><code>
 *   Class&nbsp;t&nbsp;= Class.forName("java.lang.Thread")
 * </code></ul>
 *
 * @param      className   the fully qualified name of the desired class.
 * @return     the <code>Class</code> descriptor for the class with the
 *             specified name.
 * @exception  ClassNotFoundException  if the class could not be found.
 * @since      JDK1.0
 */
void Java_java_lang_Class_forName(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Class
 * Method:    isInstance
 * Signature: (Ljava/lang/Object;)Z
 *
 * Determines if the specified <code>Object</code> is assignment-compatible
 * with the object represented by this <code>Class</code>.  This method is
 * the dynamic equivalent of the Java language <code>instanceof</code>
 * operator. The method returns <code>true</code> if the specified
 * <code>Object</code> argument is non-null and can be cast to the
 * reference type represented by this <code>Class</code> object without
 * raising a <code>ClassCastException.</code> It returns <code>false</code>
 * otherwise.
 *
 * <p> Specifically, if this <code>Class</code> object represents a
 * declared class, this method returns <code>true</code> if the specified
 * <code>Object</code> argument is an instance of the represented class (or
 * of any of its subclasses); it returns <code>false</code> otherwise. If
 * this <code>Class</code> object represents an array class, this method
 * returns <code>true</code> if the specified <code>Object</code> argument
 * can be converted to an object of the array class by an identity
 * conversion or by a widening reference conversion; it returns
 * <code>false</code> otherwise. If this <code>Class</code> object
 * represents an interface, this method returns <code>true</code> if the
 * class or any superclass of the specified <code>Object</code> argument
 * implements this interface; it returns <code>false</code> otherwise. If
 * this <code>Class</code> object represents a primitive type, this method
 * returns <code>false</code>.
 *
 * @param   obj the object to check
 * @return  true if <code>obj</code> is an instance of this class
 *
 * @since JDK1.1
 */
void Java_java_lang_Class_isInstance(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Class
 * Method:    isAssignableFrom
 * Signature: (Ljava/lang/Class;)Z
 *
 * Determines if the class or interface represented by this
 * <code>Class</code> object is either the same as, or is a superclass or
 * superinterface of, the class or interface represented by the specified
 * <code>Class</code> parameter. It returns <code>true</code> if so;
 * otherwise it returns <code>false</code>. If this <code>Class</code>
 * object represents a primitive type, this method returns
 * <code>true</code> if the specified <code>Class</code> parameter is
 * exactly this <code>Class</code> object; otherwise it returns
 * <code>false</code>.
 *
 * <p> Specifically, this method tests whether the type represented by the
 * specified <code>Class</code> parameter can be converted to the type
 * represented by this <code>Class</code> object via an identity conversion
 * or via a widening reference conversion. See <em>The Java Language
 * Specification</em>, sections 5.1.1 and 5.1.4 , for details.
 *
 * @param cls the <code>Class</code> object to be checked
 * @return the <code>boolean</code> value indicating whether objects of the
 * type <code>cls</code> can be assigned to objects of this class
 * @exception NullPointerException if the specified Class parameter is
 *            null.
 * @since JDK1.1
 */
void Java_java_lang_Class_isAssignableFrom(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Class
 * Method:    isInterface
 * Signature: ()Z
 *
 * Determines if the specified <code>Class</code> object represents an
 * interface type.
 *
 * @return  <code>true</code> if this object represents an interface;
 *          <code>false</code> otherwise.
 */
void Java_java_lang_Class_isInterface(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Class
 * Method:    isArray
 * Signature: ()Z
 *
 * Determines if this <code>Class</code> object represents an array class.
 *
 * @return  <code>true</code> if this object represents an array class;
 *          <code>false</code> otherwise.
 * @since   JDK1.1
 */
void Java_java_lang_Class_isArray(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Class
 * Method:    newInstance0
 * Signature: ()Ljava/lang/Object;
 *
 * Creates a new instance of a class.
 *
 * @return     a newly allocated instance of the class represented by this
 *             object. This is done exactly as if by a <code>new</code>
 *             expression with an empty argument list.
 * @exception  IllegalAccessException  if the class or initializer is
 *               not accessible.
 * @exception  InstantiationException  if an application tries to
 *               instantiate an abstract class or an interface, or if the
 *               instantiation fails for some other reason.
 * @since     JDK1.0
 */
void Java_java_lang_Class_newInstance0(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Class
 * Method:    getName
 * Signature: ()Ljava/lang/String;
 *
 * Returns the fully-qualified name of the entity (class, interface, array
 * class, primitive type, or void) represented by this <code>Class</code>
 * object, as a <code>String</code>.
 *
 * <p> If this <code>Class</code> object represents a class of arrays, then
 * the internal form of the name consists of the name of the element type
 * in Java signature format, preceded by one or more "<tt>[</tt>"
 * characters representing the depth of array nesting. Thus:
 *
 * <blockquote><pre>
 * (new Object[3]).getClass().getName()
 * </pre></blockquote>
 *
 * returns "<code>[Ljava.lang.Object;</code>" and:
 *
 * <blockquote><pre>
 * (new int[3][4][5][6][7][8][9]).getClass().getName()
 * </pre></blockquote>
 *
 * returns "<code>[[[[[[[I</code>". The encoding of element type names
 * is as follows:
 *
 * <blockquote><pre>
 * B            byte
 * C            char
 * D            double
 * F            float
 * I            int
 * J            long
 * L<i>classname;</i>  class or interface
 * S            short
 * Z            boolean
 * </pre></blockquote>
 *
 * The class or interface name <tt><i>classname</i></tt> is given in fully
 * qualified form as shown in the example above.
 *
 * @return  the fully qualified name of the class or interface
 *          represented by this object.
 */
void Java_java_lang_Class_getName(const u4* args, JValue* pResult);


void Java_java_lang_Class_getResourceAsStream(const u4* args, JValue* pResult);

#ifdef __cplusplus
}
#endif
#endif // __NATIVE_CLASS_H__

import os, re, sys
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-f", "--src-header", dest = "src_header", help = "The source HEADER file to compare with.", metavar = "HEADER")
parser.add_option("-d", "--output-dir", dest = "out_dir", help = "The folder to store intermediate files.", metavar = "OUTDIR")
parser.add_option("-s", "--java-dir", dest = "java_src_dir", help = "The folder of Java source files.", metavar = "JAVADIR")

type_signature_map = {
	'boolean'   : 'Z',
	'byte'      : 'B',
	'char'      : 'C',
	'short'     : 'S',
	'int'       : 'I',
	'long'      : 'J',
	'float'     : 'F',
	'double'    : 'D',
	'void'      : 'V',
	'Object'    : 'Ljava/lang/Object;',
	'Class'     : 'Ljava/lang/Class;',
	'Class<?>'  : 'Ljava/lang/Class;',
	'String'    : 'Ljava/lang/String;',
	'Throwable' : 'Ljava/lang/Throwable;',
	'DThread'   : 'Ljava/lang/DThread;',
	'Thread'    : 'Ljava/lang/Thread;',
	'Cloneable' : 'Ljava/lang/Cloneable;',
	'Object[]'  : '[Ljava/lang/Object;',
	'String[]'  : '[Ljava/lang/String;',
	'boolean[]' : '[Z',
	'byte[]'    : '[B',
	'char[]'    : '[C',
	'short[]'   : '[S',
	'int[]'     : '[I',
	'long[]'    : '[J',
	'float[]'   : '[F',
	'double[]'  : '[D',
}

jni_type_map = {
	'boolean'   : 'jboolean',
	'byte'      : 'jbyte',
	'char'      : 'jchar',
	'short'     : 'jshort',
	'int'       : 'jint',
	'long'      : 'jlong',
	'float'     : 'jfloat',
	'double'    : 'jdouble',
	'void'      : 'void',
	'Object'    : 'jobject',
	'Class'     : 'jclass',
	'Class<?>'  : 'jclass',
	'String'    : 'jstring',
	'Throwable' : 'jthrowable',
	'DThread'   : 'jobject',
	'Thread'    : 'jobject',
	'Cloneable' : 'jobject',
	'[]'        : 'jarray',
	'Object[]'  : 'jobjectArray',
	'boolean[]' : 'jbooleanArray',
	'byte[]'    : 'jbyteArray',
	'char[]'    : 'jcharArray',
	'short[]'   : 'jshortArray',
	'int[]'     : 'jintArray',
	'long[]'    : 'jlongArray',
	'float[]'   : 'jfloatArray',
	'double[]'  : 'jdoubleArray',
}

kni_fix_map = {
	'java_lang_DThread' : {
		'start0'         : 'Java_java_lang_Thread_start',
		'sleep0'         : 'Java_java_lang_Thread_sleep',
		'activeCount0'   : 'Java_java_lang_Thread_activeCount',
		'currentThread0' : 'Java_java_lang_Thread_currentThread',
		'isAlive0'       : 'Java_java_lang_Thread_isAlive',
	},
}

java_modifiers = ['public', 'protected', 'private', 'abstract', 'static', 'final', 'transient', 'volatile', 'synchronized', 'native', 'strictfp']
modifiers_pat = r'(?:(?:' + '|'.join(java_modifiers) + r')\s+)*'
type_pat = r'[a-zA-Z0-9<>\?\[\]]+\s+'
name_pat = r'[a-zA-Z0-9_]+\s*'
type_name_pat = type_pat + name_pat
throws_pat = r'(?:throws[a-zA-Z0-9_\s,]+)?' #r'(?:throws\s+(?:[a-zA-Z_0-9]+\s*,?\s*)+)?'

def get_native_methods(rootdir):
	'''Find all Java native methods declarations under the specified folder.
    Return dictionary in format:
        {class_name : [native_method_declaration]}
	'''
	global modifiers_pat
	global type_name_pat

	java_method_pat = re.compile(r'^\s*' + modifiers_pat + type_name_pat + r'\((?:' + type_name_pat + r',?\s*)*\)\s*' + throws_pat + r';$', re.MULTILINE)
	native_pat = re.compile(r'\snative\s')
	space_pat = re.compile(r'\s+')

	java_native_methods = {}

	for root, dirs, files in os.walk(rootdir):
		for fname in files:
			if not fname.endswith('.java'):
				continue
			abs_fname = os.path.abspath(os.path.join(root, fname))
			fp = open(abs_fname, 'r')
			content = fp.read()
			fp.close()

			# parse all Java method declarations
			methods = java_method_pat.findall(content)

			# check native method declarations
			native_methods = []
			for method in methods:
				if native_pat.search(method) is not None:
					native_methods.append(method)

#			natives = native_pat.findall(content)
#			if len(natives) != len(native_methods):
#				print fname,
#				print "\tfound %d 'native's while found %d native methods" % (len(natives), len(native_methods))

			if len(native_methods) > 0:
				class_name = abs_fname[len(rootdir) : -5].strip('\\').replace('\\', '.')
				if class_name not in java_native_methods:
					java_native_methods[class_name] = []
				for method in native_methods:
					java_native_methods[class_name].append(space_pat.sub(' ', method).strip())

	return java_native_methods

def dump_java_native_methods(res):
	for cls in res:
		print '-' * 40
		print cls
		for method in res[cls]:
			print '\t' + method

	print 'done'

def parse_type_name(pair):
	return pair.split()

def get_native_method_comment(ni, type_name_pairs, for_jni):
	global type_signature_map
	return_type, method_name = parse_type_name(type_name_pairs[0])
	signature = []
	for i in range(1, len(type_name_pairs)):
		type, name = parse_type_name(type_name_pairs[i])
		signature.append(type_signature_map[type])
	signature = '(' + ''.join(signature) + ')' + type_signature_map[return_type]
	return '''/*%s
 * Class:     %s
 * Method:    %s
 * Signature: %s
 */''' % ('' if for_jni else '*', ni, method_name, signature)

def get_native_method_name(ni, method_name, for_jni):
	global kni_fix_map
	if not for_jni and ni in kni_fix_map and method_name in kni_fix_map[ni]:
		return kni_fix_map[ni][method_name]
	return 'Java_%s_%s' % (ni, method_name)

def get_kni_method_info(ni, type_name_pairs):
	global type_signature_map
	return_type, method_name = parse_type_name(type_name_pairs[0])
	signature = []
	for i in range(1, len(type_name_pairs)):
		type, name = parse_type_name(type_name_pairs[i])
		signature.append(type_signature_map[type])
	signature = '(' + ''.join(signature) + ')' + type_signature_map[return_type]
	return (method_name, signature, get_native_method_name(ni, method_name, False))

def get_jni_method_declare(jni, is_static, type_name_pairs):
	global jni_type_map
	return_type, method_name = parse_type_name(type_name_pairs[0])
	signature = ['JNIEnv *', 'jclass' if is_static else 'jobject']
	for i in range(1, len(type_name_pairs)):
		type, name = parse_type_name(type_name_pairs[i])
		signature.append(jni_type_map[type])
	signature = ', '.join(signature)
	return '''JNIEXPORT %s JNICALL %s
  (%s);''' % (jni_type_map[return_type], get_native_method_name(jni, method_name, True), signature)

def get_kni_method_declare(jni, type_name_pairs):
	return_type, method_name = parse_type_name(type_name_pairs[0])
	return '''void %s(const u4* args, JValue* pResult);''' % (get_native_method_name(jni, method_name, False))

def generate_jni_header(java_native_methods):
	global modifiers_pat
	global type_name_pat
	global throws_pat

	modifiers_pat = re.compile(modifiers_pat)
	type_name_pat = re.compile(type_name_pat)
	throws_pat = re.compile(throws_pat)

	static_pat = re.compile(r'\sstatic\s')

	for cls in java_native_methods:
		jni = cls.replace('.', '_')
		fname = os.path.join('jni', jni + '.h')
		fp = open(fname, 'w')
		fp.write('''/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class %s */

#ifndef _Included_%s
#define _Included_%s
#ifdef __cplusplus
extern "C" {
#endif
''' % (jni, jni, jni))
		for method in java_native_methods[cls]:
			mthd = modifiers_pat.sub('', method)
			mthd = throws_pat.sub('', mthd)
			is_static = static_pat.search(method) is not None
			pairs = type_name_pat.findall(mthd)
			comment = get_native_method_comment(jni, pairs, True)
			fp.write(comment)
			fp.write('\n')
			fp.write(get_jni_method_declare(jni, is_static, pairs))
			fp.write('\n')
			fp.write('\n')
		fp.write('''#ifdef __cplusplus
}
#endif
#endif
''')
		fp.close()

def get_header_macro(fname):
	return '__' + fname.replace('.', '_').upper().replace('NATIVE', 'NATIVE_') + '__'

def generate_kni_header(java_native_methods, out_dir):
	global modifiers_pat
	global type_name_pat
	global throws_pat

	modifiers_pat = re.compile(modifiers_pat)
	type_name_pat = re.compile(type_name_pat)
	throws_pat = re.compile(throws_pat)

	for cls in java_native_methods:
		jni = cls.replace('.', '_')
		header = 'native' + cls.split('.')[-1] + '.h'
		macro = get_header_macro(header)
		fname = os.path.join(out_dir, header)
		fp = open(fname, 'w')
		fp.write('''/* DO NOT EDIT THIS FILE - it is machine generated */
#include <dthing.h>
#include <kni.h>

/* Header for class %s */

#ifndef %s
#define %s

#ifdef __cplusplus
extern "C" {
#endif

''' % (cls, macro, macro))
		for method in java_native_methods[cls]:
			mthd = modifiers_pat.sub('', method)
			mthd = throws_pat.sub('', mthd)
			pairs = type_name_pat.findall(mthd)
			comment = get_native_method_comment(jni, pairs, False)
			fp.write(comment)
			fp.write('\n')
			fp.write(get_kni_method_declare(jni, pairs))
			fp.write('\n')
			fp.write('\n')
		fp.write('''#ifdef __cplusplus
}
#endif
#endif // %s
''' % (macro))
		fp.close()

def generate_kni_array(java_native_methods, out_header):
	global modifiers_pat
	global type_name_pat
	global throws_pat

	modifiers_pat = re.compile(modifiers_pat)
	type_name_pat = re.compile(type_name_pat)
	throws_pat = re.compile(throws_pat)

	classes = java_native_methods.keys()

	fp = open(out_header, 'w')
	fp.write('''/* DO NOT EDIT THIS FILE - it is machine generated */
#include <kni.h>

/* Header for native method signature information. */

#ifndef __NATIVE_METHODS_H__
#define __NATIVE_METHODS_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* name;
    const char* signature;
    KniFunc fnPtr;
} KniNativeMethodInfo;

typedef struct {
    const char* classpath;
    KniNativeMethodInfo* methods;
    int methodCount;
} KniClassNativeMethodInfo;

#define NATIVE_CLASSES_COUNT %d

''' % (len(classes)))

	classes.sort()
	classes_info = []
	cls_col_width = [0, 0] # calculate width of each column of class information
	for cls in classes:
		jni = cls.replace('.', '_')
		mthd_array_name = 'gJava_' + jni
		clspath = 'L%s;' % (cls.replace('.', '/'))
		mthds = []
		mthd_col_width = [0, 0] # calculate width of each column of method information
		for method in java_native_methods[cls]:
			mthd = modifiers_pat.sub('', method)
			mthd = throws_pat.sub('', mthd)
			pairs = type_name_pat.findall(mthd)
			method_name, signature, ni_method_name = get_kni_method_info(jni, pairs)
			mthds.append((method_name, signature, ni_method_name))
			if len(method_name) > mthd_col_width[0]:
				mthd_col_width[0] = len(method_name)
			if len(signature) > mthd_col_width[1]:
				mthd_col_width[1] = len(signature)

		fp.write('''/* %s native APIs */
KniNativeMethodInfo %s[%d] = {
''' % (cls, mthd_array_name, len(mthds)))

		for method_name, signature, ni_method_name in mthds:
			fp.write('    {"%s",%s "%s",%s (KniFunc)%s},\n' % (method_name, ' ' * (mthd_col_width[0] - len(method_name)), signature, ' ' * (mthd_col_width[1] - len(signature)), ni_method_name))

		fp.write('};\n\n');

		classes_info.append((clspath, mthd_array_name, len(mthds)))
		if len(clspath) > cls_col_width[0]:
			cls_col_width[0] = len(clspath)
		if len(mthd_array_name) > cls_col_width[1]:
			cls_col_width[1] = len(mthd_array_name)

	fp.write('KniClassNativeMethodInfo gNativeMthTab[NATIVE_CLASSES_COUNT] = {\n')

	for clspath, mthd_array_name, count in classes_info:
		fp.write('    {"%s",%s %s,%s %d},\n' % (clspath, ' ' * (cls_col_width[0] - len(clspath)), mthd_array_name, ' ' * (cls_col_width[1] - len(mthd_array_name)), count))

	fp.write('''};

#ifdef __cplusplus
}
#endif
#endif // __NATIVE_METHODS_H__
''')
	fp.close()

if __name__ == '__main__':
	(options, args) = parser.parse_args()
	for opt in [options.src_header, options.out_dir, options.java_src_dir]:
		if opt is None:
			parser.print_help()
			sys.exit(2)
		if not os.path.exists(opt):
			sys.exit("'%s' doesn't exist!" % (opt))

	out_header = os.path.join(options.out_dir, os.path.basename(options.src_header))

	res = get_native_methods(options.java_src_dir)
	#dump_java_native_methods(res)
	#generate_jni_header(res)
	generate_kni_header(res, options.out_dir)
	generate_kni_array(res, out_header)
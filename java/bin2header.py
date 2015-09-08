#! /usr/bin/env python

import sys, os

def usage():
	sys.stdout.write('''Usage: %s [-h|--help] source destination

Convert the specified file to C source array header.

  source       The file to convert, binary or normal text file.
  destination  The output C header file with data array.
''' % os.path.basename(sys.argv[0]))
	sys.exit(99)

def write_line_end_if_need(fp, idx):
	if idx % 16 == 0:
		fp.seek(-1, os.SEEK_CUR) # remove trailing space
		fp.write('\n\t')

def main(argv):
	if '-h' in argv or '--help' in argv:
		usage()
		return
	if len(argv) != 2:
		usage()
		return

	srcFile = argv[0]
	dstFile = argv[1]

	if not os.path.isfile(srcFile):
		sys.stderr.write('Failed to find source file ' + srcFile)

	if not os.path.isfile(dstFile):
		dstFolder, fname = os.path.split(dstFile)
		if len(dstFolder) > 0 and not os.path.exists(dstFolder):
			try:
				os.makedirs(dstFolder)
			except OSError, e:
				sys.stderr.write('failed to create folder for destination file %s:\n\t%s\n' % (dstFile, e))
				return

	fsize = os.path.getsize(srcFile)
	fd = os.open(srcFile, os.O_RDONLY | os.O_BINARY)
	buf = os.read(fd, fsize)
	os.close(fd)

	sys.stdout.write('%s loaded\nconverting ...\n' % (srcFile))

	fp = open(dstFile, 'w')
	fp.write(r'''
/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 *
 * Created:         $Date: 2013/09/22
 * Last modified:	$Date: 2013/09/22
 * Version:         $ID: dalvik_core.h#1
 */

/*
 * Generated with perl script;
 *
 * Typical usage of this file is:
 *     static const uint8_t localdata[] =
 *     #include <dalvik_core.h>
 */



/* Generated from D:\dthing\java\bin\classes.dex */

{
	''')
	for i in range(fsize - 1):
#		sys.stdout.write('%.2f%% %d/%d\r' % (i * 100 / fsize, i, fsize))

		hex = '0x%02x, ' % (ord(buf[i : i + 1]))
		write_line_end_if_need(fp, i)
		fp.write(hex)
	write_line_end_if_need(fp, fsize - 1)
	fp.write('0x%02x\n};\n' % ord(buf[-1]))
	fp.close()

	sys.stdout.write('%s generated\n' % (dstFile))

if __name__ == "__main__":
	main(sys.argv[1:])

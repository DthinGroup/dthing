#!/usr/local/bin/perl -w

use English;
use strict;
use Config;
use File::Basename;

#
#	Prototypes
#
sub main();
sub usage(;@);
sub parseObj($);
sub parseList($);

my $s;
my (%table) = ();
my ($verbose) = 0;

#
#	Perform the work of the script.
#
if ($Config{osname} =~ /win32/i)
{
	$s = "\\";
	fileparse_set_fstype("MSDOS");
}
else
{
	$s = "/";
	fileparse_set_fstype("UNIX");
}

exit main();

#
#	main
#
#	Inputs
#		- the objects whose symbols are to be mapped
#
sub main()
{
	my (@argv) = @ARGV;                  # local copy of command-line args

	if (@argv != 2)
	{
		usage();
	}

	my ($binFile) = shift @argv;
	my ($cFile) = shift @argv;

	open (BINFILE, $binFile) or die "Couldn't open $binFile";
	binmode BINFILE;

	open (CFILE, ">".$cFile) or die "Couldn't open $cFile";
	print CFILE <<EOF

/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         \$Date: 2013/09/22
 * Last modified:	\$Date: 2013/09/22  
 * Version:         \$ID: dalvik_core.h#1
 */

/*
 * Generated with perl script;
 *
 * Typical usage of this file is:
 *     static const uint8_t localdata[] =
 *     #include <dalvik_core.h>
 */



EOF
;

	my ($name, $path, $suffix) = fileparse($binFile, '\..*');
	my ($newSuffix) = $suffix;
	$newSuffix =~ s/\./_/;
	printf CFILE "/* Generated from $name$suffix */\n\n";
	#printf CFILE "static const uint8_t %s%s[] = {", $name, $newSuffix;
    printf CFILE "{";

	my ($byte);
	my ($count) = 0;
	while (read BINFILE, $byte, 1) {
		if (($count & 15) == 0) {
			printf CFILE "\n\t0x%02x,", ord $byte;
		} else {
			printf CFILE " 0x%02x,", ord $byte;
		}

		++$count;
	}

	print CFILE "\n};\n";

	close BINFILE;
	close CFILE;

	exit 0;
}

#
#	Print the given error message and output a usage string
#	then exit.
#
sub usage(;@)
{
	my (@msg) = @_;

	@msg && print STDERR "\n\t@msg\n";
	print STDERR "\nUsage:",
		"\t$PROGRAM_NAME <binaryfile> <.h file>\n\n";

	exit ((@msg) ? -1 : 0);
}


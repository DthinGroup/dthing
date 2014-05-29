MINCPATH =
MINCPATH += rtos/export/inc
MINCPATH += common/export/inc
MINCPATH += CAF/Template/include
MINCPATH += wre/wrekernel/include/sdk
MINCPATH += MSL/l4/inc
MINCPATH += MSL/sim/inc
MINCPATH += MSL/gas/inc

MINCPATH += BASE/dthing/base/inc
MINCPATH += BASE/dthing/porting/inc
MINCPATH += BASE/dthing/rams/inc
MINCPATH += BASE/dthing/vm/impl
MINCPATH += BASE/dthing/vm/inc
MINCPATH += BASE/dthing/vm
MINCPATH += BASE/dthing/zlib

MSRCPATH  = BASE/dthing/main
MSRCPATH += BASE/dthing/base/src
MSRCPATH += BASE/dthing/porting/src
MSRCPATH += BASE/dthing/rams/src
MSRCPATH += BASE/dthing/vm/impl
MSRCPATH += BASE/dthing/vm/src
MSRCPATH += BASE/dthing/vm
MSRCPATH += BASE/dthing/zlib

SOURCES	 = ansicextend.c encoding.c leb128.c properties.c trace.c \
           dthing_main.c \
           opl_core.c opl_file.c opl_mm.c opl_net.c opl_rams.c \
           jarparser.c rAms.c \
           init.c \
           nativeAsyncIO.c nativeClass.c nativeFile.c nativeFloat.c nativeMath.c nativeNet.c nativeObject.c nativeRuntime.c nativeScheduler.c nativeString.c nativeSystem.c nativeThread.c nativeThrowable.c nativeTimeZone.c \
           accesscheck.c annotation.c array.c AsyncIO.c class.c classmisc.c compact.c dexclass.c dexfile.c dexproto.c dthread.c dvmdex.c exception.c gc.c hash.c heap.c interpApi.c interpCore.c interpOpcode.c interpStack.c kni.c mm.c native.c object.c rawdexfile.c resolve.c schd.c sync.c typecheck.c upcall.c utfstring.c vmTime.c voiderr.c \
           adler32.c compress.c crc32.c deflate.c infback.c inffast.c inflate.c inftrees.c trees.c uncompr.c zutil.c           
#  
           
######################################################
#
# Copyright [2013] - [2014] Yarlungsoft
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
######################################################


######################################################
# Global dfeinitions
######################################################

CDEFINES+=-D_UINTPTR_T_DEFINED -D_INT32_T_DEFINED -D_UINT32_T_DEFINED

GENPATH=$(CURDIR)/gen
OBJPATH=$(GENPATH)/$(TOOLCHAIN)/objects
LIBPATH=$(GENPATH)/$(TOOLCHAIN)/libs
BINPATH=$(GENPATH)/$(TOOLCHAIN)/bins
DIRS=$(GENPATH) $(OBJPATH) $(LIBPATH) $(BINPATH)
SHELL=cmd.exe

## Disable below code since meet dir issues in both gmake and make ##
#MAKEDIRS=@FOR %%i IN ($(DIRS)) DO IF NOT EXIST %%i mkdir %%~fi

MAKEDIRS=
MAKEOBJDIRS=@$(subst /,\,$(SHELL)) /c IF NOT EXIST $(subst /,\,$(dir $@)) DO MD $(subst /,\,$(dir $@))
RM=$(subst /,\,$(SHELL)) /c DEL /Q /S /F
RMDIR=$(subst /,\,$(SHELL)) /c RD /Q /S
MKDIR=$(subst /,\,$(SHELL)) /c MD

######################################################
# Compiler info
######################################################
#
# rvct compiler settings
#
ifeq ($(TOOLCHAIN), rvct)
CC=tcc.exe
AR=armar.exe
LINK=armlink.exe
OBJEXT=o
LIBEXT=a
DLLEXT=so
EXEEXT=elf
CDEFINES+=-DDEBUG -DANSI -DNO_POSIX_INCLUDES
CFLAGS=--LI --cpu ARM926EJ-S -O3 --debug --apcs /interwork/noswst --enum_is_int -D_RTOS --loose_implicit_cast -W

COMPILE=$(CC) $(CDEFINES) $(CFLAGS) $(subst /I,-I,$(INCLUDES)) -o $(subst /,\,$@) -c $(subst /,\,$<)

define BUILDVMLIB
$(AR) --create -c $(strip $(subst /,\,$(VMLIB))) $(VM_OBJECTS)
endef

define BUILDPLLIB
$(AR) --create -c $(strip $(subst /,\,$(PLLIB))) $(PORTING_OBJECTS)
endef

define BUILDZLIBLIB
$(AR) --create -c $(strip $(subst /,\,$(ZLIBLIB))) $(ZLIB_OBJECTS)
endef

define BUILDRAMSLIB
$(AR) --create -c $(strip $(subst /,\,$(RAMSLIB))) $(RAMS_OBJECTS)
endef

define BUILDLIBLIB
$(AR) --create -c $(LIBPATH)/DthingVM.a $(LIBS)
endef

#
# vc compiler settings
#
else
ifeq ($(TOOLCHAIN), msvc)

CC=cl.exe
AR=link.exe -lib
LINK=link.exe

# generated file suffix #
OBJEXT=obj
LIBEXT=lib
DLLEXT=dll
EXEEXT=exe

# compiler parameters #
CDEFINES+=-DWIN32 -DDEBUG -D_UNICODE
CFLAGS=/Zi /nologo /Zp1 /MTd /W3 /RTC1 /EHsc /Od /Fd"$(BINPATH)/vc80.pdb" $(CDEFINES) $(INCLUDES)
WIN32LIBS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib

COMPILE=$(CC) $(CFLAGS) /Fo$@ /c $<

LINKFLAGS=/INCREMENTAL /NOLOGO /MANIFEST /MANIFESTFILE:"$(BINPATH)/DthingVM.exe.intermediate.manifest" /DEBUG /PDB:$(BINPATH)/DthingVM.pdb /SUBSYSTEM:CONSOLE /MACHINE:X86 /ERRORREPORT:PROMPT $(WIN32LIBS)

define BUILDVMLIB
$(AR) /OUT:$(strip $(subst /,\,$(VMLIB))) $(VM_OBJECTS)
endef

define BUILDPLLIB
$(AR) /OUT:$(strip $(subst /,\,$(PLLIB))) $(PORTING_OBJECTS)
endef

define BUILDZLIBLIB
$(AR) /OUT:$(strip $(subst /,\,$(ZLIBLIB))) $(ZLIB_OBJECTS)
endef

define BUILDRAMSLIB
$(AR) /OUT:$(strip $(subst /,\,$(RAMSLIB))) $(RAMS_OBJECTS)
endef

define BUILDLIBLIB
$(AR) /OUT:$(LIBPATH)/DthingVM.lib $(LIBS)
endef

define BUILDEXE
$(LINK) /OUT:$(BINPATH)/DthingVM.exe $(LINKFLAGS) $(LIBS) $(PLLIB) $(BIN_OBJECTS)
endef

else
$(warning ***Unknow toolchain: $(TOOLCHAIN), assume as msvc)
#TOOLCHAIN=msvc
endif

endif

######################################################
# Source files
######################################################
#
# includes files
#
INCLUDES = \
    /Ibase/inc \
    /Iporting/inc \
    /Irams/inc \
    /Ivm \
    /Ivm/inc \
    /Ivm/impl \
    /Izlib



#
# main files
#
BIN_SOURCES =


#
# es & rams files
#
RAMS_SOURCES = \
    rams/src/eventsystem.c \
    rams/src/jarparser.c \
    rams/src/vm_app.c

#
# vm base files
#
VM_SOURCES = \
    base/src/ansicextend.c \
    base/src/encoding.c \
    base/src/leb128.c \
    base/src/properties.c \
    base/src/trace.c

#
# vm core files
#
VM_SOURCES += \
    vm/init.c \
    vm/src/accesscheck.c \
    vm/src/annotation.c \
    vm/src/array.c \
    vm/src/AsyncIO.c \
    vm/src/class.c \
    vm/src/classmisc.c \
    vm/src/compact.c \
    vm/src/dexcatch.c \
    vm/src/dexclass.c \
    vm/src/dexfile.c \
    vm/src/dexproto.c \
    vm/src/dthread.c \
    vm/src/dvmdex.c \
    vm/src/exception.c \
    vm/src/gc.c \
    vm/src/hash.c \
    vm/src/heap.c \
    vm/src/interpApi.c \
    vm/src/interpCore.c \
    vm/src/interpOpcode.c \
    vm/src/interpStack.c \
    vm/src/kni.c \
    vm/src/mm.c \
    vm/src/native.c \
    vm/src/object.c \
    vm/src/rawdexfile.c \
    vm/src/resolve.c \
    vm/src/schd.c \
    vm/src/sync.c \
    vm/src/typecheck.c \
    vm/src/upcall.c \
    vm/src/utfstring.c \
    vm/src/voiderr.c
#vmTime.c build in spd

#
# vm porting files
#
VM_SOURCES += \
    vm/impl/nativeAsyncIO.c \
    vm/impl/nativeClass.c \
    vm/impl/nativeFile.c \
    vm/impl/nativeFileInputStream.c \
    vm/impl/nativeFileOutputStream.c \
    vm/impl/nativeFloat.c \
    vm/impl/nativeMath.c \
    vm/impl/nativeNetNativeBridge.c \
    vm/impl/nativeObject.c \
    vm/impl/nativeOTADownload.c \
    vm/impl/nativeRuntime.c \
    vm/impl/nativeScheduler.c \
    vm/impl/nativeString.c \
    vm/impl/nativeSystem.c \
    vm/impl/nativeSystemInputStream.c \
    vm/impl/nativeSystemPrintStream.c \
    vm/impl/nativeThread.c \
    vm/impl/nativeThrowable.c \
    vm/impl/nativeTimeZone.c \
    vm/impl/nativeMain.c \

#vm/impl/nativeDouble.c \
#vm/impl/nativeDthread.c \

#
# zlib files
#
ZLIB_SOURCES = \
    zlib/adler32.c \
    zlib/compress.c \
    zlib/crc32.c \
    zlib/deflate.c \
    zlib/infback.c \
    zlib/inffast.c \
    zlib/inflate.c \
    zlib/inftrees.c \
    zlib/trees.c \
    zlib/uncompr.c \
    zlib/zutil.c


#
# porting files
#
PORTING_SOURCES =


#
# obj files
#
VM_OBJECTS = $(foreach i,$(VM_SOURCES),$(OBJPATH)/$(basename $i).$(OBJEXT))
PORTING_OBJECTS = $(foreach i,$(PORTING_SOURCES),$(OBJPATH)/$(basename $i).$(OBJEXT))
ZLIB_OBJECTS = $(foreach i,$(ZLIB_SOURCES),$(OBJPATH)/$(basename $i).$(OBJEXT))
RAMS_OBJECTS = $(foreach i,$(RAMS_SOURCES),$(OBJPATH)/$(basename $i).$(OBJEXT))
BIN_OBJECTS = $(foreach i,$(BIN_SOURCES),$(OBJPATH)/$(basename $i).$(OBJEXT))
OBJECTS = $(VM_OBJECTS) $(PORTING_OBJECTS) $(ZLIB_OBJECTS) $(RAMS_OBJECTS)

#
# lib files
#
VMLIB = $(LIBPATH)/dalvm.$(LIBEXT)
PLLIB = $(LIBPATH)/porting.$(LIBEXT)
ZLIBLIB = $(LIBPATH)/zlib.$(LIBEXT)
RAMSLIB = $(LIBPATH)/rams.$(LIBEXT)

LIBS=$(VMLIB) $(ZLIBLIB) $(RAMSLIB)

######################################################
# dependence
######################################################

.PHONY: bin lib dirs clean

bin: lib $(PLLIB) $(BIN_OBJECTS)
	$(BUILDEXE)

lib: dirs $(LIBS)
	$(BUILDLIBLIB)

$(BIN_OBJECTS): $(OBJPATH)/%.$(OBJEXT) : %.c
	-$(MAKEOBJDIRS)
	$(COMPILE)

$(OBJECTS): $(OBJPATH)/%.$(OBJEXT) : %.c
	-$(MAKEOBJDIRS)
	$(COMPILE)

dirs:
	-$(MKDIR) $(subst /,\,$(OBJPATH))
	-$(MKDIR) $(subst /,\,$(BINPATH))
	-$(MKDIR) $(subst /,\,$(LIBPATH))
  $(MAKEDIRS)

clean:
	-$(RMDIR) $(subst /,\,$(OBJPATH))
	-$(RMDIR) $(subst /,\,$(BINPATH))
	-$(RMDIR) $(subst /,\,$(LIBPATH))

$(VMLIB): $(VM_OBJECTS)
	$(BUILDVMLIB)

$(PLLIB): $(PORTING_OBJECTS)
	$(BUILDPLLIB)

$(ZLIBLIB): $(ZLIB_OBJECTS)
	$(BUILDZLIBLIB)

$(RAMSLIB): $(RAMS_OBJECTS)
	$(BUILDRAMSLIB)



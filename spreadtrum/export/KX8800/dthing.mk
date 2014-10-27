MINCPATH =
MINCPATH += rtos/export/inc
MINCPATH += common/export/inc
MINCPATH += CAF/Template/include
#MINCPATH += wre/wrekernel/include/sdk
MINCPATH += BASE/PS/export/inc
MINCPATH += BASE/l4/export/inc
MINCPATH += BASE/sim/export/inc
#MINCPATH += MSL/gas/inc
MINCPATH += daps/export/inc

MINCPATH += BASE/dthing/base/inc
MINCPATH += BASE/dthing/porting/inc
MINCPATH += BASE/dthing/rams/inc
MINCPATH += BASE/dthing/vm/impl
MINCPATH += BASE/dthing/vm/inc
MINCPATH += BASE/dthing/vm
#MINCPATH += BASE/dthing/www/inc
MINCPATH += BASE/dthing/zlib

MSRCPATH  = BASE/dthing/main
MSRCPATH += BASE/dthing/base/src
MSRCPATH += BASE/dthing/porting/src
MSRCPATH += BASE/dthing/rams/src
MSRCPATH += BASE/dthing/vm/impl
MSRCPATH += BASE/dthing/vm/src
MSRCPATH += BASE/dthing/vm
#MSRCPATH += BASE/dthing/www/src
MSRCPATH += BASE/dthing/zlib

SOURCES	 = ansicextend.c encoding.c leb128.c properties.c trace.c \
           dthing_main.c \
           opl_core.c opl_es.c opl_file.c opl_mm.c opl_net.c opl_rams.c \
           eventsystem.c jarparser.c ramsclient.c \
           init.c \
           nativeAsyncIO.c nativeATCommandConnection.c nativeBatteryManager.c nativeClass.c nativeEarManager.c nativeFile.c nativeFileInputStream.c nativeFileOutputStream.c nativeFloat.c nativeLCD.c nativeMath.c nativeMicophone.c nativeMyMessageSender.c nativeMySmsConnectionThread.c nativeNet.c nativeObject.c nativeOTADownload.c nativePCMChannel.c nativeRuntime.c nativeScheduler.c nativeSimManager.c nativeSpeakerManager.c nativeString.c nativeSystem.c nativeThread.c nativeThrowable.c nativeTimeZone.c nativeUSBConnection.c \
           accesscheck.c annotation.c array.c AsyncIO.c class.c classmisc.c compact.c dexcatch.c dexclass.c dexfile.c dexproto.c dthread.c dvmdex.c exception.c gc.c hash.c heap.c interpApi.c interpCore.c interpOpcode.c interpStack.c kni.c mm.c native.c object.c rawdexfile.c resolve.c schd.c sync.c typecheck.c upcall.c utfstring.c vmTime.c voiderr.c \
           adler32.c compress.c crc32.c deflate.c infback.c inffast.c inflate.c inftrees.c trees.c uncompr.c zutil.c
           
#nativeADCManager.c nativeAsyncIO.c nativeATCommandConnection.c nativeBatteryManager.c nativeClass.c nativeEarManager.c nativeFile.c nativeFileInputStream.c nativeFileOutputStream.c nativeFloat.c nativeGpio.c nativeI2CManager.c nativeLCD.c nativeMath.c nativeMicophone.c nativeMyMessageSender.c nativeMySmsConnectionThread.c nativeNet.c nativeObject.c nativeOTADownload.c nativePCMChannel.c nativePWMManager.c nativeRuntime.c nativeScheduler.c nativeSimManager.c nativeSpeakerManager.c nativeSPIManager.c nativeString.c nativeSystem.c nativeThread.c nativeThrowable.c nativeTimeZone.c nativeUSBConnection.c \           
#HTAABrow.c 
#HTCache.c HTEvent.c HTIcons.c HTMLGen.c HTProfil.c HTTChunk.c HTUser.c HTAAUtil.c HTChannl.c HTEvtLst.c HTInet.c HTMLPDTD.c HTProt.c HTTP.c HTWWWStr.c HTANSI.c HTChunk.c HTFSave.c HTInit.c HTMemLog.c HTProxy.c HTTPGen.c HTWriter.c HTAccess.c HTConLen.c HTFWrite.c HTLib.c HTMemory.c HTReader.c HTTPReq.c HTXParse.c HTAlert.c HTCookie.c HTFile.c HTLink.c HTMerge.c HTReqMan.c HTTPRes.c HText.c HTAnchor.c HTDNS.c HTFilter.c HTList.c HTMethod.c HTResponse.c HTTPServ.c SGML.c HTArray.c HTDescpt.c HTFormat.c HTLocal.c HTMulti.c HTRules.c HTTeXGen.c md5.c HTAssoc.c HTDialog.c HTGuess.c HTLog.c HTNet.c HTSChunk.c HTTee.c HTAtom.c HTDigest.c HTHInit.c HTMIME.c HTNetTxt.c HTSocket.c HTTimer.c HTBInit.c HTDir.c HTHeader.c HTMIMERq.c HTNoFree.c HTStream.c HTTrace.c HTBind.c HTEPtoCl.c HTHist.c HTMIMImp.c HTPEP.c HTString.c HTTrans.c HTBound.c HTError.c HTHome.c HTMIMPrs.c HTParse.c HTStyle.c HTUTree.c HTBufWrt.c HTEscape.c HTHost.c HTML.c HTPlain.c HTTCP.c HTUU.c
           
#

           
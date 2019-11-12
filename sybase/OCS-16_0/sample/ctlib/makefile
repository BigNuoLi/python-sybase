#############################################################################
#
# Microsoft makefile for sample programs
# --------------------------------------
#
# Copyright Notice and Disclaimer
# -------------------------------
#       (c) Copyright 2013.
#       SAP AG or an SAP affiliate company. All rights reserved.
#       Unpublished rights reserved under U.S. copyright laws.
#
#       SAP grants Licensee a non-exclusive license to use, reproduce,
#       modify, and distribute the sample source code below (the "Sample Code"),
#       subject to the following conditions:
#
#       (i) redistributions must retain the above copyright notice;
#
#       (ii) SAP shall have no obligation to correct errors or deliver
#       updates to the Sample Code or provide any other support for the
#       Sample Code;
#
#       (iii) Licensee may use the Sample Code to develop applications
#       (the "Licensee Applications") and may distribute the Sample Code in
#       whole or in part as part of such Licensee Applications, however in no
#       event shall Licensee distribute the Sample Code on a standalone basis;
#
#       (iv) and subject to the following disclaimer:
#       THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
#       INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
#       AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
#       SAP AG or an SAP affiliate company OR ITS LICENSORS BE LIABLE FOR ANY DIRECT, 
#	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#       BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
#       OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#       ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
#       TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
#       USE OF THE SAMPLE CODE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
#       DAMAGE.
#
#############################################################################

MAKEFILE=MAKEFILE

!ifndef SYBASE
SYBASEHOME=c:\sybase
!else
SYBASEHOME=$(SYBASE)
!endif

COMPILE_DEBUG = 1

# Compiler AND linker flags 
!ifdef COMPILE_DEBUG
CFLAGS = /W3 /MD /nologo /Zi /DWIN32  /Wp64 /D_CRT_SECURE_NO_DEPRECATE
LFLAGS= /MAP /SUBSYSTEM:CONSOLE /DEBUG /DEBUGTYPE:cv 
!else
CFLAGS = /W3 /MD /nologo /Od /DWIN32 /Wp64 /D_CRT_SECURE_NO_DEPRECATE
LFLAGS= /MAP /SUBSYSTEM:CONSOLE
!endif

ASYNCDEFS = -DUSE_SIG_HANDLER=0

HDRS = example.h exutils.h
MTHDRS = example.h thrdutil.h thrdfunc.h

#
# Where to get includes and libraries
#
# SYBASE is the environment variable for sybase home directory
#
SYBINCPATH =	$(SYBASEHOME)\$(SYBASE_OCS)\include
BLKLIB =	$(SYBASEHOME)\$(SYBASE_OCS)\lib\libsybblk64.lib
CTLIB =		$(SYBASEHOME)\$(SYBASE_OCS)\lib\libsybct64.lib 
CSLIB =		$(SYBASEHOME)\$(SYBASE_OCS)\lib\libsybcs64.lib
SYSLIBS =	kernel32.lib advapi32.lib

all: exasync compute csr_disp csr_disp_scrollcurs3 getsend rpc blktxt i18n multthrd usedir id_update firstapp exconfig secct wide_rpc wide_dynamic wide_curupd wide_compute twophase uctext lobdynamic lobrpc locator dynamic batch_dynamic batch_lang

uni: uni_firstapp uni_csr_disp uni_compute uni_blktxt uni_rpc

exasync compute csr_disp csr_disp_scrollcurs3 getsend rpc blktxt i18n multthrd usedir id_update firstapp exconfig secct twophase uctext lobdynamic lobrpc locator dynamic batch_dynamic batch_lang: $*.exe
	@echo Sample '$*' was built

wide_rpc wide_dynamic wide_curupd wide_compute: $*.exe
	@echo Sample '$*' was built

uni_firstapp uni_csr_disp uni_compute uni_blktxt uni_rpc: $*.exe
	@echo Sample '$*' was built

# The generalized how to make an .obj rule
.c.obj:
    cl /I. /I$(SYBINCPATH) $(ASYNCDEFS) $(CFLAGS) -Fo$@ -c $<

exutils_uctext.obj:
    cl /I. /I$(SYBINCPATH) -DPARTIAL_TEXT $(ASYNCDEFS) $(CFLAGS) -Fo$@ -c exutils.c

sample.exe: sample.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe sample.obj $(SYSLIBS) 
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest
	
exasync.exe: ex_alib.obj ex_amain.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe ex_alib.obj ex_amain.obj exutils.obj $(SYSLIBS) $(CTLIB)	$(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

compute.exe: compute.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

dynamic.exe: dynamic.obj exutils.obj wide_util.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj wide_util.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

wide_curupd.exe: wide_curupd.obj exutils.obj wide_util.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj wide_util.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

wide_dynamic.exe: wide_dynamic.obj exutils.obj wide_util.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj wide_util.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

wide_compute.exe: wide_compute.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

exconfig.exe: exconfig.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

firstapp.exe: firstapp.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

csr_disp.exe: csr_disp.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

csr_disp_scrollcurs3.exe: csr_disp_scrollcurs3.obj exutils.obj exutils2.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj exutils2.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

getsend.exe: getsend.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

rpc.exe: rpc.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

wide_rpc.exe: wide_rpc.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

blktxt.exe: blktxt.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB) $(BLKLIB) 
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

i18n.exe: i18n.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB) 
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

multthrd.exe: multthrd.obj thrdfunc.obj thrdutil.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj thrdfunc.obj thrdutil.obj $(SYSLIBS) $(CTLIB) $(CSLIB) 
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

usedir.exe: usedir.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB) 
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

id_update.exe: id_update.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(CTLIB) $(CSLIB) 
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

secct.exe: secct.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB) 
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

uni_firstapp.exe: uni_firstapp.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

uni_csr_disp.exe: uni_csr_disp.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

uni_compute.exe: uni_compute.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

uni_blktxt.exe: uni_blktxt.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB) $(BLKLIB) 
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

uni_rpc.exe: uni_rpc.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

twophase.exe: twophase.obj ctpr.obj ctxact.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj ctpr.obj ctxact.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

uctext.exe: uctext.obj exutils_uctext.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils_uctext.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

lobdynamic.exe: lobdynamic.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB) 
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

lobrpc.exe: lobrpc.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB) 
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

locator.exe: locator.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(CTLIB) $(CSLIB) 
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

batch_dynamic.exe: batch_dynamic.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

batch_lang.exe: batch_lang.obj exutils.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj exutils.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

clean:
	-del *.obj 
	-del *.map 
	-del *.exe 
	-del *.err
	-del *.ilk
	-del *.pdb

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
CFLAGS = /W3 /MD /nologo /Zi /DWIN32 /D_CRT_SECURE_NO_DEPRECATE
LFLAGS= /MAP /SUBSYSTEM:CONSOLE /DEBUG /DEBUGTYPE:cv 
!else
CFLAGS = /W3 /MD /nologo /Od /DWIN32 /D_CRT_SECURE_NO_DEPRECATE
LFLAGS= /MAP /SUBSYSTEM:CONSOLE
!endif

ASYNCDEFS = -DUSE_SIG_HANDLER=0

#
# -DSIDESHOW triggers compilation of extra cursor demonstration
# code in ex06ct.c. Set SIDESHOWDEFS to nothing if you want
# ex06ct to behave like DB-Library's example6.
#
SIDESHOWDEFS =	-DSIDESHOW=0

HDRS = dbtoctex.h 
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
DBLIB =		$(SYBASEHOME)\$(SYBASE_OCS)\lib\libsybdb64.lib
SYSLIBS =	kernel32.lib advapi32.lib
#
# Note that exutils.c and exutils.h are from
# the Client-Library product samples. We find them in the usual
# branch off $(SYBASEHOME)\$(SYBASE_OCS)\sample\ctlib.
#
EXU_DIR = 	$(SYBASE)\$(SYBASE_OCS)\sample\ctlib

SRC_EXU =	$(EXU_DIR)\exutils.c
EXU_HDR =	$(EXU_DIR)\exutils.h

OBJ_EXU = 	$(SRC_EXU:.c=.obj)


# The generalized how to make an .obj rule
.c.obj:
    cl /I. /I$(SYBINCPATH) /I$(EXU_DIR) $(ASYNCDEFS) $(SIDESHOWDEFS) $(CFLAGS) -Fo$@ -c $<


# dbfirst is a dblib application and thus only supported when dblibrary
# is installed.
# Therefore dbfirst must be made separately if all the above conditions
# are fulfilled.
all: ctfirst ex01ct ex02ct ex03ct ex04ct ex05ct ex06ct ex08ct ex09ct ex10ct ex11ct testctrb


dbfirst ctfirst ex01ct ex02ct ex03ct ex04ct ex05ct ex06ct ex08ct ex09ct ex10ct ex11ct testctrb: $*.exe
	@echo Sample '$*' was built

dbfirst.exe: dbfirst.obj $(HDRS) $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe dbfirst.obj $(SYSLIBS) $(DBLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

ctfirst.exe: ctfirst.obj $(HDRS) $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe ctfirst.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

ex01ct.exe: ex01ct.obj $(OBJ_EXU) $(HDRS) $(EXU_HDR) $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe ex01ct.obj $(OBJ_EXU) $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

ex02ct.exe: ex02ct.obj dbctutil.obj $(OBJ_EXU) $(HDRS) dbctutil.h $(EXU_HDR) $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe ex02ct.obj dbctutil.obj $(OBJ_EXU) $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

ex03ct.exe: ex03ct.obj $(OBJ_EXU) $(HDRS) $(EXU_HDR) $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe ex03ct.obj $(OBJ_EXU) $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

ex04ct.exe: ex04ct.obj dbctutil.obj ctrowbuf.obj $(OBJ_EXU) $(HDRS) dbctutil.h ctrowbuf.h $(EXU_HDR) $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe ex04ct.obj dbctutil.obj ctrowbuf.obj $(OBJ_EXU) $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

ex05ct.exe: ex05ct.obj dbctutil.obj ctrowbuf.obj $(HDRS) dbctutil.h ctrowbuf.h $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe ex05ct.obj dbctutil.obj ctrowbuf.obj $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

#
# ex06ct takes an optional compile-time define (-DSIDESHOW) for 
# compilation of additional cursor demonstration code.
#
ex06ct.exe: ex06ct.obj $(OBJ_EXU) $(HDRS) $(EXU_HDR) $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe ex06ct.obj $(OBJ_EXU) $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

ex08ct.exe: ex08ct.obj dbctutil.obj $(OBJ_EXU) $(HDRS) dbctutil.h $(EXU_HDR) $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe ex08ct.obj dbctutil.obj $(OBJ_EXU) $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

ex09ct.exe: ex09ct.obj txtimg.obj $(OBJ_EXU) $(HDRS) txtimg.h $(EXU_HDR) $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe ex09ct.obj txtimg.obj $(OBJ_EXU) $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

ex10ct.exe: ex10ct.obj txtimg.obj $(OBJ_EXU) $(HDRS) txtimg.h $(EXU_HDR) $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe ex10ct.obj txtimg.obj $(OBJ_EXU) $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

ex11ct.exe: ex11ct.obj $(OBJ_EXU) $(HDRS) $(EXU_HDR) $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe ex11ct.obj $(OBJ_EXU) $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

testctrb.exe: testctrb.obj dbctutil.obj ctrowbuf.obj $(OBJ_EXU) $(HDRS) dbctutil.h ctrowbuf.h $(EXU_HDR) $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe testctrb.obj dbctutil.obj ctrowbuf.obj $(OBJ_EXU) $(SYSLIBS) $(CTLIB) $(CSLIB)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest


clean:
	-del *.obj 
	-del $(OBJ_EXU)
	-del *.map 
	-del *.exe 
	-del *.err
	-del *.ilk
	-del *.pdb

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
# Description
# -----------
#	Sybase Makefile for ESQLC samples for Windows
#
#	This makefile makes various examples.
#	Set Environment variables:
#	SYBASE - the path to a full Sybase installation
#
#	nmake all  or nmake example1, nmake example2, nmake example4,
#		   nmake example5, nmake example6, nmake example7, 
#		   nmake exampleHA, nmake exampleHA2, nmake uni_example1 
#		   or nmake uni_example2
#
# 	To run the samples type: example1, example2, example4, example5, 
#		   example6,  example7, exampleHA, exampleHA2, uni_example1 or 
#		   uni_example2
#
#
CPR		= $(SYBASE)\$(SYBASE_OCS) 
SYBINCDIR 	= $(CPR)\include
SYBLIBDIR 	= -L$(CPR)\lib 
SYBLIBS 	= $(CPR)\lib\libsybcs64.lib \
		  $(CPR)\lib\libsybct64.lib 
SYSLIBS		= 
DEFINES		= -DWIN32

CC=cl
LINK32=link
LIB32=lib
CC_INCLUDES 	= -I$(SYBINCDIR)

#Set up the flags for the compiler
CC_MISC=/DDEBUG=1 /D_DEBUG=1 /DNET_DEBUG=1 /Od /Z7 /Wp64 
CC_WARNING=/W3
CC_GENERAL=/MD /nologo /c /DWIN32 /D_CRT_SECURE_NO_DEPRECATE
CFLAGS = $(CC_MISC) $(CC_WARNING) $(CC_INCLUDES) $(CC_GENERAL) 

###################################################################
#Set up the flags for the ESQL compiler
#CPREFLAGS=-l -d $(CPREFLAGS_EXTRA)

# Linker flags and switches
#LFLAGS=/MAP 
# for debug
LFLAGS=/MAP /DEBUG /DEBUGTYPE:cv

PROD_OBJS1 =  example1.obj sybesql.obj
PROD_OBJS2 =  example2.obj sybesql.obj
PROD_OBJS4 =  example4.obj sybesql.obj
PROD_OBJS5 =  example5.obj sybesql.obj
PROD_OBJS6 =  example6.obj sybesql.obj
PROD_OBJS7 =  example7.obj sybesql.obj
PROD_UNI_OBJS1 =  uni_example1.obj sybesql.obj
PROD_UNI_OBJS2 =  uni_example2.obj sybesql.obj
PROD_HA_OBJS1 =  exampleHA.obj sybesql.obj
PROD_HA_OBJS2 =  exampleHA2.obj sybesql.obj

.SUFFIXES: .obj .c .cbl .pc .pco 

all : example1 example2 example4 example5 example6 example7 exampleHA exampleHA2 uni_example1 uni_example2


# Build normal mode Executable
example1: $(PROD_OBJS1) 
	$(LINK32) @<<
/out:example1.exe 
$(SYBLIBS)
$(SYSLIBS)
$(LFLAGS) 
$(PROD_OBJS1)
kernel32.lib
advapi32.lib
<<
	mt -manifest $*.exe.manifest -outputresource:$*.exe
	rm $*.exe.manifest
	@echo example1.exe built.

# Build normal mode Executable
example2: $(PROD_OBJS2) 
	$(LINK32) @<<
/out:example2.exe 
$(SYBLIBS)
$(SYSLIBS)
$(LFLAGS) 
$(PROD_OBJS2)
kernel32.lib
advapi32.lib
<<
	mt -manifest $*.exe.manifest -outputresource:$*.exe
	rm $*.exe.manifest
	@echo example2.exe built.

# Build normal mode Executable
example4: $(PROD_OBJS4) 
	$(LINK32) @<<
/out:example4.exe 
$(SYBLIBS)
$(LFLAGS) 
$(PROD_OBJS4)
kernel32.lib
advapi32.lib
<<
	mt -manifest $*.exe.manifest -outputresource:$*.exe
	rm $*.exe.manifest
	@echo example4.exe built.

# Build normal mode Executable
example5: $(PROD_OBJS5) 
	$(LINK32) @<<
/out:example5.exe 
$(SYBLIBS)
$(LFLAGS) 
$(PROD_OBJS5)
kernel32.lib
advapi32.lib
<<
	mt -manifest $*.exe.manifest -outputresource:$*.exe
	rm $*.exe.manifest
	@echo example5.exe built.

# Build normal mode Executable
example6: $(PROD_OBJS6) 
	$(LINK32) @<<
/out:example6.exe 
$(SYBLIBS)
$(LFLAGS) 
$(PROD_OBJS6)
kernel32.lib
advapi32.lib
<<
	mt -manifest $*.exe.manifest -outputresource:$*.exe
	rm $*.exe.manifest
	@echo example6.exe built.

# Build normal mode Executable
example7: $(PROD_OBJS7) 
	$(LINK32) @<<
/out:example7.exe 
$(SYBLIBS)
$(LFLAGS) 
$(PROD_OBJS7)
kernel32.lib
advapi32.lib
<<
	mt -manifest $*.exe.manifest -outputresource:$*.exe
	rm $*.exe.manifest
	@echo example7.exe built.

# Build normal mode Executable
uni_example1: $(PROD_UNI_OBJS1) 
	$(LINK32) @<<
/out:uni_example1.exe 
$(SYBLIBS)
$(SYSLIBS)
$(LFLAGS) 
$(PROD_UNI_OBJS1)
kernel32.lib
advapi32.lib
<<
	mt -manifest $*.exe.manifest -outputresource:$*.exe
	rm $*.exe.manifest
	@echo uni_example1.exe built.

# Build normal mode Executable
uni_example2: $(PROD_UNI_OBJS2) 
	$(LINK32) @<<
/out:uni_example2.exe 
$(SYBLIBS)
$(SYSLIBS)
$(LFLAGS) 
$(PROD_UNI_OBJS2)
kernel32.lib
advapi32.lib
<<
	mt -manifest $*.exe.manifest -outputresource:$*.exe
	rm $*.exe.manifest
	@echo uni_example2.exe built.

# Build normal mode Executable
exampleHA: $(PROD_HA_OBJS1) 
	$(LINK32) @<<
/out:exampleHA.exe 
$(SYBLIBS)
$(SYSLIBS)
$(LFLAGS) 
$(PROD_HA_OBJS1)
kernel32.lib
advapi32.lib
<<
	mt -manifest $*.exe.manifest -outputresource:$*.exe
	rm $*.exe.manifest
	@echo exampleHA.exe built.

# Build normal mode Executable
exampleHA2: $(PROD_HA_OBJS2) 
	$(LINK32) @<<
/out:exampleHA2.exe 
$(SYBLIBS)
$(SYSLIBS)
$(LFLAGS) 
$(PROD_HA_OBJS2)
kernel32.lib
advapi32.lib
<<
	mt -manifest $*.exe.manifest -outputresource:$*.exe
	rm $*.exe.manifest
	@echo exampleHA2.exe built.

# Get the precompilation done here.
example1.c:	example1.cp
	$(CPR)\bin\cpre64 -CMSVC -O example1.c example1.cp

example2.c:	example2.cp
	$(CPR)\bin\cpre64 -CMSVC -m -O example2.c example2.cp

example4.c:	example4.cp
	$(CPR)\bin\cpre64 -CMSVC -m -O example4.c example4.cp

example5.c:	example5.cp
	$(CPR)\bin\cpre64 -CMSVC -m -O example5.c example5.cp

example6.c:	example6.cp
	$(CPR)\bin\cpre64 -CMSVC -m -O example6.c example6.cp

example7.c:	example7.cp
	$(CPR)\bin\cpre64 -CMSVC -m -O example7.c example7.cp

uni_example1.c:	uni_example1.cp
	$(CPR)\bin\cpre64 -CMSVC -O uni_example1.c uni_example1.cp

uni_example2.c:	uni_example2.cp
	$(CPR)\bin\cpre64 -CMSVC -m -O uni_example2.c uni_example2.cp

exampleHA.c:	exampleHA.cp
	$(CPR)\bin\cpre64 -CMSVC -O exampleHA.c exampleHA.cp

exampleHA2.c:	exampleHA2.cp
	$(CPR)\bin\cpre64 -CMSVC -m -O exampleHA2.c exampleHA2.cp

# Compiler command line, and C file dependency
# Normal dependencies for building Objects
example1.obj:	example1.c 
	$(CC) $(CFLAGS) example1.c 
example2.obj:	example2.c
	$(CC) $(CFLAGS) example2.c
uni_example1.obj:	uni_example1.c 
	$(CC) $(CFLAGS) uni_example1.c 
uni_example2.obj:	uni_example2.c
	$(CC) $(CFLAGS) uni_example2.c
example4.obj:	example4.c
	$(CC) $(CFLAGS) example4.c
example5.obj:	example5.c
	$(CC) $(CFLAGS) example5.c
example6.obj:	example6.c
	$(CC) $(CFLAGS) example6.c
example7.obj:	example7.c
	$(CC) $(CFLAGS) example7.c
exampleHA.obj:	exampleHA.c 
	$(CC) $(CFLAGS) exampleHA.c 
exampleHA2.obj:	exampleHA2.c
	$(CC) $(CFLAGS) exampleHA2.c
#sybesql.obj : $(SYBASE)\$(SYBASE_OCS)\include\sybesql.c
sybesql.obj : 
     $(CC) $(CFLAGS) -c \
	$(SYBASE)\$(SYBASE_OCS)\include\sybesql.c -o sybesql.obj

clean:
	-del *.obj 
	-del *.map 
	-del *.exe 
	-del *.err
	-del *.ilk
	-del *.pdb
	-del *.c

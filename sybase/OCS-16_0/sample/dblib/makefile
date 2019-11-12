#############################################################################
#
# Microsoft makefile for building SAP DB-Library Samples for Windows
# ------------------------------------------------------------------
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

COMPILE_DEBUG = 1

# Compiler and linker flags 
!ifdef COMPILE_DEBUG
CFLAGS = /W3 /MD /nologo /Z7 /Wp64 /D_CRT_SECURE_NO_DEPRECATE
LFLAGS= /MAP /SUBSYSTEM:CONSOLE /DEBUG /DEBUGTYPE:cv
!else
CFLAGS = /W3 /MD /nologo /Od /Wp64 /D_CRT_SECURE_NO_DEPRECATE
LFLAGS= /MAP /SUBSYSTEM:CONSOLE
!endif

SYSLIBS =	kernel32.lib advapi32.lib
SYBASELIBS =	$(SYBASE)\$(SYBASE_OCS)\lib\libsybdb64.lib

all: example1 example2 example3 example4 example5 example6 example7 \
     example8 example9 example10 example11 example12 bulkcopy twophase

example1 example2 example3 example4 example5 example6 example7 example8: $*.exe
	@echo Sample '$*' was built
	
example9 example10 example11 example12 bulkcopy twophase: $*.exe
	@echo Sample '$*' was built

# The generalized how to make an .obj rule
.c.obj:
    cl /I. /I$(SYBASE)\$(SYBASE_OCS)\include $(CFLAGS) -Fo$@ -c $<

example1.exe: example1.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

example2.exe: example2.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

example3.exe: example3.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

example4.exe: example4.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

example5.exe: example5.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

example6.exe: example6.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

example7.exe: example7.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

example8.exe: example8.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

example9.exe: example9.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

example10.exe: example10.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

example11.exe: example11.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

example12.exe: example12.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

bulkcopy.exe: bulkcopy.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

twophase.exe: twophase.obj $(MAKEFILE)
    link $(LFLAGS) -out:$*.exe $*.obj $(SYSLIBS) $(SYBASELIBS)
    mt -manifest $*.exe.manifest -outputresource:$*.exe
    rm $*.exe.manifest

clean:
	-del *.obj 
	-del *.map 
	-del *.exe 
	-del *.err
	-del *.pdb
	-del *.ilk


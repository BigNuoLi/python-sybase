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
#	Makefile for ESQL Cobol Samples for Windows NT
#
#	This makefile makes example1.exe through example6.exe 
#	Set environment variable SYBASE to Sybase installation path.
#
# 	To build samples: type  
#	nmake all or nmake example1, nmake example2, nmake example3
#	or nmake example4 or nmake example5 or nmake example6 or
#	nmake example7.
#
# 	To run the samples type: example1, example2, example3, example4,
# 	example5 or example6 or example7.
#

#Set up the executables
COBOL=cobol
LINK=cbllink

CPR		= $(SYBASE)\$(SYBASE_OCS) 
SYBINCDIR       = $(CPR)\include
SYBLIBDIR       = -L$(CPR)\lib
SYBLIBS         = $(CPR)\lib\libsybcs64.lib \
                  $(CPR)\lib\libsybct64.lib
SYSLIBS         =
DEFINES         = -DWIN32

PROD_OBJS1 =  example1.obj 
PROD_OBJS2 =  example2.obj 
PROD_OBJS3 =  example3.obj 
PROD_OBJS4 =  example4.obj 
PROD_OBJS5 =  example5.obj 
PROD_OBJS6 =  example6.obj 
PROD_OBJS7 =  example7.obj 

.SUFFIXES: .obj .c .cbl .pc .pco 

all : example1 example2 example3 example4 example5 example6 example7


# Build normal mode Executable
example1: $(PROD_OBJS1) 
	$(LINK) example1

example2: $(PROD_OBJS2) 
	$(LINK) example2

example3: $(PROD_OBJS3) 
	$(LINK) example3

example4: $(PROD_OBJS4) 
	$(LINK) example4

example5: $(PROD_OBJS5) 
	$(LINK) example5

example6: $(PROD_OBJS6) 
	$(LINK) example6

example7: $(PROD_OBJS7) 
	$(LINK) example7

# Precompiler dependency.
example1.cbl:	example1.pco
	$(CPR)\bin\cobpre64 example1.pco 

example2.cbl:	example2.pco
	$(CPR)\bin\cobpre64 example2.pco 

example3.cbl:	example3.pco
	$(CPR)\bin\cobpre64 example3.pco 

example4.cbl:	example4.pco
	$(CPR)\bin\cobpre64 example4.pco 

example5.cbl:	example5.pco
	$(CPR)\bin\cobpre64 example5.pco 

example6.cbl:	example6.pco
	$(CPR)\bin\cobpre64 example6.pco 

example7.cbl:	example7.pco
	$(CPR)\bin\cobpre64 example7.pco 

# Normal dependencies for building Objects
example1.obj: example1.cbl
     $(COBOL) example1.cbl omf(obj) initcall "libsybcobct64.dll";

example2.obj: example2.cbl
     $(COBOL) example2.cbl omf(obj) initcall "libsybcobct64.dll";

example3.obj: example3.cbl
     $(COBOL) example3.cbl omf(obj) initcall "libsybcobct64.dll";

example4.obj: example4.cbl
     $(COBOL) example4.cbl omf(obj) initcall "libsybcobct64.dll";

example5.obj: example5.cbl
     $(COBOL) example5.cbl omf(obj) initcall "libsybcobct64.dll";

example6.obj: example6.cbl
     $(COBOL) example6.cbl omf(obj) initcall "libsybcobct64.dll"; 

example7.obj: example7.cbl
     $(COBOL) example7.cbl omf(obj) initcall "libsybcobct64.dll"; 

clean:
	del *.obj *.int *.idy *.gnt *.exe *.cbl

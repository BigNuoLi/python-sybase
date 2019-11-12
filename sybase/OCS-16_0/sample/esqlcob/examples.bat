rem Copyright Notice and Disclaimer
rem -------------------------------
rem    (c) Copyright 2013.
rem    SAP AG or an SAP affiliate company. All rights reserved.
rem    Unpublished rights reserved under U.S. copyright laws.
rem
rem    SAP grants Licensee a non-exclusive license to use, reproduce,
rem    modify, and distribute the sample source code below (the "Sample Code"),
rem    subject to the following conditions:
rem
rem    (i) redistributions must retain the above copyright notice;
rem
rem    (ii) SAP shall have no obligation to correct errors or deliver
rem    updates to the Sample Code or provide any other support for the
rem    Sample Code;
rem
rem    (iii) Licensee may use the Sample Code to develop applications
rem    (the "Licensee Applications") and may distribute the Sample Code in
rem    whole or in part as part of such Licensee Applications, however in no
rem    event shall Licensee distribute the Sample Code on a standalone basis;
rem
rem    (iv) and subject to the following disclaimer:
rem    THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
rem    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
rem    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
rem    SAP AG or an SAP affiliate company OR ITS LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
rem    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
rem    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
rem    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
rem    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
rem    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
rem    USE OF THE SAMPLE CODE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
rem    DAMAGE.
rem
rem Description
rem -----------
rem 	This is a batch file for building esql/cobol programs
rem 	Set SYBASE before running the batch file
rem 	Make sure, you have set variables (path, COBDIR, etc) for 
rem 	running cobol compiler, cobpre64.
echo Building example1, example2, example3, example4, example5 and example6 example7
cobpre64 example1.pco
echo example3 and example4 need to run with ASE 15.0 or better
cobol example1 omf(obj) initcall "libsybcobct64.dll";
cbllink example1
echo example1 built.
cobpre64 example2.pco
cobol example2 omf(obj) initcall "libsybcobct64.dll";
cbllink example2
echo example2 built.
cobpre64 example3.pco
cobol example3 omf(obj) initcall "libsybcobct64.dll";
cbllink example3
echo example3 built.
cobpre64 example4.pco
cobol example4 omf(obj) initcall "libsybcobct64.dll";
cbllink example4
echo example4 built.
cobpre64 example5.pco
cobol example5 omf(obj) initcall "libsybcobct64.dll";
cbllink example5
echo example5 built.
cobpre64 example6.pco
cobol example6 omf(obj) initcall "libsybcobct64.dll";
cbllink example6
echo example6 built.
cobpre64 example7.pco
cobol example7 omf(obj) initcall "libsybcobct64.dll";
cbllink example7
echo example7 built.

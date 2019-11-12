/*
** Copyright Notice and Disclaimer
** -------------------------------
**      (c) Copyright 2013.
**      SAP AG or an SAP affiliate company. All rights reserved.
**      Unpublished rights reserved under U.S. copyright laws.
**
**      SAP grants Licensee a non-exclusive license to use, reproduce,
**      modify, and distribute the sample source code below (the "Sample Code"),
**      subject to the following conditions:
**
**      (i) redistributions must retain the above copyright notice;
**
**      (ii) SAP shall have no obligation to correct errors or deliver
**      updates to the Sample Code or provide any other support for the
**      Sample Code;
**
**      (iii) Licensee may use the Sample Code to develop applications
**      (the "Licensee Applications") and may distribute the Sample Code in
**      whole or in part as part of such Licensee Applications, however in no
**      event shall Licensee distribute the Sample Code on a standalone basis;
**
**      (iv) and subject to the following disclaimer:
**      THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
**      INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
**      AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
**      SAP AG or an SAP affiliate company OR ITS LICENSORS BE LIABLE FOR ANY DIRECT, 
**	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
**	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
**	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
**	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
**	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
**      USE OF THE SAMPLE CODE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
**      DAMAGE.
**
** Description
** -----------
** 	common header file for the DB-Library to 
**	Client-Library Migration sample programs.
**
*/

#define USER		"sa"
#define PASSWORD	""
#define LANGUAGE	"us_english"
#define SQLBUFLEN	255
#define ERR_CH		stderr
#define OUT_CH		stdout

#include <ctpublic.h>

/*
** The DB-Library header files defined ERREXIT and STDEXIT.
** With Client-Library, the application must define these
** symbols itself.
*/
#ifdef vms
#include <stsdef.h>
#define NORMAL_EXIT		(STS$M_INHIB_MSG | STS$K_SUCCESS)
#define ERROR_EXIT		(STS$M_INHIB_MSG | STS$K_ERROR)
#else
#define NORMAL_EXIT		0
#define ERROR_EXIT		1
#endif /* vms */

#include <stdio.h>
#include <stdlib.h>

/*
** Define a macro that exits if a function return code indicates
** failure. Accepts a CS_CONTEXT pointer, a Client-Library
** or CS-Library return code, and an error string. If the
** return code is not CS_SUCCEED, the context will be 
** cleaned up (if it is non-NULL), the error message is
** printed, and we exit to the operating system.
*/
#define EXIT_ON_FAIL(context, ret, str) \
  { if (ret != CS_SUCCEED) \
  { \
    fprintf(ERR_CH, "Fatal error line %d: (%ld) %s\n", \
            (int)__LINE__, (long)ret, (char *)str); \
    if (context != (CS_CONTEXT *) NULL) \
    { \
      (CS_VOID) ct_exit(context, CS_FORCE_EXIT); \
      (CS_VOID) cs_ctx_drop(context); \
    } \
    exit(ERROR_EXIT); \
  } }

/*
** In System 10, the ct_fetch() routine has 3 unused parameters.
** The following macro will save our little carpal tunnels from the 
** repetitive strain of typing CS_UNUSED again and again.
*/
#define CT10_FETCH(cmd, p_rows_read) \
  ct_fetch((cmd), CS_UNUSED, CS_UNUSED, CS_UNUSED, (p_rows_read))

/*
** For some platforms (e.g. windows 3.1), additional work needs to be done
** to insure that the output of some of the example programs can be displayed.
** This macro will insure that any setup is done for the platform.
**
** For windows, _wsetscreenbuf(_fileno(stdout), _WINBUFINT) will set 
** QuickWin's standard output screen buffer to unlimited size.  
*/

#if WIN3

#define EX_SCREEN_INIT() _wsetscreenbuf(_fileno(stdout), _WINBUFINF)

#else /* WIN3 */

#define EX_SCREEN_INIT() 

#endif /* WIN3 */











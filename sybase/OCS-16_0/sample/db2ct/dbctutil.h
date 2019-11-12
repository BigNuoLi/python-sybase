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
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctpublic.h>

/*
** Define a macro for return-code error checking within the
** dbct_ routines.
*/
#if !defined(ERR_CH) 
#define ERR_CH stderr
#endif
#define DBCT_FAIL_ON_FAIL(ret, str)\
{ \
  if (ret != CS_SUCCEED) \
  { \
    fprintf(ERR_CH, "dbctutil error: %s\n", str); \
    return CS_FAIL; \
  } \
}

extern CS_CHAR * CS_PUBLIC dbct_prtype PROTOTYPE ((CS_INT datatype));

extern CS_INT CS_PUBLIC dbct_datename PROTOTYPE ((
   CS_CONTEXT  *context,
   CS_CHAR     *buf,
   CS_INT      buflen,
   CS_INT      datepart,
   CS_DATETIME *datetime ));
	       
extern CS_RETCODE CS_PUBLIC dbct_parent_ctx PROTOTYPE ((
  CS_COMMAND    *cmd,
  CS_CONNECTION *conn,
  CS_CONTEXT    **ctxptr ));

extern CS_RETCODE CS_PUBLIC dbct_parent_conn PROTOTYPE ((
  CS_COMMAND    *cmd,
  CS_CONNECTION **connptr ));

extern CS_CHAR * CS_PUBLIC dbct_trim PROTOTYPE((CS_CHAR *str));

/*
** Date part codes for dbct_datename().
*/
#define DBDATE_YY       0       /* year         1753 - 9999 */
#define DBDATE_QQ       1       /* quarter      1 - 4 */
#define DBDATE_MM       2       /* month        January - December */
#define DBDATE_DW       3       /* weekday      Monday - Sunday */
#define DBDATE_DD       4       /* day          1 - 31 */
#define DBDATE_DY       5       /* dayofyear    1 - 366 */
#define DBDATE_HH       6       /* hour         0 - 23 */
#define DBDATE_MI       7       /* minute       0 - 59 */
#define DBDATE_SS       8       /* second       0 - 59 */
#define DBDATE_MS       9       /* millisecond  0 - 999 */
#define DBDATE_WK       10      /* week         1 - 54 (for leap years) */
#define	DBDATE_CY	11
#define	DBDATE_CW	12
#define	DBDATE_CD	13
	






















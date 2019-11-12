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
** 	Useful utility routines for converting DB-Library
** 	code to Client-Library code.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dbctutil.h"

CS_VOID dbct__error PROTOTYPE ((
	char *str
	));

/*
** dbct__error() -- report errors that happen in these
**   routines.
*/
CS_VOID
dbct__error(char *str)
{
	fprintf(ERR_CH, "DBCT Utilities error:\n%s", str);
	return;
} /* dbct__error() */

/*
** dbct_datename -- replace dbdatename() in DB-Library
**   programs.
**
** Parameters:
**   context -- pointer to CS_CONTEXT structure
**   buf -- character buffer to write date-part string into
**   buflen -- length, in bytes, of buffer
**   datepart -- code for the date part to put into
**       buffer. See DB-Library's dbdatename() man
**       page for possible values and meanings.
**   datetime -- pointer to a CS_DATETIME structure.
**  
** Returns:
**   The number of bytes written to buf, or -1 for error.
*/
CS_INT CS_PUBLIC
dbct_datename(CS_CONTEXT *context, CS_CHAR *buf, CS_INT buflen,
	      CS_INT datepart, CS_DATETIME *datetime)
{
	CS_RETCODE	ret;
	CS_DATEREC	daterec;
	CS_CHAR		tempbuf[64];
	CS_INT		len;
	
	/*
	** Make the caller's buffer fail-safe.
	*/
	if (buf == (CS_CHAR *)NULL || buflen <= 0)
	{
		return -1;
	}
	*buf = '\0';

	/*
	** Get a CS_DATEREC that matches the datetime.
	*/
	ret = cs_dt_crack(context, CS_DATETIME_TYPE, (CS_VOID *) datetime,
			  &daterec);
	if (ret != CS_SUCCEED)
	{
		dbct__error("dbct_datename: date crack failed.");
		return -1;
	}
 
	/*
	** Turn the CS_DATEREC field into the appropriate string.
	*/
	switch ((int) datepart)
	{
	  case DBDATE_YY:
		/* 
		** daterec.dateyear, as a character string.
		*/
		sprintf(tempbuf, "%ld", (long)daterec.dateyear);
		len = strlen(tempbuf) + 1;
		if (len > buflen)
		{
			dbct__error("dbct_datepart: Buffer too short.");
			return -1;
		}
		strcpy(buf, tempbuf);
		return len;

	  case DBDATE_QQ:
		/* Quarter: "1" to "4". 
		** Map daterec.datemonth to a quarter number. 
		*/
		if (buflen < 2)
		{
			dbct__error("dbct_datepart: Buffer too short.");
			return -1;
		}
		if (daterec.datemonth <= 2)
		{
			strcpy(buf, "1");
		}
		else if (daterec.datemonth <= 5)
		{
			strcpy(buf, "2");
		}
		else if (daterec.datemonth <= 8)
		{
			strcpy(buf, "3");
		}
		else
		{
			strcpy(buf, "4");
		}
		return 2;

	  case DBDATE_MM:
		/*
		** National language month name.
		** Use cs_dt_info() on daterec.datemonth to get the string.
		*/
		ret = cs_dt_info(context, CS_GET, NULL, CS_MONTH,
				 daterec.datemonth,
		tempbuf, CS_SIZEOF(tempbuf), &len);
		if (ret != CS_SUCCEED)
		{
			dbct__error("dbct_datename: cs_dt_info(CS_MONTH) failed.");
			return -1;
		}
		/* Make sure month name is null-terminated */
		if (tempbuf[len] != '\0')
		{
			len++;
			tempbuf[len] = '\0';
		}
		if (buflen < len)
		{
			dbct__error("dbct_datepart: Buffer too short.");
			return -1;
		}
		strcpy(buf, tempbuf);
		return len;

	  case DBDATE_DY:
		/*
		** Just print daterec.datedyear into the string.
		*/
		sprintf(tempbuf, "%ld", (long)daterec.datedyear);
		len = strlen(tempbuf) + 1;
		if (buflen < len)
		{
			dbct__error("dbct_datepart: Buffer too short.");
			return -1;
		}
		strcpy(buf, tempbuf);
		return len;

	  case DBDATE_DD:
		/*
		** Just print daterec.datedmonth into the string.
		*/
		sprintf(tempbuf, "%ld", (long)daterec.datedmonth);
		len = strlen(tempbuf) + 1;
		if (buflen < len)
		{
			dbct__error("dbct_datepart: Buffer too short.");
			return -1;
		}
		strcpy(buf, tempbuf);
		return len;

	  case DBDATE_WK:
		/*
		** Not implemented.
		*/
		dbct__error("dbct_datepart: Support for DBDATE_WK not implemented.");
		return -1;

	  case DBDATE_DW:
		/*
		** National language weekday name. Use cs_dt_info(CS_DAYNAME, 
		** daterec.datedweek).
		*/
		ret = cs_dt_info(context, CS_GET, NULL, CS_DAYNAME,
				 daterec.datedweek,
		tempbuf, CS_SIZEOF(tempbuf), &len);
		if (ret != CS_SUCCEED)
		{
			dbct__error("dbct_datename: cs_dt_info(CS_DAYNAME) failed.");
			return -1;
		}
		/* Make sure weekday name is null-terminated */
		if (tempbuf[len] != '\0')
		{
			len++;
			tempbuf[len] = '\0';
		}
		if (buflen < len)
		{
			dbct__error("dbct_datepart: Buffer too short.");
			return -1;
		}
		strcpy(buf, tempbuf);
		return len;
	  
	  case DBDATE_HH:
		/*
		** Just print daterec.datehour into the string.
		*/
		sprintf(tempbuf, "%ld", (long)daterec.datehour);
		len = strlen(tempbuf) + 1;
		if (buflen < len)
		{
			dbct__error("dbct_datepart: Buffer too short.");
			return -1;
		}
		strcpy(buf, tempbuf);
		return len;
	  
	  case DBDATE_MI:
		/*
		** Just print daterec.dateminute into the string.
		*/
		sprintf(tempbuf, "%ld", (long)daterec.dateminute);
		len = strlen(tempbuf) + 1;
		if (buflen < len)
		{
			dbct__error("dbct_datepart: Buffer too short.");
			return -1;
		}
		strcpy(buf, tempbuf);
		return len;

	  case DBDATE_MS:
		/*
		** Just print daterec.datemsecond into the string.
		*/
		sprintf(tempbuf, "%ld", (long)daterec.datemsecond);
		len = strlen(tempbuf) + 1;
		if (buflen < len)
		{
			dbct__error("dbct_datepart: Buffer too short.");
			return -1;
		}
		strcpy(buf, tempbuf);
		return len;

	  default:
		dbct__error("dbct_datename: Received unknown date part code.");
		return -1;
	}

	/* 
	** Should not get here.
	dbct__error("dbct_datename: Internal error.");
	return -1;
	*/
} /* dbct_datename */

/*
** dbct_prtype()
**
**   Function to replace dbprtype() in example programs.
**
*/
CS_CHAR * CS_PUBLIC
dbct_prtype(CS_INT type)
{
	      switch ((int) type)
	      {
	        case CS_TINYINT_TYPE:
	              return ("tinyint");
 
	        case CS_SMALLINT_TYPE:
	              return ("smallint");
 
	        case CS_INT_TYPE:
	              return ("int");
	
	        case CS_MONEY4_TYPE:
	              return ("smallmoney");
 
	        case CS_MONEY_TYPE:
	              return ("money");
 
	        case CS_REAL_TYPE:
	              return ("real");
 
	        case CS_FLOAT_TYPE:
	              return ("float");
 
	        case CS_DATETIME4_TYPE:
	              return ("smalldatetime");
 
	        case CS_DATETIME_TYPE:
	              return ("datetime");
 
	        case CS_BIT_TYPE:
	              return ("bit");
 
	        case CS_CHAR_TYPE:
	              return ("char");
 
	  /* 
	  ** NOTE: CS_VARCHAR has no ASE equivalent.
	  **       It is NOT equivalent to the ASE
	  **       "varchar" datatype.
	  */
	  case CS_VARCHAR_TYPE:
		return ("CS_VARCHAR");

	        case CS_TEXT_TYPE:
	              return ("text");
 
	        case CS_BINARY_TYPE:
	              return ("binary");
 
	        case CS_IMAGE_TYPE:
	              return("image");

	        case CS_NUMERIC_TYPE:
	              return("numeric");

	        case CS_DECIMAL_TYPE:
	              return("decimal");

	        case CS_SENSITIVITY_TYPE:
	              return("sensitivity");

	        case CS_BOUNDARY_TYPE:
	              return("boundary");

	  /*
	  ** The aggregate operator codes for compute row columns are 
	  ** returned by ct_compute_info(CS_COMP_OP). 
	  */
	        case CS_OP_SUM:
	              return ("sum");

	        case CS_OP_AVG:
	              return ("avg");

	        case CS_OP_COUNT:
	              return ("count");

	        case CS_OP_MIN:
	              return ("min");

	        case CS_OP_MAX:
	              return ("max");

	        default:
	              return ("Unknown");

	   } /* switch */
} /* dbct_prtype */

/*
** dbct_parent_ctx() --
**    Gets the parent context of a connection or the grandparent 
**    context of a command structure. 
**    
**    If the connection handle is non-null, gets the parent of 
**    the connection. Otherwise, gets the parent of the 
**    command-handle's parent connection.
**
** Returns
**   CS_SUCCEED or CS_FAIL.
*/
CS_RETCODE CS_PUBLIC
dbct_parent_ctx (cmd, conn, ctxptr)
CS_COMMAND    *cmd;
CS_CONNECTION *conn;
CS_CONTEXT    **ctxptr;
{
	CS_RETCODE retcode;

	*ctxptr = (CS_CONTEXT *) NULL;

	if (conn != (CS_CONNECTION *) NULL)
	{
	  retcode = ct_con_props(conn, CS_GET, CS_PARENT_HANDLE,
			   (CS_VOID*) ctxptr, CS_UNUSED, (CS_INT *)NULL);
	  if (retcode != CS_SUCCEED)
	    return CS_FAIL;
	  else
	    return CS_SUCCEED;
	}
	else
	{
	  /* 
	  ** Get the command handle's parent connection.
	  */
	  retcode = dbct_parent_conn(cmd, &conn);
	  if (retcode != CS_SUCCEED)
	    return retcode;

	  /*
	  ** Recurse to get connection's parent context.
	  */
	  return dbct_parent_ctx((CS_COMMAND *) NULL, conn, ctxptr);
	}
} /* dbct_parent_ctx() */

/*
** dbct_parent_conn() --
**   Gets the parent context of a connection.  
**
** Returns:
**   CS_SUCCEED or CS_FAIL.
*/
CS_RETCODE CS_PUBLIC
dbct_parent_conn (cmd,  connptr)
CS_COMMAND *cmd;
CS_CONNECTION **connptr;
{
	CS_RETCODE retcode;
	retcode = ct_cmd_props(cmd, CS_GET, CS_PARENT_HANDLE,
			 (CS_VOID *) connptr, CS_UNUSED, (CS_INT *)NULL);
	if (retcode != CS_SUCCEED)
	  return CS_FAIL;
	else
	  return CS_SUCCEED;
} /* dbct_parent_conn() */


/*
** dbct_trim() -- trim trailing blanks from a null-terminated
**   string.
**
** Parameters:
**   str -- the null-terminated string to trim.
**
** Notes:
**   This code is character set specific. It assumes a single
**   byte character set and that ' ' in the code set used by 
**   this program matches a space in the data.
**
** Returns:
**   Pointer to the original string.
*/
CS_CHAR * CS_PUBLIC dbct_trim (str)
CS_CHAR *str;
{
	int i = 0;
	char *p;

	/*
	** If str is NULL, we do nothing.
	*/
	p = str;
	if (p == (CS_CHAR *) NULL )
	{
	  return p;
	}

	i = strlen(p);

	/*
	** Trim the blanks.
	*/
	while (i > 0 && p[i - 1] == ' ')
	{
	  --i;
	  p[i] = '\0';
	}

	return str;

} /* dbct_trim */

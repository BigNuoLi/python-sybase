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
** 	This example illustrates dbconvert().  It converts a 
** 	number of constants to strings, a number of strings 
** 	to numerical or binary quantities, and a number of
** 	numerical quantities to other numerical types.
**
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sybfront.h>
#include <sybdb.h>
#include "sybdbex.h"

#define ARRAY_LEN      20

int
main(int argc, char *argv[])
{
	/* These variables will hold the results of data conversions. */
	static DBBINARY   my_binary_array[ARRAY_LEN]
	                   = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
	DBFLT8            my_flt8;
	DBINT             my_int4;
	DBMONEY           my_money;
	DBCHAR            my_string[ARRAY_LEN];
	DBDATETIME        my_date;

	printf("Demo of SQL queries in a command batch\n\n");
	fflush(stdout);

	/* Initialize DB-Library. */
	if (dbinit() == FAIL)
		exit(ERREXIT);

	/*
	** Install the user-supplied error-handling and message-handling
	** routines. They are defined at the bottom of this source file.
	*/
	dberrhandle((EHANDLEFUNC)err_handler);
	dbmsghandle((MHANDLEFUNC)msg_handler);

	/* Convert numerical and binary constants to strings. */
	dbconvert((DBPROCESS *)NULL, SYBBINARY, my_binary_array, (DBINT)8,
		 SYBCHAR, (BYTE DBFAR *)my_string, (DBINT)-1);
	printf("Binary constant 0x123456789abcdef0 converted to string "); 
	printf("\"%s\".\n\n", my_string);

	my_flt8 = 55.555;
	dbconvert((DBPROCESS *)NULL, SYBFLT8, (BYTE *)&my_flt8, (DBINT)-1,
		 SYBCHAR, (BYTE *)my_string, (DBINT)-1);
	printf("Floating-pt constant 55.555 converted to string \"%s\".\n\n",
		my_string);

	/* Convert string constants to numerical and binary quantities. */
	dbconvert((DBPROCESS *)NULL, SYBCHAR, (BYTE DBFAR * )"123", (DBINT)-1, 
		 SYBINT4, (BYTE *)&my_int4, (DBINT)-1);
	printf("String constant \"123\" converted to 4-byte integer %d.\n\n",
		my_int4);

	dbconvert((DBPROCESS *)NULL, SYBCHAR, (BYTE DBFAR * )"0xfedc", 
		 (DBINT)-1, SYBBINARY, my_binary_array, (DBINT)ARRAY_LEN);
	printf("String constant \"0xfedc\" converted to binary sequence ");
	printf("%x.\n\n", *((int *)my_binary_array));

	dbconvert((DBPROCESS *)NULL, SYBCHAR, (BYTE DBFAR * )"123.456", 
		 (DBINT)-1, SYBFLT8, (BYTE *)&my_flt8, (DBINT)-1);
	printf("String constant \"123.456\" converted to ");
	printf("floating-pt number %f.\n\n", my_flt8);

	/* Convert numerical types to other numerical types. */
	my_flt8 = 98.76;
	dbconvert((DBPROCESS *)NULL, SYBFLT8, (BYTE *)&my_flt8, (DBINT)-1, 
		  SYBMONEY, (BYTE *)&my_money, (DBINT)-1);
	dbconvert((DBPROCESS *)NULL, SYBMONEY, (BYTE *)&my_money, (DBINT)-1, 
		  SYBCHAR, (BYTE *)my_string, (DBINT)-1);
	printf("floating-pt number %f converted to money value %s.\n\n",
		my_flt8, my_string);

	dbconvert ((DBPROCESS *)NULL, SYBMONEY, (BYTE *)&my_money, (DBINT)-1, 
		  SYBFLT8, (BYTE *)&my_flt8, (DBINT)-1);
	printf("money value %s converted to floating-pt value %f.\n\n",
		my_string, my_flt8);

	/*
	**  Datetime conversions:  
	*/

        /*
        **  Many formats are acceptable for dates.  For a list
        **  of these formats, see the Commands Reference
        **  datatypes page.
        */

	dbconvert((DBPROCESS *)NULL, SYBCHAR, (BYTE DBFAR *)"November 11, 1955", 
		 (DBINT)-1, SYBDATETIME, (BYTE *)&my_date , (DBINT)-1);
	dbconvert((DBPROCESS *)NULL, SYBMONEY, (BYTE *)&my_date, 
                 (DBINT)-1, SYBCHAR, (BYTE *)my_string, (DBINT)-1);

	printf("Internal date representation converted ");
        printf("to string value %s.\n\n", my_string);

	dbdatename((DBPROCESS *)NULL, my_string, DBDATE_DY, &my_date);

	printf("Day-of-year extracted from internal representation:"); 
        printf(" %s.\n\n", my_string);

	dbdatename((DBPROCESS *)NULL, my_string, DBDATE_DW, &my_date);

	printf("Day-of-week extracted from internal representation:"); 
        printf(" %s.\n\n", my_string);

	dbexit();
	exit(STDEXIT);
}

int CS_PUBLIC 
err_handler(DBPROCESS *dbproc, int severity, int dberr, int oserr, 
	    char *dberrstr, char *oserrstr)
{
	if ((dbproc == NULL) || (DBDEAD(dbproc)))
	{
		return (INT_EXIT);
	}
	else 
	{
		fprintf(ERR_CH, "DB-Library error:\n\t%s\n", dberrstr);

		if (oserr != DBNOERR)
		{
			fprintf(ERR_CH, "Operating-system error:\n\t%s\n", oserrstr);
		}

		return (INT_CANCEL);
	}
}

int CS_PUBLIC 
msg_handler(DBPROCESS *dbproc, DBINT msgno, int msgstate, int severity,
	    char *msgtext, char *srvname, char *procname, int line)
{
	/*
	** Ignore the 'Changed database to' and 'Changed language to'
	** messages.
	*/
	if (msgno == 5701 || msgno == 5703)
	{
		return (0);
	}

	fprintf(ERR_CH, "Msg %d, Level %d, State %d\n", 
		msgno, severity, msgstate);

	if (strlen(srvname) > 0)
	{
		fprintf(ERR_CH, "Server '%s', ", srvname);
	}

	if (strlen(procname) > 0)
	{
		fprintf(ERR_CH, "Procedure '%s', ", procname);
	}

	if (line > 0)
	{
		fprintf(ERR_CH, "Line %d", line);
	}

	fprintf(ERR_CH, "\n\t%s\n", msgtext);
	return (0);
}

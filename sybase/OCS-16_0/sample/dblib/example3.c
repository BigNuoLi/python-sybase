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
** 	This example selects some information from the "pubs2" database.
** 	It illustrates binding of both aggregate and compute results.
**
** 	Note that this example will only work if the "pubs2" database exists
** 	on your ASE. Consult the Installation Guide for information
** 	about installing the "pubs2" database.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sybfront.h>
#include <sybdb.h>
#include "sybdbex.h"

#define PLEN 4 
#define DATEPRINT 26
#define MONEYPRINT 12

int
main(int argc, char *argv[])
{

	LOGINREC       *login;
	DBPROCESS      *dbproc;

	/* Declare the datatypes for the columns in the table "titles". */
	DBINT          pcount;
	DBINT          sales;
	DBINT          salesavg;
	DBINT          sumsale;
	DBCHAR         date[DATEPRINT+1];
	DBCHAR         price[MONEYPRINT+1];
	DBCHAR         priceavg[MONEYPRINT+1];
	DBCHAR         pubid[PLEN+1];
	RETCODE        result_code; /* to hold the results of dbresults(). */
	STATUS         row_code;    /* to hold the results of dbnextrow(). */

	printf("Demo of SQL queries in a command batch\n\n");
	fflush(stdout);

	/* Initialize DB-Library. */
	if (dbinit() == FAIL)
	{
		exit(ERREXIT);
	}

	/*
	** Install the user-supplied error-handling and message-handling
	** routines. They are defined at the bottom of this source file.
	*/
	dberrhandle((EHANDLEFUNC)err_handler);
	dbmsghandle((MHANDLEFUNC)msg_handler);

	/* Set up the login information. */
	login = dblogin();
	DBSETLUSER(login, USER);
	DBSETLPWD(login, PASSWORD);
	DBSETLAPP(login, "example3");
 
	dbproc = dbopen(login, NULL);

	/* Send a "use database" command. */
	dbuse(dbproc,"pubs2");

	/* Put the SQL statement into the command buffer. */
	dbcmd(dbproc, "select pub_id, pubdate, price, avg(price), total_sales,");
	dbcmd(dbproc, " avg(total_sales), sum(total_sales) from titles");
	dbcmd(dbproc, " group by pub_id");
	dbcmd(dbproc, " order by pub_id");
	dbcmd(dbproc, " compute count(pub_id) by pub_id");

	/* Send the command buffer to ASE for execution. */
	dbsqlexec(dbproc);

	/*
	** Using the aggregates "sum" and "avg" with the COMPUTE clause 
	** necessitates special handling when binding the results. Since each
	** aggregate creates a new column, this is accounted for in the bind.
	** Notice that avg(price) is the fourth column in the select-list,
	** and is also specified as the fourth column in the dbbind() routine.
	**
	** The COMPUTE clause creates a compute row, which requires a 
	** special bind routine called dbaltbind().
	*/

	while ((result_code = dbresults(dbproc)) != NO_MORE_RESULTS)
	{
		if (result_code == SUCCEED)
		{
			dbbind(dbproc, 1, NTBSTRINGBIND, (DBINT)0, 
						(BYTE DBFAR *)pubid);
			dbbind(dbproc, 2, NTBSTRINGBIND, (DBINT)0, 
						(BYTE DBFAR *)date);
			dbbind(dbproc, 3, NTBSTRINGBIND, (DBINT)0, 
						(BYTE DBFAR *)price);
			dbbind(dbproc, 4, NTBSTRINGBIND, (DBINT)0, 
						(BYTE DBFAR *)priceavg);
			dbbind(dbproc, 5, INTBIND, (DBINT)0, (BYTE *)&sales);
			dbbind(dbproc, 6, INTBIND, (DBINT)0, (BYTE *)&salesavg);
			dbbind(dbproc, 7, INTBIND, (DBINT)0, (BYTE *)&sumsale);

			/* dbaltbind() binds compute columns. */
			dbaltbind(dbproc, 1, 1, INTBIND, (DBINT)0, (BYTE *)&pcount);

			printf("\nAccounts:\n");
			printf("---------\n\n");
			printf("%-5s  %-26s  %-6s  %-10s  %-5s  %-10s  %-10s\n\n",
				"pubid", "date", "price", "avg(price)",
				"sales", "avg(sales)", "sum(sales)");

			/*
			** Print out each result row, using different statements
			** depending on whether the row is a regular row or a 
			** compute row.
			*/
			while ((row_code = dbnextrow(dbproc)) != NO_MORE_ROWS)
			{
				if (row_code == REG_ROW)
				{
					printf("%5s  %26s  %6s  %10s  %5d  %10d  %10d\n",
						pubid, date, price, priceavg, sales, 
						salesavg, sumsale);
				}
				else
				{
					printf("title count:  %d\n\n",pcount); 
				}
			}
		}
	}

	dbexit(  );
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

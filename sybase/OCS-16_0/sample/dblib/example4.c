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
** 	This example accesses the data within each row without using dbbind(),
** 	and illustrates the use of row buffering.
**
** 	It runs a query, saves all of the returned rows (up to a maximum
** 	of 1000) using DB-Library row buffering, and allows the user to
** 	examine data rows at random.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sybfront.h>
#include <sybdb.h>
#include "sybdbex.h"

DBPROCESS            *dbproc;  /* Our connection with ASE. */
LOGINREC             *dlogin;   /* Our login information. */

#define TYPELEN 2

/* Forward declarations of the error handler and message handler. */
int CS_PUBLIC err_handler();
int CS_PUBLIC msg_handler();

int
main(int argc, char *argv[])
{
	/*
	** Here are the variables which will be used to store the
	** data being examined.
	*/
	DBCHAR             name[DBMAXNAME+1]; /* DBMAXNAME is defined in
	                                      ** "sybdb.h" as the maximum
	                                      ** length for names of database
	                                      ** objects, such as tables,
	                                      ** columns, and procedures.
					      */
	DBCHAR             type[TYPELEN+1];
	DBINT              id;
	DBCHAR             datebuf[64];
	DBINT              len;
	char               numstring[32];
	int                quitflag = 0;
	RETCODE            row_code;
	int                rownum;

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

	/*
	** Get a LOGINREC structure and fill it with the necessary
	** login information.
	*/
	dlogin = dblogin();
	DBSETLUSER(dlogin, USER);
	DBSETLPWD(dlogin, PASSWORD);
	DBSETLAPP(dlogin, "example4");

	dbproc = dbopen(dlogin, NULL);
	dbcmd(dbproc, "select name, type, id, crdate from sysobjects");
	dbcmd(dbproc, " where type = 'S'");

	/*
	** Buffer the rows so we can read them any number of times.
	** Passing '0' as the number of rows to buffer will get us
	** default row buffering (currently 1000 rows).
	** If more than 1000 rows are received, this program will
	** only save the last 1000.
	**
	** Note that this parameter must be passed as an ASCII string.
	*/
	dbsetopt(dbproc, DBBUFFER, "0", NULL);
	dbsqlexec(dbproc);

	if (dbresults(dbproc) == SUCCEED) 
	{
		/* Read all of the rows into DB-Library's buffer */

		while ((row_code = dbnextrow(dbproc)) != NO_MORE_ROWS)
		{
			/*
			** If DB-Library's row buffer is full, throw
			** away the oldest row, to allow the newest
			** row to be read in.
			*/
			if (row_code == BUF_FULL)
			{
				dbclrbuf(dbproc, 1);
			}
		}
	
		/* Let the user view any row in the table. */
		printf("Type the number of the row you want to see.\n");
		printf("The first row is number 1.\n");
		printf("Asking for row 0 will terminate the program.\n");

		while (quitflag == 0)
		{
			printf("Row number: ");
			fgets(numstring, sizeof(numstring), stdin);
			if (numstring[strlen(numstring) - 1] == '\n')
			{
				numstring[strlen(numstring) - 1] = '\0';
			}

			rownum = atoi(numstring);

			if (rownum == 0)
			{
				quitflag = 1;
			}
			else
			{
				/* Print out the requested row. */
				if (dbgetrow(dbproc, rownum) == NO_MORE_ROWS)
				{
					printf("That row is not in the table.\n");
				}
				else
				{
					/*
					** Copy variable-length character data
					** (colname).
					*/
					strncpy(name, (DBCHAR *)dbdata(dbproc, 1),
						(int)(len = dbdatlen(dbproc, 1)));

					/* String needs terminating null. */
					name[len] = '\0';

					/* Copy fixed-length character data. */
					strncpy(type, (DBCHAR *)dbdata(dbproc, 2),
						(int)(len = dbdatlen(dbproc, 2)));
					type[len] = '\0';

					/* Copy integer data. */
					id = *((DBINT *)dbdata(dbproc, 3));

					/* Print out the column headers. */
					printf(" %-20s %10s %30s %15s \n",
			 			"       NAME         ",
			 			"TYPE",
			 			"             DATE             ",
			 			"ID");
					printf(" %20s %10s %30s %15s \n",
			 			"--------------------",
			 			"----",
			 			"---------------------------",
			 			"----");

					/* Convert datetime data to a printable string. */
					dbconvert(dbproc, SYBDATETIME, (dbdata(dbproc, 4)), 
						  (DBINT)-1, SYBCHAR, 
						  (BYTE DBFAR *)datebuf, (DBINT)-1);
					printf("%20s %10s %30s %15d \n",
						name, type, datebuf, id);
				}
			}
		}

	}

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

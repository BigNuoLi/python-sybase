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
**	This example illustrates how to send two queries to ASE in a
**	command batch.  It binds each set of results and prints the rows.
**
*/

#include <stdio.h>
#include <sybfront.h>
#include <sybdb.h>
#include "sybdbex.h"

#define DATELEN     26
#define TYPELEN      2

/* Forward declarations of the error handler and message handler. */
int CS_PUBLIC   err_handler();
int CS_PUBLIC   msg_handler();

int
main(int argc, char *argv[])
{
	DBPROCESS     *dbproc;       /* Our connection with ASE. */
	LOGINREC      *login;        /* Our login information. */

	/* These are the variables used to store the returning data. */

	DBCHAR         crdate[DATELEN+1];
	DBINT          id;
	DBCHAR         name[DBMAXNAME+1];   /* DBMAXNAME is defined in
	                                    * "sybdb.h" as the maximum
	                                    * length for names of database
	                                    * objects, such as tables,
	                                    * columns, and procedures.
	                                    */
	DBCHAR         type[TYPELEN+1];
	RETCODE        result_code;

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
	login = dblogin();
	DBSETLUSER(login, USER);
	DBSETLPWD(login, PASSWORD);
	DBSETLAPP(login, "example1");

	/*
	** Get a DBPROCESS structure for communicating with ASE.
	** A NULL servername defaults to the server specified by DSQUERY.
	*/
	dbproc = dbopen(login, NULL);

	/*
	** We are going to retrieve some information, from a table
	** named "sysobjects", regarding names of system tables and
	** stored procedures.
	** We will submit two queries.  The first finds all the rows 
	** that describe system tables.  The second finds all the rows
	** that describe stored procedures.  The program will only look
	** at the first 10 rows that describe stored procedures.
	*/

	/* First, put the commands into the command buffer. */
	dbcmd(dbproc, "select name, type, id, crdate from sysobjects");
	dbcmd(dbproc, " where type = 'S' ");
	dbcmd(dbproc, "select name, type, id, crdate from sysobjects");
	dbcmd(dbproc, " where type = 'P' ");

	/*
	** ASE processes the command batch in the following order:
	**
	** 1) It will check for syntax errors (i.e., "use database pubs" 
	**    is syntactically incorrect; it should be "use pubs").
	** 2) The second check is a semantic check (i.e., "select * from 
	**    titels" will be incorrect because the spelling should be 
	**    "titles".)
	** 3) The third check occurs in the actual execution phase. This 
	**    check involves issues like permissions or memory problems.
	** 
	** In the execution phase, dbsqlexec() and dbresults() can return 
	** the value "SUCCEED", which means there are more commands in the 
	** batch to process and that that command was successful. A value 
	** of "FAIL" means that the query failed but there may be more 
	** commands in the batch to process. A value of "NO_MORE_RESULTS"
	** means that there are no more commands in the batch to process.
	** Therefore, the programmer must check the return values after
	** dbsqlexec() and dbresults(), as illustrated below.
	**
	*/

	/* Send the commands to ASE and start execution. */
	dbsqlexec(dbproc);

	/* Process each command until there are no more. */
	while ((result_code = dbresults(dbproc)) != NO_MORE_RESULTS)
	{
		if (result_code == SUCCEED)
		{
			/* Bind program variables. */
	
			dbbind(dbproc, 1, NTBSTRINGBIND, (DBINT)(DBMAXNAME+1), 
			       (BYTE DBFAR *)name);
			dbbind(dbproc, 2, NTBSTRINGBIND, (DBINT)(TYPELEN+1), 
			       (BYTE DBFAR *)type);
			dbbind(dbproc, 3, INTBIND, (DBINT)0, (BYTE *)&id);
			dbbind(dbproc, 4, NTBSTRINGBIND, (DBINT)(DATELEN+1), 
			       (BYTE DBFAR *)crdate);

			/* Print appropriate header for the type of
			 * data coming back.
			 */

			printf("\n %s Objects: \n\n",
			DBCURCMD(dbproc) == 1 ? "System Table": "Procedure");

			/* Now print the rows. */
			while (dbnextrow(dbproc) != NO_MORE_ROWS)
			{
				/*
				** If this is the 2nd command and
				** 10th row, flush the rest of the
				** rows for that command.
				*/

				if ((DBCURCMD(dbproc) == 2)
					&& (DBCURROW(dbproc) > 10))
				{
					continue;
				}

				printf("%s %s %d %s\n", name, type, id, crdate);
			}
		}
	}

	/* Close our connection and exit the program. */
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

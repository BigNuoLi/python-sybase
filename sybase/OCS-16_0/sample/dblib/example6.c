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
** 	This example illustrates opening a data file, inserting data
** 	from the file into a newly created table containing several 
** 	ASE datatypes, and updating the table using browse-mode
** 	techniques.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sybfront.h>
#include <sybdb.h>
#include "sybdbex.h"

#define BUFLEN 2048

int
main(int argc, char *argv[])
{
	LOGINREC         *login;
	DBPROCESS        *q_dbproc;	/* This DBPROCESS will be used to
					** query the database.
					*/
	DBPROCESS        *u_dbproc;	/* This DBPROCESS will be used to
					** simultaneously update the database.
					*/
	char             *qualptr;	/* This will point to the WHERE clause
					** appropriate for updating q_dbproc's
					** current data row.
					*/
	RETCODE          return_code;
	DBTINYINT        age;
	char             cmdbuf[BUFLEN];
	FILE             *infile;

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
	
	/*
	** Allocate and initialize the LOGINREC structure to be used
	** to open a connection to ASE.
	*/
	login = dblogin();
	DBSETLUSER(login, USER);
	DBSETLPWD(login, PASSWORD);
	DBSETLAPP(login, "example6");

	q_dbproc = dbopen(login, NULL);
	u_dbproc = dbopen(login, NULL);

	printf("Creating the 'alltypes' table.\n");

	/* Create a table that contains several ASE data types. */
	dbcmd(q_dbproc,"create table alltypes ");
	dbcmd(q_dbproc,"(age tinyint,");
	dbcmd(q_dbproc,"userid smallint,");
	dbcmd(q_dbproc,"royalty int,");
	dbcmd(q_dbproc,"name char(25),");
	dbcmd(q_dbproc,"title_id varbinary(20),"); 
	dbcmd(q_dbproc,"us_citizen bit,");
	dbcmd(q_dbproc,"account float,");
	dbcmd(q_dbproc,"title varchar(20),");
	dbcmd(q_dbproc,"manager char(25),");
	dbcmd(q_dbproc,"timestamp)");
	dbcmd(q_dbproc, "create unique index index1 on alltypes(userid)");
	dbsqlexec(q_dbproc);
	while (dbresults(q_dbproc) != NO_MORE_RESULTS)
	{
		continue;
	}

	/*
	** Insert rows of data into the newly created table "alltypes".
	** We will read in the contents of the file and form an
	** INSERT statement.
	*/
	if ((infile=fopen("datafile","r")) == NULL)
	{
		printf("Unable to open file 'datafile'.\n");
		exit(STDEXIT);
	}

	printf("Inserting rows into the 'alltypes' table...\n");

	while ((fgets(cmdbuf,BUFLEN,infile)) != NULL)
	{
		dbfcmd(q_dbproc,"insert into alltypes \n");
		dbfcmd(q_dbproc,"values(%s, null) \n",cmdbuf);
	}

	dbsqlexec(q_dbproc);

	/* Process the results of each of the INSERT statements. */
	while ((return_code = dbresults(q_dbproc)) != NO_MORE_RESULTS)
	{
		if (return_code == FAIL)
		{
			printf("One of the insert statements FAILed.\n");
		}
	}

	/*
	** Using DB-Library's browse-mode facilities, we'll increment
	** the age of every person in the table.
	*/
	printf("Updating rows in the 'alltypes' table...\n");
	dbcmd(q_dbproc,"select * from alltypes for browse");
	dbsqlexec(q_dbproc);

	while ((return_code = dbresults(q_dbproc)) != NO_MORE_RESULTS)
	{
		if (return_code == SUCCEED)
		{
			while (dbnextrow(q_dbproc) != NO_MORE_ROWS)
			{
				age = *((DBTINYINT *)(dbdata(q_dbproc, 1)));
				qualptr = dbqual(q_dbproc, -1, "alltypes");
				if (qualptr == NULL)
				{
					printf("dbqual() failed.  Exiting.\n");
					dbexit();
					exit(ERREXIT);
				}

				dbcmd(u_dbproc, "update alltypes");
				dbfcmd(u_dbproc, " set age = %d %s", age+1, qualptr);
				dbsqlexec(u_dbproc);
				dbresults(u_dbproc);
				dbfreequal(qualptr);
			}
		}
	}

	/*
	** Now, we'll look at the updated contents of the table, to
	** verify that the ages were properly incremented.
	*/
	printf("Selecting rows from the 'alltypes' table:\n");
	dbcmd(q_dbproc, "select * from alltypes");
	dbsqlexec(q_dbproc);
	dbresults(q_dbproc);
	dbprrow(q_dbproc);

	/*
	** Now, we clean up alltypes table before exit.
	*/
	printf("Dropping 'alltypes' table\n");
	dbcmd(q_dbproc, "drop table alltypes");
	dbsqlexec(q_dbproc);
	while (dbresults(q_dbproc) != NO_MORE_RESULTS)
	{
		continue;
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

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
** 	This example opens a data file, inserts data from the file
** 	into a newly created table containing several of the
** 	ASE datatypes, and binds and prints the results.
*/

#include <stdio.h>
#include <sybfront.h>
#include <sybdb.h>
#include "sybdbex.h"

#define BUFLEN      2048
#define HEXLEN      510
#define PLEN        25 

/* Forward declarations of the error handler and message handler routines. */
int CS_PUBLIC err_handler();
int CS_PUBLIC msg_handler();

int
main(int argc, char *argv[])
{

	LOGINREC         *login;
	DBPROCESS        *dbproc;
	RETCODE          return_code;
	DBTINYINT        age;
	DBSMALLINT       userid;
	DBINT            royalty;
	DBCHAR           name[PLEN+1];
	DBBINARY         title_id[PLEN+1];
	DBBIT            us_citizen;
	DBFLT8           account;
	DBCHAR           title[PLEN+1];     /* string    */
	DBCHAR           manager[PLEN+1];   /* ntbstring */
	DBCHAR           id_buffer[HEXLEN+1];
	char             cmdbuf[BUFLEN];
	FILE             *infile;

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
	** Allocate and initialize the LOGINREC structure to be used
	** to open a connection to ASE.
	*/
	login = dblogin();
	DBSETLUSER(login, USER);
	DBSETLPWD(login, PASSWORD);
	DBSETLAPP(login, "example2");
	
	dbproc = dbopen(login, NULL);

	printf("Creating the 'test' database.\n");

	dbcmd(dbproc,"create database test ");

	dbsqlexec(dbproc);
	while (dbresults(dbproc) != NO_MORE_RESULTS)
		continue;

	dbuse(dbproc,"test");

	printf("Creating the 'alltypes' table.\n");

	/* Create a table that contains several ASE datatypes. */
	dbcmd(dbproc,"create table alltypes ");
	dbcmd(dbproc,"(age tinyint,");
	dbcmd(dbproc,"userid smallint,");
	dbcmd(dbproc,"royalty int,");
	dbcmd(dbproc,"name char(25),");
	dbcmd(dbproc,"title_id varbinary(20),"); 
	dbcmd(dbproc,"us_citizen bit,");
	dbcmd(dbproc,"account float,");
	dbcmd(dbproc,"title varchar(20),");
	dbcmd(dbproc,"manager char(25))");
  
	dbsqlexec(dbproc);
	while (dbresults(dbproc) != NO_MORE_RESULTS)
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
		exit(ERREXIT);
	}

	printf("Inserting rows into the 'alltypes' table.\n");

	while ((fgets(cmdbuf,BUFLEN,infile)) != NULL)
	{
		dbfcmd(dbproc,"insert into alltypes \n");
		dbfcmd(dbproc,"values(%s) \n",cmdbuf);
	}

	dbsqlexec(dbproc);

	/* Process the results of each of the INSERT statements. */
	while ((return_code = dbresults(dbproc)) != NO_MORE_RESULTS)
	{
		if (return_code == FAIL)
		{
			printf("One of the insert statements FAILed.\n");
		}
	}

	printf("Selecting rows from the 'alltypes' table:\n");

	dbcmd(dbproc,"select * from alltypes");
	dbsqlexec(dbproc);

	while ((return_code = dbresults(dbproc)) != NO_MORE_RESULTS)
	{
		if (return_code == SUCCEED)
		{
			dbbind(dbproc, 1, TINYBIND, (DBINT)0, (BYTE *)&age);
			dbbind(dbproc, 2, SMALLBIND, (DBINT)0, (BYTE *)&userid);
			dbbind(dbproc, 3, INTBIND, (DBINT)0, (BYTE *)&royalty);
			dbbind(dbproc, 4, CHARBIND, (DBINT)0, 
							(BYTE DBFAR *)name);
			dbbind(dbproc, 5, BINARYBIND, (DBINT)0, 
						(BYTE DBFAR *)title_id);
			dbbind(dbproc, 6, BITBIND, (DBINT)0, (BYTE *)&us_citizen);
			dbbind(dbproc, 7, FLT8BIND, (DBINT)0, (BYTE *)&account);
			dbbind(dbproc, 8, STRINGBIND, (DBINT)0, 
							(BYTE DBFAR *)title);
			dbbind(dbproc, 9, NTBSTRINGBIND, (DBINT)0, 
							(BYTE DBFAR *)manager);

			/*
			** Initialize null terminator in "name" array,
			** since CHARBIND does not add one.
			*/
			name[PLEN] = '\0';

			while (dbnextrow(dbproc) != NO_MORE_ROWS)
			{
				dbconvert(dbproc, SYBBINARY, title_id,
					 dbdatlen(dbproc, 5), SYBCHAR, 
					 (BYTE DBFAR *)id_buffer, (DBINT)-1);
				printf("%d  %d  %d  %s  0x%s\n",
					 age, userid, royalty, name, id_buffer);
				printf("%d  %8.2f  %s  %s\n",
					 us_citizen, account, title, manager);
			}
		}
	}

	printf("Dropping the 'test' database.\n");

	dbuse(dbproc,"master");
	dbcmd(dbproc,"drop database test");
	dbsqlexec(dbproc);
	while (dbresults(dbproc) != NO_MORE_RESULTS)
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

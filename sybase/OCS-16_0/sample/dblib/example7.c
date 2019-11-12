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
** 	This example illustrates the use of browse-mode routines to
** 	determine the source of result columns from ad hoc queries.
*/

#include <stdio.h>

#include <sybfront.h>
#include <string.h>
#include <stdlib.h>

#include <sybdb.h>
#include "sybdbex.h"

void          examine_results();
void          send_command();

int
main(int argc, char *argv[])
{
	LOGINREC              *login;
	DBPROCESS             *dbproc;
	int                   command_count;
	RETCODE               retcode;

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
	DBSETLAPP(login, "example7");
	
	dbproc = dbopen(login, NULL);

	/*
	** Allow the user to type in a series of queries. This program
	** will be terminated by the word "quit", appearing at the
	** beginning of the line.
	*/
	while (1)
	{
		/* Send a user-generated query to ASE. */
		send_command(dbproc);

		/*
		** Now, examine the results of any queries the user has
		** typed in.
		*/
		command_count = 1;
		while ((retcode = dbresults(dbproc)) != NO_MORE_RESULTS)
		{
			if (retcode == FAIL)
			{
				printf("Command %d failed.\n", command_count);
			}
			else
			{
				if (!(DBCMDROW(dbproc)))
				{
					printf("Command %d cannot return rows.\n",
						command_count);
				}
				else
				{
					/*
					** This is a command that can return
					** rows. Let's take a closer look at it.
					*/
					printf("Command %d:\n", command_count);
					examine_results(dbproc);

					/* Throw away all data rows. */
					dbcanquery(dbproc);
				}
			}

			command_count++;
		}
	}
}

void 
examine_results(DBPROCESS *dbproc)
{
	int            colcount;
	int            colnum;
	char           fullsource[128];
	char           *sourcecolname;
	int            tabcount;
	char           *tabname;
	int            tabnum;

	/* Determine which tables were used to generate the query results. */
	tabcount = dbtabcount(dbproc);

	/* dbtabcount returns -0 if the query didn't include the words FOR BROWSE. */
	if (tabcount > 0)
	{
		printf("The following tables were used");
		printf(" to generate these query results:\n");

		for (tabnum = 1; tabnum <= tabcount; tabnum++)
		{
			if ((tabname = dbtabname(dbproc, tabnum)) != NULL)
			{
				printf("\t%s (%s)\n", tabname,
					(dbtabbrowse(dbproc, tabnum)
					? "browsable" : "not browsable"));
			}
		}
	}
	/* dbtabcount must have returned 0: */
	else
	{
		printf("Your query was not a browse-mode select.\n");
		printf("For this reason, dbtabcount()\n");
                printf("and dbtabname() cannot be used to print\n");
                printf("the names of the tables involved in the query.\n");
	}

	/* Determine which tables were used to generate each result column. */
	colcount = dbnumcols(dbproc);
	printf("Here are the columns of the target list and their sources:\n");
	printf("\t%-20s   %-30s   %s\n\n", "Result column:", "Source:", "Browsable?");

	for (colnum = 1; colnum <= colcount; colnum++)
	{
		tabname = dbtabsource(dbproc, colnum, NULL);
		sourcecolname = dbcolsource(dbproc, colnum);

		if (tabname == NULL)
		{
			if (tabcount == 0)
			{
				strcpy(fullsource, "(not browse-mode SELECT)");
			}
			else
			{
				strcpy(fullsource, "(result of expression)");
			}
		}
		else
		{
			sprintf(fullsource, "%s.%s", tabname, sourcecolname);
		}

		printf("\t%-20s   %-30s   %s\n", dbcolname(dbproc, colnum),
			 fullsource, (dbcolbrowse(dbproc, colnum) ? "yes" : "no"));
	}
}

void
send_command(DBPROCESS *dbproc)
{
	char           cmdbuf[2048];

	/*
	** Allow the user to type in an ad hoc query. This query
	** will be terminated by the word "go" appearing at the
	** beginning of the line.
	**
	** If the user types the word "quit" at the beginning of a line,
	** we'll quit the program.
	*/
	printf("Enter a SQL query:\n");
	while (1)
	{
		printf("> ");
		fgets(cmdbuf, sizeof(cmdbuf), stdin);
		/*
		** Remove the possible '\n' from the read string.
		*/
		if (cmdbuf[strlen(cmdbuf) - 1] == '\n')
		{
			cmdbuf[strlen(cmdbuf) - 1] = '\0';
		}

		if (strcmp(cmdbuf, "go") == 0)
		{
			dbsqlexec(dbproc);
			break;
		}
		else if (strcmp(cmdbuf, "quit") == 0)
		{
			dbexit();
			exit(STDEXIT);
		}
		else
		{
			/* Keep reading SQL commands. */
			dbcmd(dbproc, cmdbuf);
		}
	}
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

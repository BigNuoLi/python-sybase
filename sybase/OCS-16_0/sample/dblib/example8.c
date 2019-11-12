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
** 	This example illustrates how to use remote procedure calls
** 	and how to process return parameter values from stored
** 	procedures.  
**
** 	The example uses the following stored procedure, 
** 	named "rpctest", which it assumes is located in the 
** 	user's default database.  Before running this example, 
** 	you must create "rpctest" in your default database.
**
**      create procedure rpctest
**         (@param1 int out,
**          @param2 int out,
**          @param3 int out,
**          @param4 int)
**      as
**      begin
**         select "rpctest is running."
**         select @param1 = 11
**         select @param2 = 22
**         select @param3 = 33
**         select @param1
**
**         return 123
**      end
**
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sybfront.h>
#include <sybdb.h>
#include "sybdbex.h"

#define FMTSTR    "%-8.8s %-8.8s %-8.8s %-8.8s\n"
#define FMTSTR1    "%-8.8s %-8.8s %8.8d %8.8d\n"

/* Forward reference
*/
void doexit(char *);

int
main(int argc, char *argv[])
{
	LOGINREC         *login;
	DBPROCESS        *dbproc;
	int              i;
	int              numrets;
	DBINT            param1 = 1;
	DBINT            param2 = 2;
	DBINT            param3 = 3;
	DBINT            param4 = 4;
	RETCODE          return_code;

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
	login = dblogin( );
	DBSETLUSER(login, USER);
	DBSETLPWD(login, PASSWORD);
	DBSETLAPP(login, "rpcexample");
	
	dbproc = dbopen(login, (char *)NULL);

	/* Make the rpc. */
	if (dbrpcinit(dbproc, "rpctest", (DBSMALLINT)0) == FAIL)
	{
		doexit("dbrpcinit failed.\n");
	}

	if (dbrpcparam(dbproc, "@param1", (BYTE)DBRPCRETURN, SYBINT4, -1, -1,
		(BYTE *)&param1) == FAIL)
	{
		doexit("dbrpcparam 1 failed.\n");
	}

	if (dbrpcparam(dbproc, "@param2", (BYTE)0, SYBINT4, -1, -1,
		(BYTE *)&param2) == FAIL)
	{
		doexit("dbrpcparam 2 failed.\n");
	}

	if (dbrpcparam(dbproc, "@param3", (BYTE)DBRPCRETURN, SYBINT4, -1, -1,
		(BYTE *)&param3) == FAIL)
	{
		doexit("dbrpcparam 3 failed.\n");
	}

	if (dbrpcparam(dbproc, "@param4", (BYTE)0, SYBINT4, -1, -1, 
		(BYTE *)&param4) == FAIL)
	{
		doexit("dbrpcparam 4 failed.\n");
	}

	if (dbrpcsend(dbproc) == FAIL)
	{
		doexit("dbrpcsend failed.\n");
	}

	if (dbsqlok(dbproc) == FAIL)
	{
		doexit("dbsqlok failed.\n");
	}

	while ((return_code = dbresults(dbproc)) != NO_MORE_RESULTS)
	{
		if (return_code == FAIL)
		{
			doexit("dbresults failed.\n");
		}

		/* Print any rows that may have been returned. */
		dbprrow(dbproc);
	}

	/* Examine any return parameters that may have arrived. */
	numrets = dbnumrets(dbproc);
	printf("%d return values received.\n\n", numrets);

	if (numrets > 0)
	{
		printf(FMTSTR, "Name", "Type", "Length", "Value");
		printf(FMTSTR,
			"------------", "------------",
			"------------", "------------");
	}

	for (i = 1; i <= numrets; i++)
	{
		printf(FMTSTR1, dbretname(dbproc, i), dbprtype(dbrettype(dbproc, i)),
			dbretlen(dbproc, i), *((DBINT *)(dbretdata(dbproc, i))));
	}

	if (dbhasretstat(dbproc))
	{
		printf("Return status = %d\n", dbretstatus(dbproc));
	}
	else
	{
		printf("No return status for this command.\n");
	}

	dbexit();
	exit(STDEXIT);
}

void 
doexit(char *s1)
{
	printf(s1);
	dbexit();            /* always call dbexit before returning to OS */
	exit(ERREXIT);
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

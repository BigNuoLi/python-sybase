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
**	Demo of Two-Phase Commit Service
**
**	This example uses the two-phase commit service
**	to perform a simultaneous update on two servers.
**	In this example, one of the servers participating 
**	in the distributed transaction also functions as 
**	the commit service.
**
**	In this particular example, the same update is 
**	performed on both servers. You can, however, use 
**	the commit server to perform completely different 
**	updates on each server.
**
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sybfront.h>
#include <sybdb.h>
#include "sybdbex.h"

char     cmdbuf[256];
char     xact_string[128];

/*  Forward declaration	 */
void abortall(DBPROCESS*, DBPROCESS*, DBPROCESS*, int);

int
main(int argc, char *argv[])
{

	DBPROCESS     *dbproc_server1; 
	DBPROCESS     *dbproc_server2;
	DBPROCESS     *dbproc_commit;
	LOGINREC      *login;
	int           commid;
	RETCODE       ret_server1;
	RETCODE       ret_server2;

	printf("Demo of SQL queries in a command batch\n\n");
	fflush(stdout);

	/* Initialize DB-Library. */
	if (dbinit() == FAIL)
		exit(ERREXIT);

	dberrhandle((EHANDLEFUNC)err_handler);
	dbmsghandle((MHANDLEFUNC)msg_handler);

	/* Open connections with the servers and the commit service. */
	login = dblogin();
	DBSETLUSER(login, USER);
	DBSETLPWD(login, PASSWORD);
	DBSETLAPP(login, "twophase");

	dbproc_server1 = dbopen (login, "SERVICE");
	dbproc_server2 = dbopen (login, "PRACTICE");
	dbproc_commit = open_commit (login, "SERVICE");

	if (dbproc_server1 == NULL || 
	    dbproc_server2 == NULL || 
	    dbproc_commit  == NULL)
	{
		dbexit();
		printf(" Connections failed!\n");
		exit (ERREXIT);
	}

	/* Use the "pubs2" database. */
	sprintf(cmdbuf, "use pubs2");
	dbcmd(dbproc_server1, cmdbuf);
	dbsqlexec(dbproc_server1);
	dbcmd(dbproc_server2, cmdbuf);
	dbsqlexec(dbproc_server2);

	/* Start the distributed transaction on the commit service. */
	commid = start_xact(dbproc_commit, "demo", "test", 2);

	/* Build the transaction name. */
	build_xact_string ("test", "SERVICE", commid, xact_string);

	/* Build the first command buffer. */
	sprintf(cmdbuf, "BEGIN TRANSACTION %s", xact_string);

	/* Begin the transactions on the different servers. */
	dbcmd(dbproc_server1, cmdbuf);
	dbsqlexec(dbproc_server1);
	dbcmd(dbproc_server2, cmdbuf);
	dbsqlexec(dbproc_server2);

	/* Do various updates. */
	sprintf(cmdbuf, " update titles set price = $1.50 where");
	strcat(cmdbuf, " title_id = 'BU1032'");
	dbcmd(dbproc_server1, cmdbuf);
	ret_server1 = dbsqlexec(dbproc_server1);
	dbcmd(dbproc_server2, cmdbuf);
	ret_server2 =dbsqlexec(dbproc_server2);
	if (ret_server1 == FAIL || ret_server2 == FAIL)
	{
		/* Some part of the transaction failed. */
		printf(" Transaction aborted -- dbsqlexec failed\n");
		abortall(dbproc_server1, dbproc_server2, dbproc_commit, commid);
	}

	/* Find out if all servers can commit the transaction. */
	sprintf(cmdbuf, "PREPARE TRANSACTION");
	dbcmd(dbproc_server1, cmdbuf);
	dbcmd(dbproc_server2, cmdbuf);
	ret_server1 = dbsqlexec(dbproc_server1);
	ret_server2 = dbsqlexec(dbproc_server2);
	if (ret_server1 == FAIL || ret_server2 == FAIL)
	{
		/* One or both of the servers failed to prepare. */
		printf(" Transaction aborted -- PREPARE failed\n");
		abortall(dbproc_server1, dbproc_server2, dbproc_commit, commid);
	}

	/* Commit the transaction. */
	if (commit_xact(dbproc_commit, commid) == FAIL)
	{
		/* The commit server failed to record the commit. */
		printf( " Transaction aborted -- commit_xact failed\n");
		abortall(dbproc_server1, dbproc_server2, dbproc_commit, commid);
	}

	/* The transaction has successfully committed.  Inform the servers. */
	sprintf(cmdbuf, "COMMIT TRANSACTION");
	dbcmd(dbproc_server1, cmdbuf);
	if (dbsqlexec(dbproc_server1) != FAIL)
	{
		remove_xact(dbproc_commit, commid, 1);
	}

	dbcmd(dbproc_server2, cmdbuf);
	if (dbsqlexec(dbproc_server2) != FAIL)
	{
		remove_xact(dbproc_commit, commid, 1);
	}

	/* Close the connection to the commit server. */
	close_commit(dbproc_commit);

	printf( "We made it!\n");
	dbexit();
	exit(STDEXIT);
}

/* Function to abort the distributed transaction. */

void abortall(DBPROCESS *dbproc_server1, DBPROCESS *dbproc_server2,
	      DBPROCESS *dbproc_commit, int commid )
{
	/* Some part of the transaction failed. */

	/* Inform the commit server of the failure. */
	abort_xact(dbproc_commit, commid);

	/* Roll back the transactions on the different servers. */
	sprintf(cmdbuf, "ROLLBACK TRANSACTION");
	dbcmd(dbproc_server1, cmdbuf);
	if (dbsqlexec(dbproc_server1) != FAIL)
	{
		remove_xact(dbproc_commit, commid, 1);
	}
	
	dbcmd(dbproc_server2, cmdbuf);
	if (dbsqlexec(dbproc_server2) != FAIL)
	{
		remove_xact(dbproc_commit, commid, 1);
	}

	dbexit();
	exit(ERREXIT);
}

/* Message and error handling functions. */

int CS_PUBLIC
msg_handler(DBPROCESS *dbproc, DBINT msgno, int msgstate, int severity,
	    char *msgtext, char *servername, char *procname, int line)
{
	/*   Msg 5701 is just a USE DATABASE message, so skip it.  */
	if (msgno == 5701)
		return (0);

	/*  Print any severity 0 message as is, without extra stuff.  */
	if (severity == 0)
	{
		fprintf(ERR_CH, "%s\n",msgtext);
		return (0);
	}

	fprintf(ERR_CH, "Msg %d, Level %d, State %d\n", 
	        msgno, severity, msgstate);

	if (strlen(servername) > 0)
	{
		fprintf(ERR_CH, "Server '%s', ", servername);
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

	if (severity >= 16)
	{
		dbexit();
		fprintf(ERR_CH, "Program Terminated! Fatal ASE error.\n");
		exit(ERREXIT);
	}

	return (0);   
}

int CS_PUBLIC 
err_handler(DBPROCESS *dbproc, int severity, int dberr, int oserr, 
	    char *dberrstr, char *oserrstr)
{
	if ((dbproc == (DBPROCESS *)NULL) || (DBDEAD(dbproc)))
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

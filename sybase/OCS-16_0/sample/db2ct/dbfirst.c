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
** 	A very simple DB-Library program that
**    	  1. Initializes DB-Library.
**    	  2. Installs callback error- and message-handler functions.
**    	  3. Connects to the server named by the DSQUERY environment
**           variable.
**    	  4. Makes a deliberate DB-Library error to demonstrate the
**           error handler.
**    	  5. Disconnects.
**    	  6. Cleans up and exits.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sybfront.h>
#include <sybdb.h>

/* Forward declarations of the error handler and message handler. */
int CS_PUBLIC err_handler PROTOTYPE ((
	DBPROCESS *dbproc,
	int severity,
	int dberr,
	int oserr,
	char *dberrstr,
	char *oserrstr
	));
int CS_PUBLIC msg_handler PROTOTYPE ((
	DBPROCESS *dbproc,
	DBINT msgno,
	int msgstate,
	int severity,
	char *msgtext,
	char *srvname,
	char *procname,
	int line
	));

#define USER        "sa"
#define PASSWORD    ""
#define ERR_CH      stderr
#define OUT_CH      stdout

int
main(int argc, char *argv[])
{
	DBPROCESS     *dbproc;       /* Our connection with ASE. */
	LOGINREC      *login;        /* Our login information.          */

	/*
	** Step 1.
	** Initialize DB-Library. 
	*/
	if (dbinit() == FAIL)
	{
		exit(ERREXIT);
	}

	/* 
	** Step 2.
	** Install the user-supplied error-handling and message-handling
	** routines. They are defined at the bottom of this source file.
	*/
	dberrhandle(err_handler);
	dbmsghandle(msg_handler);

	/*
	** Step 3. Connect to the server named by the DSQUERY environment 
	** variable.
	**
	** To open the connection, we must:
	**   3a. Allocate a LOGINREC structure.
	**   3b. Insert the username, password and other login parameters
	**       into the LOGINREC.
	**   3c. Call dbopen(), passing the LOGINREC as an argument.
	**
	** If dbopen() succeeds, it returns a non-NULL pointer to a DBPROCESS
	** structure. The DBPROCESS is the control structure for our connection
	** to the server.
	*/

	/*
	** Step 3a.
	** Get a LOGINREC structure.
	*/
	login = dblogin();
	if (login == (LOGINREC *) NULL)
	{
		fprintf(ERR_CH, "dblogin() failed. Exiting.\n");
		dbexit();
		exit(ERREXIT);
	}

	/*
	** Step 3b.
	** Fill it with the username, password, and other login information.
	*/
	DBSETLUSER(login, USER);
	DBSETLPWD(login, PASSWORD);
	DBSETLAPP(login, "dbfirst");

	/*
	** Step 3c.
	** Call dbopen(), passing the LOGINREC. If dbopen() succeeds, it
	** returns a non-NULL pointer to a DBPROCESS.
	*/

	dbproc = dbopen(login, NULL);
	if (dbproc == (DBPROCESS *) NULL)
	{
		fprintf(ERR_CH, "Connect attempt failed. Exiting.\n");
		dbexit();
		exit(ERREXIT);
	}

	/*
	** We are connected.
	*/
	fprintf(OUT_CH, "\n\nWe are connected now.\n\n");

	/*
	** Step 4.
	** Make a deliberate DB-Library error to demonstrate 
	** the error handler.
	*/
	fprintf(ERR_CH, "Ignore the next DB-Library error.\n");
	(void) dbconvert(dbproc, SYBCHAR, NULL, 0, SYBINT4, NULL, 0);

	/*
	** Step 5. 
	** Close our connection.
	*/
	dbclose(dbproc);

	/*
	** Step 6.
	** Clean up and and exit the program. 
	*/
	fprintf(OUT_CH, "\nAll done.\n\n");
	dbexit();
	exit(STDEXIT);
}

int CS_PUBLIC
err_handler(DBPROCESS *dbproc, int severity, int dberr, int oserr,
	    char *dberrstr, char *oserrstr)
{
	if ((dbproc == NULL) || (DBDEAD(dbproc)))
	{
		return INT_EXIT;
	}
	else 
	{
		fprintf(ERR_CH, "DB-Library error %d:\n\t%s\n", dberr,
			 dberrstr);

		if (oserr != DBNOERR)
		{
			fprintf(ERR_CH, "Operating-system error %d:\n\t%s\n",
				 oserr, oserrstr);
		}

		return INT_CANCEL;
	}
}

int CS_PUBLIC
msg_handler(DBPROCESS *dbproc, DBINT msgno, int msgstate, int severity,
	    char *msgtext, char *srvname, char *procname, int line)
{
	fprintf(ERR_CH, "Msg %ld, Level %d, State %d, line %d\n", 
		msgno, severity, msgstate, line);

	if (strlen(srvname) > 0)
	{
		fprintf(ERR_CH, "Server '%s', ", srvname);
	}

	if (strlen(procname) > 0)
	{
		fprintf(ERR_CH, "Procedure '%s', ", procname);
	}

	fprintf(ERR_CH, "\n\t%s\n", msgtext);
	return 0;
}

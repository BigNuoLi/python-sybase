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
** 	ctfirst.c is converted from DB-Library's dbfirst.c to use 
**	Client-Library and CS-Library. 
**
**	This program:
**    	1. Allocate a CS_CONTEXT structure and initialize Client-Library.
**    	2. Installs callback handlers for CS-Library errors, Client-Library
**         errors, and CS-Library errors.
**    	3. Connects to the server named by the DSQUERY environment
**         variable using the credentials defined in dbtoctex.h
**    	4. Makes deliberate CS-Library and Client-Library usage errors
**         to demonstrate the error handlers.
**    	5. Disconnects.
**    	6. Cleans up and exits.
**
** 	The converted DB-Library code is commented out and preceded with
** 	"--" at the beginning of each line.
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*
-- #include <sybfront.h>
-- #include <sybdb.h>
*/
#include <ctpublic.h>
#include "dbtoctex.h"

/*
** Forward declarations of callback handlers for CS-Library errors, 
** Client-Library errors, and server messages. The PROTOTYPE() macro
** is defined through the inclusion of ctpublic.h. PROTOTYPE() allows
** us to define an ANSI prototype when the compiler supports it, and
** a K&R prototype otherwise. Note the double parentheses in the
** PROTOTYPE usage. These are required.
** 
-- int             err_handler();
-- int             msg_handler();
*/
CS_RETCODE CS_PUBLIC cserror_cb PROTOTYPE((
	CS_CONTEXT *context,
	CS_CLIENTMSG *errmsg
	));
CS_RETCODE CS_PUBLIC clientmsg_cb PROTOTYPE((
	CS_CONTEXT *context,
	CS_CONNECTION *connection,
	CS_CLIENTMSG *errmsg
	));
CS_RETCODE CS_PUBLIC servermsg_cb PROTOTYPE((
	CS_CONTEXT *context,
	CS_CONNECTION *connection,
	CS_SERVERMSG *srvmsg
	));

/*
** Useful macros.
*/

/*
** STRLEN() -- strlen() that returns 0 for NULL pointers.
*/
#define STRLEN(str) ( ((str) == NULL) ? 0 : (strlen(str)) )

/* 
** ERROR_SNOL(error_numb, severity, number, origin, layer) --
**
**   This macro does error comparison for Client-Library or CS-Library 
**   errors. It accepts an error number, and constants for the 
**   four error components, then returns non-zero if the error number
**   matches.
** 
** EXAMPLE
**    *
**   ** Does clientmsg->msgnumber represent a timeout on a read from
**   ** the server?
**   *
**   if (ERROR_SNOL(clientmsg->msgnumber, CS_SV_RETRY_FAIL, 63, 2, 1))
**     ... yes, it's a timeout error ...
*/
#define ERROR_SNOL(e, s, n, o, l) \
	( (CS_SEVERITY(e) == s) && (CS_NUMBER(e) == n) \
	&& (CS_ORIGIN(e) == o) && (CS_LAYER(e) == l ) )

int
main(int argc, char *argv[])
{
	/*
	-- DBPROCESS     *dbproc;        * Our connection with ASE. * 
	-- LOGINREC      *login;         * Our login information.          * 
	*/
	CS_CONTEXT    *context = (CS_CONTEXT *) NULL;
	CS_CONNECTION *conn;
	CS_RETCODE    ret;

	/*
	** Setup screen output.
	*/
	EX_SCREEN_INIT();

	/*
	** Step 1.
	** Allocate a CS_CONTEXT structure and initialize Client-Libary. The
	** EXIT_ON_FAIL() macro used for return code error checking is defined
	** in dbtoctex.h. If the return code passed to EXIT_ON_FAIL() is not
	** CS_SUCCEED, it:
	**   - Cleans up the context structure if the pointer is not NULL.
	**   - Exits to the operating system.
	**
	-- if (dbinit() == FAIL)
	-- {
	-- 	exit(ERREXIT);
	-- }
	*/

	ret = cs_ctx_alloc(CS_VERSION_100, &context);
	EXIT_ON_FAIL(context, ret, "Could not allocate context.");

	ret = ct_init(context, CS_VERSION_100);
	EXIT_ON_FAIL(context, ret, "Client-Library initialization failed.");

	/*  
	** Step 2.
	** Install callback handlers for CS-Library errors, Client-Library
	** errors, and CS-Library errors. The handlers are defined at the
	** bottom of this source file.
	**
	-- dberrhandle(err_handler);
	-- dbmsghandle(msg_handler);
	*/

	/*
	** cs_config() installs a handler for CS-Library errors.
	*/
	ret = cs_config(context, CS_SET, CS_MESSAGE_CB, (CS_VOID *) cserror_cb,
			CS_UNUSED, NULL);
	EXIT_ON_FAIL(context, ret,
		     "Could not install CS-Library error handler.");

	/*
	** ct_callback() installs handlers for Client-Library errors and
	** server messages. 
	**
	** ct_callback() lets you install handlers in the context or the
	** connection. Here, we install them in the context so that they
	** are inherited by the connections that are allocated using this
	** context.
	*/
	ret = ct_callback(context, NULL, CS_SET, CS_CLIENTMSG_CB, 
			  (CS_VOID *) clientmsg_cb);
	EXIT_ON_FAIL(context, ret,
		     "Could not install Client-Library error handler.");

	ret = ct_callback(context, NULL, CS_SET, CS_SERVERMSG_CB, 
			  (CS_VOID *) servermsg_cb);
	EXIT_ON_FAIL(context, ret, "Could not install server message handler.");

	/* 
	** Step 3. 
	** Connect to the server named by the DSQUERY environment
	** variable using the credentials defined in dbtoctex.h
	**
	**  3a. Allocate a CS_CONNECTION structure.
	**  3b. Insert the username, password, and other login parameters 
	**      into the connection structure.
	**  3c. Call ct_connect(), passing the CS_CONNECTION as an argument.
	*/

	/* 
	** Step 3a.
	** Allocate a CS_CONNECTION structure. The CS_CONNECTION replaces 
	** DB-Library's LOGINREC and DBPROCESS structures. The LOGINREC
	** fields are connection properties in Client-Library.
	** 
	-- login = dblogin();
	-- if (login == (LOGINREC *) NULL)
	-- {
	-- 	fprintf(ERR_CH, "dblogin() failed. Exiting.\n");
	--	dbexit();
	-- 	exit(ERREXIT);
	-- }
	*/
	ret = ct_con_alloc(context, &conn);
	EXIT_ON_FAIL(context, ret, "Allocate connection structure failed.");

	/* 
	** Step 3b.
	** Put the username, password, and other login information into the
	** connection structure. We do this with ct_con_props() calls.
	** After the connection is open, Client-Library makes these properties
	** read-only.
	**
	** USER and PASSWORD are defined in dbtoctex.h
	**
	-- DBSETLUSER(login, USER);
	-- DBSETLPWD(login, PASSWORD);
	-- DBSETLAPP(login, "dbfirst");
	*/

	ret = ct_con_props(conn, CS_SET, CS_USERNAME, USER, STRLEN(USER), NULL);
	EXIT_ON_FAIL(context, ret, "Set connection username failed.");

	ret = ct_con_props(conn, CS_SET, CS_PASSWORD, PASSWORD,
			   STRLEN(PASSWORD), NULL);
	EXIT_ON_FAIL(context, ret, "Set connection password failed.");

	ret = ct_con_props(conn, CS_SET, CS_APPNAME, "ctfirst",
			   STRLEN("ctfirst"), NULL);
	EXIT_ON_FAIL(context, ret, "Set connection application name failed.");

	/* 
	** Step 3c.
	** Call ct_connect() to open the connection. Unlike dbopen(),
	** ct_connect() uses a connection structure which is already allocated.
	**
	-- dbproc = dbopen(login, NULL);
	-- if (dbproc == (DBPROCESS *) NULL)
	-- {
	-- 	fprintf(ERR_CH, "Connect attempt failed. Exiting.\n");
	-- 	dbexit();
	-- 	exit(ERREXIT);
	-- }
	*/

	ret = ct_connect(conn, NULL, STRLEN(NULL));
	EXIT_ON_FAIL(context, ret, "Connection attempt failed.");

	/* 
	** We are connected.
	*/
	fprintf(OUT_CH, "\n\nWe are connected now.\n\n");

	/*
	** Step 4. 
	** Make deliberate CS-Library and Client-Library usage errors
	** to demonstrate the error handlers.
	*/

	/* 
	** Step 4a. Deliberate CS-Library error.
	** Provoke a deliberate CS-Library error. Among other problems,
	** the call below tries to convert data into a NULL destination.
	**
	** For examples of proper cs_convert() usage, look in ex05ct.c.
	*/
	fprintf(ERR_CH, "\nIgnore the next CS-Library error message.\n");
	ret = cs_convert(context, NULL, NULL, NULL, NULL, NULL);

	/*
	** Step 4b. Deliberate Client-Library error.
	** Provoke a deliberate Client-Library error. The call below
	** attempts to set the CS_USERNAME property of the connection.
	** This call will fail because the connection is already open, 
	** and the client and server have already agreed on the login
	** credentials.
	*/
	fprintf(ERR_CH, "\nIgnore the next Client-Library error message.\n");
	ret = ct_con_props(conn, CS_SET, CS_USERNAME, NULL, STRLEN(NULL), NULL);

	/*
	** Step 5. 
	** Close our connection. CS_UNUSED as the second ct_close() parameter 
	** requests an "orderly" close. This means that we expect
	** the connection to be idle. If we had issued a command to
	** the server, but had not read all the results sent by the server,
	** then the connection would not be idle and this call would fail.
	**
	** If ct_close() were to fail here, then the code in
	** EXIT_ON_FAIL() would ct_exit(CS_FORCE_EXIT) to force
	** all connections closed before exiting.
	**
	-- dbclose(dbproc);
	*/

	ret = ct_close(conn, CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "Orderly connection-close failed.");

	ret = ct_con_drop(conn);
	EXIT_ON_FAIL(context, ret, "ct_con_drop() failed.");

	/*
	** Step 6.
	** Clean up and and exit the program. 
	** 
	-- dbexit();
	-- exit(STDEXIT);
	*/

	fprintf(OUT_CH, "\nAll done.\n\n");

	/*
	** Clean up Client-Library.
	** 
	** ct_exit(context, CS_UNUSED) requests an "orderly" exit -- this
	** call fails if we have open connections. If it fails, EXIT_ON_FAIL()
	** calls ct_exit(context, CS_FORCE_EXIT) to force cleanup of 
	** Client-Library.
	*/

	ret = ct_exit(context, CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "ct_exit(CS_UNUSED) failed.");

	/*
	** Clean up CS-Library. cs_ctx_drop() always fails if ct_init()
	** succeeded on the context but ct_exit() did not (or if ct_exit() 
	** was not called at all).
	*/
	(CS_VOID) cs_ctx_drop(context);
	context = (CS_CONTEXT *) NULL;

	exit(NORMAL_EXIT);

}

/*
** clientmsg_cb() -- Callback handler for Client-Library messages.
**	Client-Library messages inform the application of errors or 
**	significant conditions.
**
** Parameters:
**   context -- Pointer to the context structure where the error occurred.
**	The handler can retrieve context properties and set the CS_USERDATA
**	property.
**   connection -- Pointer to the connection on which the error occurred.
**	This parameter can be NULL if no connection was involved in the
**	error. If connection is non-NULL, the handler can retrieve connection
**	properties, set the CS_USERDATA property, and call 
**	ct_cancel(CS_CANCEL_ATTN) on the connection.
**   errmsg -- Pointer to a CS_CLIENTMSG structure that describes the 
**	error. See the "CS_CLIENTMSG" topics page in the Client-Library
**	reference manual for a description of the fields.
**
** Returns:
**   CS_SUCCEED
**
** Side Effects:
**   None.
*/
CS_RETCODE CS_PUBLIC
clientmsg_cb(CS_CONTEXT *context, CS_CONNECTION *connection,
	     CS_CLIENTMSG *errmsg)
{
	CS_RETCODE ret;
	CS_INT     timeout_val;  

	/*
	** Composition of error messages.
	** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	** Client-Library message numbers encode values for severity,
	** layer, origin, and number. The layer, origin, and number
	** correspond to national language strings from the ctlib.loc
	** locales file. Client-Library composes the text of the message
	** (received in errmsg->msgstring) as follows:
	**   
	**   <routine name>: <layer string>: <origin string>: <description>
	**
	** where
	**   <routine name> is the name of the Client-Library routine
	**      that was active when the exception occurred.
	**   <layer string> describes the layer where the exception occurred
	**      or was found.
	**   <origin string> indicates whether the error is internal or external
	**      to Client-Library.
	**   <description> is the error description.
	*/
	fprintf(ERR_CH, "Client-Library Message: ");
	fprintf(ERR_CH, "LAYER = (%ld) ORIGIN = (%ld) ",
		CS_LAYER(errmsg->msgnumber), CS_ORIGIN(errmsg->msgnumber));
	fprintf(ERR_CH, "SEVERITY = (%ld) NUMBER = (%ld)\n",
		CS_SEVERITY(errmsg->msgnumber), CS_NUMBER(errmsg->msgnumber));
	fprintf(ERR_CH, "Message String: %s\n", errmsg->msgstring);

	/*
	** Operating system errors.
	** ~~~~~~~~~~~~~~~~~~~~~~~
	** Some exceptions reported by Client-Library are caused by exceptions
	** in the underlying system software. When this occurs, Client-Library
	** forwards the system error information to the application.
	*/
	if (errmsg->osstringlen > 0)
	{
		fprintf(ERR_CH, "Operating System Error: %s\n",
		errmsg->osstring);
	}

	/*
	** Handler return values and their meaning.
	** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	** Client-Library error handlers must return CS_SUCCEED or CS_FAIL.
	** Returning any other value "kills" the connection -- Client-
	** Library responds by marking the connection "dead", which makes
	** it unuseable. You can test for dead connections by retrieving
	** the value of the CS_CON_STATUS connection property, which is
	** a bit-masked value. The CS_CONSTAT_DEAD bit is set if the connection
	** is dead. This functionality replaces DB-Library's DBDEAD() macro.
	**
	** Unlike the DB-Library error handler, there is no return code that 
	** causes Client-Library to exit to the operating system.
	** The application must check return codes in the main-line code
	** and abort from the main-line code.
	*/

	/* 
	** (Optional) Test for specific error conditions.
	** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	** The ERROR_SNOL() macro is defined at the top of this file. 
	** The component byte values of a message number (origin, layer, and 
	** number) are defined in the Client-Library locales file.
	*/

	/*
	** Test for timeout errors. Timeout errors will
	** be received when you:
	**   -- are using a synchronous mode connection,
	**   -- have set the CS_TIMEOUT context property to
	**      a non-zero positive value (representing a number of
	**      seconds).
	**   -- the server takes longer than the given time to respond
	**      to a command.
	** 
	** For timeout errors, the command can be canceled with 
	** ct_cancel(CS_CANCEL_ATTN). Other ct_cancel() options are not 
	** to be used in an error handler. If we return CS_SUCCEED
	** without canceling, then Client-Library will wait for another
	** timeout period, then call this error handler again. If the
	** we return CS_FAIL, then Client-Library kills the 
	** connection, making it unuseable.
	*/
	if (ERROR_SNOL(errmsg->msgnumber, CS_SV_RETRY_FAIL, 63, 2, 1))
	{
		/*
		** Get the timeout period. This is not really neccessary, but 
		** demonstrated to show the correlation between timeout errors
		** and the CS_TIMEOUT context property.
		*/
		ret = ct_config(context, CS_GET, CS_TIMEOUT, 
				(CS_VOID *)&timeout_val, CS_UNUSED,
				(CS_INT *)NULL);
		if (ret != CS_SUCCEED)
		{
			timeout_val = 0;
		}

		fprintf(ERR_CH, 
			"\nServer has not responded in at least %ld seconds. Canceling.\n",
			timeout_val);
		(CS_VOID)ct_cancel(connection, (CS_COMMAND *)NULL,
				   CS_CANCEL_ATTN);
	}

	return CS_SUCCEED;
} /* clientmsg_cb() */

/*
** cserror_cb() -- Callback handler for CS-Library errors.
**
** Parameters:
**   context -- Pointer to the context structure passed to the CS-Library 
**	call where the error occurred. The handler can retrieve any
**	context property, and set the CS_USERDATA property.
**   errmsg -- Pointer to a CS_CLIENTMSG structure that describes the 
**	error. See the "CS_CLIENTMSG" topics page in the Client-Library
**	reference manual for a description of the fields.
**
** Returns:
**   CS_SUCCEED
**
** Side Effects:
**   None
*/
CS_RETCODE CS_PUBLIC
cserror_cb(CS_CONTEXT *context, CS_CLIENTMSG *errmsg)
{

	/*
	** Composition of error messages.
	** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	** CS-Library message numbers are decoded the same way as Client-
	** Library messages. See the comments in clientmsg_cb() for a 
	** description.
	*/
	fprintf(ERR_CH, "CS-Library error: ");
	fprintf(ERR_CH, "LAYER = (%ld) ORIGIN = (%ld) ",
		CS_LAYER(errmsg->msgnumber),
		(long)CS_ORIGIN(errmsg->msgnumber));
	fprintf(ERR_CH, "SEVERITY = (%ld) NUMBER = (%ld)\n",
		CS_SEVERITY(errmsg->msgnumber),
		(long)CS_NUMBER(errmsg->msgnumber));
	fprintf(ERR_CH, "Message String: %s\n", errmsg->msgstring);

	/*
	** Operating System Errors.
	** ~~~~~~~~~~~~~~~~~~~~~~~
	** If an operating system error occurred and CS-Library was notified,
	** then CS-Library forwards the error information to the application.
	*/
	if (errmsg->osstringlen > 0)
	{
		fprintf(ERR_CH, "Operating System Error: %s\n",
		errmsg->osstring);
	}

	/*
	** Handler Return Values.
	** ~~~~~~~~~~~~~~~~~~~~~
	** CS-Library error handlers should return CS_SUCCEED.
	*/
	return CS_SUCCEED;

} /* cserror_cb */

/*
** servermsg_cb() -- Callback handler for server messages.
**	The server sends messages to describe errors or significant
**	events. Client-Library calls this function to forward
**	server messages to the client program.
**
** Parameters:
**   context -- Pointer to the context structure that is the parent of
**	the connection. The handler can retrieve context properties 
**	and set the CS_USERDATA property.
**   connection -- Pointer to the connection on which the message was 
**	received. The handler can retrieve any connection property, set 
**	the CS_USERDATA property, and call ct_cancel(CS_CANCEL_ATTN) 
**	on the connection. In addition, when the server sends 
**	extended error data with a message, the handler can retrieve
**	the data. This handler ignores extended error data.
**   srvmsg -- Pointer to a CS_SERVERMSG structure that contains the 
**	message info. See the "CS_SERVERMSG" topics page in the Client-
**	Library reference manual for a description of the fields. All the
**	information that the DB-Library message handler received as
**	parameters is available in the CS_SERVERMSG structure.
**
** Returns:
**   CS_SUCCEED
**
** Side Effects:
**   None
*/
CS_RETCODE CS_PUBLIC
servermsg_cb(CS_CONTEXT *context, CS_CONNECTION *connection,
	     CS_SERVERMSG *srvmsg)
{
	/*
	** CS_SERVERMSG Fields.
	** ~~~~~~~~~~~~~~~~~~~
	** When connected to a ASE, most of the CS_SERVERMSG fields
	** have corresponding columns in the sysmessages system table. When
	** connected to an Open Server, it's up to the Open Server programmer
	** to set the fields for the messages sent by the Open Server.
	*/
	fprintf(ERR_CH, "Server message: ");

	/*
	** For ASE connections, srvmsg->number and srvmsg->severity come 
	** from the sysmessages system table, columns 'error' and 'severity', 
	** respectively.
	*/
	fprintf(ERR_CH, "Number %ld, Severity %ld, ",
		srvmsg->msgnumber, srvmsg->severity);

	/*
	** For ASE connections, srvmsg->line is the line number 
	** in a language batch, or, if srvmsg->proclen field is > 0, the 
	** line number within the stored procedure named in srvmsg->proc.
	** srvmsg->state is the ASE error state, which provides
	** information to SAP Technical Support about serious SAP ASE
	** Server errors.
	*/
	fprintf(ERR_CH, "State %ld, Line %ld\n",
		srvmsg->state, srvmsg->line);

	/*
	** For ASE connections, srvmsg->srvname is the value of
	** the @@servername global variable. See the ASE documentation
	** for information on how to set or change @@servername.
	*/
	if (srvmsg->svrnlen > 0)
	{
		fprintf(ERR_CH, "Server '%s'\n", srvmsg->svrname);
	}

	/*
	** For ASE connections, srvmsg->proclen is > 0 if the message
	** was raised while executing a stored procedure. srvmsg->proc is the
	** procedure name in this case, and srvmsg->line is the line in the
	** procedure's code where the error or condition was raised.
	*/
	if (srvmsg->proclen > 0)
	{
		fprintf(ERR_CH, " Procedure '%s'\n", srvmsg->proc);
	}

	/*
	** Finally, for ASE connections, srvmsg->text is the text of the 
	** message from the 'description' column in sysmessages.
	*/
	fprintf(ERR_CH, "Message String: %s\n", srvmsg->text);

	/*
	** The Client-Library message handler must return CS_SUCCEED.
	** Returning any other value "kills" the connection -- Client-
	** Library responds by marking the connection "dead", which makes
	** it unuseable.
	*/
	return CS_SUCCEED;

} /* servermsg_cb() */


/*
** The original DB-Library error handler, replaced with cserror_cb()
** and clientmsg_cb().
**
-- int CS_PUBLIC
-- err_handler(DBPROCESS *dbproc, int severity, int dberr, int oserr,
-- 	    char *dberrstr, char *oserrstr)
-- {
-- 	if ((dbproc == NULL) || (DBDEAD(dbproc)))
-- 	{
-- 		return INT_EXIT;
-- 	}
-- 	else 
-- 	{
-- 		fprintf(ERR_CH, "DB-Library error %d:\n\t%s\n", dberr,
-- 			 dberrstr);
-- 
-- 		if (oserr != DBNOERR)
-- 		{
-- 			fprintf(ERR_CH, "Operating-system error %d:\n\t%s\n",
-- 				 oserr, oserrstr);
-- 		}
-- 
-- 		return INT_CANCEL;
-- 	}
-- }
*/

/*
** The original DB-Library server message handler, replaced with
** servermsg_cb().
**
-- int CS_PUBLIC
-- msg_handler(DBPROCESS *dbproc, DBINT msgno, int msgstate, int severity,
-- 	    char *msgtext, char *srvname, char *procname, int line)
-- {
-- 	fprintf(ERR_CH, "Msg %ld, Level %d, State %d, line %d\n", 
-- 		msgno, severity, msgstate, line);
-- 
-- 	if (strlen(srvname) > 0)
-- 	{
-- 		fprintf(ERR_CH, "Server '%s', ", srvname);
-- 	}
-- 
-- 	if (strlen(procname) > 0)
-- 	{
-- 		fprintf(ERR_CH, "Procedure '%s', ", procname);
-- 	}
-- 
-- 	fprintf(ERR_CH, "\n\t%s\n", msgtext);
-- 	return 0;
-- }
*/

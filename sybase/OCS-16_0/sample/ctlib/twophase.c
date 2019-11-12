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
**	This program tests CTLIB two phase commit library.
**
** Usage
** -----
**	twophase
**	
** Returns
** -------
**	1 on error or 0
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	"example.h"
#include	"ctxact.h"


#define SERVER1		"snoopy"
#define SERVER2		"woodstock"
#define COMMIT		"snoopy"
#define	USERNAME	"sa"
#define	PASSWORD	""
#define	QUERY		"update titles set price = $1.50 where title_id = 'BU1032'"

/*
** Function decls.
*/
CS_CONNECTION	*init_conn PROTOTYPE((
		CS_CONTEXT      *cp,
		CS_CHAR		*username,
		CS_CHAR		*password
		));
CS_RETCODE	open_conns PROTOTYPE((
		CS_CONTEXT	*cp,
		CS_CONNECTION	**conn1,
		CS_CONNECTION	**conn2,
		CS_CONNECTION	**commit
		));
CS_RETCODE	exec_conn PROTOTYPE((
		CS_CONNECTION	*conn,
		CS_CHAR		*query
		));
CS_RETCODE	close_conn PROTOTYPE((
		CS_CONNECTION	*conn
		));
CS_RETCODE	perform_twophase PROTOTYPE((
		CS_CONNECTION	*conn1,
		CS_CONNECTION	*conn2,
		CS_CONNECTION	*commit,
		CS_CHAR		*query
		));
CS_VOID		abort_all PROTOTYPE((
		CS_CONNECTION	*conn1,
		CS_CONNECTION	*conn2,
		CS_CONNECTION	*commit,
		CS_INT		commid
		));

CS_RETCODE 	cs_err_handler PROTOTYPE ((      /* CS-Library handler     */
		CS_CONTEXT      *cp,
		CS_CLIENTMSG    *msg
		));

CS_RETCODE 	client_err_handler PROTOTYPE ((  /* Client handler         */
		CS_CONNECTION   *chp,
		CS_COMMAND      *cmdp,
		CS_CLIENTMSG    *emsgp
		));

CS_RETCODE	server_err_handler PROTOTYPE ((  /* Server handler       */
		CS_CONTEXT      *cp,
		CS_CONNECTION   *chp,
		CS_SERVERMSG    *msgp
		));

int		main PROTOTYPE((
		int		argc,
		char		*argv[]
		));

int
main(int argc, char *argv[])
{
	CS_CONTEXT	*cp;
	CS_CONNECTION	*conn1, *conn2, *commit;

	/* allocate memory for the context structure			*/
	if (cs_ctx_alloc(EX_CTLIB_VERSION, &cp) != CS_SUCCEED)
	{
		(CS_VOID)fprintf(stderr, 
			"%s: Context allocate failed. Exiting.\n", argv[0]);

		exit(1);
	}

	/* Install a CS-Library error handler.                          */
	if (cs_config(cp, CS_SET, CS_MESSAGE_CB, (CS_VOID *)cs_err_handler,
		CS_UNUSED, NULL) != CS_SUCCEED)
	{
		/* Release the context structure.			*/
		(CS_VOID)cs_ctx_drop(cp);

		(CS_VOID)fprintf(stderr, 
			"%s: Can't init CSLIB handlers. Exiting\n", argv[0]);
		exit(1);
	}

	/* Initialize Open Client version.				*/
	if (ct_init(cp, EX_CTLIB_VERSION) != CS_SUCCEED)
	{
		/* Release the context structure.			*/
		(CS_VOID)cs_ctx_drop(cp);

		(CS_VOID)fprintf(stderr, 
			"%s: Can't init CTLIB version. Exiting\n", argv[0]);
		exit(1);
	}

	/*
	** Install message and error handlers.
	*/
	if (ct_callback(cp, (CS_CONNECTION *)NULL, CS_SET, CS_CLIENTMSG_CB, 
		(CS_VOID *)client_err_handler) != CS_SUCCEED)
	{
		/* Release the context structure.			*/
		(CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
		(CS_VOID)cs_ctx_drop(cp);

		(CS_VOID)fprintf(stderr, 
			"%s: Can't init message handlers. Exiting\n", argv[0]);
		exit(1);
	}
	if (ct_callback(cp, (CS_CONNECTION *)NULL, CS_SET, CS_SERVERMSG_CB, 
		(CS_VOID *)server_err_handler) != CS_SUCCEED)
	{
		/* Release the context structure.			*/
		(CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
		(CS_VOID)cs_ctx_drop(cp);

		(CS_VOID)fprintf(stderr, 
			"%s: Can't init error handlers. Exiting\n", argv[0]);
		exit(1);
	}

	/* Allocate/initialize the connection handles 			*/
	if (open_conns(cp, &conn1, &conn2, &commit) != CS_SUCCEED)
	{
		/* Release the context structure.			*/
		(CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
		(CS_VOID)cs_ctx_drop(cp);

		(CS_VOID)fprintf(stderr, 
			"%s: Can't open all connections. Exiting\n", argv[0]);
		exit(1);
	}

	/* Use the "pubs2" database.					*/
	if ((exec_conn(conn1, "use pubs2") != CS_SUCCEED) ||
	    (exec_conn(conn2, "use pubs2") != CS_SUCCEED))
	{
		/* Release the context structure.			*/
		(CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
		(CS_VOID)cs_ctx_drop(cp);

		(CS_VOID)fprintf(stderr, 
			"%s: Can't issue 'use pubs2' on connections. Exiting\n",
			argv[0]);
		exit(1);
	}


	if (perform_twophase(conn1, conn2, commit, QUERY) != CS_SUCCEED)
	{
		(CS_VOID)fprintf(stderr, 
			"perform_twophase() failed for '%s'\n", QUERY);

		(CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
		(CS_VOID)cs_ctx_drop(cp);
		exit(1);
	}

	/* Close the connections used					*/
	if ((close_conn(conn1) != CS_SUCCEED) ||
	    (close_conn(conn2) != CS_SUCCEED) ||
	    (close_commit(commit) != CS_SUCCEED))
	{
		(CS_VOID)fprintf(stderr, "Failure closing connections...\n");

		(CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
		(CS_VOID)cs_ctx_drop(cp);
		exit(1);
	}

	/* Release allocated control structures				*/
	(CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
	(CS_VOID)cs_ctx_drop(cp);

	return(0);
}

/*
**  PERFORM_TWOPHASE - 
**	Perform two phase commit.
**
**  Parameters:
**	conn1		Pointer to the first connection.
**	conn2		Pointer to the second connection.
**	commit		Pointer to the commit connection.
**	query1		Query to perform on.
**
**  Returns:
**	CS_SUCCEED | CS_FAIL
**
*/
CS_RETCODE
perform_twophase(CS_CONNECTION *conn1, CS_CONNECTION *conn2, 
		 CS_CONNECTION *commit, CS_CHAR *query)
{
	CS_INT		commid;
	CS_INT		outlen;
	CS_CHAR		xact_string[CS_MAX_MSG + 1];
	CS_CHAR		command[CS_MAX_MSG];

	/* Start the distributed transaction on the commit service.	*/
	if ((commid = start_xact(commit, "demo", CS_NULLTERM, "Twophase", 
		CS_NULLTERM, 2)) <= 0)
	{
		return (CS_FAIL);
	}

	/* Build the transaction name. 					*/
	if (build_xact_string ("Twophase", CS_NULLTERM, SERVER2, CS_NULLTERM,
		commid, xact_string, CS_MAX_MSG, &outlen) != CS_SUCCEED)
	{
		(CS_VOID)fprintf(stderr, "Build transaction failed...\n");
		abort_xact(commit, commid);
		(CS_VOID)remove_xact(commit, commid, 2);
		return (CS_FAIL);
	}
	xact_string[outlen] = '\0';

	/* Build the first command buffer. 				*/
	(CS_VOID)sprintf(command, "BEGIN TRANSACTION %s", xact_string);

	/* Begin the transaction on the different servers		*/
	if ((exec_conn(conn1, command) != CS_SUCCEED) ||
	    (exec_conn(conn2, command) != CS_SUCCEED))
	{
		(CS_VOID)fprintf(stderr, "Begin transaction failed...\n");
		abort_xact(commit, commid);
		(CS_VOID)remove_xact(commit, commid, 2);
		return (CS_FAIL);
	}

	/* Scan for the transaction					*/
	if (scan_xact(commit, commid) != CS_SUCCEED)
	{
		(CS_VOID)fprintf(stderr, "Scan transaction failed...\n");
		abort_all(conn1, conn2, commit, commid);
		return (CS_FAIL);
	}

	/* Perform update						*/
	if ((exec_conn(conn1, query) != CS_SUCCEED) ||
	    (exec_conn(conn2, query) != CS_SUCCEED))
	{
		(CS_VOID)fprintf(stderr, "Update query failed...\n");
		abort_all(conn1, conn2, commit, commid);
		return (CS_FAIL);
	}

	/* Find out if all servers can commit the transaction. 		*/
	if ((exec_conn(conn1, "PREPARE TRANSACTION") != CS_SUCCEED) ||
	    (exec_conn(conn2, "PREPARE TRANSACTION") != CS_SUCCEED))
	{
		(CS_VOID)fprintf(stderr, "PREPARE transaction failed...\n");
		abort_all(conn1, conn2, commit, commid);
		return (CS_FAIL);
	}

	/* Commit the transaction. 					*/
	if (commit_xact(commit, commid) != CS_SUCCEED)
	{
		/* The commit server failed to record the commit. 	*/
		fprintf(stderr, "Transaction aborted -- commit_xact failed\n");
		abort_all(conn1, conn2, commit, commid);
		return (CS_FAIL);
	}

	/* 
	** The transaction has successfully committed.  
	** Inform the servers. 
	*/
	if (exec_conn(conn1, "COMMIT TRANSACTION") == CS_SUCCEED)
		(CS_VOID)remove_xact(commit, commid, 1);
	if (exec_conn(conn2, "COMMIT TRANSACTION") == CS_SUCCEED)
		(CS_VOID)remove_xact(commit, commid, 1);

	return (CS_SUCCEED);
}

/*
**  ABORT_ALL - 
**	Abort the connections.
**
**  Parameters:
**	conn1		Pointer to the first connection.
**	conn2		Pointer to the second connection.
**	commit		Pointer to the commit connection.
**	commid		The commit id.
**
**  Returns:
**	none.
**
*/
CS_VOID	
abort_all(CS_CONNECTION *conn1, CS_CONNECTION *conn2, CS_CONNECTION *commit, CS_INT commid)
{
	/* Inform the commit server of the failure			*/
	abort_xact(commit, commid);

	/* Roll back the transactions on the different servers. */
	(CS_VOID)exec_conn(conn1, "ROLLBACK TRANSACTION");
	(CS_VOID)exec_conn(conn2, "ROLLBACK TRANSACTION");
	(CS_VOID)remove_xact(commit, commid, 2);
}

/*
**  OPEN_CONNS - 
**	Initialize/Open the three connections needed.
**
**  Parameters:
**	cp		The application context.
**	conn1		Pointer to the first connection.
**	conn2		Pointer to the second connection.
**	commit		Pointer to the commit connection.
**
**  Returns:
**	CS_SUCCEED | CS_FAIL
**
**  Side Effects:
** 	Performs no clean-up of connection handles on failure.
**
*/
CS_RETCODE
open_conns(CS_CONTEXT *cp, CS_CONNECTION **conn1, CS_CONNECTION **conn2, CS_CONNECTION **commit)
{
	if ((*conn1 = init_conn(cp, USERNAME, PASSWORD)) 
		== (CS_CONNECTION *)NULL)
	{
		return (CS_FAIL);
	}
	if ((*conn2 = init_conn(cp, USERNAME, PASSWORD)) 
		== (CS_CONNECTION *)NULL)
	{
		return (CS_FAIL);
	}
	if ((*commit = init_conn(cp, USERNAME, PASSWORD)) 
		== (CS_CONNECTION *)NULL)
	{
		return (CS_FAIL);
	}

	/* Attempt a connection to the remote servers		*/
	if (ct_connect(*conn1, SERVER1, CS_NULLTERM) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	if (ct_connect(*conn2, SERVER2, CS_NULLTERM) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	if (open_commit(*commit, SERVER2, CS_NULLTERM) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	return (CS_SUCCEED);
}

/*
**  INIT_CONN - 
**	Initialize the connection to the Server
**
**  Parameters:
**	cp		The application context.
**	username	The user name to initialize
**	password	The password.
**
**  Returns:
**	The initialized connection on success, NULL on failure.
**
*/
CS_CONNECTION *
init_conn(CS_CONTEXT *cp, CS_CHAR *username, CS_CHAR *password)
{
	CS_CONNECTION	*conn;		/* Connection control struct	*/

	/* Initialization						*/
	conn = (CS_CONNECTION *)NULL;	/* not allocated yet		*/

	/* Allocate a connection handle for the connection attempt	*/
	if (ct_con_alloc(cp, &conn) != CS_SUCCEED)
	{
		/*
		** Send a message that we could not build a remote 
		** connection because of a memory allocation failure
		*/
		return ((CS_CONNECTION *)NULL);
	}

	/*
	** Set up the login information for the remote DBMS.
	** Also save control structures for access from other routines.
	*/

	if (ct_con_props(conn, CS_SET, CS_USERNAME, USERNAME, 
		CS_NULLTERM, (CS_INT *)NULL) != CS_SUCCEED)
	{
		(CS_VOID)ct_con_drop(conn);
		return ((CS_CONNECTION *)NULL);
	}

	if (ct_con_props(conn, CS_SET, CS_PASSWORD, PASSWORD, 
		CS_NULLTERM, (CS_INT *)NULL) != CS_SUCCEED)
	{
		(CS_VOID)ct_con_drop(conn);
		return ((CS_CONNECTION *)NULL);
	}

	if (ct_con_props(conn, CS_SET, CS_APPNAME, "Twophase",
		CS_NULLTERM, (CS_INT *)NULL) != CS_SUCCEED)
	{
		(CS_VOID)ct_con_drop(conn);
		return ((CS_CONNECTION *)NULL);
	}

	return (conn);
}

/*
**  CLOSE_CONN - Close a connection
**
**  Parameters:
**	conn - the connection to close
**
**  Returns:
**	none.
*/
CS_RETCODE
close_conn(CS_CONNECTION *conn)
{
	/* If connection with server active, close it now		*/
	if (conn != (CS_CONNECTION *)NULL)
	{
		if (ct_close(conn, CS_UNUSED) != CS_SUCCEED)
		{
			(CS_VOID)ct_close(conn, CS_FORCE_CLOSE);
			return (CS_FAIL);
		}
		if (ct_con_drop(conn) != CS_SUCCEED)
			return (CS_FAIL);
	}

	return (CS_SUCCEED);
}

/*
**  server_err_handler - Server errors 
**	Open Client calls this routine when it receives messages from the
**	server. It reads the message information and prints it to the
**	console. 
**
**  Parameters:
**	cp		A pointer to the context structure for this 
**			application.
**
**	chp		A pointer to the connection handle on which the 
**			error occurred.	(It can be NULL.)
**
**	msgp		A pointer to a structure containing information 
**			about the server message.
**  Returns:
**	CS_SUCCEED
*/
CS_RETCODE 
server_err_handler(CS_CONTEXT *cp, CS_CONNECTION *chp, CS_SERVERMSG *msgp)
{
	/* 
	** If chp is (CS_CONNECTION *)NULL, this error occurred when no 
	** connection handle was active.  Just return.
	*/
	if (chp == (CS_CONNECTION *)NULL) 
		return (CS_SUCCEED);

	if (msgp->msgnumber == 5701 || msgp->msgnumber == 5703 ||
	    msgp->msgnumber == 5704)
	{
		return (CS_SUCCEED);
	}

	/* Build and print the client message. 				*/
	(CS_VOID)fprintf(stdout, 
		"Server: msgno(%d) severity(%d) state(%d)\n",
		msgp->msgnumber, msgp->severity, msgp->state);

	/* Null terminate message.					*/
	switch((int)msgp->textlen)
	{
	case CS_NULLTERM:
		/* Message already null terminated.			*/
		break;
	case CS_UNUSED:
	case 0:
		/* No message specified.				*/
		msgp->text[0] = '\0';
		break;
	default:
		if (msgp->textlen < (CS_MAX_MSG - 1))
			msgp->text[msgp->textlen] = '\0';
		else
			msgp->text[CS_MAX_MSG - 1] = '\0';
		break;
	}

	/* Print the null terminated message. 				*/
	(CS_VOID)fprintf(stdout, "\t%s\n", msgp->text);

	return(CS_SUCCEED);
}

/*
**  client_err_handler - Open Client errors.
**	This is the Open Client error handler.  All Open Client connections
**	use this error handler.  The error messages are printed to the 
**	console.
**
**  Parameters:
**	chp		A pointer to the connection handle on which this
**			error occurred. (It can be NULL.)
**
**	cmdp		A pointer to the command handle active when error 
**			occurred. (It can be NULL.)
**
**	emsgp		A pointer to an Open Client CS_CLIENTMSG structure.
**
**  Returns:
**	CS_SUCCEED
*/
CS_RETCODE
client_err_handler(CS_CONNECTION *chp, CS_COMMAND *cmdp, CS_CLIENTMSG *emsgp)
{
	/* 
	** If chp is (CS_CONNECTION *)NULL, this error occurred when no 
	** connection handle was active.  Just return cancel.  
	*/
	if (chp == (CS_CONNECTION *)NULL) 
		return (CS_SUCCEED);

	/*
	** Format and print the message. */
	(CS_VOID)fprintf(stdout, 
		"CT-Library error(L%d O%d S%d N%d) serverity(%d)\n",
		CS_LAYER(emsgp->msgnumber), 
		CS_ORIGIN(emsgp->msgnumber), 
		CS_SEVERITY(emsgp->msgnumber), 
		CS_NUMBER(emsgp->msgnumber), 
		emsgp->severity);

	(CS_VOID)fprintf(stdout, "\t%s\n", emsgp->msgstring);

	/*
	** If there was an operating system error, send the error
	** message text for the OS error.
	 */
	if ((emsgp->osstringlen != 0) && (emsgp->osstringlen != CS_UNUSED))
	{
		(CS_VOID)fprintf(stdout, 
			"CT-Library OS error %d - %s.\n",
			emsgp->osnumber, emsgp->osstring);
	}
	return (CS_SUCCEED);
}

/*
**  CS_ERR_HANDLER - CS-Library error handler.
** 	This routine is the CS-Library error handler used by this
** 	application. It is called by CS-Library whenever an error
** 	occurs. Here, we simply print the error and return.
**
**  Parameters:
**      cp              A pointer to the connection handle for connection
**		 	on which the message was received.
** 			(It can be NULL.)
**
**      msg             The structure containing information about the
**			server message.
**
**  Returns:
**      CS_SUCCEED
*/
CS_RETCODE
cs_err_handler(CS_CONTEXT *cp, CS_CLIENTMSG *msg)
{
	/*
	** Build the error message string to be logged.
	*/
	(CS_VOID)fprintf(stdout, 
		"CS-Library error(L%d O%d S%d N%d) serverity(%d)\n",
		CS_LAYER(msg->msgnumber), 
		CS_ORIGIN(msg->msgnumber), 
		CS_SEVERITY(msg->msgnumber), 
		CS_NUMBER(msg->msgnumber), 
		msg->severity);

	(CS_VOID)fprintf(stdout, "\t%s\n", msg->msgstring);

	/*
	** Print any operating system error information.
	*/
	if( msg->osstringlen > 0 )
	{
		(CS_VOID)fprintf(stdout, 
			"CS-Library OS error %d - %s.\n",
			msg->osnumber, msg->osstring);
	}

	/*
	** All done.
	*/
	return (CS_SUCCEED);
}

/*
** EXEC_CONN
**
**	This routine sends a command to the Server and ignores any resulting 
**	errors.
**	
** Parameters:
**	CS_CONNECTION	*conn	The server connection.
**	CS_CHAR		*query	The query to execute.
**
** Returns:
**	CS_SUCCEED		Test command executed as expected.
**	CS_FAIL			Test command detected an unexpected result.
*/
CS_RETCODE 
exec_conn(CS_CONNECTION *conn, CS_CHAR *query)
{
	CS_INT		ret;
	CS_INT		retcode;
	CS_COMMAND	*cmd;

	/* Allocate a command buffer					*/
	if (ct_cmd_alloc(conn, &cmd) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	if (ct_command(cmd, CS_LANG_CMD, query, CS_NULLTERM, 
		CS_UNUSED) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	if (ct_send(cmd) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	while ((retcode = ct_results(cmd, &ret)) == CS_SUCCEED)
	{
		switch ((int)ret)
		{
		case CS_ROW_RESULT:
		case CS_STATUS_RESULT:
			while ((retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, 
				CS_UNUSED, NULL)) == CS_SUCCEED)
			{
				continue;
			}

			if (retcode != CS_END_DATA)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (CS_FAIL);
			}

		case CS_CMD_FAIL:
			(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
				cmd, CS_CANCEL_ALL);
			(CS_VOID)ct_cmd_drop(cmd);
			return (CS_FAIL);

		default:
			if (ct_cancel((CS_CONNECTION *)NULL, cmd, 
				CS_CANCEL_CURRENT) != CS_SUCCEED)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (CS_FAIL);
			}
			break;
		}
	}

	if (retcode != CS_END_RESULTS)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	if (ct_cmd_drop(cmd) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	return (CS_SUCCEED);
}

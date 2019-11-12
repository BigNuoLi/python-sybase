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
** 	Async Language Query Example Program.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctpublic.h>
#include "example.h"
#include "exutils.h"
#include "exasync.h"

/*****************************************************************************
** 
** defines used 
** 
*****************************************************************************/

#ifdef HAFAILOVER

/*
**  Global variable for HAFailover 
*/
CS_INT hafailoversucceed;

#endif  /* HAFAILOVER */

/*
** sample query to send
*/
#define EX_QUERY	"waitfor delay \"0:00:01\"\nselect name, dbid, version from sysdatabases"

/*
** Maximum memory allocation for result area
*/
#define MEM_POOL_SIZE	(1000 * 32)

/*
** Enable error tracing
*/
#define ERROR_DEBUG	0

/*
** what async mode to use. Possible values are:
**
**	CS_ASYNC_IO	True async, runs at interrupt level.
**	CS_DEFER_IO	Deferred async, runs only when ct_poll() is called.
*/
#define EX_IO_MODE	CS_DEFER_IO

/*
** Install a signal handler
*/
#ifndef  USE_SIG_HANDLER

#define USE_SIG_HANDLER		1
#include <signal.h>

#endif /* USE_SIG_HANDLER */

/*****************************************************************************
** 
** global data used 
** 
*****************************************************************************/

CS_CHAR *Ex_server   = EX_SERVER;
CS_CHAR *Ex_username = EX_USERNAME;
CS_CHAR *Ex_password = EX_PASSWORD;

/*
** define a global context handle to use
*/
CS_STATIC CS_CONTEXT	*Ex_context;
CS_STATIC CS_CONNECTION	*Ex_connection;

/*
** The following variables are used to determine when a completion has
** occurred. "completed" is set to true by the installed async callback,
** "completed_retstat" reflects the status of the completion, once
** "completed" is set to true.
*/
CS_STATIC CS_BOOL	completed         = CS_FALSE;
CS_STATIC CS_RETCODE	completed_retstat = CS_FAIL;

/*****************************************************************************
** 
** static prototypes used 
** 
*****************************************************************************/

CS_RETCODE CS_PUBLIC CompletionCB PROTOTYPE((
	CS_CONNECTION *connection,
	CS_COMMAND *cmd,
	CS_INT function,
	CS_RETCODE status
	));
CS_STATIC CS_VOID CS_INTERNAL BusyClear PROTOTYPE((void));
CS_STATIC CS_RETCODE CS_INTERNAL BusyWait PROTOTYPE((
	CS_CONNECTION *connection,
	char *where
	));
CS_STATIC CS_VOID CS_INTERNAL Init PROTOTYPE((void));
CS_STATIC CS_VOID CS_INTERNAL DoConnect PROTOTYPE((void));
CS_STATIC CS_VOID CS_INTERNAL Cleanup PROTOTYPE((
	CS_CONNECTION *connection
	));
CS_STATIC CS_VOID CS_INTERNAL Display PROTOTYPE((
	EX_RESULTS *results
	));

/*****************************************************************************
** 
** functions 
** 
*****************************************************************************/

#ifdef USE_SIG_HANDLER
void
send_attn(int sig)
{
	CS_RETCODE	retstat;

	fprintf(stdout, "\nCalling ct_cancel\n");
	fflush(stdout);
	retstat = ct_cancel(Ex_connection, NULL, CS_CANCEL_ATTN);
	if (retstat != CS_SUCCEED && retstat != CS_TRYING)
	{
		fprintf(stdout,
			"ct_cancel returned unexpected status of %d\n",
			retstat);
		fflush(stdout);
	}

	return;
}
#endif /* USE_SIG_HANDLER */

/*
** CompletionCB()
**
** Type of function:
**	async example program client completion handler
**
** Purpose:
**	Installed as a callback into Open Client. Mark the completed
**	flag for the busy functions to use.
**
** Returns:
**	CS_SUCCEED
**
** Side Effects:
**	None
**
*/

CS_RETCODE CS_PUBLIC
CompletionCB(CS_CONNECTION *connection, CS_COMMAND *cmd, CS_INT function, CS_RETCODE status)
{
	fprintf(stdout, "\nCompletionCB: function %d Completed\n",
			function);
	fflush(stdout);

	completed_retstat      = status;
	completed = CS_TRUE;

	return CS_SUCCEED;
}

/*
** BusyClear()
**
** Type of function:
**	async example program api
**
** Purpose:
**	Resets the completed flag used by the busy_wait function.
**
** Returns:
**	Nothing.
**
** Side Effects:
**	None
*/

CS_STATIC CS_VOID CS_INTERNAL
BusyClear(void)
{
	completed = CS_FALSE;
}

/*
** BusyWait()
**
** Type of function:
**	async example program api
**
** Purpose:
**	Silly routine that prints out dots while waiting for an async
**	operation to complete. It demonstrates the ability to do other work
**	while an async operation is pending.
**
** Returns:
**	completion status.
**
** Side Effects:
**	None
*/

CS_STATIC CS_RETCODE CS_INTERNAL
BusyWait(CS_CONNECTION *connection, char *where)
{
	CS_COMMAND	*compcmd;
	CS_INT		compid;
	CS_RETCODE	compstat;
	CS_RETCODE	retstat;

	fprintf(stdout, "\nWaiting [%s]", where);
	fflush(stdout);

	do
	{
		fprintf(stdout, ".");
		fflush(stdout);
		retstat = ct_poll(NULL, connection, 100, NULL, &compcmd,
			&compid, &compstat);
		if (retstat != CS_SUCCEED && retstat != CS_TIMED_OUT &&
		    retstat != CS_INTERRUPT)
		{
#ifdef  HAFAILOVER
			if (hafailoversucceed && (retstat == CS_QUIET))
			{
				printf("\nFailover occured, ct_poll returned CS_QUIET\n");
				return CS_SUCCEED;
			}
#endif  /* HAFAILOVER */

			fprintf(stdout,
				"\nct_poll returned unexpected status of %d\n",
				retstat);
			fflush(stdout);
			break;
		}
	} while (retstat != CS_SUCCEED);

	if (retstat == CS_SUCCEED)
	{
		fprintf(stdout,
			"\nct_poll completed: compid = %d, compstat = %d\n",
			compid, compstat);
		fflush(stdout);
	}

	return compstat;
}

/*
** Init()
**
** Type of function:
**      async example program api
**
** Purpose:
**	Set up context related information.
**
** Returns:
**      Nothing.
**
** Side Effects:
**      None
*/

CS_STATIC CS_VOID CS_INTERNAL
Init(void)
{
	CS_RETCODE	retstat;

	/*
	** Get a context handle to use.
	*/
	retstat = cs_ctx_alloc(EX_CTLIB_VERSION, &Ex_context);
	if (retstat != CS_SUCCEED)
	{
		ex_panic("cs_ctx_alloc failed");
	}

	/*
	** Initialize Open Client.
	*/
	retstat = ct_init(Ex_context, EX_CTLIB_VERSION);
	if (retstat != CS_SUCCEED)
	{
		ex_panic("ct_init failed");
	}

	/*
	** ct_debug stuff. Enable this function right before any call to
	** Client-Library that is returning failure.
	*/
#if ERROR_DEBUG
	retstat = ct_debug(Ex_context, NULL, CS_SET_FLAG, CS_DBG_ERROR,
		NULL, CS_UNUSED);
	if (retstat != CS_SUCCEED)
	{
		ex_panic("ct_debug failed");
	}
#endif

#ifdef API_DEBUG
	retstat = ct_debug(Ex_context, NULL, CS_SET_FLAG, CS_DBG_API_STATES,
				NULL, CS_UNUSED);
	if (retstat != CS_SUCCEED)
	{
		ex_panic("ct_debug failed");
	}
#endif

#ifdef ASYNC_DEBUG
	/*
	** Open a file to write to it
	*/
        retstat = ct_debug(Ex_context, NULL, CS_SET_DBG_FILE, CS_UNUSED,
                        "ctdebug.out", CS_NULLTERM);
        if (retstat != CS_SUCCEED)
        {
                ex_panic("ct_debug CS_SET_DBG_FILE failed");
        }


	retstat = ct_debug(Ex_context, NULL, CS_SET_FLAG, CS_DBG_ASYNC,
				NULL, CS_UNUSED);
	if (retstat != CS_SUCCEED)
	{
		ex_panic("ct_debug failed");
	}
#endif

	/*
	** Install message and completion handlers.
	*/
	retstat = ct_callback(Ex_context, NULL, CS_SET, CS_CLIENTMSG_CB,
				(CS_VOID *)ex_clientmsg_cb);
	if (retstat != CS_SUCCEED)
	{
		ex_panic("ct_callback failed");
	}
	retstat = ct_callback(Ex_context, NULL, CS_SET, CS_SERVERMSG_CB,
				(CS_VOID *)ex_servermsg_cb);
	if (retstat != CS_SUCCEED)
	{
		ex_panic("ct_callback failed");
	}
	retstat = ct_callback(Ex_context, NULL, CS_SET, CS_COMPLETION_CB,
				(CS_VOID *)CompletionCB);
	if (retstat != CS_SUCCEED)
	{
		ex_panic("ct_callback failed");
	}

}

/*
** DoConnect()
**
** Type of function:
**	async example program api
**
** Purpose:
**	Established an asynchronous connection to a server.
**
** Returns:
**	Nothing.
**
** Side Effects:
**	None
*/

CS_STATIC CS_VOID CS_INTERNAL 
DoConnect(void)
{
	CS_INT		netio_type = EX_IO_MODE;
	CS_RETCODE	retstat;
	CS_INT		slen;

#ifdef  HAFAILOVER
	CS_INT          bhafailover;
#endif  /* HAFAILOVER */

	/*
	** Open a connection to the server.
	*/
	retstat = ct_con_alloc(Ex_context, &Ex_connection);
	if (retstat != CS_SUCCEED)
	{
		ex_panic("ct_con_alloc failed");
	}

#if USE_SIG_HANDLER
	signal(SIGINT, send_attn);
#endif 

	retstat = ct_con_props(Ex_connection, CS_SET, CS_USERNAME,
			  Ex_username, CS_NULLTERM, NULL);
	if (retstat != CS_SUCCEED)
	{
		ex_panic("ct_con_props failed");
	}

	retstat = ct_con_props(Ex_connection, CS_SET, CS_PASSWORD,
			  Ex_password, CS_NULLTERM, NULL);
	if (retstat != CS_SUCCEED)
	{
		ex_panic("ct_con_props failed");
	}

	retstat = ct_con_props(Ex_connection, CS_SET, CS_NETIO,
			  (CS_VOID *)&netio_type, CS_UNUSED, NULL);
	if (retstat != CS_SUCCEED)
	{
		ex_panic("ct_con_props failed");
	}

#ifdef HAFAILOVER
        bhafailover = CS_TRUE;
        retstat = ct_con_props(Ex_connection, CS_SET, CS_HAFAILOVER,
                          &bhafailover, CS_UNUSED, NULL);
        if (retstat != CS_SUCCEED)
        {
                ex_panic("ct_con_props setting CS_HAFAILOVER failed");
        }

#endif  /* HAFAILOVER */


	BusyClear();
	printf("TRYING TO CONNECT TO %s.\n", (Ex_server == NULL)
						? "NULL" : Ex_server);
	slen = (Ex_server == NULL) ? 0 : CS_NULLTERM;
	retstat = ct_connect(Ex_connection, Ex_server, slen);
	if (retstat != CS_SUCCEED && retstat != CS_PENDING)
	{
		ex_panic("ct_connect failed");
	}
	retstat = BusyWait(Ex_connection, "ct_connect");
	if (retstat != CS_SUCCEED && retstat != CS_PENDING)
	{
		ex_panic("ct_connect failed");
	}
	printf("WE ARE CONNECTED NOW\n");

	return;
}

/*
** Cleanup()
**
** Type of function:
**	async example program api
**
** Purpose:
**	Closes the connection and shuts down CT-Lib.
**
** Returns:
**	Nothing.
**
** Side Effects:
**	None
*/

CS_STATIC CS_VOID CS_INTERNAL
Cleanup(CS_CONNECTION *connection)
{
	CS_RETCODE	retstat;

	BusyClear();
	retstat = ct_close(connection, CS_UNUSED);
	if (retstat != CS_SUCCEED && retstat != CS_PENDING)
	{
		ex_panic("ct_close failed");
	}
	retstat = BusyWait(connection, "ct_close");
	if (retstat != CS_SUCCEED && retstat != CS_PENDING)
	{
		ex_panic("BusyWait (ct_close) failed");
	}
	if (ct_con_drop(connection) != CS_SUCCEED)
	{
		ex_panic("ct_con_drop failed");
	}

	if (ct_exit(Ex_context, CS_UNUSED) != CS_SUCCEED)
	{
		ex_panic("ct_exit failed");
	}

	if (cs_ctx_drop(Ex_context) != CS_SUCCEED)
	{
		ex_panic("cs_ctx_drop failed");
	}
}

/*
** Display()
**
** Type of function:
**	async example program api
**
** Purpose:
**
** Returns:
**	Nothing.
**
** Side Effects:
**	None
*/

CS_STATIC CS_VOID CS_INTERNAL
Display(EX_RESULTS *results)
{
	CS_INT		row;
	CS_INT		col;

	ex_display_header(results->numcols, results->colfmts);

/*
** Macro to access the column data the was fetched in the async library.
** Probably should be moved to exasync.h
*/
#define DATA_OFFSET(RES, COL, ROW) \
		((RES)->data[COL] + (ROW * (RES)->colfmts[COL].maxlength))

	for (row = 0; row < results->numrows; row++)
	{
		for (col = 0; col < results->numcols; col++)
		{
			ex_display_column(Ex_context, &results->colfmts[col],
				DATA_OFFSET(results, col, row),
				results->datalen[col][row],
				results->indicator[col][row]);
		}
		fputc('\n', stdout);
	}
}

/*
** main()
**
** Type of function:
**	main routine for async example program
**
** Parameters:
**	None, argc and argv will not be used.
**
** Purpose:
**	This routine will call the async library to send a query to a server
**	and retrieve any rows sent back. It then displays the fetched rows.
**
** Returns:
** 	EX_EXIT_ERROR  or EX_EXIT_SUCCEED
**	
*/

int
main(int argc, char *argv[])
{
	CS_RETCODE	retstat;
	EX_ASYNC	*ex_async;
	EX_RESULTS	results;
	CS_VOID		*mempool;
	CS_CHAR		buf[MAX_CHAR_BUF];

#if USE_SIG_HANDLER
	signal(SIGINT, SIG_IGN);
#endif
	EX_SCREEN_INIT();

	fprintf(stdout, "Asynchronous IO Example\n");
	fflush(stdout);

	/*
	** Initialize global stuff
	*/
	Init();

	/*
	** establish a connection
	*/
	DoConnect();

	/*
	** Allocate a memory pool to use for results
	*/
	mempool = (CS_VOID *)malloc(MEM_POOL_SIZE);
	if (mempool == NULL)
	{
		ex_panic("malloc failed");
	}

	/*
	** allocate an async handle
	*/
	retstat = ex_async_alloc(Ex_connection, &results, 
				mempool, MEM_POOL_SIZE, &ex_async);
	if (retstat != CS_SUCCEED)
	{
		ex_panic("ex_async_alloc failed");
	}

	for (;;)
	{

#ifdef HAFAILOVER
		hafailoversucceed = CS_FALSE;
#endif  /* HAFAILOVER */

		/*
		** call ex_async to send a query.
		*/
		BusyClear();
		fprintf(stdout, "\nEnter query to send or type quit: ");
		fflush(stdout);
		fgets(buf, sizeof (buf), stdin);
		if (strncmp(buf, "quit", 4) == 0)
		{
			break;
		}

		retstat = ex_async_query(ex_async, buf, CS_NULLTERM);
		if (retstat != CS_SUCCEED && retstat != CS_PENDING)
		{
			ex_panic("ex_async_query failed");
		}
		retstat = BusyWait(Ex_connection, "ex_async_query");
		if (retstat != CS_SUCCEED && retstat != CS_PENDING &&
		    retstat != CS_CANCELED)
		{
			ex_panic("BusyWait (ex_async_query) failed");
		}

#ifdef HAFAILOVER
                /*
                ** if HA has succeeded , start afresh - that means
                ** bring it to the state of a new connection.
                ** clean up the old ex_async structure and get a new one.
                */
                if(hafailoversucceed)
                {
			/*
			** Re-install user completion routine
			*/
			retstat = ct_callback(NULL, Ex_connection, CS_SET,
				CS_COMPLETION_CB, (CS_VOID *)CompletionCB);
			if (retstat != CS_SUCCEED)
			{
				ex_panic("ct_callback failed");
			}

                        if (ex_async != (EX_ASYNC *)NULL)
                        {
				ex_async_free(ex_async);
                        }

                        if (mempool != (char *)NULL)
                        {
                                free(mempool);
                        }

                        /*
                        ** Allocate a memory pool to use for results
                        */
                        mempool = (CS_VOID *)malloc(MEM_POOL_SIZE);
                        if (mempool == NULL)
                        {
                                ex_panic("malloc failed");
                        }

                        /*
                        ** allocate an async handle
                        */
                        retstat = ex_async_alloc(Ex_connection, &results,
                                mempool, MEM_POOL_SIZE, &ex_async);
                        if (retstat != CS_SUCCEED)
                        {
                                ex_panic("ex_async_alloc failed");
                        }

			/*
			** Print a message to tell the user to resend the query.
			*/
			fprintf(stdout,"Please resend the query\n");
                }
                else
                {
 
#endif  /* HAFAILOVER */

		/*
		** display results
		*/
		Display(&results);

#ifdef HAFAILOVER
		}
#endif /* HAFAILOVER */

	}

	/*
	** drop async handle and free memory pool
	*/
	ex_async_free(ex_async);
	free(mempool);

	/*
	** clean up
	*/
	Cleanup(Ex_connection);

	return EX_EXIT_SUCCEED;
}

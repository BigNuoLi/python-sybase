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
** 	Example async library built on top of CT-Lib
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctpublic.h>
#include "example.h"
#include "exasync.h"

/*****************************************************************************
** 
** defines used 
** 
*****************************************************************************/

/*
** Memory alignment boundary in bytes
*/
#ifndef MEM_BOUNDRY
#define MEM_BOUNDRY		(CS_INT)8
#endif

/*
** alignment macro for memory
*/
#ifndef MEM_SIZE_ALIGN
#define MEM_SIZE_ALIGN(S)	( (((S) + MEM_BOUNDRY - 1) \
					/ MEM_BOUNDRY) * MEM_BOUNDRY )
#endif

/*
** Internal state defines used
*/
#define EX_ASEND		(CS_INT)1
#define EX_ARESULTS		(CS_INT)2
#define EX_AFETCH		(CS_INT)3
#define EX_ACANCEL_CURRENT	(CS_INT)4
#define EX_ACANCEL_ALL		(CS_INT)5

#ifdef HAFAILOVER
extern CS_INT hafailoversucceed;
#endif  /* HAFAILOVER */

/*****************************************************************************
** 
** typedefs used 
** 
*****************************************************************************/

/*
** callback handle typedef
*/
typedef CS_RETCODE	(CS_INTERNAL * EX_CB_FUNC)PROTOTYPE((
						CS_CONNECTION *connection,
						CS_COMMAND *cmd,
						CS_INT function,
						CS_RETCODE status
						));

/*
** Define the internal async handle we use
*/
typedef struct _ex_async
{
	/*
	** Used defined input area
	*/
	EX_RESULTS	*results;	/* place to keep results */
	CS_BYTE		*mempool;	/* pointer to user pmemory pool */
	CS_INT		maxpool;	/* number of bytes available in pool */
	CS_BYTE		*memcurpool;	/* current pointer in mempool */
	CS_INT		poolsize;	/* current number of bytes available */
	CS_CONNECTION	*connection;	/* connection to use */
	/*
	** Internal info
	*/
	CS_INT		state;		/* what state our library is in */
	EX_CB_FUNC	completion_cb;	/* store user callback here */
	CS_BOOL		didfetch;	/* CS_TRUE if we have already fetched
					** rows from the server
					*/
	CS_INT		res_type;	/* passed to ct_results() */
	CS_COMMAND	*cmd;		/* command handle we allocate */

} ExAsync;

/*****************************************************************************
** 
** prototypes used 
** 
*****************************************************************************/

CS_STATIC CS_VOID CS_INTERNAL ex_apanic PROTOTYPE((
	char *msg
	));
CS_STATIC CS_RETCODE CS_INTERNAL ex_adone_comp PROTOTYPE((
	ExAsync *ex_async,
	CS_CONNECTION *connection,
	CS_COMMAND *cmd,
	CS_INT function,
	CS_RETCODE status
	));
CS_STATIC CS_RETCODE CS_INTERNAL ex_aerror PROTOTYPE((
	ExAsync *ex_async,
	CS_CONNECTION *connection,
	CS_COMMAND *cmd,
	CS_INT function,
	CS_RETCODE status
	));
CS_STATIC CS_VOID * CS_INTERNAL ex_amalloc PROTOTYPE((
	ExAsync *ex_async,
	CS_INT size
	));
CS_STATIC CS_RETCODE CS_INTERNAL ex_abind PROTOTYPE((
	ExAsync *ex_async,
	CS_COMMAND *cmd
	));
CS_STATIC CS_RETCODE CS_INTERNAL ex_asend_comp PROTOTYPE((
	ExAsync *ex_async,
	CS_CONNECTION *connection,
	CS_COMMAND *cmd,
	CS_INT function,
	CS_RETCODE status
	));
CS_STATIC CS_RETCODE CS_INTERNAL ex_aresults_comp PROTOTYPE((
	ExAsync *ex_async,
	CS_CONNECTION *connection,
	CS_COMMAND *cmd,
	CS_INT function,
	CS_RETCODE status
	));
CS_STATIC CS_RETCODE CS_INTERNAL ex_afetch_comp PROTOTYPE((
	ExAsync *ex_async,
	CS_CONNECTION *connection,
	CS_COMMAND *cmd,
	CS_INT function,
	CS_RETCODE status
	));
CS_RETCODE CS_PUBLIC ex_acompletion_cb PROTOTYPE((
	CS_CONNECTION *connection,
	CS_COMMAND *cmd,
	CS_INT function,
	CS_RETCODE status
	));

/*****************************************************************************
** 
** functions
** 
*****************************************************************************/

/*
** ex_apanic()
**
** Type of function:
** internal example async lib
**
** Purpose:
** Reports a string message to stderr, and exits program.
**
** Returns:
** nothing
**
** Side Effects:
** Terminates program
**
*/

CS_STATIC CS_VOID CS_INTERNAL  
ex_apanic(char *msg)
{
	fprintf(stderr, "\n\nex_apanic: FATAL ERROR: %s\n", msg);
	fflush(stderr);
	exit(ERROR_EXIT);
}

/*
** ex_aerror()
**
** Type of function:
** internal example async lib
**
** Purpose:
** Error function which is called when ct_results or ct_fetch fails.
** It attempts to force a wakeup  of the user api callback.
** 
** Returns:
** Status passed in.
**
** Side Effects:
** None.
*/

CS_STATIC CS_RETCODE CS_INTERNAL
ex_aerror(ExAsync *ex_async, CS_CONNECTION *connection, CS_COMMAND *cmd, 
	  CS_INT function, CS_RETCODE status)
{
	if (status == CS_CANCELED)
	{
		fprintf(stderr, "\nex_aerror: client lib returned canceled\n");
	}
	else
	{
		fprintf(stderr, "\nex_aerror: client lib error encountered\n");
	}
	fprintf(stderr, "function %d, status = %d\n", function, status);
	fflush(stderr);

	/*
	** Try and force a wakeup
	*/
	(void)ex_adone_comp(ex_async, connection, cmd, function, status);

	return status;
}

/*
** ex_amalloc()
**
** Type of function:
** internal example async lib
**
** Purpose:
** returns a chunk of memory from the memory pool
**
** Returns:
** ptr to memory or NULL
** 
*/

CS_STATIC CS_VOID * CS_INTERNAL 
ex_amalloc(ExAsync *ex_async, CS_INT size)
{
	CS_VOID		*buff;

	size = MEM_SIZE_ALIGN(size);
	if (size > ex_async->poolsize)
	{
		return NULL;
	}
	buff = (CS_VOID *)ex_async->memcurpool;
	ex_async->memcurpool += size;
	ex_async->poolsize   -= size;

	return buff;
}

/*
** ex_abind()
**
** Type of function:
** internal example async lib
**
** Purpose:
** Allocated row data from the user define memory pool and bind the
** memory via ct_bind(). It uses the array binding feature of ct_lib
** to cut down the number of async operations needed.
**
** Returns:
** CS_SUCCEED	If ok.
** CS_MEM_ERROR	If the memory pool is exhausted.
** 
*/

CS_STATIC CS_RETCODE CS_INTERNAL
ex_abind(ExAsync *ex_async, CS_COMMAND *cmd)
{
	EX_RESULTS	*results = ex_async->results;
	CS_RETCODE	retstat;
	CS_INT		size;
	CS_INT		i;
	CS_INT		maxrows;	/* number of rows to fetch */

	/*
	** Determine the number of columns.
	*/
	retstat = ct_res_info(cmd, CS_NUMDATA, &results->numcols, CS_UNUSED, NULL);
	if (retstat != CS_SUCCEED)
	{
		return retstat;
	}

	/*
	** Allocate column description and data pointers from memory pool
	*/
	size = sizeof (CS_DATAFMT) * results->numcols;
	results->colfmts = (CS_DATAFMT *)ex_amalloc(ex_async, size);
	if (results->colfmts == NULL)
	{
		return CS_MEM_ERROR;
	}
	size = sizeof (CS_BYTE *) * results->numcols;
	results->data = (CS_BYTE **)ex_amalloc(ex_async, size);
	if (results->data == NULL)
	{
		return CS_MEM_ERROR;
	}
	size = sizeof (CS_INT *) * results->numcols;
	results->datalen = (CS_INT **)ex_amalloc(ex_async, size);
	if (results->datalen == NULL)
	{
		return CS_MEM_ERROR;
	}
	size = sizeof (CS_INT *) * results->numcols;
	results->indicator = (CS_INT **)ex_amalloc(ex_async, size);
	if (results->indicator == NULL)
	{
		return CS_MEM_ERROR;
	}

	/*
	** For each column, get the description, and calculation the total
	** size needed for a row. Note that the MEM_SIZE_ALIGN() is needed,
	** but does represent a worst case scenario for memory usage.
	*/
	size = 0;
	for (i = 0; i < results->numcols; i++)
	{
		/*
		** Get the column description 
		*/
		retstat = ct_describe(cmd, i + 1, &results->colfmts[i]);
		if (retstat != CS_SUCCEED)
		{
			return retstat;
		}

		size += MEM_SIZE_ALIGN(results->colfmts[i].maxlength);
		size += MEM_SIZE_ALIGN(sizeof (CS_INT));
		size += MEM_SIZE_ALIGN(sizeof (CS_SMALLINT));
	}

	/*
	** size now represents the maximum number of bytes needed to
	** process a row. Here we set maxrows to best optimize the
	** space left in the memory pool.
	*/
	maxrows = ex_async->poolsize / size;

	/*
	** Allocate space for the column data.
	*/
	for (i = 0; i < results->numcols; i++)
	{
		results->colfmts[i].count = maxrows;

		results->data[i] = (CS_BYTE *)ex_amalloc(ex_async,
					results->colfmts[i].maxlength *
					maxrows);
		if (results->data[i] == NULL)
		{
			return CS_MEM_ERROR;
		}
		results->datalen[i] = (CS_INT *)ex_amalloc(ex_async,
					sizeof (CS_INT) * maxrows);
		if (results->datalen[i] == NULL)
		{
			return CS_MEM_ERROR;
		}
		results->indicator[i] = (CS_INT *)ex_amalloc(ex_async,
					sizeof (CS_INT) * maxrows);
		if (results->indicator[i] == NULL)
		{
			return CS_MEM_ERROR;
		}

		/*
		** Bind the results to the variable.
		*/
		retstat = ct_bind(cmd, i + 1, &results->colfmts[i],
					results->data[i],
					results->datalen[i],
					(CS_SMALLINT *)results->indicator[i]);
		if (retstat != CS_SUCCEED)
		{
			return retstat;
		}
	}
	return CS_SUCCEED;
}

/*
** ex_adone_comp()
**
** Type of function:
** internal example async lib
**
** Purpose:
** Re-installs the user complete callback and calls wakeup to invoke
** the callback.
**
** Returns:
** CS_SUCCEED
**
** Side Effects:
** None.
*/

CS_STATIC CS_RETCODE CS_INTERNAL
ex_adone_comp(ExAsync *ex_async, CS_CONNECTION *connection, CS_COMMAND *cmd, 
	      CS_INT function, CS_RETCODE status)
{
	CS_RETCODE	retstat;
	CS_BOOL		pollprop;

	/*
	** Re-install user completion routine
	*/
	retstat = ct_callback(NULL, connection, CS_SET, CS_COMPLETION_CB,
					(CS_VOID *)ex_async->completion_cb);
	if (retstat != CS_SUCCEED)
	{
		return retstat;
	}

	/*
	** Enable polling
	*/
	pollprop = CS_FALSE;
	retstat = ct_con_props(connection, CS_SET, CS_DISABLE_POLL,
			  &pollprop, CS_UNUSED, NULL);
	if (retstat != CS_SUCCEED)
	{
		return retstat;
	}

	/*
	** Force a wakeup on the connection handle
	*/
	retstat = ct_wakeup(connection, NULL, EX_ASYNC_QUERY, status);
	if (retstat != CS_SUCCEED)
	{
		return retstat;
	}

	return CS_SUCCEED;
}

/*
** ex_asend_comp()
**
** Type of function:
** internal example async lib
**
** Purpose:
** This function will change the async state to results, and issue a
** ct_results()
**
** Returns:
** return of ct_results()
**
** Side Effects:
** None.
*/

CS_STATIC CS_RETCODE CS_INTERNAL
ex_asend_comp(ExAsync *ex_async, CS_CONNECTION *connection, CS_COMMAND *cmd, 
	      CS_INT function, CS_RETCODE status)
{
	/*
	** check status to see what happened in last command. If
	** an error occurred, call ex_aerror() to clean up.
	*/
	switch ((int)status)
	{
	    case CS_SUCCEED:
		break;

#ifdef HAFAILOVER
	    case CS_RET_HAFAILOVER:
		return ex_adone_comp(ex_async, connection, cmd, function, CS_SUCCEED);
#endif  /* HAFAILOVER */

	    default:
	    	return ex_aerror(ex_async, connection, cmd, function, status);
	}
	ex_async->state = EX_ARESULTS;
	return ct_results(cmd, &ex_async->res_type);
}

/*
** ex_aresults_comp()
**
** Type of function:
** internal example async lib
**
** Purpose:
** This function will process the result type from a ct_results(). If
** rows are to be fetched, the async state is updated and a ct_fetch()
** is done.
**
** Returns:
** return of ct_results()
**
** Side Effects:
** None.
*/

CS_STATIC CS_RETCODE CS_INTERNAL
ex_aresults_comp(ExAsync *ex_async, CS_CONNECTION *connection, CS_COMMAND *cmd, 
		 CS_INT function, CS_RETCODE status)
{
	CS_RETCODE	retstat;

	/*
	** check status to see what happened in last ct_results(). If
	** completed, call ex_adone_comp() to clean up.
	*/
	switch ((int)status)
	{
	    case CS_SUCCEED:
		break;

#ifdef HAFAILOVER
	    case CS_RET_HAFAILOVER:
		return ex_adone_comp(ex_async, connection, cmd, function, CS_SUCCEED);
#endif  /* HAFAILOVER */

	    case CS_END_RESULTS:
		return ex_adone_comp(ex_async, connection, cmd, function, CS_SUCCEED);

	    default:
	    	return ex_aerror(ex_async, connection, cmd, function, status);
	}

	switch ((int)ex_async->res_type)
	{
	    case CS_ROW_RESULT:
		if (ex_async->didfetch == CS_TRUE)
		{
			/*
			** We don't handle multiple result sets. Issue a
			** cancel_all. The cancel attention should not be
			** used here. Instead, it would be used in the main
			** line code to cancel this library's processing.  
			*/
			fprintf(stderr, "\nMultiple result sets not supported\n");
			fflush(stderr);
			ex_async->state = EX_ACANCEL_ALL;
			retstat = ct_cancel(NULL, cmd, CS_CANCEL_ALL);
		}
		else
		{
			/*
			** Now we are in a fetching state. If the bind fails
			** (most likely not enough memory was provided). We
			** still need to continue async processing, since
			** ex_abind() is not async. Since we can't process
			** the results, we just throw them away.
			*/
			retstat = ex_abind(ex_async, cmd);
			if (retstat == CS_SUCCEED)
			{
 				ex_async->state = EX_AFETCH;
 				retstat = ct_fetch(cmd, CS_UNUSED,
 						CS_UNUSED, CS_UNUSED,
 						&ex_async->results->numrows);
			}
 			else
 			{
 				fprintf(stderr, "\nError: ex_abind() failed\n");
				fflush(stderr);
 				ex_async->state = EX_ACANCEL_ALL;
 				retstat = ct_cancel(NULL, cmd, CS_CANCEL_ALL);
 			}
		}
		break;

	    case CS_CMD_SUCCEED:
	    case CS_CMD_DONE:
	    case CS_CMD_FAIL:
		/*
		** This means no rows were returned, remain in the result
		** state and call results again.
		*/
		ex_async->state = EX_ARESULTS;
		retstat = ct_results(cmd, &ex_async->res_type);
		break;

	    case CS_STATUS_RESULT:
		/*
		** throw the status value away.
		*/
		ex_async->state = EX_ACANCEL_CURRENT;
		retstat = ct_cancel(NULL, cmd, CS_CANCEL_CURRENT);
		break;

	    case CS_PARAM_RESULT:
	    case CS_COMPUTE_RESULT:
	    default:
		fprintf(stderr, "\nUnexpected result type encountered\n");
		fflush(stderr);
		ex_async->state = EX_ACANCEL_ALL;
		retstat = ct_cancel(NULL, cmd, CS_CANCEL_ALL);
		break;
	}

	return retstat;
}

/*
** ex_afetch_comp()
**
** Type of function:
** internal example async lib
**
** Purpose:
** This function will determine if all the rows have been fetched. If so,
** the async state is switched backed to results, and a call to 
** ct_results() is made.
**
** Returns:
** return of ct_results() or
** return of ct_fetch()
**
** Side Effects:
** None.
*/

CS_STATIC CS_RETCODE CS_INTERNAL
ex_afetch_comp(ExAsync *ex_async, CS_CONNECTION *connection, CS_COMMAND *cmd, 
	       CS_INT function, CS_RETCODE status)
{
	CS_RETCODE	retstat;

	/*
	** check status to see what happened in last ct_fetch(). If
	** completed, call ct_results() to continue processing.
	*/
	switch ((int)status)
	{
	    case CS_SUCCEED:
		break;

#ifdef HAFAILOVER
	    case CS_RET_HAFAILOVER:
		return ex_adone_comp(ex_async, connection, cmd, function, CS_SUCCEED);
#endif  /* HAFAILOVER */

	    case CS_END_DATA:
		ex_async->state    = EX_ARESULTS;
		ex_async->didfetch = CS_TRUE;
		return ct_results(cmd, &ex_async->res_type);

	    default:
	    	return ex_aerror(ex_async, connection, cmd, function, status);
	}

	/*
	** The fetch was supposed to be a one shot deal. Here we clear the
	** binds and call fetch again to return CS_END_DATA. Note that this
	** is not the best way to handle this. Multiple fetchs may cause
	** recursion on the process stack. The code was left the way it is to
	** demonstrate that multiple fetchs are possible.
	**
	** The best way to handle this would be to use:
	**
	** ex_async->state = EX_ACANCEL_CURRENT;
	** return ct_cancel(NULL, cmd, CS_CANCEL_CURRENT);
	*/
	retstat = ct_bind(cmd, CS_UNUSED, NULL, NULL, NULL, NULL);
	if (retstat != CS_SUCCEED)
	{
		return retstat;
	}
	return ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, NULL);
}

/*
** ex_acompletion_cb()
**
** Type of function:
** internal example async lib
**
** Purpose:
** Installed as a callback into Open Client. It will dispatch to the
** appropriate completion processing routine based on async state.
** 
** Another approach to callback processing is to have each completion
** routine install the completion callback for the next step in
** processing. We use one dispatch point to aid in debugging the
** async processing (only need to set one breakpoint).
**
** Returns:
** return of completion processing routine.
**
** Side Effects:
** None
*/

CS_RETCODE CS_PUBLIC
ex_acompletion_cb(CS_CONNECTION *connection, CS_COMMAND *cmd, CS_INT function, CS_RETCODE status)
{
	CS_RETCODE	retstat;
	ExAsync		*ex_async;
#ifdef HAFAILOVER
	CS_INT          hafail;
#endif  /* HAFAILOVER */

	fprintf(stdout, "\nex_acompletion_cb: function %d Completed",
				function);
	fflush(stdout);

#ifdef HAFAILOVER
	/*
	**  If HA failover has succeeded then it has to go back to the
	**  connected state. Need to send a query now
	*/
	ct_con_props(connection, CS_GET, CS_HAFAILOVER,
		&hafail, CS_UNUSED, NULL);

        if((hafail) && (status == CS_RET_HAFAILOVER))
        {
                hafailoversucceed = CS_TRUE;

                return(CS_HAFAILOVER);
        }
        if ((hafail) && (status == CS_SUCCEED) && function == CT_CLOSE)
        {
                return(CS_SUCCEED);

        }
#endif  /* HAFAILOVER */

	/*
	** Extract the user area out of the command handle.
	*/
	retstat = ct_cmd_props(cmd, CS_GET, CS_USERDATA, &ex_async,
					CS_SIZEOF(ex_async), NULL);
	if (retstat != CS_SUCCEED)
	{
		return retstat;
	}

	/*
	** Based on async state, do the right thing
	*/
	switch ((int)ex_async->state)
	{
	    case EX_ASEND:
	    case EX_ACANCEL_CURRENT:
		retstat = ex_asend_comp(ex_async, connection, cmd,
				function, status);
		break;

	    case EX_ARESULTS:
		retstat = ex_aresults_comp(ex_async, connection, cmd,
				function, status);
		break;

	    case EX_AFETCH:
		retstat = ex_afetch_comp(ex_async, connection, cmd,
				function, status);
		break;

	    case EX_ACANCEL_ALL:
		retstat = ex_adone_comp(ex_async, connection, cmd,
				function, status);
		break;

	    default:
		ex_apanic("ex_acompletion_cb: unexpected async state");
		break;
	}

	return retstat;
}

/*
** ex_async_query()
**
** Type of function:
** example async lib api
**
** Purpose:
** This routine will asynchronously send a query to a server and
** retrieve any rows sent back.
**
** Returns:
** CS_PENDING
**
** Side Effects:
** Will install a different async completion callback.
**
*/

CS_RETCODE CS_PUBLIC
ex_async_query(EX_ASYNC *ex_handle, CS_CHAR *query, CS_INT querylen)
{
	ExAsync		*ex_async;
	CS_RETCODE	retstat;
	CS_BOOL		pollprop;

	/*
	** set up async handle for query
	*/
	ex_async = (ExAsync *)ex_handle;
	ex_async->poolsize   = ex_async->maxpool;
	ex_async->memcurpool = ex_async->mempool;
	ex_async->didfetch   = CS_FALSE;

	/*
	** clear results area
	*/
	ex_async->results->numrows   = 0;
	ex_async->results->numcols   = 0;
	ex_async->results->colfmts   = NULL;
	ex_async->results->data      = NULL;
	ex_async->results->datalen   = NULL;
	ex_async->results->indicator = NULL;

	/*
	** Extract the user completion callback for this connection
	*/
	retstat = ct_callback(NULL, ex_async->connection, CS_GET,
					CS_COMPLETION_CB,
					(CS_VOID *)&ex_async->completion_cb);
	if (retstat != CS_SUCCEED)
	{
		return retstat;
	}

	/*
	** Install our completion callback for this connection. Since this
	** will overwrite the user installed callback, all subsequent error
	** processing should make sure to call ex_aerror() to recover from
	** the error.
	*/
	retstat = ct_callback(NULL, ex_async->connection, CS_SET,
					CS_COMPLETION_CB,
					(CS_VOID *)ex_acompletion_cb);
	if (retstat != CS_SUCCEED)
	{
		return retstat;
	}

	/*
	** Disable polling. We do this to insure that the ct_poll() routine
	** will not report a completion for this connection until we call the
	** user installed callback.
	*/
	pollprop = CS_TRUE;
	retstat = ct_con_props(ex_async->connection, CS_SET, CS_DISABLE_POLL,
			  &pollprop, CS_UNUSED, NULL);
	if (retstat != CS_SUCCEED)
	{
	    	return ex_aerror(ex_async, ex_async->connection, ex_async->cmd,
				 EX_ASYNC_QUERY, retstat);
	}

	/*
	** Install the exhandle into the user area of the command handle.
	*/
	retstat = ct_cmd_props(ex_async->cmd, CS_SET, CS_USERDATA, &ex_async,
					CS_SIZEOF(ex_async), NULL);
	if (retstat != CS_SUCCEED)
	{
	    	return ex_aerror(ex_async, ex_async->connection, ex_async->cmd,
				 EX_ASYNC_QUERY, retstat);
	}

	/*
	** Tell Open Client about the query we want to send.
	*/
	retstat = ct_command(ex_async->cmd, CS_LANG_CMD, query,
					querylen, CS_UNUSED);
	if (retstat != CS_SUCCEED)
	{
	    	return ex_aerror(ex_async, ex_async->connection, ex_async->cmd,
				 EX_ASYNC_QUERY, retstat);
	}

	/*
	** The async manager can be debugged by the following api. This is
	** mostly useful for developers who are working with the internal async
	** routines.
	*/
#if EX_ASYNC_DEBUG
{
	CS_CONTEXT	*context;
	ct_con_props(ex_async->connection, CS_GET, CS_PARENT_HANDLE,
		&context, CS_UNUSED, NULL);
	ct_debug(context, NULL, CS_SET_FLAG, CS_DBG_ASYNC, NULL, CS_UNUSED);
}
#endif

	/*
	** Now send the query to the server.
	*/
	ex_async->state = EX_ASEND;
	retstat = ct_send(ex_async->cmd);
	if (retstat != CS_PENDING)
	{
	    	return ex_aerror(ex_async, ex_async->connection, ex_async->cmd,
				 EX_ASYNC_QUERY, retstat);
	}
	return retstat;
}

/*
** ex_async_alloc()
**
** Type of function:
** example async lib api
**
** Purpose:
** This routine will allocate and initialize a async handle
**
** Returns:
** CS_SUCCEED
**
** Side Effects:
** None.
*/

CS_RETCODE CS_PUBLIC
ex_async_alloc(CS_CONNECTION *connection, EX_RESULTS *results, CS_VOID *mempool, 
	       CS_INT maxpool, EX_ASYNC **ex_handle)
{
	ExAsync		*ex_async;
	CS_RETCODE	retstat;

	/*
	** allocate and initialize the async data area to store async
	** information.
	*/
	ex_async = (ExAsync *)malloc(sizeof (ExAsync));
	if (ex_async == NULL)
	{
		return CS_MEM_ERROR;
	}
	ex_async->results    = results;
	ex_async->mempool    = (CS_BYTE *)mempool;
	ex_async->maxpool    = maxpool;
	ex_async->connection = connection;

	/*
	** get a command handle.
	*/
	retstat = ct_cmd_alloc(connection, &ex_async->cmd);
	if (retstat != CS_SUCCEED)
	{
		return retstat;
	}

	*ex_handle = (CS_VOID *)ex_async;

	return CS_SUCCEED;
}

/*
** ex_async_free()
**
** Type of function:
** example async lib api
**
** Purpose:
** Free the async handle passed in.
**
** Returns:
** CS_PENDING
**
** Side Effects:
** None.
*/

CS_RETCODE CS_PUBLIC
ex_async_free(EX_ASYNC *ex_handle)
{
	ExAsync		*ex_async;
	CS_RETCODE	retstat;

	/*
	** free allocate and initialize the async data area to store async
	** information.
	*/
	ex_async = (ExAsync *)ex_handle;

	/*
	** drop the command handle.
	*/
	retstat = ct_cmd_drop(ex_async->cmd);
	if (retstat != CS_SUCCEED)
	{
		return retstat;
	}

	/*
	** Free the async handle
	*/
	memset(ex_async, 0, sizeof (*ex_async));
	free(ex_async);

	return CS_SUCCEED;
}

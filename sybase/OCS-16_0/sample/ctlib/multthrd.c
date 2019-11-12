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
**	This example demonstrates using Client library from multiple 
**	application threads. It spawns 5 threads.  Each thread processes 
**	a cursor or a regular query. The application main thread waits for 
**	the other threads to complete query processing and terminate. 
**	
**	This sample follows a one-thread, one-connection model. In this model,
**	the program performs all needed library initialization and cleanup
**	in single threaded initialization and cleanup code. In this program,
**	it is performed in application main thread. It creates a dedicated
**	thread for each connection and limits all use of a particular
**	connection to its dedicated thread. This model is the simplest
**	and requires the least amount of inter-thread synchronization.
**
**      The basic steps followed in this sample are,	
**
**	o	All thread-unsafe library/context initialization calls
**		are made from the application main thread. 
**	 
** 	o	After the initialization has been performed, the main thread 
**		spawns one thread for each connection to be created. The 
**		thread then allocates its own connection with ct_con_alloc, 
**		connects to the server, and performs processing for that
**		connection. 
**
**	o	After spawning the threads, the application main thread waits 
**		for the threads to terminate. When the threads complete 
**		processing the query, each thread closes its own connection 
**		and terminates itself. Before terminating, it also signals the 
**	 	main thread. The application main thread then performs the 
**		necessary cleanup in the process limiting thread unsafe calls 
**		to a single thread, itself. 
**
**	Synchronization/serialization, and thread/connection specific data:
**
**	o	Thread number is stored as user data in the connection/context
**		associated with that thread. This is useful since it
**		may be retrieved in callbacks or other places where it
**		may be needed to notify the user about the connection 
**		that resulted in a error message. An alternative would be to
**		use thread-specific data primitives provided by the OS thread  
**		layer. However, this alternative cannot be used when
**		callbacks are called in the context of a client library internal
**		thread (this happens when async property is set to
**		full async mode).  
**
**	o	A semaphore is used to notify the application main thread
**		about termination of the query threads.
**
**	o	A global mutex is used to protect usage of a shared resource
**		- printing to stdout/stderr - This is really not essential
**		for this sample to work. It is merely used to avoid output 
**		from different threads from being garbled.
**		  
**
**	Cursor query thread: 
**	This thread demonstrates using a read-only cursor.  It opens a cursor
**	with a canned query.  It processes the results using the standard
**	ct_results() while loop.  It binds the column values to program
**	variables.  It then fetches and displays the rows in the standard
**	ct_fetch() while loop.                                          
**
**	Regular query thread:
**	This demonstrates processing a regular query. It sends a canned
**	query to the server and processes the results using the standard
**	ct_results() while loop. It binds the column values to program
**	variables using array binding. It then fetches and displays the
**	result rows in the standard ct_fetch() while loop.
**
**	Canned queries:
**		select au_fname, au_lname, postalcode from authors
**		select * from sysobjects
**		select * from stores, titles
**
** Routines Used
** -------------
**	cs_ctx_alloc()
**	ct_init()
**	ct_config()
**	ct_callback()
**	ct_con_alloc()
**	ct_con_props()
**	ct_connect()
**	ct_cmd_alloc()
**	ct_cursor(CS_CURSOR_DECLARE)
**	ct_cursor(CS_CURSOR_ROWS)
**	ct_cursor(CS_CURSOR_OPEN)
**	ct_cursor(CS_CURSOR_CLOSE)
**	ct_send()
**	ct_results()
**	ct_res_info()
**	ct_describe()
**	ct_bind()
**	ct_fetch()
**	ct_cmd_drop()
**	ct_close()
**	ct_con_drop()
**	ct_exit()
**	ct_ctx_drop()
**
** Input
** -----
**	This example uses hard-coded queries of the authors, stores and titles
**	tables in the pubs2 database. One of the query uses the server
**	system table sysobjects. 
**
** Output
** ------
**	This example simply displays each row of the result set. Output
**	from different threads will be interleaved. Each line of output
**	specifies the thread from which it was generated. 
**
** Server Dependencies
** -------------------
**	This example requires ASE version 10.0 or higher.
**
** Server Tables
** -------------
**	This example relies on the pubs2 database - authors, stores and
**	titles tables and the server system table sysobjects.
**
** Other Requirements
** ------------------
**	This sample requires a thread package to be available for the build
**	and execution. Not all platforms support a thread package. Please read 
**	the readme file associated with the samples and the platform supplement 
**	for Open Client for information on the thread packages that are 
**	supported. If no thread package is available on a platform, this
**	sample is not supported. 
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctpublic.h>
#include "example.h"
#include "thrdutil.h"
#include "thrdfunc.h"

/*****************************************************************************
** 
** defines and globals used.
** 
*****************************************************************************/

/*
** Define  sample select statements
*/
#define QUERY1	"select au_fname, au_lname, postalcode from authors"
#define QUERY2	"select * from sysobjects"
#define QUERY3	"select * from stores, titles"

#define PUBS2_DB "pubs2"
#define MASTER_DB "master"

#define EX_MAX_THREADS 5

/*
** Global names used in this module
*/
CS_CHAR *Ex_appname  = "Multithreaded_sample";
CS_CHAR *Ex_server   = EX_SERVER;
CS_CHAR *Ex_username = EX_USERNAME;
CS_CHAR *Ex_password = EX_PASSWORD;

/* Other globals */

/* task list */
CS_STATIC EX_TASK Ex_tasks[] =
{
	{0, NULL, NULL, QUERY1, PUBS2_DB, EX_CURSOR_QUERY}, 
	{0, NULL, NULL, QUERY2, MASTER_DB, EX_REGULAR_QUERY},  
	{0, NULL, NULL, QUERY1, PUBS2_DB, EX_CURSOR_QUERY},
	{0, NULL, NULL, QUERY2, MASTER_DB,  EX_CURSOR_QUERY}, 
	{0, NULL, NULL, QUERY3, PUBS2_DB, EX_REGULAR_QUERY}, 
};
 
/*
** Semaphore for synchronizing thread completions
*/
CS_STATIC CS_VOID *Ex_threadcompsem;


/*
** Prototypes for routines 
*/
CS_STATIC CS_RETCODE DoCursor PROTOTYPE((
	CS_CONNECTION *connection,
	EX_TASK *task
	));
CS_STATIC CS_RETCODE DoQuery PROTOTYPE((
	CS_CONNECTION *connection,
	EX_TASK *task
	));
CS_STATIC CS_RETCODE SendCmdAndCheckResults PROTOTYPE((
	CS_INT		threadnum,
	CS_COMMAND 	*cmd,
	CS_INT	   	restype
	));

CS_RETCODE thread_main PROTOTYPE((
	EX_TASK	*task
	));

/*
** main()
** 
** Purpose:
**	Entry point for the multithreaded example program. 
**	It does the following:
**		Sets up a global mutex mechanism.
**		Creates a synchronization semaphore for reporting thread
**			completions.
**		Allocates and initializes a cs context structure.
**		Sets up the thread task structures and spawns the threads
**			to perform these tasks.
**		Waits for the threads to terminate.
**		Performs the cleanup of all the resources.
** 
** Parameters:
**	--show-connected-address
**
** Return:
** 	EX_EXIT_ERROR  or EX_EXIT_SUCCEED
**
*/
#define	ARG_PARAM	"--show-connected-address"

int
main(int argc, char *argv[])
{
	CS_CONTEXT	*context;
	CS_RETCODE	retcode;
	CS_INT		threadnum;
	CS_INT		i;
	CS_INT		threadcount;

	EX_SCREEN_INIT();

	threadnum = 0;
	threadcount = 0;
	
	fprintf(stdout, "Multithreaded Example\n");
	fflush(stdout);

	if (argc == 2)
	{
		/*
		** Check for argument.
		*/
		if (strncmp(argv[1], ARG_PARAM, strlen(ARG_PARAM)) == 0)
		{
			display_server_address = CS_TRUE;
		}
	}

	/*
	** Initialize global mutex mechanism
	*/
	if (ex_create_global_mutex() != CS_SUCCEED)
	{
		/* panic */
		fprintf(stdout, "ex_init_global_mutex() failed\n");
		exit(EX_EXIT_FAIL);
	}

	/*
	** Create a semaphore to be used later to
	** synchronize completion of threads 
	*/
	retcode = ex_create_sem(&Ex_threadcompsem, EX_MAX_THREADS);
	if (retcode != CS_SUCCEED)
	{
		ex_panic(threadnum, "ex_create_sem failed");
	}

	/* 
	** Allocate a context structure and initialize Client-Library 
	*/
	retcode = ex_init(threadnum, &context);
	if (retcode != CS_SUCCEED)
	{
		(void)ex_delete_sem(Ex_threadcompsem);
		ex_panic(threadnum, "ex_init failed");
	}

	/* set context */
	for (i = 0; i < EX_MAX_THREADS; i++)
	{ 
		Ex_tasks[i].context = context; 
		Ex_tasks[i].threadnum = i + 1;
	}

	/*
	** Spawn threads to do the tasks 
	*/
	for (i = 0; i < EX_MAX_THREADS; i++)
	{ 
		if (ex_create_thread(&(Ex_tasks[i].thread), 
					(CS_VOID *)thread_main, 
					&Ex_tasks[i]) != CS_SUCCEED)
		{
			ex_error(threadnum, "ex_create_thread failed");
			break;
		} 
		threadcount++;
	}

	/*
	** Wait for the threads to complete
	*/
	while (threadcount > 0)
	{
		if (ex_waitfor_sem(Ex_threadcompsem) != CS_SUCCEED)
		{
			ex_panic(threadnum, "ex_waitfor_sem failed");
		}
		threadcount--;
	}

	/* cleanup */
	for (i = 0; i < EX_MAX_THREADS; i++)
	{ 
		(void)ex_detach_thread(Ex_tasks[i].thread);
	}

	if (context != NULL)
	{
		retcode = ex_ctx_cleanup(context, retcode);
	}

	(void)ex_delete_sem(Ex_threadcompsem);
	(void)ex_delete_global_mutex();

	return (retcode == CS_SUCCEED) ? EX_EXIT_SUCCEED : EX_EXIT_FAIL;
}


/*
** thread_main(task)
**
** Type of function:
**	Multithreaded sample - thread main routine.
**
** Purpose:
**	This is the thread main routine. When a thread is spawned 
**	with this as start routine, it creates a connection; performs the
**	operation specified in the task entry and cleans up the
**	connection before terminating itself.
**
** Parameters:
**      task - pointer to EX_TASK entry with sql query and db name
**
** Return:
**	CS_SUCCEED - if the task specified completed successfully
**	Otherwise a Client-Library failure code.
*/

CS_RETCODE
thread_main(EX_TASK *task)
{
        CS_CONNECTION   *connection;
        CS_RETCODE      retcode;

        /*
        ** Allocate a connection structure, set its properties, and
        ** establish a connection.
        */
        retcode = ex_connect(task->threadnum, task->context, &connection, 
				Ex_appname, Ex_username, Ex_password, 
				Ex_server);
 
        /*
        ** Execute the routine for processing the query 
        */
        if (retcode == CS_SUCCEED)
        {
		switch (task->operation)
		{
		 	case EX_CURSOR_QUERY:
                		retcode = DoCursor(connection, task);
				break;
			case EX_REGULAR_QUERY:
				retcode = DoQuery(connection, task);
				break;
			default:
				/* ignore */
				break;
		}
        }
 
        /*
        ** Deallocate the allocated structures, close the connection,
        ** and exit Client-Library.
        */
        if (connection != NULL)
        {
                retcode = ex_con_cleanup(connection, retcode);
        }

	/* signal waiter */
	(void)ex_release_sem(Ex_threadcompsem); 

	/* Exit the thread */
	return(retcode);
}

/*
** DoCursor(connection, task)
**
** Type of function:
** 	sample program internal api
**
** Purpose:
**	This function is the core of the cursor example.  It declares
**	and opens a cursor.  It then processes the results in the 
**	standard ct_results() loop, calling ex_fetch_data()	
**	when the type of result is CS_CURSOR_RESULT.
**
** Parameters:
**	connection - Pointer to connection structure
**	task - pointer to EX_TASK entry with sql query and db name
**
** Return:
**	CS_SUCCEED if cursor result set was processed correctly
**	Otherwise a Client-Library failure code.
**
*/

CS_STATIC CS_RETCODE
DoCursor(CS_CONNECTION *connection, EX_TASK *task)
{
	CS_RETCODE	retcode;
	CS_COMMAND	*cmd;	
	CS_CHAR		msg[512];
        CS_INT  	threadnum = 0;

        /*
        ** Get Connection specific data - get thread number from the
        ** user data stored in connection structure
        */
        (void)ex_get_threadnum(NULL, connection, &threadnum);

	/*
	** NOTE: Add capability check here
	*/

	/*
	** Use the specified database
	*/
        if ((retcode = ex_use_db(connection, task->dbname)) != CS_SUCCEED)
        {
		sprintf(msg, "DoCursor: ex_use_db(%s) failed", task->dbname);
                ex_error(threadnum, msg); 
                return retcode;
        }

	/*
	** Allocate a command handle to declare the cursor on.
	*/
        if ((retcode = ct_cmd_alloc(connection, &cmd)) != CS_SUCCEED)
        {
                ex_error(threadnum, "DoCursor: ct_cmd_alloc() failed");
                return retcode;
        }

	/*
	** Declare the cursor
	*/
	retcode = ct_cursor(cmd, CS_CURSOR_DECLARE, "cursor_a", CS_NULLTERM,
			task->query, CS_NULLTERM, CS_READ_ONLY);
	if (retcode != CS_SUCCEED)
	{
                ex_error(threadnum, "DoCursor: ct_cursor(declare) failed");
		return retcode;
	}
	
	/*
	** Set cursor rows to 10
	*/
	retcode = ct_cursor(cmd, CS_CURSOR_ROWS, NULL, CS_UNUSED, NULL, 
			CS_UNUSED, (CS_INT)10);
	if (retcode != CS_SUCCEED)
	{
                ex_error(threadnum, "DoCursor: ct_cursor(currows) failed");
		return retcode;
	}

	/*
	** Open the cursor.  
	*/
	retcode = ct_cursor(cmd, CS_CURSOR_OPEN, NULL, CS_UNUSED, NULL,
			CS_UNUSED, CS_UNUSED);
	if (retcode != CS_SUCCEED)
	{
                ex_error(threadnum, "DoCursor: ct_cursor() failed");
		return retcode;
	}

	/*
	** Send (batch) the last 3 cursor commands to the server, 
	** and check results. It will generate cursor result set.
	*/
	if ((retcode = SendCmdAndCheckResults(threadnum, cmd, 
				CS_CURSOR_RESULT)) != CS_SUCCEED)
	{
		/* error already reported */
		return retcode;
	}

	/*
	** Close and deallocate the cursor. Note that we don't have to do
	** this, since it is done automatically when the connection is
	** closed.
	*/
	retcode = ct_cursor(cmd, CS_CURSOR_CLOSE, NULL, CS_UNUSED, NULL,
			CS_UNUSED, CS_DEALLOC);
	if (retcode != CS_SUCCEED)
	{
                ex_error(threadnum, "DoCursor: ct_cursor(dealloc) failed");
		return retcode;
	}

	/*
	** Send the cursor command to the server and check its results. 
	** The command won't generate a result set.
	*/
	if ((retcode = SendCmdAndCheckResults(threadnum, cmd, CS_CMD_SUCCEED)) 
			!= CS_SUCCEED)
	{
		/* error already reported */
		return retcode;
	}

	/*
	** Drop the cursor's command structure
	*/
	if ((retcode = ct_cmd_drop(cmd)) != CS_SUCCEED)
	{
                ex_error(threadnum, "DoCursor: ct_cmd_drop() failed");
                return retcode;
	}

	return retcode;
}

/*
** DoQuery(connection, task)
**
** Type of function:
** 	sample program internal api
**
** Purpose:
**	Synchronously sends a query to server and processes results 
**	in a standard ct_results loop, calling ex_fetch_data()	
**	when the type of result is CS_ROW_RESULT.
**
** Parameters:
**	connection - Pointer to connection structure
**	task - pointer to EX_TASK entry with sql query and db name
**
** Return:
**	CS_SUCCEED if regular row result set was processed correctly
**	Otherwise a Client-Library failure code.
**
*/

CS_STATIC CS_RETCODE
DoQuery(CS_CONNECTION *connection, EX_TASK *task)
{
	CS_RETCODE	retcode;
	CS_COMMAND	*cmd;	
	CS_CHAR		msg[512];
        CS_INT  	threadnum = 0;

        /*
        ** Get Connection specific data - get thread number from the
        ** user data stored in connection structure
        */
        (void)ex_get_threadnum(NULL, connection, &threadnum);

	/*
	** Use the specified database
	*/
        if ((retcode = ex_use_db(connection, task->dbname)) != CS_SUCCEED)
        {
		sprintf(msg, "DoQuery: ex_use_db(%s) failed", task->dbname);
                ex_error(threadnum, msg); 
                return retcode;
        }

	/*
	** Allocate a command handle to send the query 
	*/
        if ((retcode = ct_cmd_alloc(connection, &cmd)) != CS_SUCCEED)
        {
                ex_error(threadnum, "DoQuery: ct_cmd_alloc() failed");
                return retcode;
        }

	/*
	** Initiate the command with the specified query 
	*/
	retcode = ct_command(cmd, CS_LANG_CMD, task->query, 
				CS_NULLTERM, CS_UNUSED);
	if (retcode != CS_SUCCEED)
	{
                sprintf(msg, "DoQuery: ct_command(CS_LANG_CMD:%s) failed", 
			task->query);
                ex_error(threadnum, msg);
		return retcode;
	}

	/*
	** Send the command to the server and check its results. 
	** It will generate regular row result set.
	*/
	if ((retcode = SendCmdAndCheckResults(threadnum, cmd, CS_ROW_RESULT)) 
			!= CS_SUCCEED)
	{
		/* error already reported */
		return retcode;
	}

        /*
        ** Drop our command structure
	*/
	if ((retcode = ct_cmd_drop(cmd)) != CS_SUCCEED)
	{
                ex_error(threadnum, "DoQuery: ct_cmd_drop() failed");
                return retcode;
	}

	return retcode;
}

/*
** SendCmdAndCheckResults(thrdnum, cmd, restype)
**
** Type of function:
**      sample program internal api
**
** Purpose:
**      It sends a command to server and processes the results in the 
**	standard ct_results() loop, calling ex_fetch_data() when the 
**	type of result is CS_CURSOR_RESULT or CS_ROW_RESULT.
**
** Parameters:
**	threadnum - current thread number (logical)
**	cmd - pointer to command structure 
**	restype - restype expected
**
** Return:
**      CS_SUCCEED if result set was processed correctly
**      Otherwise a Client-Library failure code.
**
*/
 
CS_STATIC CS_RETCODE
SendCmdAndCheckResults(CS_INT threadnum, CS_COMMAND *cmd, CS_INT restype)
{
	CS_INT res_type;
	CS_RETCODE retcode;

	/*
	** Send the command to the server 
	*/
	if ((retcode = ct_send(cmd)) != CS_SUCCEED)
	{
                ex_error(threadnum, "SendCmdAndCheckResults: ct_send() failed");
		return retcode;
	}

	/*
	** Process the results.  Loop while ct_results() returns CS_SUCCEED.
	*/
	while((retcode = ct_results(cmd, &res_type)) == CS_SUCCEED)
	{
		switch ((int)res_type)
		{
		case CS_CMD_SUCCEED:
			/*
			** This means no rows were returned.  
			*/
			break;

		case CS_CMD_DONE:
			/*
			** This means we're done with one result set.
			*/
   			break;

		case CS_CMD_FAIL:
			/*
			** This means that the server encountered an error while
			** processing our command.
			*/
			ex_error(threadnum, 
		"SendCmdAndCheckResults: ct_results() returned CMD_FAIL");
			break;

		case CS_CURSOR_RESULT:
		case CS_ROW_RESULT:
			/* Did we get expected result type? */
			if (restype == res_type)
			{
				retcode = ex_fetch_data(cmd);
				if (retcode != CS_SUCCEED)
				{
					ex_error(threadnum, 
			"SendCmdAndCheckResults: ex_fetch_data() failed");
					return retcode;
				}
				break;
			}
			/* otherwise fall through to default */ 
		default:
			/*
			** We got an unexpected result type.
			*/
			ex_error(threadnum,
	"SendCmdAndCheckResults: ct_results() returned unexpected result type");
			return CS_FAIL;
		}
	}

	/*
	** We're done processing results. Let's check the
	** return value of ct_results() to see if everything
	** went ok.
	*/
	switch ((int)retcode)
	{
		case CS_END_RESULTS:
			/*
			** Everything went fine.
			*/
			break;

		case CS_FAIL:
			/*
			** Something terrible happened.
			*/
			ex_error(threadnum, 
				"SendCmdAndCheckResults: ct_results() failed");
			return retcode;

		default:
			/*
			** We got an unexpected return value.
			*/
			ex_error(threadnum, 
	"SendCmdAndCheckResults: ct_results() returned unexpected result code");
			return retcode;
	}

	/* return success */
	return CS_SUCCEED;
}  


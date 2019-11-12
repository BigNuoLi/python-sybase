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
** 	ex06ct.c is converted from DB-Library's example6.c to use 
**	Client-Library.
**
**    	The DB-Library browse-mode code has been converted to use
**    	a Client-Library cursor.
**
** 	This example illustrates opening a data file, inserting data
** 	from the file into a newly created 'alltypes' table. The table
** 	contains several ASE datatypes, including an 'age', 'name', 
** 	and 'manager' field. The program traverses the table rows and 
** 	increments the age column in each row. The DB-Library version 
** 	updated the table using browse-mode techniques. This Client-
** 	Library version uses a Client-Library cursor to do the same thing.
**
** 	This program also includes a small sideshow to illustrate
** 	how a Client-Library cursor allows more than one active command on a
** 	single connection. While fetching the Client-Library cursor
** 	rows, the program selects the names of people who work for the person
** 	named in the current cursor row. The code for is not included
** 	in the compiled program unless the macro SIDESHOW is defined
** 	on the compiler command line.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*
-- #include <sybfront.h>
-- #include <sybdb.h>
-- #include "sybdbex.h"
*/
#include <ctpublic.h>
#include "dbtoctex.h"

/*
** We use the routines in exutils.c to initialize Client-Library,
** install error handlers, open the connection, and perform 
** connection- and context-level cleanup. The handlers
** for server messages and Client-Library errors are defined
** in exutils.c
**
** For an example that shows conversion of these DB-Library
** operations, look at ctfirst.c
**
*/
#include <exutils.h>

#define BUFLEN 2048
#define NAMELEN 32
#define LINE_SEP \
"-----------------------------------------------------------------------"

/*
** Local prototypes -- Some of the code that was in main() for the
** DB-Library function has been abstracted to seperate functions.
*/
CS_RETCODE ShowTable PROTOTYPE ((
	CS_CONNECTION *conn,
	CS_CHAR       *select_stmt
	));

CS_RETCODE CreateAlltypes PROTOTYPE ((
	CS_CONNECTION *conn,
	char          *fname
	));

CS_RETCODE RunCursor PROTOTYPE ((
	CS_CONNECTION *conn
	));

CS_RETCODE DoCursorRows PROTOTYPE ((
	CS_CONNECTION *conn,
	CS_COMMAND *cmd
	));

CS_RETCODE CleanCursor  PROTOTYPE ((
	CS_COMMAND *cmd
	));

CS_RETCODE HandleResults PROTOTYPE ((
	CS_COMMAND *cmd
	));

/*
** Define a macro for simple return code checking within
** a function. ERR_CH is defined in dbtoctex.h
*/
#define FAIL_ON_FAIL(ret, str) \
  { if (ret != CS_SUCCEED) \
  { \
   fprintf(ERR_CH, "Error (line %d): %s\n", (int)__LINE__, str); \
   return CS_FAIL; \
  } }

int
main(int argc, char *argv[])
{
	/*
	-- LOGINREC         *login;
	-- DBPROCESS        *q_dbproc;   * This DBPROCESS will be used to
	--                               * query the database.
	--                               *
	-- DBPROCESS        *u_dbproc;   * This DBPROCESS will be used to
	--                               * simultaneously update the database.
	--                               * 
	-- char             *qualptr;    * This will point to the WHERE clause
	--                               * appropriate for updating q_dbproc's
	--                               * current data row.
	--                               * 
	-- RETCODE          return_code;
	-- DBTINYINT        age;
	*/
	CS_CONTEXT	*context;
	CS_CONNECTION	*conn;
	CS_RETCODE	ret;
	char		sqlbuf[BUFLEN];	/* renamed from 'cmdbuf' */

	/*
	** Setup screen output.
	*/
	EX_SCREEN_INIT();

	/*
	--  * Initialize DB-Library. * 
	-- if (dbinit() == FAIL)
	-- {
	--   exit(ERREXIT);
	-- }
	--
	--  * Install the user-supplied error-handling and message-handling
	--  * routines. They are defined at the bottom of this source file.
	--  * 
	-- dberrhandle((EHANDLEFUNC)err_handler);
	-- dbmsghandle((MHANDLEFUNC)msg_handler);
	*/

	/*
	** Initialize CS-Library and Client-Library. We do this with
	** ex_init() from exutils.c
	** 
	** ex_init() allocates a context structure, initializes Client-Library,
	** and installs callbacks for the error and message handlers.
	*/
	ret = ex_init(&context);
	EXIT_ON_FAIL(context, ret, "Client-Library setup failed!");

	/*
	--  * Allocate and initialize the LOGINREC structure to be used
	--  * to open a connection to ASE.
	--  * 
	--
	-- login = dblogin();
	-- DBSETLUSER(login, USER);
	-- DBSETLPWD(login, PASSWORD);
	-- DBSETLAPP(login, "example6");
	--
	-- q_dbproc = dbopen(login, NULL);
	-- u_dbproc = dbopen(login, NULL);
	*/

	/*
	** Connect to the server. Unlike browse mode updates, we only need one
	** connection to do cursor updates.
	**
	** ex_connect() is defined exutils.c
	*/
	ret = ex_connect(context, &conn, "ex03ct", USER, PASSWORD, NULL);
	EXIT_ON_FAIL(context, ret, "Connection attempt failed.");

	/*
	** Use the 'pubs2' database. The DB-Library program did not do this,
	** but it's good practice not to rely on the user's default database
	** choice in your code.
	*/
	ret = ex_use_db(conn, "pubs2");
	EXIT_ON_FAIL(context, ret, "'USE pubs2' failed.");

	/*
	** Create and populate the 'alltypes' table. CreateAlltypes() is
	** defined in this source file.
	*/
	ret = CreateAlltypes(conn, "datafile");
	EXIT_ON_FAIL(context, ret, "CreateAlltypes() failed.");

	/*
	** The DB-Library example6 used two connnections and
	** browse-mode to increment the age of every person in
	** the table.
	**
	** This program uses a Client-Library cursor and 
	** ct_cursor(CS_CURSOR_UPDATE) calls to do the same thing over
	** a single connection.
	**
	** This code has been abstracted as the RunCursor() function
	** defined in this source file.
	*/

	printf("Updating rows in the 'alltypes' table...\n");
	
	/*
	** The SIDESHOW macro controls conditional compilation of
	** code that demonstrates additional cursor functionality.
	** This is additional processing that was not done in the
	** DB-Library program.
	*/

#ifdef SIDESHOW
	printf("And will also show employee relationships ...\n");
#endif /* SIDESHOW */

	ret = RunCursor(conn);
	EXIT_ON_FAIL(context, ret, "RunCursor() failed.");

	/*
	** Now, we'll look at the updated contents of the table, to
	** verify that the ages were properly incremented.
	**
	** The code to do the SELECT has been abstracted as the
	** function ShowTable(), defined in this source file.
	*/
	
	printf("\nSelecting rows from the 'alltypes' table:\n");

	/*
	** Ports of this example were tested by comparing output. The
	** timestamp data differs for every run, so don't select if 
	** testing a port.
	*/
#if defined(TESTING_PORT)
	strcpy(sqlbuf, 
	       "select age, userid, royalty, name, title_id, us_citizen, \
	        account, title, manager from alltypes");
#else
	strcpy(sqlbuf, "select * from alltypes"); 
#endif
	ret = ShowTable(conn, sqlbuf);
	EXIT_ON_FAIL(context, ret, "ShowTable() failed.");

	/*
	** This program hasn't really shown a reasonable use of cursor
	** or browse-mode updates. Just to prove a point, the following 
	** command will undo all the cursor updates with a single SQL 
	** UPDATE statement. ex_execute_cmd() is in exutils.c.
	*/
	strcpy(sqlbuf, "update alltypes set age = age - 1");
	ret = ex_execute_cmd(conn, sqlbuf);
	EXIT_ON_FAIL(context, ret, "'UPDATE alltypes ...' failed.");

	/*
	-- dbexit();
	*/

	/*
	** Cleanup. ex_con_cleanup() and ex_ctx_cleanup() are defined
	** in exutils.c
	*/
	ret = ex_con_cleanup(conn, CS_SUCCEED);
	ret = ex_ctx_cleanup(context, ret);

	exit(NORMAL_EXIT);
	
} /* main () */

/*
** RunCursor() -- Using a Client-Library cursor, go through the rows of 
**    the 'alltypes' table and increment the age in each.
**
**    Optionally (controlled by whether SIDESHOW is defined at compile
**    time), show the manager/employee relation in the 'alltypes'
**    table.
**
** Parameters:
**   conn -- The connection to the ASE.
**
** Returns:
**   CS_SUCCEED for success; CS_FAIL if an error occurred.
*/
CS_RETCODE
RunCursor(CS_CONNECTION *conn)
{
	CS_COMMAND	*cmd;
	CS_RETCODE	ret;
	CS_RETCODE	results_ret;
	CS_INT		result_type;
	CS_CHAR		sqlbuf[BUFLEN];

	/*
	** Allocate a command structure. 
	*/
	ret = ct_cmd_alloc(conn, &cmd);
	FAIL_ON_FAIL(ret, "RunCursor: ct_cmd_alloc() failed.\n");

	/*
	** Here is the DB-Library code that did the browse mode update.
	** This code was in main() in the DB-Library example6 program.
	**
	--  * Using DB-Library's browse-mode facilities, we'll increment
	--  * the age of every person in the table.
	--  * 
	--
	-- printf("Updating rows in the 'alltypes' table...\n");
	--
	-- dbcmd(q_dbproc,"select * from alltypes for browse");
	-- dbsqlexec(q_dbproc);
	--
	-- while ((return_code = dbresults(q_dbproc)) != NO_MORE_RESULTS)
	-- {
	--   if (return_code == SUCCEED)
	--   {
	--     while (dbnextrow(q_dbproc) != NO_MORE_ROWS)
	--     {
	--       age = *((DBTINYINT *)(dbdata(q_dbproc, 1)));
	--       qualptr = dbqual(q_dbproc, -1, "alltypes");
	--       if (qualptr == NULL)
	--       {
	--         printf("dbqual() failed.  Exiting.\n");
	--         dbexit();
	--         exit(ERREXIT);
	--       }
	--       dbcmd(u_dbproc, "update alltypes");
	--       dbfcmd
	--         (u_dbproc, " set age = %d %s", age+1, qualptr);
	--       dbsqlexec(u_dbproc);
	--       dbresults(u_dbproc);
	--       dbfreequal(qualptr);
	--     }
	--   }
	-- }
	*/

	/*
	** These are the steps used here to declare and open a 
	** cursor. Not all options are illustrated here (see the
	** Client-Library reference manual for the others):
	**  1. Format the select statement for the cursor body.
	**  2. Call ct_cursor(CS_CURSOR_DECLARE) to build the
	**     cursor-declare command.
	**  3. Call ct_cursor(CS_CURSOR_OPEN) to build the 
	**     cursor-open command.
	**  4. Call ct_send() to send both the cursor-declare and
	**     the cursor-open commands. Client-Library allows
	**     "batching" of these commands.
	**  5. Process the results returned by the cursor. The
	**     rows returned by the cursor have a CS_CURSOR_RESULT
	**     type.
	*/

	/*
	** Step 1:
	** Build the SELECT for the cursor. This statement is called the 
	** cursor body, and it determines the row set for the cursor.
	**
	** You should specify whether the cursor is read-only or
	** if certain columns can be updated with a FOR READ ONLY or
	** FOR UPDATE OF clause. You can omit this clause and specify
	** updateability in the ct_cursor() call. In general, with 
	** updateable cursors it's better to use FOR UPDATE OF and
	** only list the columns that you intend to update. This 
	** prevents inadvertent updates of primary-key columns.
	**
	** There is one snag when the table has a timestamp column --
	** since timestamps are automatically updated, we must include
	** the timestamp in the selected columns and in the columns for
	** update.
	*/
	strcpy(sqlbuf, 
	       "select userid, name, age, timestamp from alltypes \
	       for update of age, timestamp");

	/*
	** Step 2:
	** Call ct_cursor to declare the cursor. The user must give a name 
	** when declaring a cursor (3rd parameter). We name our cursor "fred".
	** 
	** The last parameter in a cursor-declare call can
	** be CS_FOR_UPDATE, CS_READ_ONLY, or CS_UNUSED. CS_FOR_UPDATE
	** and CS_READ_ONLY are useful when the cursor body lacks a FOR
	** UPDATE or FOR READ ONLY clause. We pass CS_UNUSED, since
	** we specified FOR UPDATE in the cursor body.
	*/
	ret = ct_cursor(cmd, CS_CURSOR_DECLARE, "Fred", strlen("Fred"),
			sqlbuf, strlen(sqlbuf), CS_UNUSED);
	if (ret != CS_SUCCEED)
	{
		fprintf(ERR_CH,
			"RunCursor: build cursor-declare command failed.\n");
		(CS_VOID) ct_cmd_drop(cmd);
		return CS_FAIL;
	}

	/*
	** Step 3:
	** Call ct_cursor(CS_CURSOR_OPEN) to open the cursor.
	*/
	ret = ct_cursor(cmd, CS_CURSOR_OPEN, NULL, CS_UNUSED,
			NULL, CS_UNUSED, CS_UNUSED);
	if (ret != CS_SUCCEED)
	{
		fprintf(ERR_CH,
			"RunCursor: build cursor-open command failed.\n");
		(CS_VOID) ct_cmd_drop(cmd);
		return CS_FAIL;
	}

	/*
	** Step 4:
	** Send the "batched" cursor-declare and cursor-open commands.
	*/
	ret = ct_send(cmd);
	if (ret != CS_SUCCEED)
	{
		fprintf(ERR_CH, "RunCursor: ct_send() failed.\n");
		(CS_VOID) ct_cmd_drop(cmd);
		return CS_FAIL;
	}

	/*
	** Step 5:
	** Process the results returned by the cursor. The
	** rows returned by the cursor have a CS_CURSOR_RESULT
	** type.
	*/
	while ((results_ret = ct_results(cmd, &result_type)) == CS_SUCCEED)
	{
		switch ((int) result_type)
		{
		  case CS_CURSOR_RESULT:

			/* 
			** This will be the cursor row set.
			*/
			ret = DoCursorRows(conn, cmd); 
			if (ret != CS_SUCCEED)
			{
				/*
				** Clean up the best we can and fail.
				** We don't check retcodes here because
				** we are already failing. CleanCursor()
				** closes and deallocates the cursor.
				*/
				fprintf(ERR_CH,
					"RunCursor: DoCursorRows() failed.\n");
				ret = CleanCursor(cmd);
				FAIL_ON_FAIL(ret,
					     "RunCursor: CleanCursor() failed.");
				(CS_VOID) ct_cmd_drop(cmd);
				return CS_FAIL;
			}
			break;

		  case CS_CMD_SUCCEED:
		  case CS_CMD_DONE:

			/*
			** No action is required.
			*/
			break;

		  case CS_CMD_FAIL:

			/*
			** Command failed on the server.
			**
			** Clean up the best we can and fail. We don't check 
			** retcodes here because we are already failing.
			*/
			fprintf(ERR_CH,
				"RunCursor: Command failed on server.\n");
			ret = CleanCursor(cmd);
			FAIL_ON_FAIL(ret,
				     "RunCursor: CleanCursor() failed.");
			(CS_VOID) ct_cmd_drop(cmd);
			return CS_FAIL;

			default:

			/*
			** Unexpected result type from the server.
			**
			** Clean up the best we can and fail. We don't check 
			** retcodes here because we are already failing.
			*/
			fprintf(ERR_CH,
				"RunCursor: Received unexpected type of result.\n");
			ret = CleanCursor(cmd);
			FAIL_ON_FAIL(ret, "RunCursor: CleanCursor() failed.");
			(CS_VOID) ct_cmd_drop(cmd);
			return CS_FAIL;
	    
		} /* switch */
	} /* while */

	/*
	** Now check for errors in the return code from ct_results().
	*/
	switch((int) results_ret)
	{
	  case CS_END_RESULTS: 
		/* No error. */
		break;
	  case CS_CANCELED:
		/* No error. */
		break;
	  case CS_FAIL:
		/* Network or other client-side error. */
		FAIL_ON_FAIL(CS_FAIL, "RunCursor: ct_results() failed.");
	  default:
		/* Unexpected ct_results() return code. */
		FAIL_ON_FAIL(CS_FAIL,
			     "RunCursor: unknown ct_results() return code.");
	}

	/*
	** Now close and deallocate the cursor.
	*/
	ret = CleanCursor(cmd);
	FAIL_ON_FAIL(ret, "RunCursor: CleanCursor() failed");

	/*
	** Clean up and return.
	*/
	ret = ct_cmd_drop(cmd);
	FAIL_ON_FAIL(ret, "RunCursor: ct_cmd_drop() failed.\n");

	return CS_SUCCEED;
	
} /* RunCursor() */

/*
** DoCursorRows -- Process the cursor rows returned by the cursor
**    with body 
**      "select userid, name, age from alltypes for update of age, timestamp"
**
**    This function fetches each row and increments the age with a nested 
**    cursor-update command. Optionally (controlled by whether SIDESHOW
**    is defined at compile time), the employees who work for the employee
**    in each row will be queried and displayed using another command
**    structure.
**
** Parameters:
**    conn -- The connection to the server.
**    cmd -- CS_COMMAND with the cursor open and the rows ready to
**           be fetched.
**
** Returns:
**    CS_SUCCEED for success; CS_FAIL if an error occurred.
**    After failure, the command structure may have results pending.
*/
CS_RETCODE
DoCursorRows(CS_CONNECTION *conn, CS_COMMAND *cmd)
{
	CS_RETCODE	ret;
	CS_RETCODE	fetch_ret;
	CS_DATAFMT	datafmt;
	CS_TINYINT	age;
	CS_SMALLINT	age_indicator;
	CS_CHAR		name[NAMELEN];
	CS_SMALLINT	name_indicator;
	CS_CHAR		sqlbuf[BUFLEN];

	/*
	** We're going to bind to the 'name' columns and 'age' columns
	** only.
	*/

	/*
	** Get the description of the 2nd column (name) and verify it's
	** character and that it fits into our name[] array.
	*/
	ret = ct_describe(cmd, 2, &datafmt);
	FAIL_ON_FAIL(ret, "DoCursorRows: ct_describe(2) failed.");

	if (datafmt.datatype != CS_CHAR_TYPE && datafmt.maxlength >= NAMELEN)
	{
		FAIL_ON_FAIL(CS_FAIL,
			     "DoCursorRows: column 2 has bad datatype.\n");
	}

	/*
	** Now bind to the name in column 2. We also bind an indicator 
	** so that we know when the value is null.
	*/
	datafmt.maxlength = NAMELEN;
	datafmt.format = CS_FMT_NULLTERM;
	ret = ct_bind(cmd, 2, &datafmt, (CS_VOID *)name, NULL, &name_indicator);
	FAIL_ON_FAIL(ret, "DoCursorRows: bind to name failed.\n");

	/*
	** Get the description of the 3rd column (age) and verify it's
	** a tiny integer.
	*/
	ret = ct_describe(cmd, 3, &datafmt);
	FAIL_ON_FAIL(ret, "DoCursorRows: ct_describe(3) failed.");

	if (datafmt.datatype != CS_TINYINT_TYPE)
	{
		FAIL_ON_FAIL(CS_FAIL,
			     "DoCursorRows: column 3 has bad datatype.\n");
	}

	/*
	** Now bind to the age in column 3. We also bind an indicator 
	** so that we know when the value is null.
	*/
	ret = ct_bind(cmd, 3, &datafmt, (CS_VOID *)&age, NULL, &age_indicator);
	FAIL_ON_FAIL(ret, "DoCursorRows: bind to age failed.\n");

	/*
	** Client-Library cursors are useful in two situations:
	** 
	** (1) When fetching cursor results, we can send cursor-update
	**     or cursor-delete commands using the same command structure.
	**
	** (2) Using a different command structure on the same connection, we 
	**     can send new commands while processing a cursor result set. If 
	**     you try this in any other situation (or in DB-Library), you will 
	**     get "results pending" errors.
	** 
	** To illustrate (1), we check age_indicator for each fetched row.
	** If the age is not null, we increment the age column by sending
	** a cursor-update command.
	** 
	** Item (2) is illustrated only if SIDESHOW is defined at compile time.
	** For the sideshow, we check name_indicator for each fetched
	** row. If the name is not null, we compose a SELECT statement and
	** call ShowTable() to select the employees managed by the employee
	** in the current row. ShowTable() will allocate a new command structure
	** and use it to send the SELECT.
	*/
	while ((fetch_ret = ct_fetch(cmd,CS_UNUSED,CS_UNUSED,CS_UNUSED,NULL))
	       == CS_SUCCEED)
	{

		/*
		** (1) Cursor update commands.
		**
		** Check for a null age value. We can't increment a null
		** value: the server will complain if we try.
		** 0 indicator values indicate good data.
		*/
		if (age_indicator == 0)
		{
			/*
			** Format the update statement.
			*/
			strcpy(sqlbuf, "update alltypes set age = age + 1");

			/*
			** Build the cursor-update command.
			*/
			ret = ct_cursor(cmd, CS_CURSOR_UPDATE, "alltypes",
					strlen("alltypes"),
			sqlbuf, strlen(sqlbuf), CS_UNUSED);
			FAIL_ON_FAIL(ret,
				     "DoCursorRows: Build cursor-update failed.\n");

			/* Send it. */
			ret = ct_send(cmd);
			FAIL_ON_FAIL(ret,
				     "DoCursorRows: Send cursor-update failed.\n");

			/* Flush the results and check for errors. */
			ret = HandleResults(cmd);
			FAIL_ON_FAIL(ret,
				     "DoCursorRows: Cursor update command failed.");

		} /* if (age_indicator ... */

		/*
		** (2) Simultaneous commands using the same connection.
		**
		** If name is not null, then print the names of employees who
		** work for the person represented by this cursor row.
		** We compose a SELECT from the name value, then use
		** ShowTable() to send the command and print the names. 
		**
		** Note that we can't initiate a new command on the same command
		** structure used by the cursor, but we can do it on a different
		** command structure.
		** 
		** ShowTable() allocates a new command structure to send and 
		** process the SELECT. Both command structures use the same
		** connection to the server.
		**
		** Note that this code has no equivelent in the DB-Library
		** example6. "SIDESHOW" must be defined at compile-time
		** to get this output. If it is not defined, the processing 
		** of the two programs is equivalent.
		*/
#ifdef SIDESHOW
		if (name_indicator == 0)
		{
			printf("\n%s\nThese people work for %s\n%s\n", 
			LINE_SEP, name, LINE_SEP);
			sprintf(sqlbuf,
				"select name from alltypes where manager = '%s'",
				name);
			ret = ShowTable(conn, sqlbuf);
			if (ret != CS_SUCCEED)
			{
				FAIL_ON_FAIL(ret,
					     "DoCursorRows: Select employees failed.");
			}

		} 
#endif /* SIDESHOW */

	} /* while ((fetch_ret ... */

	/*
	** Check for ct_fetch errors.
	*/
	if (fetch_ret != CS_END_DATA)
	{
		FAIL_ON_FAIL(CS_FAIL, "DoCursorRows: ct_fetch() failed.");
	}

	return CS_SUCCEED;

} /* DoCursorRows() */

/*
** ShowTable() -- Run a SELECT statement
**   and print the rows.
**
** Parameters:
**   conn -- the connection to the ASE
**   sqlbuf -- the select statement to run
**
** Returns:
**   CS_SUCCEED for success, CS_FAIL if an error occurred.
*/
CS_RETCODE
ShowTable(CS_CONNECTION *conn, CS_CHAR *sqlbuf)
{
	CS_COMMAND	*cmd;
	CS_RETCODE	ret;
	CS_RETCODE	results_ret;
	CS_INT		result_type;

	/*
	** Here is the DB-Library code that did the same thing.
	**
	-- dbcmd(q_dbproc, "select * from alltypes");
	-- dbsqlexec(q_dbproc);
	-- dbresults(q_dbproc);
	-- dbprrow(q_dbproc);
	**
	** The Client-Library code is below. 
	*/

	/*
	** Allocate a command structure.  As an alternative, we could have
	** coded this function to take a CS_COMMAND as an argument.
	*/
	ret = ct_cmd_alloc(conn, &cmd);
	FAIL_ON_FAIL(ret, "ShowTable: ct_cmd_alloc() failed.\n");

	/*
	** Build and send the SELECT language command.
	*/
	ret = ct_command(cmd, CS_LANG_CMD, sqlbuf, strlen(sqlbuf), CS_UNUSED);
	if (ret != CS_SUCCEED)
	{
		fprintf(ERR_CH, "ShowTable: ct_cmd_alloc() failed.\n");
		(CS_VOID) ct_cmd_drop(cmd);
		return CS_FAIL;
	}
	ret = ct_send(cmd);
	if (ret != CS_SUCCEED)
	{
		fprintf(ERR_CH, "ShowTable: ct_send() failed.\n");
		(CS_VOID) ct_cmd_drop(cmd);
		return CS_FAIL;
	}
	
	/*
	** Process the results. We will use ex_fetch_data() from
	** exutils.c to print out the regular rows. ex_fetch_data()
	** is similar to DB-Library's dbprrow.
	*/
	while ((results_ret = ct_results(cmd, &result_type)) == CS_SUCCEED)
	{
		switch ((int) result_type)
		{
		  case CS_ROW_RESULT:
			/* 
			** This will be the row data returned by the select.
			** Client-Library provides no equivalent for dbprrow(),
			** but the exutils.c routine demonstrates how a user
			** function can be written to do the same thing as
			** dbprrow().
			** ex_fetch_data() prints a header containing the
			** column names and the row data.
			*/
			ret = ex_fetch_data(cmd);
			if (ret != CS_SUCCEED)
			{
				/*
				** Clean up the best we can and fail. We don't
				** check retcodes here because we are already
				** failing.
				*/
				fprintf(ERR_CH,
					"ShowTable: Fetch data failed.\n");
				(CS_VOID) ct_cancel(NULL, cmd, CS_CANCEL_ALL);
				(CS_VOID) ct_cmd_drop(cmd);
				return CS_FAIL;
			}
			break;

		  case CS_CMD_SUCCEED:
		  case CS_CMD_DONE:
			/*
			** CS_CMD_SUCCEED means a command ran successfully.
			** CS_CMD_DONE means we've seen all the results from
			** one 'logical cmd' -- that is, a SQL statement in
			** the language command batch, or a SELECT executed
			** by a stored procedure.
			**
			** No action is required.
			*/
			break;

		  case CS_CMD_FAIL:
			/*
			** Command failed on the server.
			**
			** Clean up the best we can and fail. We don't check 
			** retcodes here because we are already failing.
			*/
			FAIL_ON_FAIL(CS_FAIL,
				     "ShowTable: Command failed on server.\n");

		  default:
			/*
			** Unexpected result type the server.
			**
			** Clean up the best we can and fail. We don't check 
			** retcodes here because we are already failing.
			*/
			fprintf(ERR_CH,
				"ShowTable: Received unexpected type of result.\n");
			(CS_VOID) ct_cancel(NULL, cmd, CS_CANCEL_ALL);
			(CS_VOID) ct_cmd_drop(cmd);
			return CS_FAIL;

		}
	}   

	/*
	** Now check for errors in the return code from ct_results.
	*/
	switch((int) results_ret)
	{
	  case CS_END_RESULTS: 
		/* No error. */
		break;
	  case CS_CANCELED:
		/* No error. */
		break;
	  case CS_FAIL:
		/* Network or other client-side error. */
		FAIL_ON_FAIL(CS_FAIL, "ShowTable: ct_results() failed.");
	  default:
		/* Unexpected ct_results() return code. */
		FAIL_ON_FAIL(CS_FAIL, "ShowTable: unknown ct_results() status");
	}

	/*
	** Clean up and return.
	*/
	ret = ct_cmd_drop(cmd);
	FAIL_ON_FAIL(ret, "ShowTable: ct_cmd_drop() failed.\n");

	return CS_SUCCEED;

} /* ShowTable() */

/*
** CreateAlltypes() -- Create and populate the 'alltypes' table used in
**    this example.
**
** Parameters:
**    conn -- An open connection to the server. 
**    fname -- The null-terminated name of a file containing data to 
**        populate the alltypes table.
**
** Returns:
**    CS_SUCCEED for success, CS_FAIL for failure.
*/
CS_RETCODE
CreateAlltypes(CS_CONNECTION *conn, char *fname)
{
	CS_RETCODE	ret;
	CS_CHAR		sqlbuf[BUFLEN];		/* renamed from 'cmdbuf' */
	CS_CHAR		inputbuf[BUFLEN];
	FILE		*infile;

	printf("Creating the 'alltypes' table.\n");
	
	/*
	** Here is the DB-Library code to create the table. This code
	** was in main() in the DB-Library example6 program.
	**
	--  * Create a table that contains several ASE data types. * 
	-- dbcmd(q_dbproc,"create table alltypes ");
	-- dbcmd(q_dbproc,"(age tinyint,");
	-- dbcmd(q_dbproc,"userid smallint,");
	-- dbcmd(q_dbproc,"royalty int,");
	-- dbcmd(q_dbproc,"name char(25),");
	-- dbcmd(q_dbproc,"title_id varbinary(20),"); 
	-- dbcmd(q_dbproc,"us_citizen bit,");
	-- dbcmd(q_dbproc,"account float,");
	-- dbcmd(q_dbproc,"title varchar(20),");
	-- dbcmd(q_dbproc,"manager char(25),");
	-- dbcmd(q_dbproc,"timestamp)");
	-- dbcmd(q_dbproc, "create unique index index1 on alltypes(userid)");
	-- dbsqlexec(q_dbproc);
	-- while (dbresults(q_dbproc) != NO_MORE_RESULTS)
	-- {
	--   continue;
	-- }
	*/

	/*
	** Unlike the DB-Library program, we're going to drop the
	** table if it already exists. We use ex_execute_cmd() from
	** exutils.c to send the language command. ex_execute_cmd()
	** handles any language command that does not return fetchable
	** data.
	*/
	strcpy(sqlbuf, "if exists (select name from sysobjects ");
	strcat(sqlbuf, "where name = 'alltypes' and type = 'U' ");
	strcat(sqlbuf, "and uid = user_id()) ");
	strcat(sqlbuf, "begin drop table alltypes end");
	ret = ex_execute_cmd(conn, sqlbuf);
	FAIL_ON_FAIL(ret, "CreateAlltypes: 'IF ... DROP TABLE' failed.\n");

	strcpy(sqlbuf, "create table alltypes ");
	strcat(sqlbuf, "(age tinyint,");
	strcat(sqlbuf, "userid smallint,");
	strcat(sqlbuf, "royalty int,");
	strcat(sqlbuf, "name char(25),");
	strcat(sqlbuf, "title_id varbinary(20),"); 
	strcat(sqlbuf, "us_citizen bit,");
	strcat(sqlbuf, "account float,");
	strcat(sqlbuf, "title varchar(20),");
	strcat(sqlbuf, "manager char(25),");
	strcat(sqlbuf, "timestamp) ");
	strcat(sqlbuf, "create unique index index1 on alltypes(userid)");
	
	ret = ex_execute_cmd(conn, sqlbuf);
	FAIL_ON_FAIL(ret, "CreateAlltypes: 'CREATE TABLE' failed.\n");

	/*
	** Insert rows of data into the newly created table "alltypes".
	** We will read in the contents of the file and form an
	** INSERT statement.
	*/
	  
	if ((infile=fopen(fname,"r")) == NULL)
	{
		fprintf(ERR_CH, "Unable to open file '%s'.\n", fname);
		return CS_FAIL;
	}

	printf("Inserting rows into the 'alltypes' table...\n");

	/*
	** Here is the DB-Library code that inserted the rows.
	** This code was in main() in the DB-Library example6
	** program.
	**
	-- while ((fgets(cmdbuf,BUFLEN,infile)) != NULL)
	-- {
	--   dbfcmd(q_dbproc,"insert into alltypes \n");
	--   dbfcmd(q_dbproc,"values(%s, null) \n",cmdbuf);
	-- }
	-- 
	-- dbsqlexec(q_dbproc);
	-- 
	--  * Process the results of each of the INSERT statements. * 
	--
	-- while ((return_code = dbresults(q_dbproc)) != NO_MORE_RESULTS)
	-- {
	--   if (return_code == FAIL)
	--   {
	--     printf("One of the insert statements FAILed.\n");
	--   }
	-- }
	*/

	while (fgets(inputbuf,BUFLEN,infile) != NULL)
	{
		/*
		** This code is slightly different than the code in the
		** example6 DB-Library program. That code built a
		** monolithic language batch containing all the 
		** INSERT statements, then sent them all as one language
		** command (with one call to dbsqlexec).
		**
		** We send each insert individually. If we did it the
		** other way we might run out of space in our sqlbuf[]
		** array.
		*/

		sprintf(sqlbuf, "%s %s %s",
			"insert into alltypes \n values(",
			inputbuf,
			", null) \n");
		ret = ex_execute_cmd(conn, sqlbuf);
		if (ret != CS_SUCCEED)
		{
			fprintf(ERR_CH, "CreateAlltypes: Insert failed.\n");
			fclose(infile);
			return CS_FAIL;
		}
	}

	fclose(infile);

	return CS_SUCCEED;

} /* CreateAlltypes() */


/*
** CleanCursor() -- Close and deallocate a Client-Library cursor.
**
**   If no cursor is declared on the command structure, this function
**   does nothing. If a cursor is open, it is closed and deallcated.
**   If a cursor is declared but not open, it is deallocated.
**
** PARAMETERS
**   cmd -- an allocated CS_COMMAND structure.
**
** RETURNS
**  CS_SUCCEED -- Success: An open cursor was closed and deallocated,
**                a closed or declared cursor was deallocated, or there
**                was no cursor on the command structure.
**  CS_FAIL -- An error occurred.
**
** NOTE
**  If a cursor exists on the command structure, CleanCursor() will 
**  flush all pending results from the command structure. Take care
**  when calling this function from within a results loop -- ct_results()
**  fails if there are no results left to process.
*/
CS_RETCODE
CleanCursor(CS_COMMAND *cmd)
{
	CS_RETCODE	ret;
	CS_INT		cur_status;

	/*
	** Get the cursor status property from the command structure.
	*/
	ret = ct_cmd_props(cmd, CS_GET, CS_CUR_STATUS, &cur_status, 
			   CS_UNUSED, (CS_INT *) NULL);
	FAIL_ON_FAIL(ret, "clean_cursor: ct_cmd_props(CUR_STATUS)");

	/*
	** Is there an open cursor? If so, issue a command to both
	** close and deallocate it. 
	*/
	if ((cur_status & CS_CURSTAT_OPEN) == CS_CURSTAT_OPEN)
	{
		ret = ct_cursor(cmd, CS_CURSOR_CLOSE, (CS_CHAR *)NULL,
				CS_UNUSED, (CS_CHAR *)NULL, CS_UNUSED,
				CS_DEALLOC);
		FAIL_ON_FAIL(ret,
			     "CleanCursor: ct_cursor(CS_CURSOR_CLOSE) failed");
	      
		ret = ct_send(cmd);
		FAIL_ON_FAIL(ret, "CleanCursor: ct_send() CLOSE failed");
		ret = HandleResults(cmd);
		FAIL_ON_FAIL(ret, "CleanCursor: HandleResults() CLOSE failed");
	}

	/*
	** Is there a declared or closed cursor? If so, issue a command
	** to deallocate it.
	*/
	else if ((cur_status & CS_CURSTAT_CLOSED) == CS_CURSTAT_CLOSED
	         || (cur_status & CS_CURSTAT_DECLARED) == CS_CURSTAT_DECLARED)
	{
		ret = ct_cursor(cmd, CS_CURSOR_DEALLOC, (CS_CHAR *)NULL,
				CS_UNUSED, (CS_CHAR *)NULL, CS_UNUSED,
				CS_UNUSED);
		FAIL_ON_FAIL(ret,
			     "CleanCursor: ct_cursor(CS_CURSOR_DEALLOC) failed");
		ret = ct_send(cmd);
		FAIL_ON_FAIL(ret, "CleanCursor: ct_send() for DEALLOC failed");
		ret = HandleResults(cmd);
		FAIL_ON_FAIL(ret,
			     "CleanCursor: HandleResults() for DEALLOC failed");
	}

	return CS_SUCCEED;
} /* CleanCursor() */

/*
** HandleResults() -- Handle results of commands that return
**   no fetchable results.
**
** Parameters
**   cmd -- A CS_COMMAND structure ready to return results.
**   
** Returns
**  CS_SUCCEED -- Results handled. No unexpected results.
**  CS_FAIL -- Error occurred or the command returned 
**    fetchable results. 
*/
CS_RETCODE
HandleResults(CS_COMMAND *cmd)
{
	CS_RETCODE	results_ret;
	CS_INT		result_type;
	
	/* 
	** Flush the results and check for errors. 
	*/
	while ((results_ret = ct_results(cmd, &result_type)) == CS_SUCCEED)
	{
		switch((int) result_type)
		{
		  case  CS_CMD_FAIL:
			FAIL_ON_FAIL(CS_FAIL,
				     "HandleResults: Command failed on server.");
			/*
			** break never reached: FAIL_ON_FAIL jumps out.
			*/

		  case CS_CMD_DONE:
		  case CS_CMD_SUCCEED:
			/*
			** All went fine.
			*/
			break;

		  case  CS_CURSOR_RESULT:
			/*
			** We're currently not interested in the results:
			** just skip them.
			*/
			break;

		  default: 
			FAIL_ON_FAIL(CS_FAIL,
				     "HandleResults: Unexpected results.");
			/*
			** break never reached: FAIL_ON_FAIL jumps out.
			*/
		} /* switch */
	} /* while */

	/*
	** Now check for errors in the return code from ct_results().
	*/
	switch((int) results_ret)
	{
	  case CS_END_RESULTS: 
		/* No error. */
		break;
	  case CS_CANCELED:
		/* No error. */
		break;
	  case CS_FAIL:
		/* Network or other client-side error. */
		FAIL_ON_FAIL(CS_FAIL, "HandleResults: ct_results() failed.");
		/*
		      ** break never reached: FAIL_ON_FAIL jumps out.
		*/
	  default:
		/* Unexpected ct_results() return code. */
		FAIL_ON_FAIL(CS_FAIL, "RunCursor: unknown ct_results() status");
		/*
		** break never reached: FAIL_ON_FAIL jumps out.
		*/
	}

	return CS_SUCCEED;

} /* HandleResults() */

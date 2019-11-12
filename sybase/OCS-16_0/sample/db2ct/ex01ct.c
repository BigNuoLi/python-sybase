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
**  	ex01ct.c is converted from DB-Library's example1.c to use 
**	Client-Library.
**
**  	This example illustrates how to send two queries to
**  	ASE in a command batch.  It binds each set
**  	of results and prints the rows.
** 
**  	This example requires that the 'pubs2' database exist and that
**  	the user has select permission on the 'sysobjects' table.
**
**  	Where DB-Library code has been replaced with Client-Library
**  	code, the DB-Library calls are commented out and preceded
**  	with '--' at the beginning of each line.
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

#define DATELEN     26
#define TYPELEN      2

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
-- * Forward declarations of the error handler and message handler. *
-- int             err_handler();
-- int             msg_handler();
*/
#include <exutils.h>

/*
** trim() is a simple function that trims trailing blanks from
** a null-terminated character string.
*/
CS_CHAR *trim PROTOTYPE ((
	CS_CHAR *s
	));

int
main(int argc, char *argv[])
{
	/*
	-- DBPROCESS     *dbproc;        * Our connection with ASE. * 
	-- LOGINREC      *login;         * Our login information. * 
	-- RETCODE        result_code;
	*/
	CS_CONTEXT     *context; 
	CS_CONNECTION  *conn;
	CS_COMMAND     *cmd;
	CS_RETCODE     ret;
	CS_RETCODE     results_ret;
	CS_RETCODE     fetch_ret;
	CS_INT         result_type;
	CS_CHAR        sql_string[1024];

	/*
	--  * These are the variables used to store the returning data. * 
	--
	-- DBCHAR         crdate[DATELEN+1];
	-- DBINT          id;
	-- DBCHAR         name[DBMAXNAME+1];    * DBMAXNAME is defined in
	--                                     * "sybdb.h" as the maximum
	--                                     * length for names of database
	--                                     * objects, such as tables,
	--                                     * columns, and procedures.
	--                                     * 
	-- DBCHAR         type[TYPELEN+1];
	*/
	CS_CHAR         crdate[DATELEN+1];
	CS_INT          id;
	CS_CHAR         name[CS_MAX_NAME + 1];
	CS_CHAR         type[TYPELEN+1];
	CS_DATAFMT      char_datafmt;
	CS_DATAFMT      int_datafmt;
	CS_INT          cur_cmdnum;
	CS_INT          cur_rownum;
	CS_INT          intval;

	/*
	** Setup screen output.
	*/
	EX_SCREEN_INIT();

	/*
	--  * Initialize DB-Library. * 
	-- if (dbinit() == FAIL)
	-- {
	-- 	 exit(ERREXIT);
	-- }
	--
	--  * Install the user-supplied error-handling and message-handling
	--  * routines. They are defined at the bottom of this source file.
	--  * 
	-- dberrhandle(err_handler);
	-- dbmsghandle(msg_handler);
	*/

	/*
	** ex_init() is defined in exutils.c
	** It allocates the CS_CONTEXT, initializes Client-Library, and installs
	** Client-Library and server message handlers.
	*/
	ret = ex_init(&context);
	EXIT_ON_FAIL(context, ret, "Initialization failed.");

	/*
	--  *
	-- ** Get a LOGINREC structure and fill it with the necessary
	-- ** login information.
	-- * 
	-- login = dblogin();
	-- DBSETLUSER(login, USER);
	-- DBSETLPWD(login, PASSWORD);
	-- DBSETLAPP(login, "example1");
	--
	--  *
	-- ** Get a DBPROCESS structure for communicating with ASE.
	-- ** A NULL servername defaults to the server specified by DSQUERY.
	-- * 
	-- dbproc = dbopen(login, NULL);
	*/

	/*
	** Allocate a connection structure, set the login credentials
	** (username, password) and call ct_connect. ex_connect does
	** this for us. ex_connect() is defined in exutils.c
	**
	** As with the DB-Library example, the default server name is taken
	** as the contents of the DSQUERY environment variable.
	*/
	ret = ex_connect(context, &conn, "ex01ct", USER, PASSWORD, NULL);
	EXIT_ON_FAIL(context, ret, "Could not connect to server.");  

	/*
	** Change to the 'pubs2' database.
	** This step is extra -- DB-Library's example1.c just played in the
	** user's default database.
	**
	** ex_use_db() is defined in exutils.c. It sends a USE language 
	** and handles the results.
	*/
	ret = ex_use_db(conn, "pubs2");
	EXIT_ON_FAIL(context, ret, "'USE pubs2' command failed");

	/*
	** Allocate a command structure to send commands and receive results
	** with.
	*/
	ret = ct_cmd_alloc(conn, &cmd);
	EXIT_ON_FAIL(context, ret, "Command structure alloc failed.");

	/*
	** We are going to retrieve some information, from a table
	** named "sysobjects", regarding names of system tables and
	** stored procedures.
	**
	** We will submit two queries.  The first finds all the rows 
	** that describe system tables.  The second finds all the rows
	** that describe stored procedures.  The program will only look
	** at the first 10 rows that describe stored procedures.
	*/ 

	/*
	--  * First, put the commands into the command buffer. * 
	--
	-- dbcmd(dbproc, "select name, type, id, crdate from sysobjects");
	-- dbcmd(dbproc, " where type = 'S' ");
	-- dbcmd(dbproc, "select name, type, id, crdate from sysobjects");
	-- dbcmd(dbproc, " where type = 'P' ");
	*/

	/*
	** Build up a language command. ct_command() constructs language, 
	** RPC, and some other server commands.
	**
	** Note that the application manages the language buffer: You
	** must format the language string with stdlib calls before
	** passing it to ct_command(). Unlike dbcmd(), consecutive calls to 
	** ct_command() do not concatenate the strings. 
	*/
	strcpy(sql_string, "select name, type, id, crdate from sysobjects");
	strcat(sql_string, " where type = 'S' ");
	strcat(sql_string, "select name, type, id, crdate from sysobjects");
	strcat(sql_string, " where type = 'P' ");
	ret = ct_command(cmd, CS_LANG_CMD, (CS_VOID *) sql_string, CS_NULLTERM,
			 CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "Init language command failed.");

	/*
	--  * Send the commands to ASE and start execution. * 
	-- dbsqlexec(dbproc);
	*/

	/*
	** Send the command. Unlike dbsqlexec(), ct_send() returns as 
	** soon as the command has been sent. It does not wait for
	** the results from the first statement to arrive.
	*/
	ret = ct_send(cmd);
	EXIT_ON_FAIL(context, ret, "Send language command failed.");  

	/*
	** ASE processes the command batch in the following order:
	**
	** 1) It will check for syntax errors (i.e., "use database pubs" 
	**    is syntactically incorrect; it should be "use pubs").
	** 2) The second check is a semantic check (i.e., "select * from 
	**    titels" will be incorrect because the spelling should be 
	**    "titles".)
	** 3) The third check occurs in the actual execution phase. This 
	**    check involves issues like permissions or memory problems.
	** 
	-- In DB-Library:
	-- In the execution phase, dbsqlexec() and dbresults() can return 
	-- the value "SUCCEED", which means there are more commands in the 
	-- batch to process and that that command was successful. A value 
	-- of "FAIL" means that the query failed but there may be more 
	-- commands in the batch to process. A value of "NO_MORE_RESULTS"
	-- means that there are no more commands in the batch to process.
	-- Therefore, the programmer must check the return values after
	-- dbsqlexec() and dbresults(), as illustrated below.
	** 
	** In Client-Library, ct_results() sets a result_type value to tell
	** about the success or failure of command execution on the server.
	**
	** The loop logic for using ct_results() is quite different than using
	** dbsqlexec() and dbresults(). See chapter 5 in the Client-Library
	** Migration Guide for a detailed comparison of results handling in 
	** Client-Library versus DB-Library.
	*/

	/*
	--  * Process each command until there are no more. * 
	--
	-- while ((result_code = dbresults(dbproc)) != NO_MORE_RESULTS)
	-- {
	--   if (result_code == SUCCEED)
	--   {
	--      * Bind program variables. * 
	-- 
	--     dbbind(dbproc, 1, NTBSTRINGBIND, (DBINT)0,
	--                             (BYTE DBFAR *)name);
	--     dbbind(dbproc, 2, NTBSTRINGBIND, (DBINT)0,
	--                             (BYTE DBFAR *)type);
	--     dbbind(dbproc, 3, INTBIND, (DBINT)0, (BYTE *)&id);
	--     dbbind(dbproc, 4, NTBSTRINGBIND, (DBINT)0,
	--                             (BYTE DBFAR *)crdate);
	--
	--      * Print appropriate header for the type of
	--      * data coming back.
	--      * 
	--
	--     printf("\n %s Objects: \n\n",
	--     DBCURCMD(dbproc) == 1 ? "System Table": "Procedure");
	--
	--      * Now print the rows. * 
	--
	--     while (dbnextrow(dbproc) != NO_MORE_ROWS)
	--     {
	--        *
	--       ** If this is the 2nd command and
	--       ** 10th row, flush the rest of the
	--       ** rows for that command.
	--       * 
	--
	--       if ((DBCURCMD(dbproc) == 2)
	--         && (DBCURROW(dbproc) > 10))
	--       {
	--        continue;
	--       }
	--
	--       printf
	--         ("%s %s %ld %s\n", name, type, id, crdate);
	--     }
	--   }
	-- }
	*/

	/*
	** DB-Library's DBCURROW() macro has no Client-Library equivalent, but
	** it's easy to count retrieved rows ourselves. Start by zeroing our 
	** counter.
	*/
	cur_rownum = 0;  

	/*
	** Here's our results loop. It runs until ct_results() returns a 
	** value other than CS_SUCCEED. Each time it returns CS_SUCCEED,
	** ct_results() sets our result_type variable. If all goes well, 
	** we will see the following result_type values for the SQL batch 
	** that we sent:
	**   CS_ROW_RESULT:
	**      Indicates we can bind and fetch the rows returned by the
	**      query.
	**   CS_CMD_DONE:
	**      Indicates that we've finished the results for one of the 
	**      select statements we sent.
	** If the SQL fails for some reason, say we don't have permission 
	** for what we selected, then we will see CS_CMD_FAIL.
	**
	** After loop exit, we check for errors indicated by the ct_results
	** return code. These will never indicate server-side errors.
	*/      
	while ( (results_ret = ct_results(cmd, &result_type)) == CS_SUCCEED)
	{
		switch((int) result_type)
		{
		  case CS_ROW_RESULT:
			/*
			** Bind and fetch the rows. 
			**
			** ct_bind() requires a CS_DATAFMT structure which
			** describes the datatype and formatting information
			** for the destination variable. We will use
			** char_datafmt for character destinations and
			** int_datafmt for integer destinations. With character
			** destinations, we need to specify the length of
			** the destination array with char_datafmt.maxlength.
			** With integer destinations, maxlength is
			** set to CS_UNUSED.
			*/
			char_datafmt.datatype = CS_CHAR_TYPE;
			char_datafmt.format = CS_FMT_NULLTERM;
			char_datafmt.count = 1;
			char_datafmt.locale = (CS_LOCALE *) NULL;

			int_datafmt.datatype = CS_INT_TYPE;
			int_datafmt.format = CS_FMT_UNUSED;
			int_datafmt.count = 1;
			int_datafmt.locale = (CS_LOCALE *) NULL;
			int_datafmt.maxlength = CS_UNUSED; 

			/*
			** Bind the 'name' column.
			*/
			char_datafmt.maxlength = CS_MAX_NAME + 1;
			ret = ct_bind(cmd, 1, &char_datafmt, (CS_VOID *)name,
				      (CS_INT *) NULL, (CS_SMALLINT *) NULL);
			EXIT_ON_FAIL(context, ret, "Bind to name failed.");

			/*
			** Bind the 'type' column.
			*/
			char_datafmt.maxlength = TYPELEN + 1;
			ret = ct_bind(cmd, 2, &char_datafmt, (CS_VOID *)type,
				      (CS_INT *) NULL, (CS_SMALLINT *) NULL);
			EXIT_ON_FAIL(context, ret, "Bind to type failed.");

			/*
			** Bind the 'id' column.
			*/
			ret = ct_bind(cmd, 3, &int_datafmt, (CS_VOID *) &id,
				      (CS_INT *) NULL, (CS_SMALLINT *) NULL);
			EXIT_ON_FAIL(context, ret, "Bind to id failed.");

			/*
			** Bind the 'crdate' column.
			*/ 
			char_datafmt.maxlength = DATELEN + 1;
			ret = ct_bind(cmd, 4, &char_datafmt, (CS_VOID *)crdate,
				      (CS_INT *) NULL, (CS_SMALLINT *) NULL);
			EXIT_ON_FAIL(context, ret, "Bind to crdate failed.");

			/*
			** In our language batch, the first statement
			** returns system tables, and the second returns
			** procedures. So, we can use the command number
			** to tell which is which. ct_res_info(CS_CMD_NUMBER)
			** replaces DBCURCMD().
			*/
			ret = ct_res_info(cmd, CS_CMD_NUMBER, &cur_cmdnum,
					  CS_UNUSED, NULL);
			EXIT_ON_FAIL(context, ret,
				     "ct_res_info(CMD_NUMBER) failed.");
			fprintf(OUT_CH, "\n %s Objects: \n\n", 
				cur_cmdnum == 1 ? "System Table" :
				(cur_cmdnum == 2 ? "Procedure" : "???????") );

			/*
			** Now fetch the rows. We'll test the command number,
			** and if we're on the second SELECT,
			** we'll only print the first ten rows.
			**
			** ct_fetch() is roughly equivalent to dbnextrow().
			*/ 
			while ((fetch_ret 
				= ct_fetch(cmd, CS_UNUSED, CS_UNUSED,
				CS_UNUSED, NULL)) == CS_SUCCEED)
			{

				/* 
				** Increment our row count.
				*/
				++cur_rownum;

				/*
				** If this is the 10th row of the 2nd SELECT's
				** result set, then don't print the results.
				** We could have could achieved the same results
				** more efficiently by bracketing the
				** SELECT statement with the statements
				** "SET ROWCOUNT 10" and "SET ROWCOUNT 0".  
				*/
				if (cur_cmdnum == 2 && cur_rownum > 10)
				{
					continue;
				}

				/*
				** Since Client-Library lacks an equivalent for
				** dbbind's NTBSTRINGBIND() datatype, we need
				** to trim the values ourselves. trim() is
				** defined at the bottom of this source file.
				*/
				printf("%s %s %ld %s\n", name, trim(type),
				       (long)id, crdate);
	
			}

			/*
			** Check for ct_fetch() errors. We should have
			** exited the loop with CS_END_DATA.
			*/
			if (fetch_ret != CS_END_DATA)
			{
				EXIT_ON_FAIL(context, CS_FAIL,
					     "ct_fetch() failed.");
			}

			break;

		  case CS_CMD_DONE:
			/*
			** This result type represents the boundary between the
			** result sets from the two SELECT statements.
			** CS_CMD_DONE is roughly equivalent to the end of one
			** iteration of the dbresults() loop.
			**
			** Here, we can retrieve the number of rows affected
			** with ct_res_info(). We print it and our own
			** row count just to show that they are the same.
			**
			** ct_res_info(CS_ROW_COUNT) is equivalent to
			** DB-Library's DBCOUNT() macro.
			*/
			ret = ct_res_info(cmd, CS_ROW_COUNT,
					  (CS_VOID *)&intval, CS_UNUSED, NULL);
			EXIT_ON_FAIL(context, ret,
				     "ct_res_info(CS_ROW_COUNT) failed.");

			fprintf(OUT_CH,
				"(%ld rows affected, %ld rows processed)\n",
				cur_rownum, intval);

			/*
			** Reset our row count to get ready for
			** the next set of rows.
			*/
			cur_rownum = 0;
			break;

		  case CS_CMD_SUCCEED:
			/*
			** This result type indicates that a command that does
			** not (itself) return rows has succeeded.
			** INSERT, EXEC, UPDATE, USE, CREATE statements are
			** examples of such a command. Note that stored
			** procedure execution commands are in this category,
			** even though the stored procedure might execute
			** code which returns rows.
			** 
			** We don't expect this case because we only
			** sent SELECT queries.
			*/
			fprintf(OUT_CH, "Got CS_CMD_SUCCEED result type.\n");
			break; 

		  case CS_CMD_FAIL:
			/*
			** This value says that a command failed on the server.
			** In DB-Library, dbresults(), dbsqlexec(),
			** or dbsqlok() returned FAIL to indicate this.
			**
			** As with DB-Library, the server sends a message
			** to tell us what happened. The message is printed
			** by the server message callback.
			*/
			EXIT_ON_FAIL(context, CS_FAIL,
				     "Command failed on the server.");
			break;

		  default:
			/*
			** We don't expect any other value.
			*/
			fprintf(ERR_CH,
				"Received unexpected result type %ld\n",
				(long)result_type);
			EXIT_ON_FAIL(context, CS_FAIL,
				     "Aborting on unexpected results.");
			break;

		} /* switch */
	} /* while */
  
	/*
	** Check for ct_results() errors. We expect CS_END_RESULTS.
	*/
	if (results_ret != CS_END_RESULTS)
	{
		EXIT_ON_FAIL(context, ret, "ct_results() failed.");
	}

	/*
	-- dbexit();
	-- exit(STDEXIT);
	*/

	/*
	** Clean up Client-Library and CS-Library. ex_con_cleanup() closes and 
	** drops our connection. ex_ctx_cleanup() calls ct_exit() and drops the
	** context structure. 
	**
	** ex_con_cleanup() and ex_ctx_cleanup() are defined in exutils.c
	*/
	ret = ex_con_cleanup(conn, CS_SUCCEED);
	ret = ex_ctx_cleanup(context, ret);
	exit(NORMAL_EXIT);

} /* main() */

/*
** trim() -- trim trailing blanks from a null-terminated
**   string.
**
** Parameters:
**   str -- the null-terminated string to trim.
**
** Notes:
**   This is character set specific. It assumes a single
**   byte character set and that ' ' in the code set used by 
**   this program matches a space in the data.
**
** Returns:
**   Pointer to the original string.
*/
CS_CHAR
*trim(CS_CHAR *str)
{
	int i = 0;
	char *p;

	/*
	** If str is NULL, we do nothing.
	*/
	p = str;
	if (p == (CS_CHAR *) NULL )
	{
		return p;
	}

	i = strlen(p);

	/*
	** Trim the blanks.
	*/
	while (i > 0 && p[i - 1] == ' ')
	{
		--i;
		p[i] = '\0';
	}

	return str;

} /* trim */

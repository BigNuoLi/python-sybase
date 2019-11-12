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
** 	ex02ct.c is converted from DB-Library's example2.c to use 
**	Client-Library.
**
** 	This example opens a data file, inserts data from the file
** 	into a newly created table containing several of the
** 	ASE datatypes, and binds and prints the results.
**
** 	To run this example, the user needs 'create database' permission.
**
** 	Where DB-Library code has been replaced with Client-Library code,
** 	the DB-Library code is commented out and preceded with "--".
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
--  * Forward declarations of the error handler and message handler routines. *
-- int CS_PUBLIC err_handler();
-- int CS_PUBLIC msg_handler();
*/
#include <exutils.h>

/*
** dbctutil.c defines utility routines that are useful
** for converting DB-Library applications. We call
** dbct_parent_ctx() and dbct_trim().
*/
#include "dbctutil.h"

/*
** Prototypes for internal functions. The DB-Library example2.c did
** everything in main. 
*/
CS_RETCODE CreateAlltypes PROTOTYPE ((
	CS_CONNECTION *conn,
	char          *fname
	));
CS_RETCODE ShowAlltypes PROTOTYPE ((
	CS_CONNECTION *conn
	));

#define INPUTBUF_LEN 128
#define SQLBUF_LEN   2048
#define HEXLEN      510
#define PLEN        25 

/*
** main()
*/
int
main(int argc, char *argv[])
{
	/*
	-- LOGINREC         *login;
	-- DBPROCESS        *dbproc;
	-- RETCODE          return_code;
	*/
	CS_CONTEXT          *context;
	CS_CONNECTION       *conn;
	CS_RETCODE          ret;

	/*
	** Setup screen output.
	*/
	EX_SCREEN_INIT();

	/*
	--  * Initialize DB-Library. * 
	-- if (dbinit() == FAIL)
	--   exit(ERREXIT);
	--
	--  * Install the user-supplied error-handling and message-handling
	--  * routines. They are defined at the bottom of this source file.
	--  * 
	-- dberrhandle((EHANDLEFUNC)err_handler);
	-- dbmsghandle((MHANDLEFUNC)msg_handler);
	*/

	/*
	** ex_init() is defined in exutils.c
	** It allocates the CS_CONTEXT, initializes Client-Library, and installs
	** Client-Library and server message handlers.
	*/
	ret = ex_init(&context);
	EXIT_ON_FAIL(context, ret, "Initialization failed.");

	/* 
	--  * Allocate and initialize the LOGINREC structure to be used
	--  * to open a connection to ASE.
	--  * 
	-- login = dblogin();
	-- DBSETLUSER(login, USER);
	-- DBSETLPWD(login, PASSWORD);
	-- DBSETLAPP(login, "example2");
	-- 
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
	ret = ex_connect(context, &conn, "ex02ct", USER, PASSWORD, NULL);
	EXIT_ON_FAIL(context, ret, "Connect failed.");

	printf("Creating the 'test' database.\n");

	/*
	-- dbcmd(dbproc,"create database test ");
	--
	-- dbsqlexec(dbproc);
	-- while (dbresults(dbproc) != NO_MORE_RESULTS)
	--        continue;
	-- 
	-- dbuse(dbproc,"test");
	--
	*/

	/*
	** Create a database to run in. We use ex_execute_cmd()
	** to send a CREATE DATABASE language command and handle
	** the results. ex_execute_cmd() is defined in exutils.c
	**
	** We could also call the routine ex_create_db() from
	** exutils.c. But that routine drops the database if
	** it already exists, and we don't want any side effects
	** that were not in DB-Library's example2.c
	*/
	ret = ex_use_db(conn, "master");
	EXIT_ON_FAIL(context, ret, "'USE MASTER' failed");

	ret = ex_execute_cmd(conn, "create database test");
	EXIT_ON_FAIL(context, ret, "'CREATE DATABASE' failed");

	/*
	** ex_use_db() is also defined in exutils.c. 
	*/
	ret = ex_use_db(conn, "test");
	EXIT_ON_FAIL(context, ret, "'USE test' failed");

	/*
	** Create and populate the 'alltypes' table. CreateAlltypes() is
	** defined in this source file.
	*/
	ret = CreateAlltypes(conn, "datafile");
	EXIT_ON_FAIL(context, ret, "CreateAlltypes() failed.");

	printf("Selecting rows from the 'alltypes' table:\n");

	ret = ShowAlltypes(conn);
	EXIT_ON_FAIL(context, ret, "ShowAlltypes() failed.");

	/*
	** Remove the 'test' database. ex_remove_db() is defined in
	** exutils.c
	**
	** THIS STEP IS EXTRA -- DB-Library's example2 left the toy
	**   database behind.
	*/
	ret = ex_remove_db(conn, "test");
	EXIT_ON_FAIL(context, ret, "ex_remove_db() failed.");

	/*
	-- dbexit();
	-- exit(STDEXIT);
	*/ 

	ret = ex_con_cleanup(conn, CS_SUCCEED);
	ret = ex_ctx_cleanup(context, ret);

	exit (NORMAL_EXIT);

} /* main() */

/*
** This macro is used for return code error checking in
** the ShowAlltypes() and CreateAlltypes() code.
*/
#define BAIL_ON_FAIL(cmd, ret, str) \
  { if (ret != CS_SUCCEED) \
    { \
      fprintf(ERR_CH, "ERROR line %d: %s\n", (int)__LINE__, str); \
      if (cmd != (CS_COMMAND *) NULL) \
      { \
	(CS_VOID) ct_cancel(NULL, cmd, CS_CANCEL_ALL); \
        (CS_VOID) ct_cmd_drop(cmd); \
      } \
      return CS_FAIL; \
    } } 

/*
** ShowAlltypes() -- Select the rows from the alltypes table and
**   display the returned data. The equivalent DB-Library code
**   was in example2.c's main()
** 
** Parameters
**   conn -- An open, idle connection.
** 
** Returns
**   CS_SUCCEED or CS_FAIL
*/
CS_RETCODE
ShowAlltypes(CS_CONNECTION *conn)
{
	CS_CONTEXT    *ctx;
	CS_COMMAND    *cmd = (CS_COMMAND *) NULL;
	CS_RETCODE    ret;
	CS_RETCODE    results_ret;
	CS_INT        result_type;
	CS_RETCODE    fetch_ret;
	CS_CHAR       sqlbuf[SQLBUF_LEN];
	CS_TINYINT    age;
	CS_SMALLINT   userid;
	CS_INT        royalty;
	CS_CHAR       name[PLEN+1];
	CS_BINARY     title_id[PLEN+1];
	CS_INT        title_id_len;
	CS_BIT        us_citizen;
	CS_FLOAT      account;
	CS_CHAR       title[PLEN+1];
	CS_CHAR       manager[PLEN+1];
	CS_CHAR       id_buffer[HEXLEN+1];
	CS_DATAFMT    datafmt;
	CS_DATAFMT    datafmt2;

	ret = ct_cmd_alloc(conn, &cmd);
	BAIL_ON_FAIL(cmd, ret, "ShowAlltypes: Command alloc failed.");

	/*
	-- dbcmd(dbproc,"select * from alltypes");
	-- dbsqlexec(dbproc);
	*/

	/*
	** Build and send the SELECT language command.
	*/

	strcpy(sqlbuf, "select * from alltypes");
	ret = ct_command(cmd, CS_LANG_CMD, (CS_VOID *) sqlbuf,
			 CS_NULLTERM, CS_UNUSED);
	BAIL_ON_FAIL(cmd, ret, "ShowAlltypes: ct_command() failed.");

	ret = ct_send(cmd);
	BAIL_ON_FAIL(cmd, ret, "ShowAlltypes: ct_send() failed.");

	/*
	-- while ((return_code = dbresults(dbproc)) != NO_MORE_RESULTS)
	-- {
	--   if (return_code == SUCCEED)
	--   {
	--     dbbind(dbproc, 1, TINYBIND, (DBINT)0, (BYTE *)&age);
	--     dbbind(dbproc, 2, SMALLBIND, (DBINT)0, (BYTE *)&userid);
	--     dbbind(dbproc, 3, INTBIND, (DBINT)0, (BYTE *)&royalty);
	--     dbbind(dbproc, 4, CHARBIND, (DBINT)0, (BYTE DBFAR *)name);
	--     dbbind(dbproc, 5, BINARYBIND, (DBINT)0, (BYTE DBFAR *)title_id);
	--     dbbind(dbproc, 6, BITBIND, (DBINT)0, (BYTE *)&us_citizen);
	--     dbbind(dbproc, 7, FLT8BIND, (DBINT)0, (BYTE *)&account);
	--     dbbind(dbproc, 8, STRINGBIND, (DBINT)0, (BYTE DBFAR *)title);
	--     dbbind(dbproc, 9, NTBSTRINGBIND, (DBINT)0,
	--                                     (BYTE DBFAR *)manager);
	--
	--      *
	--     ** Initialize null terminator in "name" array,
	--     ** since CHARBIND does not add one.
	--     * 
	--     name[PLEN] = '\0';
	--
	--     while (dbnextrow(dbproc) != NO_MORE_ROWS)
	--     {
	--       dbconvert
	--         (dbproc, SYBBINARY, title_id,
	--          dbdatlen(dbproc, 5), SYBCHAR,
	--          (BYTE DBFAR *)id_buffer, (DBINT)-1);
	--       printf
	--         ("%d  %d  %ld  %s  0x%s\n",
	--          age, userid, royalty, name, id_buffer);
	--       printf
	--         ("%d  %8.2f  %s  %s\n",
	--          us_citizen, account, title, manager);
	--     }
	--   }
	-- }
	*/

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

	while((results_ret = ct_results(cmd, &result_type)) == CS_SUCCEED)
	{
		switch((int) result_type)
		{
		  case CS_CMD_FAIL:
			/*
			** The command failed on the server. This is similar
			** to dbsqlexec() or dbresults() returning FAIL
			** in DB-Library. However, in DB-Library these
			** functions can also fail due to network trouble.
			** In Client-Library, this result type indicates that
			** the command reached the server and the server
			** responded by telling us that the command failed.
			*/
			BAIL_ON_FAIL(cmd, ret,
				     "ShowAlltypes: SELECT failed on server.");
			break;

		  case CS_CMD_SUCCEED:
		  case CS_CMD_DONE:
			/*
			** No action required.
			*/
			break;

		  case CS_ROW_RESULT:
			/*
			** This result type indicates that the regular rows
			** are ready to process. In DB-Library, we learned
			** this when dbnextrow returned REG_ROW.
			*/

			/*
			** First bind to each column.
			*/
			datafmt.locale = (CS_LOCALE *)NULL;
			datafmt.count = 1;

			/*
			** 'age' is server datatype tinyint.
			*/
			datafmt.datatype = CS_TINYINT_TYPE;
			datafmt.format = CS_FMT_UNUSED;
			datafmt.maxlength = CS_UNUSED;
			ret = ct_bind(cmd, 1, &datafmt, (CS_VOID *)&age, NULL,
				      NULL);
			BAIL_ON_FAIL(cmd, ret,
				     "ShowAlltypes: bind to age failed.");

			/*
			** 'userid' is a smallint.
			*/
			datafmt.datatype = CS_SMALLINT_TYPE;
			datafmt.format = CS_FMT_UNUSED;
			datafmt.maxlength = CS_UNUSED;
			ret = ct_bind(cmd, 2, &datafmt, (CS_VOID *)&userid,
				      NULL, NULL);
			BAIL_ON_FAIL(cmd, ret,
				     "ShowAlltypes: bind to userid failed.");

			/*
			** 'royalty' is an int.
			*/
			datafmt.datatype = CS_INT_TYPE;
			datafmt.format = CS_FMT_UNUSED;
			datafmt.maxlength = CS_UNUSED;
			ret = ct_bind(cmd, 3, &datafmt, (CS_VOID *)&royalty,
				      NULL, NULL);
			BAIL_ON_FAIL(cmd, ret,
				     "ShowAlltypes: bind to royalty failed.");

			/*
			** 'name' is char(25). We'll null-terminate the bound
			** variable here, and give ct_bind() a length
			** (in datafmt.maxlength) that does not include
			** the null terminator.
			*/
			datafmt.datatype = CS_CHAR_TYPE;
			datafmt.format = CS_FMT_PADBLANK;
			datafmt.maxlength = PLEN;
			name[PLEN] = '\0';
			ret = ct_bind(cmd, 4, &datafmt, (CS_VOID *)name,
				      NULL, NULL);
			BAIL_ON_FAIL(cmd, ret,
				     "ShowAlltypes: bind to name failed.");

			/*
			** Bind 'title_id' to the binary array title_id.
			** We also bind a length variable (&title_id_len as
			** the 5 argument) so we know the length of each value.
			** We need to know this in order to convert the
			** binary array to a character string.
			*/
			datafmt.datatype = CS_BINARY_TYPE;
			datafmt.format = CS_FMT_UNUSED;
			datafmt.maxlength = PLEN+1;
			ret = ct_bind(cmd, 5, &datafmt, (CS_VOID *)title_id,
				      &title_id_len, NULL);
			BAIL_ON_FAIL(cmd, ret,
				     "ShowAlltypes: bind to title_id failed.");

			/*
			** 'us_citizen' is server datatype bit.
			*/
			datafmt.datatype = CS_BIT_TYPE;
			datafmt.format = CS_FMT_UNUSED;
			datafmt.maxlength = CS_UNUSED;
			ret = ct_bind(cmd, 6, &datafmt, (CS_VOID *)&us_citizen,
				      NULL, NULL);
			BAIL_ON_FAIL(cmd, ret,
				     "ShowAlltypes: bind to us_citizen failed.");

			/*
			** 'account' is server datatype float.
			*/
			datafmt.datatype = CS_FLOAT_TYPE;
			datafmt.format = CS_FMT_UNUSED;
			datafmt.maxlength = PLEN;
			ret = ct_bind(cmd, 7, &datafmt, (CS_VOID *)&account,
				      NULL, NULL);
			BAIL_ON_FAIL(cmd, ret,
				     "ShowAlltypes: bind to account failed.");

			/*
			** 'title' is varchar(20)
			*/
			datafmt.datatype = CS_CHAR_TYPE;
			datafmt.format = CS_FMT_NULLTERM;
			datafmt.maxlength = PLEN + 1;
			ret = ct_bind(cmd, 8, &datafmt, (CS_VOID *)title,
				      NULL, NULL);
			BAIL_ON_FAIL(cmd, ret,
				     "ShowAlltypes: bind to title failed.");

			/*
			** 'manager' is char(25). DB-Library's example2
			** uses NTBSTRINGBIND to bind to this column.
			**
			** There is no equivalent to NTBSTRBIND in
			** Client-Library. For the manager column values,
			** we will trim trailing blanks using our own
			** function after each row is fetched. 
			*/
			datafmt.datatype = CS_CHAR_TYPE;
			datafmt.format = CS_FMT_NULLTERM;
			datafmt.maxlength = PLEN+1;
			ret = ct_bind(cmd, 9, &datafmt, (CS_VOID *)manager,
				      NULL, NULL);
			BAIL_ON_FAIL(cmd, ret,
				     "ShowAlltypes: bind to manager failed.");

			/*
			** Now we are ready to fetch rows and print the data.
			*/
			while((fetch_ret 
			      = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED,
			      (CS_INT *)NULL)) == CS_SUCCEED)
			{
				/*
				** Convert the binary data in title_id to
				** character. We need retrieve the command
				** structure's grandmother context to 
				** call cs_convert().
				** 
				** dbct_parent_ctx() is defined in dbctutil.c
				*/
				ret = dbct_parent_ctx(cmd, NULL, &ctx);
				BAIL_ON_FAIL(cmd, ret,
					     "ShowAlltypes: dbct_parent_context() failed.");

				/*
				** cs_convert() takes two CS_DATAFMT
				** structures, one to describe the source data,
				** one to describe the destination. We set 
				** datafmt's fields to describe the binary data
				** in title_id and use datafmt2 to describe
				** the character destination.
				**
				** Recall that title_id_len is bound to the
				** length of the retrieved data for 'title_id'.
				*/
	
				datafmt.locale = (CS_LOCALE *) NULL;
				datafmt.datatype = CS_BINARY_TYPE;
				datafmt.maxlength = title_id_len;
	
				datafmt2.locale = (CS_LOCALE *) NULL;
				datafmt2.datatype = CS_CHAR_TYPE;
				datafmt2.format = CS_FMT_NULLTERM;
				datafmt2.maxlength = HEXLEN+1;

				ret = cs_convert(ctx, &datafmt,
						 (CS_VOID *) title_id,
						 &datafmt2,
						 (CS_VOID *) id_buffer, NULL);
				BAIL_ON_FAIL(cmd, ret,
					     "ShowAlltypes: cs_convert() failed.");
	
				/*
				** Now print the row, including the converted
				** 'title_id' value. We need to trim the
				** 'manager' value to duplicate the output
				** of DB-Library's example2.
				*/
				printf("%d  %d  %ld  %s  0x%s\n", (int)age,
				       (int)userid, (long)royalty, name,
				       id_buffer);
				printf("%d  %8.2f  %s  %s\n", us_citizen,
				       account, title, dbct_trim(manager));

			}

			/*
			** Check that no ct_fetch() errors occurred.
			*/
			if (fetch_ret != CS_END_DATA)
			{
				BAIL_ON_FAIL(cmd, CS_FAIL,
					     "ShowAlltypes: ct_fetch() failed.");
			}

			break;
		} /* switch */
	} /* while */

	/*
	** Now, we check for ct_results failure.
	*/
	switch((int) results_ret)
	{
	  case CS_END_RESULTS:
		/* 
		** This is what we expect.
		*/
		break;

	  case CS_CANCELED:
		/*
		** This return code is seen when you do
		** ct_cancel(CS_CANCEL_ALL) or ct_cancel(CS_CANCEL_ATTN)
		** while results are being processed.
		** Shouldn't see this in this program, but we show the code
		** so that you know that this can happen.
		*/
		fprintf(OUT_CH, "\nShowAlltypes: Results were canceled.\n");
		break;

	  case CS_FAIL:
		/*
		** Not good.
		*/
		BAIL_ON_FAIL(cmd, CS_FAIL,
			     "ShowAlltypes: ct_results() failed.");
		break;

	  default:
		/*
		** Unexpected.
		*/
		BAIL_ON_FAIL(cmd, CS_FAIL, 
			     "ShowAlltypes: ct_results() returned unexpected value.");
		break;
	}

	return CS_SUCCEED;

} /* ShowAlltypes() */

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
	CS_CHAR		sqlbuf[SQLBUF_LEN];
	CS_CHAR		inputbuf[INPUTBUF_LEN];
	FILE		*infile;
	int		sqlbuf_used;

	printf("Creating the 'alltypes' table.\n");

	/*
	** Here is the DB-Library code to create the table. This code
	** was in main() in the DB-Library example2 program.
	**
	--  * Create a table that contains several ASE datatypes. * 
	-- dbcmd(dbproc,"create table alltypes ");
	-- dbcmd(dbproc,"(age tinyint,");
	-- dbcmd(dbproc,"userid smallint,");
	-- dbcmd(dbproc,"royalty int,");
	-- dbcmd(dbproc,"name char(25),");
	-- dbcmd(dbproc,"title_id varbinary(20),"); 
	-- dbcmd(dbproc,"us_citizen bit,");
	-- dbcmd(dbproc,"account float,");
	-- dbcmd(dbproc,"title varchar(20),");
	-- dbcmd(dbproc,"manager char(25))");
	-- 
	-- dbsqlexec(dbproc);
	-- while (dbresults(dbproc) != NO_MORE_RESULTS)
	-- {
	-- 	continue;
	-- }
	--
	--  * Insert rows of data into the newly created table "alltypes".
	--  * We will read in the contents of the file and form an
	--  * INSERT statement.
	--  * 
	--
	-- if ((infile=fopen("datafile","r")) == NULL)
	-- {
	-- 	printf("Unable to open file 'datafile'.\n");
	-- 	exit(ERREXIT);
	-- }
	--
	-- printf("Inserting rows into the 'alltypes' table.\n");
	-- 
	-- while ((fgets(cmdbuf,BUFLEN,infile)) != NULL)
	-- {
	-- 	dbfcmd(dbproc,"insert into alltypes \n");
	-- 	dbfcmd(dbproc,"values(%s) \n",cmdbuf);
	-- }
	--
	-- dbsqlexec(dbproc);
	--
	--  * Process the results of each of the INSERT statements. * 
	--
	-- while ((return_code = dbresults(dbproc)) != NO_MORE_RESULTS)
	-- {
	-- 	if (return_code == FAIL)
	-- 	{
	-- 		printf("One of the insert statements FAILed.\n");
	-- 	}
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
	if (ret != CS_SUCCEED)
	{
		fprintf(ERR_CH,
			"CreateAlltypes: 'IF ... DROP TABLE' failed.\n");
		return CS_FAIL;
	}

	/*
	** Now create the table. 
	*/
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
	if (ret != CS_SUCCEED)
	{
		fprintf(ERR_CH, "CreateAlltypes: 'CREATE TABLE' failed.\n");
		return CS_FAIL;
	}

	/*
	** Insert rows of data into the newly created table.
	** We will read in the contents of the file and form an
	** INSERT statement.
	*/
    
	if ((infile=fopen(fname,"r")) == NULL)
	{
		printf("Unable to open file '%s'.\n", fname);
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
	-- 	dbfcmd(q_dbproc,"insert into alltypes \n");
	-- 	dbfcmd(q_dbproc,"values(%s, null) \n",cmdbuf);
	-- }
	-- 
	-- dbsqlexec(q_dbproc);
	-- 
	--  * Process the results of each of the INSERT statements. * 
	--
	-- while ((return_code = dbresults(q_dbproc)) != NO_MORE_RESULTS)
	-- {
	-- 	if (return_code == FAIL)
	-- 	{
	-- 		printf("One of the insert statements FAILed.\n");
	-- 	}
	-- }
	*/

	/*
	** Build insert statements with values read from the file
	** and send the batch.
	**
	** This code is slightly different than the code in the
	** example6 DB-Library program. That code built a
	** monolithic language batch containing all the 
	** INSERT statements, then sent them all as one language
	** command (with one call to dbsqlexec).
	**
	** We have a fixed length buffer, and we need to watch out
	** that we don't overflow it.
	*/
#define OPEN_INSERT_STMT "insert into alltypes \n values( "
#define CLOSE_INSERT_STMT " , null) \n"
	sqlbuf_used = 0;
	sqlbuf[0] = '\0';
	while (fgets(inputbuf,INPUTBUF_LEN,infile) != NULL)
	{
		/*
		**  If the buffer is full, send the batch.
		*/
		if ((sqlbuf_used + strlen(OPEN_INSERT_STMT) + strlen(inputbuf)
		     + strlen(CLOSE_INSERT_STMT)) > SQLBUF_LEN )
		{
			ret = ex_execute_cmd(conn, sqlbuf);
			if (ret != CS_SUCCEED)
			{
				fprintf(ERR_CH,
					"CreateAlltypes: Insert failed.\n");
				fclose(infile);
				return CS_FAIL;
			}
			sqlbuf_used = 0;
			sqlbuf[0] = '\0';
		}

		/*
		** Append another insert statement to the language batch.
		*/
		strcat(sqlbuf, OPEN_INSERT_STMT);
		strcat(sqlbuf, inputbuf);
		strcat(sqlbuf, CLOSE_INSERT_STMT);
		sqlbuf_used = sqlbuf_used + strlen(OPEN_INSERT_STMT) 
			      + strlen(inputbuf) + strlen(CLOSE_INSERT_STMT);
	} 

	/*
	** Send the last batch of insert statements.
	*/
	ret = ex_execute_cmd(conn, sqlbuf);
	if (ret != CS_SUCCEED)
	{
		fprintf(ERR_CH, "CreateAlltypes: Insert failed.\n");
		fclose(infile);
		return CS_FAIL;
	}

	fclose(infile);

	return CS_SUCCEED;

} /* CreateAlltypes() */

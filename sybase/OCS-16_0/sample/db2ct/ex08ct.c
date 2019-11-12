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
**  	ex08ct.c is converted from DB-Library example8.c to use
**	Client-Library.
**
** 	The original DB-Library code has been commented out and replaced
** 	with Client-Library code -- the old DB-Library code is preceded
** 	with the characters "--" on each line.
**
** 	This example illustrates how to use remote procedure calls
** 	and how to process return parameter values from stored
** 	procedures.  
**
** 	Note that the rpc sample program in the Client-Library examples 
** 	provides a more modular and robust demonstration of similar 
** 	processing.
**
** 	This example uses the following stored procedure, 
** 	named "rpctest", which it assumes is located in the 
** 	pubs2 database.  Before running this example, 
** 	you must create "rpctest" by running the rpctest.sql
** 	isql script. The text of "rpctest" is as 
** 	follows:
**
**     		create procedure rpctest
**         	    (@param1 int out,
**          	     @param2 int out,
**          	     @param3 int out,
**          	     @param4 int)
**     		as
**     		begin
**         	    select "rpctest is running."
**         	    select @param1 = 11
**         	    select @param2 = 22
**         	    select @param3 = 33
**         	    select @param1
**
**         	    return 123
**     		end
**
** 	NOTE: Unlike the DB-Library example8 program, this program
**  	assumes that "rpctest" is in the pubs2 database, not the
**  	user's default database. The rpctest.sql script creates
**  	the procedure in pubs2 and will drop any existing procedure
**  	with the same name.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*
** Client-Library header file is ctpublic.h
**
-- #include <sybfront.h>
-- #include <sybdb.h>
-- #include "sybdbex.h"
*/
#include <ctpublic.h>
#include "dbtoctex.h"

/*
** dbctutil.c defines utility routines that are useful for converting
** DB-Library applications. We call dbct_prtype() in this program.
*/
#include "dbctutil.h"

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

/*
** The original format definitions from example8.c
*/
#define FMTSTR	"%-8.8s %-8.8s %-8.8s %-8.8s\n"
#define FMTSTR1	"%-8.8s %-8.8s %8.8ld %8.8ld\n"

/*
** Useful macro.
*/
#define MAKEIT_INT_FORMAT(datafmt) \
{ datafmt.datatype = CS_INT_TYPE; \
  datafmt.format = CS_FMT_UNUSED; \
  datafmt.maxlength = CS_UNUSED; \
  datafmt.locale = (CS_LOCALE *)NULL; \
  datafmt.count = 1; }

int
main(int argc, char *argv[])
{
	/* 
	--  LOGINREC       *login;
	--  DBPROCESS      *dbproc;
	*/
	CS_CONTEXT	*context = (CS_CONTEXT *) NULL;
	CS_CONNECTION	*conn;
	CS_COMMAND	*cmd;

	int		i;
	CS_INT		numrets;

	/*
	-- DBINT            param1 = 1;
	-- DBINT            param2 = 2;
	-- DBINT            param3 = 3;
	-- DBINT            param4 = 4;
	*/
	CS_DATAFMT	datafmt;
	CS_INT		paramvals[4]; 
	CS_DATAFMT	paramfmts[4];
	CS_INT		sproc_status;

	/*
	-- RETCODE          return_code;
	*/
	CS_RETCODE      ret;         /* for return-code error checking       */
	CS_RETCODE      results_ret; /* driver for ct_results() loop         */  
	CS_INT          result_type; /* ct_results() result type description */
	CS_RETCODE      fetch_ret;   /* driver for ct_fetch() loop           */

	/*
	** Setup screen output.
	*/
	EX_SCREEN_INIT();

	/*
	-- * Initialize DB-Library. *
	-- if (dbinit() == FAIL)
	-- {
	--   exit(ERREXIT);
	-- }
	-- * Install the user-supplied error-handling and message-handling
	-- * routines. They are defined at the bottom of this source file.
	-- *
	-- dberrhandle(err_handler);
	-- dbmsghandle(msg_handler);
	*/

	/*
	** Initialize CS-Library and Client-Library. We do this with
	** ex_init() from exutils.c
	** 
	** ex_init() allocates a context structure, initializes Client-Library,
	** and installs callbacks for the error and message handlers.
	*/
	ret = ex_init(&context);
	EXIT_ON_FAIL(context, ret, "ex_init() failed.");

	/*
	--  * Allocate and initialize the LOGINREC structure to be used
	--  * to open a connection to ASE.
	--  *
	-- login = dblogin();
	-- DBSETLUSER(login, USER);
	-- DBSETLPWD(login, PASSWORD);
	-- DBSETLAPP(login, "rpcexample");
	--
	-- dbproc = dbopen(login, NULL);
	*/

	/*
	** Connect to the server. We use ex_connect from exutils.c to 
	** do this. ex_connect() sets the login information (with
	** ct_con_props) and calls ct_connect().
	**
	** As with the DB-Library example, the default server name is taken
	** as the contents of the DSQUERY environment variable.
	*/
	ret = ex_connect(context, &conn, "ex03ct", USER, PASSWORD, NULL);
	EXIT_ON_FAIL(context, ret, "Connection attempt failed.");

	/*
	** Change to the 'pubs2' database. ex_use_db() is in exutils.c.
	**
	** The DB-Library example8 just ran in the user's default database; it
	** did not change to pubs2. That was not nice.
	*/
	ret = ex_use_db(conn, "pubs2");
	EXIT_ON_FAIL(context, ret, "SQL 'USE pubs2' failed");

	/* 
	** Allocate a command structure. 
	*/
	ret = ct_cmd_alloc(conn, &cmd);
	EXIT_ON_FAIL(context, ret, "Could not allocate command structure.");


	/*
	-- * Make the rpc. *
	-- if (dbrpcinit(dbproc, "rpctest", (DBSMALLINT)0) == FAIL)
	-- {
	--   doexit("dbrpcinit failed.\n");
	-- }
	*/

	/*
	** Initiate an RPC command. In Client-Library ct_command is used for 
	** language commands (dbsqlexec or dbsqlsend commands in DB-Library), 
	** RPC commands (dbrpcinit), and text/image "send-data" commands 
	** (dbwritetext).
	*/

	ret = ct_command(cmd, CS_RPC_CMD, "rpctest", CS_NULLTERM, CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "Could not initiate RPC command.");

	/*
	** Pass a value for each RPC parameter with ct_param. In this case,
	** the required RPC parameters are the parameters in the definition
	** of the rpctest stored procedure.
	**
	** The parameter's name, datatype, and status (input-only or output) are 
	** passed within a CS_DATAFMT structure.
	*/

	/*
	-- if (dbrpcparam
	--     (dbproc, "@param1", (BYTE)DBRPCRETURN, SYBINT4, -1, -1, &param1)
	--     == FAIL)
	-- {
	--   doexit("dbrpcparam 1 failed.\n");
	-- }
	*/

	for (i = 1; i <= 4; i++)
	{
		paramvals[i - 1] = i;
	}

	/* 
	** @param1 is integer (CS_INT) and is a return parameter.
	** The datafmt.status field must be set to indicate whether
	** each parameter is 'for output' (CS_RETURN) or not 
	** (CS_INPUTVALUE)
	*/

	datafmt.datatype = CS_INT_TYPE;
	datafmt.maxlength = CS_UNUSED;
	datafmt.status = CS_RETURN;
	strcpy(datafmt.name, "@param1");
	datafmt.namelen = strlen(datafmt.name);

	ret = ct_param(cmd, &datafmt, (CS_VOID *) (paramvals+0), CS_UNUSED, 0);
	EXIT_ON_FAIL(context, ret, "ct_param() for @param1 failed.");

	/*
	-- if (dbrpcparam(dbproc, "@param2", (BYTE)0, SYBINT4, -1, -1, &param2)
	--     == FAIL)
	-- {
	--   doexit("dbrpcparam 2 failed.\n");
	-- }
	*/

	/*
	** @param2 is integer (CS_INT) and is not a return parameter.
	*/
	datafmt.datatype = CS_INT_TYPE;
	datafmt.maxlength = CS_UNUSED;
	datafmt.status = CS_INPUTVALUE;         
	strcpy(datafmt.name, "@param2");
	datafmt.namelen = strlen(datafmt.name);

	ret = ct_param(cmd, &datafmt, (CS_VOID *) (paramvals+1), CS_UNUSED, 0);
	EXIT_ON_FAIL(context, ret, "ct_param() for @param2 failed.");

	/*
	-- if (dbrpcparam
	--     (dbproc, "@param3", (BYTE)DBRPCRETURN, SYBINT4, -1, -1, &param3)
	--     == FAIL)
	-- {
	--   doexit("dbrpcparam 3 failed.\n");
	-- }
	*/

	/* 
	** @param3 is integer (CS_INT) and is a return parameter.
	*/

	datafmt.datatype = CS_INT_TYPE;
	datafmt.maxlength = CS_UNUSED;           
	datafmt.status = CS_RETURN;
	strcpy(datafmt.name, "@param3");
	datafmt.namelen = strlen(datafmt.name);

	ret = ct_param(cmd, &datafmt, (CS_VOID *) (paramvals+2), CS_UNUSED, 0);
	EXIT_ON_FAIL(context, ret, "ct_param() for @param3 failed.");

	/*
	-- if (dbrpcparam(dbproc, "@param4", (BYTE)0, SYBINT4, -1, -1, &param4)
	--     == FAIL)
	-- {
	--   doexit("dbrpcparam 4 failed.\n");
	-- }
	*/

	/*
	** @param4 is integer (CS_INT) and is not a return parameter.
	*/
	datafmt.datatype = CS_INT_TYPE;
	datafmt.maxlength = CS_UNUSED;
	datafmt.status = CS_INPUTVALUE;
	strcpy(datafmt.name, "@param4");
	datafmt.namelen = strlen(datafmt.name);

	ret = ct_param(cmd, &datafmt, (CS_VOID *) (paramvals+3), CS_UNUSED, 0);
	EXIT_ON_FAIL(context, ret, "ct_param() for @param4 failed.");
	  
	/*
	-- if (dbrpcsend(dbproc) == FAIL)
	-- {
	--   doexit("dbrpcsend failed.\n");
	-- }
	*/

	/*
	** Send the command to the server. The ct_send routine sends
	** any kind of command, not just RPC commands.
	*/
	ret = ct_send(cmd);
	EXIT_ON_FAIL(context, ret, "ct_send() failed.");


	/*
	-- if (dbsqlok(dbproc) == FAIL)
	-- {
	--   doexit("dbsqlok failed.\n");
	-- }
	-- while ((return_code = dbresults(dbproc)) != NO_MORE_RESULTS)
	-- {
	*/


	/*
	** Here's our results loop. It runs until ct_results() returns a 
	** value other than CS_SUCCEED. Each time it returns CS_SUCCEED,
	** ct_results() sets our result_type variable. If all goes well, 
	** we will see the following result_type values in the procedure's
	** results:
	**   CS_ROW_RESULT:
	**      Indicates we can bind and fetch the rows returned by the
	**      select in the stored procedure.
	**   CS_PARAM_RESULT:
	**      Indicates the output parameter values can be bound and 
	**      fetched.
	**   CS_STATUS_RESULT:
	**      Indicates that the return status can be bound and fetched.
	**   CS_CMD_SUCCEED:
	**      In this case, indicates that the stored procedure executed
	**      sucessfully.
	**   CS_CMD_DONE:
	**      Indicates that we've finished the results for logical command.
	**      In this case, we'll see this once after the SELECT in the 
	**      procedure has returned all it's rows, and once after all the
	**      results from the RPC have been processed.
	**
	** If the RPC fails on the server for some reason, say for instance 
	** the stored procedure did not exist, then we will see CS_CMD_FAIL.
	** 
	** After loop exit, we check for errors indicated by the ct_results()
	** return code. These will never indicate server-side errors.
	*/

	while ((results_ret = ct_results(cmd, &result_type)) == CS_SUCCEED)
	{
		switch ((int) result_type)
		{
		  case CS_CMD_FAIL:

			/* 
			** Command failed on server. In DB-Library,
			** the application learned this from dbsqlok's
			** or dbresults' return code.
			**
			** As with DB-Library, the server sends a message
			** to tell us what happened. 
			*/
			printf("Command failed on server.\n");

			break;

		  case CS_CMD_SUCCEED:

			/* 
			** Command ran successfully on the server.
			** In DB-Library, the application learned this
			** from dbsqlexec's or dbresults' return code.
			** No action is required here in this program.
			*/
			break;

		  case CS_ROW_RESULT:
		  case CS_COMPUTE_RESULT:

			/*
			** Regular and compute rows.
			**
			** We don't expect compute results, but if they come
			** back, we will print them.
			**
			** The function ex_fetch_data() from exutils.
			** is similar to DB-Library's dbprrow().
			** Client-Library provides no equivalent to dbprrow,
			** but dbprrow() can easily be replaced by an
			** application routine such as ex_fetch_data().
			**
			-- * Print any rows that may have been returned. *
			-- dbprrow(dbproc);
			*/
			ret = ex_fetch_data(cmd);
			EXIT_ON_FAIL(context, ret, "ex_fetch_data() failed.");

			break;

		  case CS_PARAM_RESULT:
			/*
			** Return parameter values. In DB-Library, you
			** checked for the condition (dbnumrets(dbproc) > 0)
			** to see if return parameter values were present
			** in the results. Client-Library returns
			** the result type CS_PARAM_RESULT to tell you when the 
			** return status values have arrived.
			**
			** DB-Library required the special routines dbretdata,
			** dbretlen, etc. to retrieve return parameter values.
			**
			** In Client-Library, the return parameters are just
			** another fetchable result type. They appear as one
			** 'row' of data values to the application.
			** The application can call ct_res_info() and
			** ct_describe() to find the number of values
			** and the format of each. 
			** 
			** We could just call ex_fetch_data() (from exutils.c)
			** to print the return parameter values. But instead
			** we show the Client-Library code here so that
			** it can be compared to the DB-Library code.
			*/
	    
			/* 
			** Get the number of return parameter values with
			** ct_res_info(CS_NUMDATA).
			**
			--  *
			-- ** Examine any return parameters that may have
			-- ** arrived.
			-- *
			-- numrets = dbnumrets(dbproc);
			-- printf("%d return values received.\n\n", numrets);
			--
			-- if (numrets > 0)
			-- {
			--   printf
			--    (FMTSTR, "Name", "Type", "Length", "Value");
			--   printf
			--     (FMTSTR,
			--      "------------", "------------",
			--      "------------", "------------");
			-- }
			*/

			ret = ct_res_info(cmd, CS_NUMDATA, (CS_VOID *)&numrets, 
					  CS_UNUSED, (CS_INT *) NULL);
			EXIT_ON_FAIL(context, ret,
				     "ct_res_info for return params failed");
			printf("%ld return values received.\n", (long)numrets);

			if (numrets > 0)
			{
				printf(FMTSTR,
				       "Name", "Type",
				       "Length", "Value");
				printf(FMTSTR,
				       "------------", "------------",
				       "------------", "------------");
			}

			/*
			** Get the name and format of each value plus
			** the value itself, then print everything out.
			**
			-- for (i = 1; i <= numrets; i++)
			-- {
			--   printf
			--     (FMTSTR1, dbretname(dbproc, i),
			--      dbprtype(dbrettype(dbproc, i)),
			--      dbretlen(dbproc, i),
			--      *((DBINT *)(dbretdata(dbproc, i))));
			-- }
			*/

			/*
			** ct_describe() fills a CS_DATAFMT to describe each
			** return value. The returned field values correspond
			** to the values that DB-Library returns for dbrettype,
			** dbretlen, and dbretname.
			*/
			for (i = 0; i < numrets && i < 4; i++)
			{
				/* 
				** Call ct_describe to get the format
				** information.
				*/
				ret = ct_describe(cmd, i + 1, (paramfmts + i));
				EXIT_ON_FAIL(context, ret,
					     "ct_describe failed.");

				/*
				** Bind the return parameters.
				*/
				if (datafmt.datatype != CS_INT_TYPE)
				{
					EXIT_ON_FAIL(context, CS_FAIL, 
						     "Unexpected return parameter datatype.");
				}
				ret = ct_bind(cmd, i + 1, (paramfmts + i), 
					      (CS_VOID *)(paramvals + i), 
					      (CS_INT *)NULL,
					      (CS_SMALLINT *)NULL);  
				EXIT_ON_FAIL(context, ret,
					     "ct_bind failed for return parameter.");
			}

			/*
			** In Client-Library, we must call ct_fetch()
			** to retrieve the parameters. In DB-Library, we just
			** called dbretdata() directly.
			**
			** Fetch the rows -- we always call ct_fetch()
			** until it fails or indicates that all rows
			** have been fetched.
			*/
			while((fetch_ret = ct_fetch(cmd, CS_UNUSED, CS_UNUSED,
						    CS_UNUSED,(CS_INT *) NULL))
			     == CS_SUCCEED)
			{
				for (i = 0; i < numrets; i++)
				{
					printf(FMTSTR1, paramfmts[i].name, 
					       dbct_prtype(paramfmts[i].datatype),
					       (long)paramfmts[i].maxlength, 
					       (long)paramvals[i]);
				}
			}
			if (fetch_ret != CS_END_DATA)
			EXIT_ON_FAIL(context, CS_FAIL,
				     "ct_fetch failed on return params.");

			break;

		  case CS_STATUS_RESULT:

			/*
			** The stored procedure's return status.
			** In DB-Library, you called dbhasretstat()
			** to determine if there was a return status.
			** In Client-Library, a result_type of CS_STATUS_RESULT
			** tells the application that the server has sent
			** the return status.
			** 
			** Unlike DB-Library, Client-Library has no special
			** routine to get a return status. The return status
			** is just another fetchable result type.
			** As with any fetchable result type,
			** we can bind and fetch the return status.
			**
			-- if (dbhasretstat(dbproc))
			-- {
			--   printf("Return status = %ld\n", dbretstatus(dbproc));
			-- }
			-- else
			-- {
			--   printf("No return status for this command.\n");
			-- }
			*/

			MAKEIT_INT_FORMAT(datafmt);
			ret = ct_bind(cmd, 1, &datafmt,
				      (CS_VOID *)&sproc_status,
				      (CS_INT *)NULL, (CS_SMALLINT *)NULL);
			EXIT_ON_FAIL(context, ret,
				     "ct_bind() for return status failed.");

			while((fetch_ret = ct_fetch(cmd, CS_UNUSED, CS_UNUSED,
						     CS_UNUSED,(CS_INT *)NULL)) 
			      == CS_SUCCEED)
			{
				printf("\nReturn status = %ld\n\n",
				       (long)sproc_status);
			}
			/* Check for ct_fetch() errors. */
			if (fetch_ret != CS_END_DATA)
			{
				EXIT_ON_FAIL(context, CS_FAIL,
					     "ct_fetch() failed.");
			}

			break;

		  case CS_CMD_DONE:

			/* 
			** CS_CMD_DONE marks the boundary of a
			** 'logical command'. 
			** When a ASE stored procedure is invoked,
			** a result type of CS_CMD_DONE is returned once
			** after the execution of every select within 
			** he procedure, and once after the procedure 
			** inishes execution.
			*/
			break;

		  default:

			/*
			** Unexpected result type. In production code,
			** we might want to report an error, call ct_cancel(),
			** and continue. In this simple example, we just exit.
			*/
			fprintf(ERR_CH,
				"Unexpected result type = %ld\n",
				(long)result_type);
			EXIT_ON_FAIL(context, CS_FAIL,
				     "Unexpected result type.");

		} /* switch */  

	} /* while ct_results ... */

	
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
		EXIT_ON_FAIL(context, CS_FAIL, "ct_results() failed.");

	  default:
		/* Unexpected ct_results() return code. */
		EXIT_ON_FAIL(context, CS_FAIL, "unknown ct_results() status");

	} /* switch */

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

}

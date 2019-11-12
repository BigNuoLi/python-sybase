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
** 	ex04ct.c is converted from DB-Library's example4.c to use
**	Client-Library.
**
** 	This example accesses the data within each row without using dbbind(),
** 	and illustrates the use of row buffering.
**
** 	DB-Library's example4 runs a query, saves the last 1000 of the 
** 	returned rows using DB-Library row buffering, and allows the user to
** 	examine data rows (within the buffer) at random.
**
** 	Client-Library does not have any equivalent for DB-Library's row
** 	buffering feature. However, we can emulate it here using array
** 	binding. For array binding, the applications binds columns to 
** 	arrays of program variables. Each call to ct_fetch() fetches 
** 	enough rows to fill the arrays (or less if the end of the result
** 	set is hit or if a conversion error occurs).
**
** 	The code Client-Library array binding is considerably more complex 
** 	than code that uses DB-Library row buffering. Essentially, Client-
** 	Library requires the application to manage a row buffer itself.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
/*
** Client-Library requires ctpublic.h 
**
-- #include <sybfront.h>
-- #include <sybdb.h>
-- #include "sybdbex.h"
*/
#include <ctpublic.h>
#include "dbtoctex.h"

/*
** To perform array binding, we use the utility routines in ctrowbuf.c
*/
#include "ctrowbuf.h"

/*
** We rely on the error handlers and initialization functions in
** exutils.c
**
-- * Forward declarations of the error handler and message handler. *
-- int CS_PUBLIC err_handler();
-- int CS_PUBLIC msg_handler();
*/
#include <exutils.h>

/*
** We use dbct_parent_ctx() from dbctutil.c
*/
#include "dbctutil.h"

/*
** Prototype the CS-Library error handler used by this example.
** We call cs_convert in this example, and a CS-Library error
** handler is required to trap any cs_convert() errors.
*/
CS_STATIC CS_RETCODE CS_PUBLIC cslib_err_handler    PROTOTYPE((
      CS_CONTEXT    *cp,
      CS_CLIENTMSG  *msg
      ));

/*
** Private prototypes. DoBufferedSelect() and GetRows() replace
** code that was in the DB-Library example's main() function.
*/
CS_STATIC CS_RETCODE CS_INTERNAL DoBufferedSelect PROTOTYPE ((
   CS_CONNECTION *conn
   ));
CS_STATIC CS_RETCODE CS_INTERNAL GetRows PROTOTYPE ((
   CS_COMMAND          *cmd
   ));

#define TYPELEN 2

/*
** Define macros for simple return code checking within
** a function. ERR_CH is defined in dbtoctex.h
*/
#define ERROR_OUT(str) \
  { fprintf(ERR_CH, "%s\n", str); }
#define FAIL_ON_FAIL(ret, str) \
  { if (ret != CS_SUCCEED) \
  { \
   fprintf(ERR_CH, "Error (line %d): %s\n", (int)__LINE__, str); \
   return CS_FAIL; \
  } }

int
main(int argc, char *argv[])
{
	CS_CONTEXT	*context;
	CS_CONNECTION	*conn;
	CS_RETCODE	ret;

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
	** Initialize CS-Library and Client-Library. ex_init() is defined in
	** exutils.c; ex_init() also installs handlers for Client-Library
	** errors and server messages
	*/
	context = (CS_CONTEXT *) NULL;
	ret = ex_init(&context);
	EXIT_ON_FAIL(context, ret, "Initialization failed.");

	/* 
	** ex_init() does not install a CS-Library error handler. We need
	** to install one to trap cs_convert() errors. cslib_err_handler() is
	** defined at the bottom of this source file.
	*/ 
	ret = cs_config(context, CS_SET, CS_MESSAGE_CB, 
			(CS_VOID *)cslib_err_handler,
			CS_UNUSED, NULL);
	EXIT_ON_FAIL(context, ret, "Could not install CS-Lib error handler.");

	/*
	-- dlogin = dblogin();
	-- DBSETLUSER(dlogin, USER);
	-- DBSETLPWD(dlogin, PASSWORD);
	-- DBSETLAPP(dlogin, "example4");
	-- dbproc = dbopen(dlogin, NULL);
	*/

	/*
	** Connect to the server. ex_connect() is defined in exutils.c
	*/
	ret = ex_connect(context, &conn, "ex04ct", USER, PASSWORD, NULL);
	EXIT_ON_FAIL(context, ret, "Connect attempt failed");

	/*
	** Send the select command, buffer the rows, and let the
	** user view them.
	*/
	ret = DoBufferedSelect(conn);
	EXIT_ON_FAIL(context, ret, "DoBufferedSelect failed.");

	/*
	-- dbexit();
	*/

	/*
	** Cleanup. ex_con_cleanup() and ex_ctx_cleanup() are defined
	** in exutils.c
	*/
	ret = ex_con_cleanup(conn, CS_SUCCEED);
	ret = ex_ctx_cleanup(context, ret);
	EXIT_ON_FAIL(context, ret, "Cleanup failed");  

	exit(NORMAL_EXIT);
} /* main() */

/*
** DoBufferedSelect() -- Sends a SELECT language command to
**   against the sysobjects table and calls GetRows() to
**   fetch the rows and display them to the user.
**
** Parameters
**  conn -- open and idle connection.
**
** Returns
**  CS_SUCCEED or CS_FAIL
*/
CS_STATIC CS_RETCODE CS_INTERNAL
DoBufferedSelect(CS_CONNECTION *conn)
{
	CS_RETCODE	ret;
	CS_COMMAND	*cmd;
	CS_RETCODE	results_ret;
	CS_INT		result_type;

	/*
	** Allocate a command structure.
	*/
	ret = ct_cmd_alloc(conn, &cmd);
	FAIL_ON_FAIL(ret, "DoBufferedSelect: ct_cmd_alloc failed.");

	/*
	** Build and send the language command.
	*/
	ret = ct_command(cmd, CS_LANG_CMD, 
			 "select name, type, id, crdate from sysobjects where type = 'S'",
			 CS_NULLTERM, CS_UNUSED);
	FAIL_ON_FAIL(ret, "DoBufferedSelect: ct_command failed");
	ret = ct_send(cmd);
	FAIL_ON_FAIL(ret, "DoBufferedSelect: ct_send failed");

	/*
	** Handle the results. When result_type is CS_ROW_RESULT, then
	** use the ctrowbuffer.c routines to get the rows and interactively
	** display them.
	*/
	while ((results_ret = ct_results(cmd, &result_type)) == CS_SUCCEED)
	{
		switch ((int)result_type)
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
			** This means that the server encountered an error 
			** while processing our command.
			*/
			FAIL_ON_FAIL(CS_FAIL,
				     "DoBufferedSelect: command failed on server");

		  case CS_ROW_RESULT:
			/*
			** GetRows() retrieves and displays the rows.
			*/
			ret = GetRows(cmd);
			FAIL_ON_FAIL(ret,
				     "DoBufferedSelect: GetBufferedRows() failed.");
			break;

		  default:
			/*
			** We got an unexpected result type.
			*/
			FAIL_ON_FAIL(CS_FAIL,
				     "DoBufferedSelect: ct_results() returned unexpected result type");
		}
	}

	/*
	** We're done processing results. Let's check the
	** return value of ct_results() to see if everything
	** went ok.
	*/
	switch ((int)results_ret)
	{
	  case CS_END_RESULTS:
		/*
		** Everything went fine.
		*/
		break;

	  case CS_FAIL:
		/*
		** Something went wrong.
		*/
		FAIL_ON_FAIL(CS_FAIL, "DoBufferedSelect: ct_results() failed");

	  default:
		/*
		** We got an unexpected return value.
		*/
		FAIL_ON_FAIL(CS_FAIL, 
			     "DoBufferedSelect: ct_results() returned unexpected result code");
	}
 
	/*
	** Cleanup.
	*/
	ret = ct_cmd_drop(cmd);
	FAIL_ON_FAIL(ret, "DoBufferedSelect: ct_cmd_drop failed.");

	return CS_SUCCEED;

} /* DoBufferedSelect() */

/*
** GetRows() -- Uses the ctrowbuf.c routines to fetch rows into
**  a row buffer. Asks the user for a row number, then displays
**  the row.
**
** Parameters
**  cmd -- The command structure. The command should be in a state
**   to fetch the regular rows returned from the query.
**
** Returns
**  CS_SUCCEED or CS_FAIL
*/
CS_STATIC CS_RETCODE CS_INTERNAL
GetRows(CS_COMMAND *cmd)
{
	CS_CONTEXT	*context;
	CS_RETCODE	ret;
	CS_RETCODE	fetch_ret;
	ROW_BUFFER	*rowbuf;
	CS_INT		nfetched;
	CS_INT		ncols;
	/*
	** Declare equivalent variables for data retrieval.
	**
	-- DBCHAR             name[DBMAXNAME+1]; 
	-- DBCHAR             type[TYPELEN+1];
	-- DBINT              id;
	-- DBCHAR             datebuf[64];
	-- DBINT              len;
	-- char               numstring[32];
	-- int                quitflag = 0;
	-- RETCODE            row_code;
	-- int                rownum;
	*/
	CS_CHAR       name[CS_MAX_NAME + 1];
	CS_CHAR       type[TYPELEN+1];
	CS_INT        id;
#define DATEBUF_LEN  64
	CS_CHAR       datebuf[DATEBUF_LEN];
	CS_INT        len;
	CS_CHAR       numstring[32];
	CS_BOOL       quitflag = CS_FALSE;
	CS_INT        rownum;
	CS_DATAFMT    srcdatafmt;
	CS_DATAFMT    chardatafmt;
	CS_BYTE       *srcdata;

	/*
	** We will need the command structure's grandmama context to call
	** cs_convert(). dbct_parent_ctx() is defined in dbctutil.c
	*/
	ret = dbct_parent_ctx(cmd, (CS_CONNECTION *)NULL, &context);
	FAIL_ON_FAIL(ret, "GetRows: dbct_parent_ctx() failed.");

	/*
	** rb_createbuf() creates a "row buffer" consisting of value arrays
	** for each column in the result set. rb_createbuf() allocates these
	** arrays and binds to them using Client-Library's array binding 
	** feature.
	** 
	** We create arrays large enough to hold 1000 rows long. This is 
	** probably much larger than we need, but we use the same size as the 
	** DB-Library program.
	*/
	ret = rb_createbuf(cmd, 1000, &ncols, &rowbuf);
	FAIL_ON_FAIL(ret, "GetRows: rb_createbuf() failed");
	
	/*
	** rb_fillbuf() fills the row buffer with a single call to 
	** ct_fetch(). rb_fillbuf() sets nfetched to the number 
	** of valid rows in the buffer.
	**
	** NOTE: This code does not quite follow the logic of the DB-Library 
	** example. DB-Library's row buffering allowed the application
	** to selectively delete rows. The DB-Library example deleted the
	** "oldest" row in the buffer and kept the rest. We can't do that 
	** with array binding, so we'll just fetch up to 1000 rows and 
	** discard the remainder. If the query returns less than 1000 rows,
	** the behavior will be the same.
	**
	** For a more robust mechanism, either:
	** - save all the rows by using multiple buffers. Call rb_createbuf() 
	**   and rb_fillbuf() in a loop and save each filled row buffer.
	** - use one buffer, but proccess all the contents before refilling
	**   it.
	*/
	fetch_ret = rb_fillbuf(cmd, rowbuf, &nfetched);

	if ( fetch_ret == CS_SUCCEED || fetch_ret == CS_END_DATA )
	{
		/*
		** Discard any remaining rows.
		*/
		ret = ct_cancel((CS_CONNECTION *) NULL, cmd, CS_CANCEL_CURRENT);
		if (ret != CS_SUCCEED)
		{
			ret = rb_dropbuf(rowbuf);
			FAIL_ON_FAIL(CS_FAIL,
				     "GetRows: ct_cancel(CURRENT) failed.");
		}
	}
	else
	{
		/*
		** We include the CS_ROW_FAIL case here.
		*/
		ret = rb_dropbuf(rowbuf);
		FAIL_ON_FAIL(CS_FAIL, "GetRows: rb_fillbuf() failed.");
	}

	/*
	** Let the user view any row in the buffer.
	*/
	quitflag = CS_FALSE;
	while (quitflag == CS_FALSE)
	{
		printf("Type the number of the row you want to see (1 to %ld).\n",
		       (long) nfetched);
		printf("Asking for row 0 will terminate the program.\n");
		printf("Row number: ");
		fgets(numstring, CS_SIZEOF(numstring), stdin);
		rownum = atoi(numstring);
		if (rownum == 0)
		{
			quitflag = CS_TRUE;
		}
		else if (rownum < 0 || rownum > nfetched)
		{
			printf("That row is not in the table.\n");
		}
		else
		{
			/*
			** Print the requested row. Unlike DB-Library,
			** there is no concept of a "current row"
			** (as set by dbsetrow()). Instead, we just supply
			** the row number to the ctrowbuf.c routines.
			*/

			/*
			** Get the 'name' column (1). We assume it is
			** a CS_CHAR that fits into the name[] array.
			*/
			srcdata = rb_dataptr(rowbuf, rownum, 1);
			len = rb_datalen(rowbuf, rownum, 1);
			if (srcdata == (CS_BYTE *)NULL
			    || len > CS_MAX_NAME )
			{
				ret = rb_dropbuf(rowbuf);
				FAIL_ON_FAIL(CS_FAIL,
					     "GetRows: Bad 'name' column data.");
			}
			else
			{
				strncpy(name, (CS_CHAR *)srcdata, len);
				name[len] = '\0';
			}

			/*
			** Get the 'type' column (2). We assume it is
			** a CS_CHAR that fits into the type[] array.
			*/
			srcdata  = rb_dataptr(rowbuf, rownum, 2);
			len = rb_datalen(rowbuf, rownum, 2);
			if ((srcdata) == (CS_BYTE *)NULL
			    || (len) > TYPELEN )
			{
				ret = rb_dropbuf(rowbuf);
				FAIL_ON_FAIL(CS_FAIL,
					     "GetRows: Bad 'type' column data.");
			}
			else
			{
				strncpy(type, (CS_CHAR *)srcdata, len);
				type[len] = '\0';
			}

			/*
			** Get the 'id' column (3). We assume it's a CS_INT.
			*/
			srcdata = rb_dataptr(rowbuf, rownum, 3);
			len = rb_datalen(rowbuf, rownum, 3);
			if (srcdata == (CS_BYTE *) NULL
			    || len != CS_SIZEOF(CS_INT) )
			{
				ret = rb_dropbuf(rowbuf);
				FAIL_ON_FAIL(CS_FAIL,
					     "GetRows: Bad 'id' column data.");
			}        
			id = * ((CS_INT *) srcdata);

			/*
			** Get the 'crdate' column (4). We assume it's
			** a CS_DATETIME value, and we convert it
			** to character. rb_describe() returns the 
			** format of the column.
			*/
			ret = rb_describe(rowbuf, 4, &srcdatafmt);
			if (ret != CS_SUCCEED)
			{
				ret = rb_dropbuf(rowbuf);
				FAIL_ON_FAIL(CS_FAIL,
					     "GetRows: rb_describe() failed.");
			}

			srcdata = rb_dataptr(rowbuf, rownum, 4);
			srcdatafmt.maxlength = rb_datalen(rowbuf, rownum, 4);
			if (srcdata == (CS_BYTE *)NULL
			    || srcdatafmt.maxlength == 0)
			{
				ret = rb_dropbuf(rowbuf);
				FAIL_ON_FAIL(CS_FAIL,
					     "GetRows: Bad 'crdate' column data.");
			}

			chardatafmt.datatype = CS_CHAR_TYPE;
			chardatafmt.maxlength = DATEBUF_LEN;
			chardatafmt.format = CS_FMT_NULLTERM;
			chardatafmt.locale = (CS_LOCALE *)NULL;

			ret = cs_convert(context, &srcdatafmt,
					 (CS_VOID *)srcdata,
					 &chardatafmt, (CS_VOID *)datebuf,
					 (CS_INT *)NULL);
			if (ret != CS_SUCCEED)
			{
				ret = rb_dropbuf(rowbuf);
				FAIL_ON_FAIL(CS_FAIL,
					     "GetRows: cs_convert() failed.");
			}

			/*
			** Print out column headers.
			*/
			printf (" %-20s %10s %30s %15s \n",
				"       NAME         ",
				"TYPE",
				"             DATE             ",
				"ID");
			printf (" %20s %10s %30s %15s \n",
				"--------------------",
				"----",
				"---------------------------",
				"----");

			/*
			** Print out the row data.
			*/
			printf ("%20s %10s %30s %15ld \n",
				name, type, datebuf, (long)id);

		} /* else */
	} /* while */

	/*
	** Clean up. Drop the row buffer that rb_createbuf() allocated.
	*/
	ret = rb_dropbuf(rowbuf);
	FAIL_ON_FAIL(ret, "GetRows: rb_dropbuf() failed.");

	return CS_SUCCEED;

} /* GetRows() */

/*
**  cslib_err_handler() - CS-Library error handler.
**
**   This routine is the CS-Library error handler used by this
**   application. It is called by CS-Library whenever an error
**   occurs. Here, we simply print the error and return.
**
**  Parameters:
**      context         A pointer to the context handle for context
**                      on which the error occurred.
**
**      error_msg       The structure containing information about the
**      error.
**
**  Returns:
**      CS_SUCCEED
*/
CS_STATIC CS_RETCODE CS_PUBLIC
cslib_err_handler(CS_CONTEXT *context, CS_CLIENTMSG *errmsg)
{
	/*
	** Print the error details.
	*/
	fprintf(ERR_CH, "CS-Library error: ");
	fprintf(ERR_CH, "LAYER = (%ld) ORIGIN = (%ld) ",
		(long) CS_LAYER(errmsg->msgnumber), 
		(long) CS_ORIGIN(errmsg->msgnumber) );
	fprintf(ERR_CH, "SEVERITY = (%ld) NUMBER = (%ld)\n",
		(long) CS_SEVERITY(errmsg->msgnumber), 
		(long) CS_NUMBER(errmsg->msgnumber) ); 
	fprintf(ERR_CH, "\t%s\n", errmsg->msgstring);

	/*
	** Print any operating system error information.
	*/
	if( errmsg->osstringlen > 0 )
	{
		fprintf(ERR_CH, "CS-Library OS error %ld - %s.\n",
			(long) errmsg->osnumber, errmsg->osstring);
	}

	/*
	** All done.
	*/
	return (CS_SUCCEED);

} /* cslib_err_handler() */

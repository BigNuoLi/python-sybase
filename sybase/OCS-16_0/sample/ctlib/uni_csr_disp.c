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
** 	This is an unichar16 example program modified from csr_disp.c
** 	for unichar/univarchar datatype demo.
**
**	This example demonstrates using a read-only cursor.  It opens a cursor
**	with a canned query.  It processes the results using the standard
**	ct_results() while loop.  It binds the column values to program
**	variables.  It then fetches and displays the rows in the standard
**	ct_fetch() while loop.                                          
**
**	This is the canned query:
**		select au_fname, au_lname, postalcode from authors
**
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
**	This example uses a hard-coded query of the authors table in the
**	unipubs2 database. The query is defined by SELECT below
**
** Output
** ------
**	This example simply displays each row of the result set.
**
** Server Dependencies
** -------------------
**	This example requires ASE 12.5 or higher 
**	with unichar/univarchar datatype support.
**
** Server Tables
** -------------
**	This example relies on the unipubs2 database and the authors table.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctpublic.h>
#include "example.h"
#include "exutils.h"

/*****************************************************************************
** 
** defines and globals used.
** 
*****************************************************************************/
/*
** Global names used in this module
*/
CS_CHAR *Ex_appname  = "uni_csr_disp";
CS_CHAR *Ex_dbname   = "unipubs2";
CS_CHAR *Ex_server   = EX_SERVER;
CS_CHAR *Ex_username = EX_USERNAME;
CS_CHAR *Ex_password = EX_PASSWORD;


/*
** Define a sample select statement
*/
#define SELECT	"select au_fname, au_lname, postalcode from authors"

/*
** Prototypes for routines in uni_csr_disp.c
*/
CS_STATIC CS_RETCODE DoCursor PROTOTYPE((
	CS_CONNECTION *connection
	));

/*
** main()
** 
** Purpose:
**	Entry point for example program.
** 
** Parameters:
**	None, argc and argv will not be used.
**
** Return:
** 	EX_EXIT_ERROR  or EX_EXIT_SUCCEED
**
*/

int
main(int argc, char *argv[])
{
	CS_CONTEXT	*context;
	CS_CONNECTION	*connection;
	CS_RETCODE	retcode;

	EX_SCREEN_INIT();
	
	fprintf(stdout, "Unichar16 Cursor Example\n");
	fflush(stdout);

	/* 
	** Allocate a context structure and initialize Client-Library 
	*/
	retcode = ex_init(&context);
	if (retcode != CS_SUCCEED)
	{
		ex_panic("ex_init failed");
	}

	/* 
	** Allocate a connection structure, set its properties, and  
	** establish a connection.
	*/
	retcode = ex_connect(context, &connection, Ex_appname,
				Ex_username, Ex_password, Ex_server);

	/*
	** Execute the routines for the cursor sample
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = DoCursor(connection);
	}

	/*
	** Deallocate the allocated structures, close the connection,
	** and exit Client-Library.
	*/
	if (connection != NULL)
	{
		retcode = ex_con_cleanup(connection, retcode);
	}
	
	if (context != NULL)
	{
		retcode = ex_ctx_cleanup(context, retcode);
	}

	return (retcode == CS_SUCCEED) ? EX_EXIT_SUCCEED : EX_EXIT_FAIL;
}

/*
** DoCursor(connection)
**
** Type of function:
** 	cursor program internal api
**
** Purpose:
**	This function is the core of the cursor example.  It declares
**	and opens a cursor.  It then processes the results in the 
**	standard ct_results() loop, calling ex_fetch_data()	
**	when the type of result is CS_CURSOR_RESULT.
**
** Parameters:
**	connection - Pointer to connection structure
**
** Return:
**	CS_SUCCEED if compute result set was processed correctly
**	Otherwise a Client-Library failure code.
**
*/

CS_STATIC CS_RETCODE
DoCursor(CS_CONNECTION *connection)
{
	CS_RETCODE	retcode;
	CS_COMMAND	*cmd;	
	CS_INT		res_type;

	/*
	** NOTE: Add capability check here
	*/

	/*
	** Use the unipubs2 database
	*/
        if ((retcode = ex_use_db(connection, Ex_dbname)) != CS_SUCCEED)
        {
                ex_error("DoCursor: ex_use_db(unipubs2) failed");
                return retcode;
        }

	/*
	** Allocate a command handle to declare the cursor on.
	*/
        if ((retcode = ct_cmd_alloc(connection, &cmd)) != CS_SUCCEED)
        {
                ex_error("DoCursor: ct_cmd_alloc() failed");
                return retcode;
        }

	/*
	** Declare the cursor
	** SELECT is a select statement defined in the header file.
	*/
	retcode = ct_cursor(cmd, CS_CURSOR_DECLARE, "cursor_a", CS_NULLTERM,
			SELECT, CS_NULLTERM, CS_READ_ONLY);
	if (retcode != CS_SUCCEED)
	{
                ex_error("DoCursor: ct_cursor(declare) failed");
		return retcode;
	}
	
	/*
	** Set cursor rows to 10
	*/
	retcode = ct_cursor(cmd, CS_CURSOR_ROWS, NULL, CS_UNUSED, NULL, 
			CS_UNUSED, (CS_INT)10);
	if (retcode != CS_SUCCEED)
	{
                ex_error("DoCursor: ct_cursor(currows) failed");
		return retcode;
	}

	/*
	** Open the cursor.  
	*/
	retcode = ct_cursor(cmd, CS_CURSOR_OPEN, NULL, CS_UNUSED, NULL,
			CS_UNUSED, CS_UNUSED);
	if (retcode != CS_SUCCEED)
	{
                ex_error("DoCursor: ct_cursor() failed");
		return retcode;
	}

	/*
	** Send (batch) the last 3 cursor commands to the server 
	*/
	if ((retcode = ct_send(cmd)) != CS_SUCCEED)
	{
                ex_error("DoCursor: ct_send() failed");
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
			ex_error("DoCursor: ct_results() returned CMD_FAIL");
			break;

		case CS_CURSOR_RESULT:
			/*
			** Binding the client CS_CHAR_TYPE to sever
			** unichar/univarchar will automaticly do the implicit
			** CS_UNICHAR_TYPE to CS_CHAR_TYPE conversion.
			*/
			retcode = ex_fetch_data(cmd);
			if (retcode != CS_SUCCEED)
			{
				ex_error("DoCursor: ex_fetch_data() failed");
				return retcode;
			}
			break;
 
		default:
			/*
			** We got an unexpected result type.
			*/
			ex_error("DoCursor: ct_results() returned unexpected result type");
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
			ex_error("DoCursor: ct_results() failed");
			return retcode;

		default:
			/*
			** We got an unexpected return value.
			*/
			ex_error("DoCursor: ct_results() returned unexpected result code");
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
                ex_error("DoCursor: ct_cursor(dealloc) failed");
		return retcode;
	}

	/*
	** Send the cursor command to the server 
	*/
	if ((retcode = ct_send(cmd)) != CS_SUCCEED)
	{
                ex_error("DoCursor: ct_send() failed");
		return retcode;
	}

	/*
	** Check its results.  The command won't generate a result set.
	*/
	while((retcode = ct_results(cmd, &res_type)) == CS_SUCCEED)
	{
		switch ((int)res_type)
		{
		    case CS_CMD_SUCCEED:
		    case CS_CMD_DONE:
			/*
			** These are the only two types we expect.
			*/
			break;
 
		    case CS_CMD_FAIL:
			/*
			** This means that the server encountered an error while
			** processing our command.
			*/
			ex_error("DoCursor: ct_results() returned CMD_FAIL");
			break;

		    default:
			/*
			** We got an unexpected result type.
			*/
			ex_error("DoCursor: ct_results() returned unexpected result type");
			return CS_FAIL;
		}
	}

	if (retcode != CS_END_RESULTS)
	{
		ex_error("DoCursor: ct_results() failed");
		return retcode;
	}

	/*
	** Drop the cursor's command structure
	*/
	if ((retcode = ct_cmd_drop(cmd)) != CS_SUCCEED)
	{
                ex_error("DoCompute: ct_cmd_drop() failed");
                return retcode;
	}

	return retcode;
}

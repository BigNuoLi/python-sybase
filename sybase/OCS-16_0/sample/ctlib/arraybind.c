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
**      This example demonstrates use of array binding in conjunction with a
**	CS_LANG_CMD initiated by ct_command(). It processes the results using
**      the standard ct_results() while loop. It binds the column values to
**      program arrays. It then fetches and displays the rows in the standard
**      ct_fetch() loop.
**
** Routines Used
** -------------
**
**
** Input
** -----
**	This example uses a hard-coded query of a hard-coded table in the pubs2
**	database.  The query is defined by a language command using a select
**	statement as defined below by SELECT.
**
** Output
** ------
**	This example displays the rows as they are returned from the server.
**	
** Server Dependencies
** -------------------
**	None.
**
** Server Tables
** -------------
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
CS_CHAR *Ex_appname  = "arraybind";
CS_CHAR	*Ex_dbname   = "pubs2";
CS_CHAR *Ex_server   = EX_SERVER;
CS_CHAR *Ex_username = EX_USERNAME;
CS_CHAR *Ex_password = EX_PASSWORD;

/*
** Global context structure
*/
CS_CONTEXT	*Ex_context;

/*
** Data structure for array binding. This is a contrived
** example -- a real program would be more flexible with
** the column length.
**
** #define ARRAY_BND_LEN	4
** #define MAX_COLUMN_LEN 	8
** 
** typedef struct _column_array {
**   CS_DATAFMT       datafmt;
**   CS_CHAR          value[ARRAY_BND_LEN][MAX_COLUMN_LEN];
**   CS_INT           valuelen[ARRAY_BND_LEN];
**   CS_SMALLINT      indicator[ARRAY_BND_LEN];
** } COLUMN_ARRAY;
** 
*/

CS_STATIC COLUMN_ARRAY k_values;
CS_STATIC COLUMN_ARRAY s_values;

#define SEP_LEN 2
#define PRINT_SEP(f) {int i;for (i = 0; i < SEP_LEN; i++) fputc(' ', f);}
#define TRUNC_SUBST "***"

/*
** Define a language batch to create a table with
** some character data.
*/
#define DROP_TABLE " \
if exists(select name from sysobjects where type = 'U' \
	  and name = 'sample_table') \
begin \
  drop table sample_table \
end \
"

#define CREATE_TABLE " \
create table sample_table ( \
   k    int, \
   s    varchar(32), \
   unique clustered(k) \
) \
"

#define POPULATE_TABLE " \
insert sample_table values (1, 'abc') \
insert sample_table values (2, 'def') \
insert sample_table values (3, 'ghi') \
insert sample_table values (4, 'kjl') \
insert sample_table values (5, 'mno') \
insert sample_table values (6, 'pqr') \
insert sample_table values (7, 'this value will overflow') \
insert sample_table values (8, 'vwx') \
insert sample_table values (9, 'yz') \
"

/*
** Define a sample select statment
*/
#define SELECT	"select k, s from sample_table"

/*
** Prototypes for routines in sample code
*/ 
CS_STATIC CS_RETCODE CS_INTERNAL DoQuery(CS_CONNECTION *connection);
CS_STATIC CS_RETCODE CS_INTERNAL FetchRows(CS_COMMAND *cmd);

/*
** main()
** 
** Purpose:
**	Entry point for example program.
** 
** Parameters:
**	None, argc and argv will not be used.
**
** Returns:
** 	EX_EXIT_ERROR  or EX_EXIT_SUCCEED
**
*/

int
main(int argc, char *argv[])
{
	CS_CONNECTION	*connection;
	CS_RETCODE	retcode;

	EX_SCREEN_INIT();
	
	fprintf(stdout, "Array Bind Example\n");
	fflush(stdout);

	/* 
	** Allocate a context structure and initialize Client-Library 
	*/
	retcode = ex_init(&Ex_context);
	if (retcode != CS_SUCCEED)
	{
		ex_panic("ex_init failed");
	}

	/* 
	** Allocate a connection structure, set its properties, and  
	** establish a connection.
	*/
	retcode = ex_connect(Ex_context, &connection, Ex_appname,
			     Ex_username, Ex_password, Ex_server);

	/*
	** Execute the routines for the compute sample
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = DoQuery(connection);
	}

	/*
	** Deallocate the allocated structures, close the connection,
	** and exit Client-Library.
	*/
	if (connection != NULL)
	{
		retcode = ex_con_cleanup(connection, retcode);
	}
	
	if (Ex_context != NULL)
	{
		retcode = ex_ctx_cleanup(Ex_context, retcode);
	}

	return (retcode == CS_SUCCEED) ? EX_EXIT_SUCCEED : EX_EXIT_FAIL;
}

/*
** DoQuery(connection)
**
** Type of function:
** 	compute program internal api
**
** Purpose:
**	This function is the core of the compute example.  It sends 
**	a select statment with a compute clause to the server.  It 
**	then processes the results in the standard ct_results() loop, 
**	calling ProcessRowResults()	when the type of result is 
**	CS_ROW_RESULT and calling ProcessComputeResults() when the
**	type of result is CS_COMPUTE_RESULT.
**	
** Parameters:
**	connection - Pointer to connection structure
**
** Return:
**	CS_SUCCEED if compute result set was processed correctly
**	Otherwise a Client-Library failure code.
**
*/
CS_STATIC CS_RETCODE CS_INTERNAL
DoQuery(CS_CONNECTION *connection)
{

	CS_RETCODE	retcode;
	CS_COMMAND	*cmd;
	CS_INT		res_type;	/* result type from ct_results */

	/*
	** Use the pubs2 database
	*/
	if ((retcode = ex_use_db(connection, Ex_dbname)) != CS_SUCCEED)
	{
		ex_error("DoQuery: ex_use_db(pubs2) failed");
		return retcode;
	}

	/* 
	** Setup the test table.
	*/
	if ((retcode = ex_execute_cmd(connection, DROP_TABLE)) != CS_SUCCEED)
	{
		ex_error("DoQuery: drop table failed");
		return retcode;
	}
	if ((retcode = ex_execute_cmd(connection, CREATE_TABLE)) != CS_SUCCEED)
	{
		ex_error("DoQuery: create table failed");
		return retcode;
	}
	if ((retcode = ex_execute_cmd(connection, POPULATE_TABLE)) != CS_SUCCEED)
	{
		ex_error("DoQuery: insert rows failed");
		return retcode;
	}

	/*
	** Allocate a command handle to send the query with
	*/
	if ((retcode = ct_cmd_alloc(connection, &cmd)) != CS_SUCCEED)
	{
		ex_error("DoQuery: ct_cmd_alloc() failed");
		return retcode;
	}

	/*
	** Define a language command.
	*/
	retcode = ct_command(cmd, CS_LANG_CMD, SELECT, CS_NULLTERM, CS_UNUSED);
	if (retcode != CS_SUCCEED)
	{
		ex_error("DoQuery: ct_command() failed");
		return retcode;
	}

	/*
	** Send the command to the server 
	*/
	if (ct_send(cmd) != CS_SUCCEED)
	{
		ex_error("DoQuery: ct_send() failed");
		return retcode;
	}
					
	/*
	** Process the results.  Loop while ct_results() returns CS_SUCCEED.
	*/
	while ((retcode = ct_results(cmd, &res_type)) == CS_SUCCEED)
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
			** This means that the server encountered an error 
			** while processing our command.
			*/
			ex_error("DoQuery: ct_results() returned CMD_FAIL");
			break;

		  case CS_ROW_RESULT:
			retcode = FetchRows(cmd);
			if (retcode != CS_SUCCEED)
			{
				ex_error("DoQuery: FetchRows() failed");
				return retcode;
			}
			break; 

		  default:
			/*
			** We got an unexpected result type.
			*/
			ex_error("DoQuery: ct_results() returned unexpected result type");
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
		** Something went wrong.
		*/
		ex_error("DoQuery: ct_results() failed");
		return retcode;

	  default:
		/*
		** We got an unexpected return value.
		*/
		ex_error("DoQuery: ct_results() returned unexpected result code");
		return retcode;
	}

	/*
	** Drop our command structure
	*/
	if ((retcode = ct_cmd_drop(cmd)) != CS_SUCCEED)
	{
		ex_error("DoQuery: ct_cmd_drop() failed");
		return retcode;
	}

	return retcode;
}

/*
** FetchRows()
**
** Type of function:
** 	compute program internal api
**
** Purpose:
** 	This function processes compute results. It is called by DoQuery()
** 	when the type of result is CS_COMPUTE_RESULT. We could have used 
**	ex_fetch_data(), but the display header would not have reflected
**	the compute operator used.
**
** Parameters:
** 	cmd - Pointer to command structure
**
** Return:
**	CS_MEM_ERROR	If an memory allocation failed.
**	CS_SUCCEED	If the data was displayed.
**	CS_FAIL		If no columns were present.
** 	<retcode>	Result of the Client-Library function if a failure was
**			returned.
*/
CS_STATIC CS_RETCODE CS_INTERNAL
FetchRows(CS_COMMAND *cmd)
{
	CS_RETCODE		retcode;
	CS_INT			i;
	CS_INT			j;
	CS_INT			row_count = 0;
	CS_INT			rows_read;
	CS_DATAFMT              *dfmt;

	/*
	** Bind to first column ('k')
	*/
	dfmt = &(k_values.datafmt);
	strcpy(dfmt->name, "k");
	dfmt->namelen = strlen(dfmt->name);
	dfmt->datatype = CS_CHAR_TYPE;
	dfmt->maxlength = MAX_COLUMN_LEN;
	dfmt->format = CS_FMT_NULLTERM;
	dfmt->count = ARRAY_BND_LEN;
	dfmt->locale = NULL;

	retcode = ct_bind(cmd, 1, dfmt,
			  (CS_VOID *)(k_values.value), (k_values.valuelen),
			  (k_values.indicator));
	if (retcode != CS_SUCCEED)
	{
		ex_error("FetchRows: ct_bind(1) failed");
		return CS_FAIL;
	}

	/*
	** Bind to second column ('s')
	*/
	dfmt = &(s_values.datafmt);
	strcpy(dfmt->name, "s");
	dfmt->namelen = strlen(dfmt->name);
	dfmt->datatype = CS_CHAR_TYPE;
	dfmt->maxlength = MAX_COLUMN_LEN;
	dfmt->format = CS_FMT_NULLTERM;
	dfmt->count = ARRAY_BND_LEN;
	dfmt->locale = NULL;

	retcode = ct_bind(cmd, 2, dfmt,
			  (CS_VOID *)(s_values.value), (s_values.valuelen),
			  (s_values.indicator));
	if (retcode != CS_SUCCEED)
	{
		ex_error("FetchRows: ct_bind(2) failed");
		return CS_FAIL;
	}

	/*
	** Display column header
	*/
	fprintf(stdout, "%s", k_values.datafmt.name);
	for (i = k_values.datafmt.namelen; 
	     i < k_values.datafmt.maxlength; 
	     i++)
	{
		fputc(' ', stdout);
	}
	PRINT_SEP(stdout);
	fprintf(stdout, "%s", s_values.datafmt.name);
	for (i = s_values.datafmt.namelen; 
	     i < s_values.datafmt.maxlength; 
	     i++)
	{
		fputc(' ', stdout);
	}
	fputc('\n', stdout);
	for (i = 0; i < k_values.datafmt.maxlength; i++)
	{
		fputc('-', stdout);
	}
	PRINT_SEP(stdout);
	for (i = 0; i < s_values.datafmt.maxlength; i++)
	{
		fputc('-', stdout);
	}
	fputc('\n', stdout);

	/*
	** Fetch the rows.  Loop while ct_fetch() returns CS_SUCCEED or 
	** CS_ROW_FAIL
	*/
	while (((retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, 
		&rows_read)) == CS_SUCCEED) || (retcode == CS_ROW_FAIL))
	{
		/*
		** Increment our row count by the number of rows just fetched.
		*/
		row_count = row_count + rows_read;

		/*
		** Check if we hit a recoverable error.
		*/
		if (retcode == CS_ROW_FAIL)
		{
			fprintf(stdout, 
				"Error on row %ld.\n",(long)row_count);
			fflush(stdout);
		}

		/*
		** We have a row. Loop through the columns displaying the
		** column values.
		*/
		fprintf(stdout, "%ld rows read\n", (long)rows_read);
		for (i = 0; i < rows_read; i++)
		{
			/*
			** Value for column 'k'
			*/
			if (k_values.indicator[i] <= 0)
			{
				/* Good data or null data */
				for (j = 0; j < k_values.valuelen[i]; j++)
				{
					fputc(k_values.value[i][j], stdout);
				}
				for (; j < k_values.datafmt.maxlength; j++)
				{
					fputc(' ', stdout);
				}
			}
			else
			{
				/* Buffer overflow */ 
				fprintf(stdout, "%s", TRUNC_SUBST);
				for (j = strlen(TRUNC_SUBST); 
					j < k_values.datafmt.maxlength; 
					j++)
				{
					fputc(' ', stdout);
				}
			}
			PRINT_SEP(stdout);

			/*
			** Value for column 's'
			*/
			if (s_values.indicator[i] <= 0)
			{
				/* Good data or null data */
				for (j = 0; j < s_values.valuelen[i]; j++)
				{
					fputc(s_values.value[i][j], stdout);
				}
				for (; j < s_values.datafmt.maxlength; j++)
				{
					fputc(' ', stdout);
				}
			}
			else
			{
				/* Buffer overflow */ 
				fprintf(stdout, "%s", TRUNC_SUBST);
				for (j = strlen(TRUNC_SUBST); 
					j < s_values.datafmt.maxlength; 
					j++)
				{
					fputc(' ', stdout);
				}
			}

			fputc('\n', stdout);


		 }
	}

	/*
	** We're done processing rows.  Let's check the final return
	** value of ct_fetch().
	*/
	switch ((int)retcode)
	{
	  case CS_END_DATA:
		/*
		** Everything went fine.
		*/
		fprintf(stdout, "All done processing compute rows.\n");
		fflush(stdout);
		retcode = CS_SUCCEED;
		break;

	  case CS_FAIL:
		/*
		** Something terrible happened.
		*/
		ex_error("FetchRows: ct_fetch() failed");
		break;

	  default:
		/*
		** We got an unexpected return value.
		*/
		ex_error("FetchRows: ct_fetch() returned an expected retcode");
		break;
	}

	return retcode;
}

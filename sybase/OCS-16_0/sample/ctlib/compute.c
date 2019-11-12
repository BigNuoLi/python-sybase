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
**	This example demonstrates processing compute results.  It sends a
**	canned query to the server via a language command.  It processes
**	the results using the standard ct_results() while loop.  It binds
**	the column values to program variables.  It then fetches and displays 
**	the rows in the standard ct_fetch() while loop.
**
**	This is the canned query:
**	select type, price from titles where type like "%cook" 
**	order by type, price 
**	compute sum(price) by type 
**	compute sum(price)
**
**	This query returns both regular rows and compute rows.  The compute
**	rows are generated by the 2 compute clauses.  The first compute
**	clause, 'compute sum(price) by type' generates a compute row each
**	time the value of type changes.  The second compute clause 'compute
**	sum(price)' generates one compute row which is the last to be 
**	returned.
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
**	ct_send()
**	ct_results()
**	ct_res_info()
**	ct_describe()
**	ct_compute_info()
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
**	This example uses a hard-coded query of the titles table in the pubs2
**	database.  The query is defined by SELECT
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
**	This example relies on the pubs2 database and the titles table.
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
CS_CHAR *Ex_appname  = "compute";
CS_CHAR	*Ex_dbname   = "pubs2";
CS_CHAR *Ex_server   = EX_SERVER;
CS_CHAR *Ex_username = EX_USERNAME;
CS_CHAR *Ex_password = EX_PASSWORD;


/*
** Global context structure
*/
CS_CONTEXT	*Ex_context;

/*
** Define a sample select statment
*/
#define SELECT	"select type, price from titles \
				where type like \"%cook\" \
				order by type, price \
				compute sum(price) by type \
				compute sum(price)"


/*
** Prototypes for routines in sample code
*/ 
CS_STATIC CS_RETCODE CS_INTERNAL DoCompute PROTOTYPE((
	CS_CONNECTION *connection
	));
CS_STATIC CS_RETCODE CS_INTERNAL FetchComputeResults PROTOTYPE((
	CS_COMMAND *cmd
	));
CS_STATIC CS_CHAR * CS_INTERNAL GetAggOp PROTOTYPE((
	CS_INT op
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
	
	fprintf(stdout, "Compute Example\n");
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
		retcode = DoCompute(connection);
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
** DoCompute(connection)
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
DoCompute(CS_CONNECTION *connection)
{

	CS_RETCODE	retcode;
	CS_COMMAND	*cmd;
	CS_INT		res_type;	/* result type from ct_results */

	/*
	** Use the pubs2 database
	*/
        if ((retcode = ex_use_db(connection, Ex_dbname)) != CS_SUCCEED)
        {
                ex_error("DoCompute: ex_use_db(pubs2) failed");
                return retcode;
        }

	/*
	** Allocate a command handle to send the compute query with
	*/
        if ((retcode = ct_cmd_alloc(connection, &cmd)) != CS_SUCCEED)
        {
                ex_error("DoCompute: ct_cmd_alloc() failed");
                return retcode;
        }

	/*
	** Define a language command that contains a compute clause.  SELECT
	** is a select statment defined in the header file.
	*/
	retcode = ct_command(cmd, CS_LANG_CMD, SELECT, CS_NULLTERM, CS_UNUSED);
	if (retcode != CS_SUCCEED)
	{
                ex_error("DoCompute: ct_command() failed");
                return retcode;
	}

	/*
	** Send the command to the server 
	*/
	if (ct_send(cmd) != CS_SUCCEED)
	{
                ex_error("DoCompute: ct_send() failed");
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
			** This means that the server encountered an error while
			** processing our command.
			*/
			ex_error("DoCompute: ct_results() returned CMD_FAIL");
			break;

		case CS_ROW_RESULT:
			retcode = ex_fetch_data(cmd);
			if (retcode != CS_SUCCEED)
			{
				ex_error("DoCompute: ex_fetch_data() failed");
				return retcode;
			}
			break;

		case CS_COMPUTE_RESULT:
			retcode = FetchComputeResults(cmd);
			if (retcode != CS_SUCCEED)
			{
				ex_error("DoCompute: FetchComputeResults() failed");
				return retcode;
			}
			break; 

		default:
			/*
			** We got an unexpected result type.
			*/
			ex_error("DoCompute: ct_results() returned unexpected result type");
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
			ex_error("DoCompute: ct_results() failed");
			return retcode;

		default:
			/*
			** We got an unexpected return value.
			*/
			ex_error("DoCompute: ct_results() returned unexpected result code");
			return retcode;
	}

	/*
	** Drop our command structure
	*/
	if ((retcode = ct_cmd_drop(cmd)) != CS_SUCCEED)
	{
                ex_error("DoCompute: ct_cmd_drop() failed");
                return retcode;
	}

	return retcode;
}

/*
** FetchComputeResults()
**
** Type of function:
** 	compute program internal api
**
** Purpose:
** 	This function processes compute results. It is called by DoCompute()
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
FetchComputeResults(CS_COMMAND *cmd)
{
	CS_RETCODE		retcode;
	CS_INT			num_cols;
	CS_INT			i;
	CS_INT			j;
	CS_INT			agg_op;
	CS_CHAR			*agg_op_name;
	CS_INT			row_count = 0;
	CS_INT			rows_read;
	CS_INT			outlen;
	CS_DATAFMT		*datafmt;
	EX_COLUMN_DATA		*coldata;

	/*
	** Find out how many columns there are in this result set.
	*/
	retcode = ct_res_info(cmd, CS_NUMDATA, &num_cols, CS_UNUSED, NULL);
	if (retcode != CS_SUCCEED)
	{
		ex_error("FetchComputeResults: ct_res_info() failed");
		return retcode;
	}

	/*
	** Make sure we have at least one column
	*/
	if (num_cols <= 0)
	{
		ex_error("FetchComputeResults: ct_res_info() returned zero columns");
		return CS_FAIL;
	}

	/*
	** Our program variable, called 'coldata', is an array of 
	** EX_COLUMN_DATA structures. Each array element represents
	** one column.  Each array element will re-used for each row.
	**
	** First, allocate memory for the data element to process.
	*/
	coldata = (EX_COLUMN_DATA *)malloc(num_cols * sizeof (EX_COLUMN_DATA));
	if (coldata == NULL)
	{
		ex_error("FetchComputeResults: malloc() failed");
		return CS_MEM_ERROR;
	}

	datafmt = (CS_DATAFMT *)malloc(num_cols * sizeof (CS_DATAFMT));
	if (datafmt == NULL)
	{
		ex_error("FetchComputeResults: malloc() failed");
		free(coldata);
		return CS_MEM_ERROR;
	}

	/*
	** Loop through the columns getting a description of each one
	** and binding each one to a program variable.
	**
	** We're going to bind each column to the native ASE type.
	** When fetching, we'll call ex_display_column() to perform the
	** conversion and print out the column value. The column could have
	** also been bound to a string, and have Client-Library perform the
	** conversion for you. The ex_fetch_data() routine shows this approach.
	**
	** We're going to use the same datafmt structure for both the describe
	** and the subsequent bind.
	**
	** If an error occurs within the for loop, a break is used to get out
	** of the loop and the data that was allocated is free'd before
	** returning.
	*/
	for (i = 0; i < num_cols; i++)
	{
		/*
		** Get the column description.  ct_describe() fills the
		** datafmt parameter with a description of the column.
		*/
		retcode = ct_describe(cmd, (i + 1), &datafmt[i]);
		if (retcode != CS_SUCCEED)
		{
			ex_error("FetchComputeResults: ct_describe() failed");
			break;
		}

		/*
		** Let's find out what aggregate operator was used for this
		** column.
		*/
		retcode = ct_compute_info(cmd, CS_COMP_OP, (i + 1), &agg_op,
				CS_UNUSED, &outlen);
		if (retcode != CS_SUCCEED)
		{
			ex_error("FetchComputeResults: ct_compute_info() failed");
			return retcode;
		}

		/*
		** Get a string that represents the aggregate operator.
  		*/
		agg_op_name = GetAggOp(agg_op);

		/*
		** Copy it into the name portion of the datafmt structure.
		** The ex_display_header() routine will then use this as
		** the name to display.
		*/
		strcpy(datafmt[i].name, agg_op_name);

		/*
		** Allocate memory for the column data element based on the
		** above describe.
		*/
		coldata[i].value = (CS_CHAR *)malloc(datafmt[i].maxlength);
		if (coldata[i].value == NULL)
		{
			ex_error("FetchComputeResults: malloc() failed");
			retcode = CS_MEM_ERROR;
			break;
		}

		/*
		** Now bind.
		*/
		retcode = ct_bind(cmd, (i + 1), &datafmt[i],
				coldata[i].value, &coldata[i].valuelen,
				(CS_SMALLINT *)&coldata[i].indicator);
		if (retcode != CS_SUCCEED)
		{
			ex_error("FetchComputeResults: ct_describe() failed");
			break;
		}
	}
	if (retcode != CS_SUCCEED)
	{
		for (j = 0; j < i; j++)
		{
			free(coldata[j].value);
		}
		free(coldata);
		free(datafmt);
		return retcode;
	}

	/*
	** Display column header
	*/
	ex_display_header(num_cols, datafmt);

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
			fprintf(stdout, "Error on row %d.\n", row_count);
			fflush(stdout);
		}

		/*
		** We have a row.  Loop through the columns displaying the
		** column values.
		*/
		for (i = 0; i < num_cols; i++)
		{	  
			ex_display_column(Ex_context, &datafmt[i],
						coldata[i].value,
						coldata[i].valuelen,
						coldata[i].indicator);
		} 
		fprintf(stdout, "\n");
		fflush(stdout);
	}

	/*
	** Free allocated space.
	*/
	for (i = 0; i < num_cols; i++)
	{
		free(coldata[i].value);
	}
	free(coldata);
	free(datafmt);

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
			ex_error("FetchComputeResults: ct_fetch() failed");
			break;

		default:
			/*
			** We got an unexpected return value.
			*/
			ex_error("FetchComputeResults: ct_fetch() returned an expected retcode");
			break;

	}
	return retcode;
}

/*
** GetAggOp(op)
**
** Purpose:
** 	This function takes an aggregate operator type and returns
** 	a string representing that type.  E.g. if the type is 
** 	CS_OP_SUM, the returned string is "sum".
**
** Parameters:
** 	op - the aggregate operator type (a symbolic value)
**
** Return:
**	A pointer to the string name.
*/
CS_STATIC CS_CHAR * CS_INTERNAL 
GetAggOp(CS_INT op)
{
	CS_CHAR		*name;

	switch ((int)op)
	{
		case CS_OP_SUM:
			name = "sum";
			break;

		case CS_OP_AVG:
			name = "avg";
			break;

		case CS_OP_COUNT:
			name = "count";
			break;

		case CS_OP_MIN:
			name = "min";
			break;

		case CS_OP_MAX:
			name = "max";
			break;

		default:
			name = "unknown";
			break;
	}
	return name;
}


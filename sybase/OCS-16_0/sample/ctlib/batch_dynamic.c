/*
** batch_dynamic.c
**
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
** This example demonstrates the use of ct_send_params(). It uploads some sample
** data into a table using a dynamic statement with batched parameters.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctpublic.h>
#include "example.h"
#include "exutils.h"

/*
** A simple application data structure containing some data that needs
** to be sent to the server.
*/
typedef struct _my_data
{
	CS_INT	number;
	CS_CHAR *string;
} MY_DATA;

/*
** send_array
**
** Sends an array of structures to the server.
*/
CS_RETCODE
send_array(CS_COMMAND *cmd, CS_CHAR *dyn_id, MY_DATA da[], CS_INT count)
{
	CS_RETCODE	retcode;
	CS_INT		i;
	CS_DATAFMT	datafmt1;
	CS_DATAFMT	datafmt2;
	CS_INT		unused = CS_UNUSED;
	CS_INT		nullterm = CS_NULLTERM;
	CS_SMALLINT	gooddata = CS_GOODDATA;


	/*
	** We execute the dynamic statement identified by 'dyn_id'
	*/
	retcode = ct_dynamic(cmd, CS_EXECUTE, dyn_id, CS_NULLTERM, NULL,
		CS_UNUSED);
	EX_RETURN_IF(retcode, "ct_dynamic(CS_EXECUTE) failed");

	/*
	** Describe the parameters
	*/
	memset(&datafmt1, 0, sizeof(CS_DATAFMT));
	datafmt1.namelen = CS_NULLTERM;
	datafmt1.datatype = CS_INT_TYPE;
	datafmt1.status = CS_INPUTVALUE;


	memset(&datafmt2, 0, sizeof(CS_DATAFMT));
	datafmt2.namelen = CS_NULLTERM;
	datafmt2.datatype = CS_CHAR_TYPE;
	datafmt2.maxlength = 100;
	datafmt2.status = CS_INPUTVALUE;

	/*
	** Initial bind to provide the datafmt to CT-Lib,
	*/
	retcode = ct_setparam(cmd, &datafmt1, NULL, NULL, NULL);
	EX_RETURN_IF(retcode, "ct_setparam(datafmt1) failed");

	retcode = ct_setparam(cmd, &datafmt2, NULL, NULL, NULL);
	EX_RETURN_IF(retcode, "ct_setparam(datafmt2) failed");

	for (i = 0; i < count; i++)
	{
		printf("Sending: %i, %s\n", da[i].number, da[i].string);

		/*
		** We need to (re)bind the parameters to the current location
		** in the array.
		*/
		retcode = ct_setparam(cmd, NULL, &da[i].number, &unused,
			&gooddata);
		EX_RETURN_IF(retcode, "ct_setparam()");

		retcode = ct_setparam(cmd, NULL, da[i].string, &nullterm,
			&gooddata);
		EX_RETURN_IF(retcode, "ct_setparam()");

		/*
		** Send the parameter. This also starts sending the command if
		** it's the first set of parameters.
		*/
		retcode = ct_send_params(cmd, CS_UNUSED);
		EX_RETURN_IF(retcode, "ct_send_params");
	}

	/*
	** We have sent all the data. Now call ct_send() to indicate we are
	** done.  This ends sending the command to the server.  Note that this
	** ct_send() call will not send any (additional) parameters.
	*/
	retcode = ct_send(cmd);
	EX_RETURN_IF(retcode, "ct_send() failed");

	/*
	** Handle the results
	*/
	retcode = ex_handle_results(cmd);
	EX_RETURN_IF(retcode, "ex_handle_results() failed");

	return CS_SUCCEED;
}

/*
** main()
**	Entry point for this example program.
** 
** Returns:
** 	EX_EXIT_ERROR or EX_EXIT_SUCCEED
*/
int
main(int argc, char *argv[])
{
	CS_RETCODE	retcode;
	CS_CONTEXT	*context = NULL;
	CS_CONNECTION	*connection = NULL;
	CS_COMMAND	*cmd = NULL;

	MY_DATA data_insert1[] = {{1, "One"}, {2, "Two"}, {3, "Tree"}};
	MY_DATA data_insert2[] = {{4, "For"}, {5, "Five"}};
	MY_DATA data_delete[] = {{3, "Tree"}, {4, "For"}};
	MY_DATA data_insert3[] = {{3, "Three"}, {4, "Four"}};
	MY_DATA data_insert4[] = {{6, "Six"}, {7, "Seven"}, {8, "Eight"},
				  {9, "Nine"},  {10, "Ten"}};
	
	/* 
	** Allocate a context structure and initialize Client-Library 
	*/
	retcode = ex_init(&context);
	EX_EXIT_IF(retcode, "ex_init failed");

	/* 
	** Establish a connection.
	*/
	retcode = ex_connect(context, &connection, "send_params example",
		EX_USERNAME, EX_PASSWORD, EX_SERVER);
	EX_EXIT_IF(retcode, "ex_connect failed");

	/*
	** This sample uses data in tempdb
	*/
	retcode = ex_use_db(connection, "tempdb");
	EX_EXIT_IF(retcode, "ex_use_db(tempdb) failed");

	/*
	** Create a simple table to contain our data.
	*/
	retcode = ex_execute_cmd(connection,
		"if exists (select name from sysobjects where "
		"name = \"test_batch_dynamic\") drop table test_batch_dynamic");
	EX_EXIT_IF(retcode, "ex_execute_cmd() failed");
	retcode = ex_execute_cmd(connection, "create table test_batch_dynamic "
		"(number unsigned int, string varchar(100) null)");
	EX_EXIT_IF(retcode, "ex_execute_cmd() failed");

	/*
	** Allocate a command structure.
	*/
	retcode = ct_cmd_alloc(connection, &cmd);
	EX_EXIT_IF(retcode, "ct_cmd_alloc() failed");

	/*
	** Prepare the sql statements. First the insert:
	*/
	retcode = ct_dynamic(cmd, CS_PREPARE, "my_insert", CS_NULLTERM,
		"insert into test_batch_dynamic values (?, ?)", CS_NULLTERM);
	EX_EXIT_IF(retcode, "ct_dynamic(CS_PREPARE) failed");

	retcode = ct_send(cmd);
	EX_EXIT_IF(retcode, "ct_send() failed after ct_dynamic(CS_PREPARE)");

	retcode = ex_handle_results(cmd);
	EX_EXIT_IF(retcode, "ex_handle_results failed");

	/*
	** Now a delete statement:
	*/
	retcode = ct_dynamic(cmd, CS_PREPARE, "my_delete", CS_NULLTERM,
		"delete test_batch_dynamic where number = ? and string = ?",
		CS_NULLTERM);
	EX_EXIT_IF(retcode, "ct_dynamic(CS_PREPARE) failed");

	retcode = ct_send(cmd);
	EX_EXIT_IF(retcode, "ct_send() failed after ct_dynamic(CS_PREPARE)");

	retcode = ex_handle_results(cmd);
	EX_EXIT_IF(retcode, "ex_handle_results failed");

	/*
	** Execute the insert for the entire array of structures
	*/
	retcode = send_array(cmd, "my_insert", data_insert1,
		sizeof(data_insert1)/sizeof(MY_DATA));
	EX_EXIT_IF(retcode, "send_array() failed");

	/*
	** Now another array:
	*/
	retcode = send_array(cmd, "my_insert", data_insert2,
		sizeof(data_insert2)/sizeof(MY_DATA));
	EX_EXIT_IF(retcode, "send_array() failed");

	/*
	** Now we send another query, to delete the faulty values
	*/
	retcode = send_array(cmd, "my_delete", data_delete,
		sizeof(data_delete)/sizeof(MY_DATA));
	EX_EXIT_IF(retcode, "send_array() failed");

	/*
	** And one to correct them.
	*/
	retcode = send_array(cmd, "my_insert", data_insert3,
		sizeof(data_insert3)/sizeof(MY_DATA));
	EX_EXIT_IF(retcode, "send_array() failed");

	/*
	** And some more just for fun.
	*/
	retcode = send_array(cmd, "my_insert", data_insert4,
		sizeof(data_insert4)/sizeof(MY_DATA));
	EX_EXIT_IF(retcode, "send_array() failed");

	/*
	** Show the resulting data from the table
	*/
	retcode = ct_command(cmd, CS_LANG_CMD, "select * from "
		"test_batch_dynamic order by number", CS_NULLTERM, CS_END);
	EX_EXIT_IF(retcode, "ct_command(CS_LANG_CMD)");

	retcode = ct_send(cmd);
	EX_EXIT_IF(retcode, "ct_send()");

	retcode = ex_handle_results(cmd);
	EX_EXIT_IF(retcode, "ex_handle_results()");

	/*
	** Deallocate the prepared statements
	*/
	retcode = ct_dynamic(cmd, CS_DEALLOC, "my_insert", CS_NULLTERM, NULL,
		CS_UNUSED);
	EX_EXIT_IF(retcode, "ct_dynamic(CS_DEALLOC, my_insert) failed.");
	retcode = ct_send(cmd);
	EX_EXIT_IF(retcode, "ct_send() failed.");
	retcode = ex_handle_results(cmd);
	EX_EXIT_IF(retcode, "ex_handle_results() failed.");

	retcode = ct_dynamic(cmd, CS_DEALLOC, "my_delete", CS_NULLTERM, NULL,
		CS_UNUSED);
	EX_EXIT_IF(retcode, "ct_dynamic(CS_DEALLOC, my_delete) failed.");
	retcode = ct_send(cmd);
	EX_EXIT_IF(retcode, "ct_send() failed.");
	retcode = ex_handle_results(cmd);
	EX_EXIT_IF(retcode, "ex_handle_results() failed.");

	/*
	** Deallocate the allocated structures, close the connection,
	** and exit Client-Library.
	*/
	retcode = ct_cmd_drop(cmd);
	retcode = ex_con_cleanup(connection, retcode);
	retcode = ex_ctx_cleanup(context, retcode);

	if (retcode != CS_SUCCEED)
	{
		return EX_EXIT_FAIL;
	}

	return EX_EXIT_SUCCEED;
}


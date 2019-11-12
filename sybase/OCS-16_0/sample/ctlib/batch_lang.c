/*
** batch_lang.c
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
** This example demonstrates the use of ct_send_params(). It uploads a text file
** into a table using a language command with batched parameterized.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctpublic.h>
#include "example.h"
#include "exutils.h"

#define MAX_LINE 100


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
** 	EX_EXIT_ERROR or EX_EXIT_SUCCEED
**
*/
int
main(int argc, char *argv[])
{
	CS_RETCODE	retcode;
	CS_CONTEXT	*context = NULL;
	CS_CONNECTION	*connection = NULL;
	CS_COMMAND	*cmd = NULL;

	char *filename = "lopsum.txt";
	FILE *file;

	CS_DATAFMT	datafmt1;
	CS_DATAFMT	datafmt2;
	CS_INT		unused = CS_UNUSED;
	CS_SMALLINT	gooddata = CS_GOODDATA;
	CS_CHAR		sqlcommand[CS_MAX_CHAR];

	CS_CHAR		linedata[MAX_LINE];
	CS_UINT		linenum;
	CS_INT		linelen;
	
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
	** This sample stores the sample data in tempdb.
	*/
	retcode = ex_use_db(connection, "tempdb");
	EX_EXIT_IF(retcode, "ex_use_db(tempdb) failed");

	ex_execute_cmd(connection, "if exists (select name from sysobjects "
		"where name = \"test_batch_lang\") drop table test_batch_lang");
	EX_EXIT_IF(retcode, "ex_execute_cmd() failed");
	cs_snprintf(sqlcommand, sizeof(sqlcommand),
		"create table test_batch_lang (filename varchar(30), "
		"linenum unsigned int, linedata varchar(%i) null)", MAX_LINE);
	ex_execute_cmd(connection, sqlcommand);
	EX_EXIT_IF(retcode, "ex_execute_cmd() failed");

	/*
	** Open the file
	*/
	file = fopen(filename, "r");
	EX_EXIT_IF(retcode, "fopen(filename) failed");

	/*
	** Allocate a command structure.
	*/
	retcode = ct_cmd_alloc(connection, &cmd);
	EX_EXIT_IF(retcode, "ct_cmd_alloc() failed");

	/*
	** Define the language command.
	** The filename is passed as part of the command since it is the same
	** for all rows in our command.
	** Linenumber and line data are described below and sent as parameters.
	*/
	cs_snprintf(sqlcommand, sizeof(sqlcommand), "insert into "
		"test_batch_lang values ('%s', @linenum, @linedata)", filename);
	retcode = ct_command(cmd, CS_LANG_CMD, sqlcommand, CS_NULLTERM, CS_END);
	EX_EXIT_IF(retcode, "ct_command(CS_LANG_CMD) failed");

	/*
	** Describe the linenum parameter.
	*/
	memset(&datafmt1, 0, sizeof(CS_DATAFMT));
	strcpy(datafmt1.name, "@linenum");
	datafmt1.namelen = CS_NULLTERM;
	datafmt1.datatype = CS_UINT_TYPE;
	datafmt1.maxlength = sizeof(linenum);
	datafmt1.status = CS_INPUTVALUE;

	/*
	** Describe the linedata parameter.
	*/
	memset(&datafmt2, 0, sizeof(CS_DATAFMT));
	strcpy(datafmt2.name, "@linedata");
	datafmt2.namelen = CS_NULLTERM;
	datafmt2.datatype = CS_CHAR_TYPE;
	datafmt2.maxlength = sizeof(linedata);
	datafmt2.status = CS_INPUTVALUE;

	gooddata = CS_GOODDATA; /* Not CS_NULLDATA */

	/*
	** ct_setparam copies the location of the buffer. This means we do not
	** have to call it again if we are using the same buffer for the next
	** set of parameters.
	*/
	retcode = ct_setparam(cmd, &datafmt1, &linenum, &unused, &gooddata);
	EX_EXIT_IF(retcode, "ct_setparam(1) failed");


	retcode = ct_setparam(cmd, &datafmt2, &linedata, &linelen, &gooddata);
	EX_EXIT_IF(retcode, "ct_setparam(2) failed");

	linenum = 0; 

	/*
	** Read the lines from the file and process them.
	*/
	while (fgets(linedata, sizeof(linedata), file) != NULL)
	{
		/*
		** Strip the newlines. As an example.
		*/
		linelen = strlen(linedata);
		if (linelen && (linedata[linelen - 1] == '\n'))
		{
			linelen--;
		}

		/*
		** Set the linenumber. Line numbers start at 1
		*/
		linenum++;

		/*
		** Send the parameters. This call reads and sends the bound
		** parameters (&linenum and &linedata).
		** This first sends the command itself to the server if this is
		** the first set of parameters for this command.
		*/
		retcode = ct_send_params(cmd, CS_UNUSED);
		EX_EXIT_IF(retcode, "ct_send_params() failed");
	}

	/*
	** We have sent all the lines of text from the file. Now call ct_send()
	** to indicate we are done.  This ends sending the command to the
	** server.  Note that this ct_send() call will not send any (additional)
	** parameters.
	*/
	retcode = ct_send(cmd);
	EX_EXIT_IF(retcode, "ct_send() failed");

	retcode = ex_handle_results(cmd);
	EX_EXIT_IF(retcode, "ex_handle_results() failed");

	printf("Done sending %i lines\n", linenum);

	cs_snprintf(sqlcommand, sizeof(sqlcommand),
		"select count(linenum) lines, "
		"count(linenum) - count(linedata) empty_lines, "
		"sum(len(linedata)) sum_len, avg(len(linedata)) avg_len "
		"from tempdb..test_batch_lang where filename = '%s'", filename);
	retcode = ct_command(cmd, CS_LANG_CMD, sqlcommand, CS_NULLTERM,
		CS_UNUSED);
	retcode = ct_send(cmd);

	/*
	** Handle the results from the server.
	*/
	retcode = ex_handle_results(cmd);
	EX_EXIT_IF(retcode, "ex_handle_results() failed");

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


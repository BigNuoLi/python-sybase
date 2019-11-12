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
**	This example demonstrates how to send CS_TEXT_TYPE and CS_IMAGE_TYPE
**	parameters in chunks in a dynamic SQL statement to a server.
**	This sample can be run against ASE 15.7 or up, or against Open Server 
**	sample 'paramreader'. The sample will create a table 'LOBtable',
**	insert two rows into the table and the use a dynamic select statement
**	to run a couple of queries on the table.
**
** Inputs
** ------
**	None.
**
** Server Tables
** -------------
**	LOBtable, created and dropped in this example.
** 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctpublic.h>
#include "example.h"
#include "exutils.h"

#define MAXCOLUMNS	2
#define MAXSTRING	80
#define MAXBUF		2048
#define ERR_CH		stderr
#define CHUNKSIZE1	5
#define CHUNKSIZE2	20

/*
** Username and password are defined in example.h
*/
CS_CHAR *Ex_username = EX_USERNAME;
CS_CHAR *Ex_password = EX_PASSWORD;
CS_CHAR *Ex_tabname = "LOBtable";

CS_CHAR *text1 = "'The Open Client Client-Library/C Reference Manual, contains \
reference information for the C version of Open Client Client-Library. \
This manual is a reference manual for programmers who are writing \
Client-Library applications. It is written for application programmers \
who are familiar with the C programming language. \
 Use this manual as a source of reference information, when you are \
writing a Client-Library application. \
Chapter 1, Introducing Client-Library, contains a brief \
introduction to Client-Library. \
Chapter 2, Client-Library Topics, contains information on how to \
accomplish specific programming tasks, such as using Client-Library \
routines to read a text or image value from the server. This chapter \
also contains information on Client-Library structures, options, error \
messages, and conventions.'";

CS_CHAR *text2 = "'The Open Server Server-Library/C Reference Manual, \
contains reference information for the C version of Open Server Server-Library. \
Audience The Open Server Server-Library/C Reference Manual is designed as a \
reference manual for programmers who are writing Open Server \
applications. It is written for application programmers who are familiar \
with the C programming language. \
How to use this book When writing an Open Server application, use the \
Open Server Server-Library/C Reference Manual as a source of reference information. \
Chapter 1, Introducing Open Server, contains a brief introduction to Open Server. \
Chapter 2, Topics, contains information on how to accomplish specific \
programming tasks, such as using Server-Library routines to read a text \
or image value from the server. This chapter also contains information on \
Open Server structures, programming techniques, and error handling.'";


/*
** Callback routines for library errors and server messages.
*/
CS_RETCODE CS_PUBLIC
csmsg_callback(CS_CONTEXT *context, CS_CLIENTMSG *clientmsg);
CS_RETCODE CS_PUBLIC
clientmsg_callback(CS_CONTEXT *context, CS_CONNECTION *connection,
		CS_CLIENTMSG *clientmsg);
CS_RETCODE CS_PUBLIC
servermsg_callback(CS_CONTEXT *context, CS_CONNECTION *connection,
		CS_SERVERMSG *servermsg);

CS_RETCODE CS_PUBLIC create_table(CS_CONNECTION *connection);
CS_RETCODE CS_PUBLIC drop_table(CS_CONNECTION *connection);
CS_RETCODE CS_PUBLIC handle_results(CS_COMMAND *cmd);


/*
** Main entry point for the program.
*/
int 
main(int argc, char *argv[])
{
	CS_CONTEXT         *context;	/* Context structure     */
	CS_CONNECTION      *connection;	/* Connection structure. */
	CS_COMMAND         *cmd;	/* Command structure.    */

	/*
	** Data format structures for column descriptions:
	*/
	CS_RETCODE	ret;
	CS_CHAR		cmd_text[MAXSTRING];
	CS_DATAFMT	fmt;
	CS_CHAR		buf[MAXSTRING];
	CS_CHAR		blobbuf[MAXBUF];
	CS_CHAR		*bufp;
	CS_INT		bytesleft;
	CS_INT		sendbytes;
	CS_BOOL		lobparamsupport;

	/*
	** Initialize the application.
	** First allocate a context structure.
	*/
	context = NULL;
	if ((ret = cs_ctx_alloc(EX_CTLIB_VERSION, &context)) != CS_SUCCEED)
	{
		ex_error("cs_ctx_alloc failed");
		return ret;
	}

	/*
	** Initialize Client-Library.
	*/
	if ((ret = ct_init(context, EX_CTLIB_VERSION)) != CS_SUCCEED)
	{
		ex_error("ct_init failed");
		return ret;
	}

	/*
	** Set up the error handling. Install callback handlers
	** for CS-Library errors, Client-Library errors and Server
	** messages.
	** Install a callback function to handle CS-Library errors.
	*/
	if ((ret = cs_config(context, CS_SET, CS_MESSAGE_CB, (CS_VOID *)csmsg_callback,
			CS_UNUSED, NULL)) != CS_SUCCEED)
	{
		ex_error("cs_config(CS_MESSAGE_CB) failed");
		return ret;
	}

	/*
	** Install a callback function to handle Client-Library errors.
	** 
	** The client message callback receives error or informational
	** messages discovered by Client-Library.
	*/
	if ((ret = ct_callback(context, NULL, CS_SET, CS_CLIENTMSG_CB,
			  (CS_VOID *)clientmsg_callback)) != CS_SUCCEED)
	{
		ex_error("ct_callback for client messages failed");
		return ret;
	}

	/*
	** The server message callback receives server messages sent by
	** the server. These are error or informational messages.
	*/
	if ((ret = ct_callback(context, NULL, CS_SET, CS_SERVERMSG_CB,
			  (CS_VOID *)servermsg_callback)) != CS_SUCCEED)
	{
		ex_error("ct_callback for server messages failed");
		return ret;
	}

	/*
	** Allocate a connection structure.
	*/
	if ((ret = ct_con_alloc(context, &connection)) != CS_SUCCEED)
	{
		ex_error("ct_con_alloc() failed");
		return ret;
	}

	/*
	** These two calls set the user credentials (username and
	** password) for opening the connection.
	*/
	if ((ret = ct_con_props(connection, CS_SET, CS_USERNAME,
			   Ex_username, CS_NULLTERM, NULL)) != CS_SUCCEED)
	{
		ex_error("Could not set user name");
		return ret;
	}

	if ((ret = ct_con_props(connection, CS_SET, CS_PASSWORD,
			   Ex_password, CS_NULLTERM, NULL)) != CS_SUCCEED)
	{
		ex_error("Could not set password");
		return ret;
	}

	/*
	** Create the connection.
	*/
	if (EX_SERVER == NULL)
	{
		ret = ct_connect(connection, NULL, 0);
	}
	else
	{
		ret = ct_connect(connection, EX_SERVER, CS_NULLTERM);
	}

	if (ret != CS_SUCCEED)
	{
		ex_error("Could not connect!");
		return ret;
	}

	/*
	** Check if the use of LOB parameters is allowed on the connection.
	** If it is not, leave here.
	*/ 
	if ((ret = ct_capability(connection, CS_GET, CS_CAP_REQUEST,
		CS_RPCPARAM_LOB, &lobparamsupport)) != CS_SUCCEED)
	{
		ex_error("ct_capability() failed");
		return ret;
	}

	if (lobparamsupport == CS_FALSE)
	{
		fprintf(stdout, "LOB parameters are not supported on this connection\n");
		return EX_EXIT_FAIL;
	}

	/*
	** Create a table for the example.
	*/
	if ((ret = create_table(connection)) != CS_SUCCEED)
	{
		ex_error("Could not create table!");
		return ret;
	}

	/*
	** Allocate a command structure.
	*/
	if ((ret = ct_cmd_alloc(connection, &cmd)) != CS_SUCCEED)
	{
		ex_error("ct_cmd_alloc() failed");
		return ret;
	}

	/*
	** Prepare the dynamic statement
	*/
	strcpy(cmd_text, "select textcol from LOBtable where textcol like ?");

	printf("\nCalling  ct_dynamic(CS_PREPARE)\n");
	printf("Prepare statement: %s\n", cmd_text);

	if ((ret = ct_dynamic(cmd, CS_PREPARE, "my_dyn_stmt", CS_NULLTERM,
		cmd_text, CS_NULLTERM)) != CS_SUCCEED)
	{
		ex_error("ct_dynamic(CS_PREPARE) failed");
		return ret;
	}

	if ((ret = ct_send(cmd)) != CS_SUCCEED)
	{
		ex_error("ct_send() failed");
		return ret;
	}

	if ((ret = handle_results(cmd)) != CS_SUCCEED)
	{
		ex_error("handle_results() failed");
		return ret;
	}

	/*
	** Describe input parameter.
	*/
	printf("\nCalling  ct_dynamic(CS_DESCRIBE_INPUT)\n");

	if ((ret = ct_dynamic(cmd, CS_DESCRIBE_INPUT, "my_dyn_stmt", CS_NULLTERM,
		NULL, CS_UNUSED)) != CS_SUCCEED)
	{
		ex_error("ct_dynamic(CS_DESCRIBE_OUTPUT) failed");
		return ret;
	}

	if ((ret = ct_send(cmd)) != CS_SUCCEED)
	{
		ex_error("ct_send() failed");
		return ret;
	}

	if ((ret = handle_results(cmd)) != CS_SUCCEED)
	{
		ex_error("handle_results() failed");
		return ret;
	}

	/*
	** Execute the dynamic statement
	*/
	printf("\nCalling  ct_dynamic(CS_EXECUTE)\n");
	if ((ret = ct_dynamic(cmd, CS_EXECUTE, "my_dyn_stmt", CS_NULLTERM,
			NULL, CS_UNUSED)) != CS_SUCCEED)
	{
		ex_error("ct_dynamic(CS_EXECUTE) failed");
		return ret;
	}

	/*
	** Define the parameters
	*/
	memset(&fmt, 0, sizeof(fmt));
	strcpy(fmt.name, "@param1");
	fmt.namelen = CS_NULLTERM;
	fmt.datatype = CS_TEXT_TYPE;
	fmt.status = CS_INPUTVALUE;
	fmt.maxlength = 100;

	strcpy(buf, "%Client-Library%");
	printf("    Parameter value: %s\n", buf);
	printf("    Sending CS_TEXT_TYPE parameter with maxlength %d\n", fmt.maxlength);

	ct_param(cmd, &fmt, buf, CS_NULLTERM, 0);
	if ((ret = ct_send(cmd)) != CS_SUCCEED)
	{
		ex_error("ct_send() failed");
		return ret;
	}

	if ((ret = handle_results(cmd)) != CS_SUCCEED)
	{
		ex_error("handle_results() failed");
		return ret;
	}

	/*
	** Another execute, a CS_TEXT_TYPE parameter in chunks.
	*/
	printf("\nCalling  ct_dynamic(CS_EXECUTE)\n");
	if ((ret = ct_dynamic(cmd, CS_EXECUTE, "my_dyn_stmt", CS_NULLTERM,
			NULL, CS_UNUSED)) != CS_SUCCEED)
	{
		ex_error("ct_dynamic(CS_EXECUTE) failed");
		return ret;
	}

	memset(&fmt, 0, sizeof(fmt));
	strcpy(fmt.name, "@param1");
	fmt.namelen = CS_NULLTERM;
	fmt.datatype = CS_TEXT_TYPE;
	fmt.maxlength = CS_UNUSED;
	fmt.status = CS_INPUTVALUE;
	fmt.locale = NULL;

	ct_param(cmd, &fmt, NULL, 0, 0);

	/*
	** Send parameter(s) in chunks.
	*/
	strcpy(blobbuf, "You Can Combat Computer Stress!");
	printf("    Parameter value: %s\n", blobbuf);
	printf("    Sending CS_TEXT_TYPE parameter in chunks of %d bytes\n", CHUNKSIZE1);
	bytesleft = sizeof(blobbuf);
	bufp = blobbuf;

	do 
	{
		sendbytes = MIN(bytesleft, CHUNKSIZE1);
		ct_send_data(cmd, (CS_VOID *)bufp, sendbytes);
		bufp += sendbytes;
		bytesleft -= sendbytes;
	}
	while (bytesleft > 0);

	/*
	** End with sending 0 bytes to indicate the end of data
	*/
	ct_send_data(cmd, (CS_VOID *)bufp, 0);
	
	if ((ret = ct_send(cmd)) != CS_SUCCEED)
	{
		ex_error("ct_send() failed");
		return ret;
	}

	if ((ret = handle_results(cmd)) != CS_SUCCEED)
	{
		ex_error("handle_results() failed");
		return ret;
	}

	/*
	** Another one with a CS_TEXT_TYPE parameter.
	*/
	printf("\nCalling  ct_dynamic(CS_EXECUTE)\n");
	if ((ret = ct_dynamic(cmd, CS_EXECUTE, "my_dyn_stmt", CS_NULLTERM,
			NULL, CS_UNUSED)) != CS_SUCCEED)
	{
		ex_error("ct_dynamic(CS_EXECUTE) failed");
		return ret;
	}

	memset(&fmt, 0, sizeof(fmt));
	strcpy(fmt.name, "@param1");
	fmt.namelen = CS_NULLTERM;
	fmt.datatype = CS_TEXT_TYPE;
	fmt.maxlength = CS_UNUSED;
	fmt.status = CS_INPUTVALUE;
	fmt.locale = NULL;

	ct_param(cmd, &fmt, NULL, 0, 0);

	/*
	** Send CS_TEXT_TYPE parameter(s) in chunks.
	*/
	strcpy(blobbuf, "%When writing an Open Server application%");
	printf("    Parameter value: %s\n", blobbuf);
	printf("    Sending CS_TEXT_TYPE parameter in chunks of %d bytes\n", CHUNKSIZE2);
	bytesleft = sizeof(blobbuf);
	bufp = blobbuf;

	do 
	{
		sendbytes = MIN(bytesleft, CHUNKSIZE2);
		ct_send_data(cmd, (CS_VOID *)bufp, sendbytes);
		bufp += sendbytes;
		bytesleft -= sendbytes;
	}
	while (bytesleft > 0);

	/*
	** End with sending 0 bytes to indicate the end of data
	*/
	ct_send_data(cmd, (CS_VOID *)bufp, 0);

	/*
	** Make sure everything is sent.
	*/	
	if ((ret = ct_send(cmd)) != CS_SUCCEED)
	{
		ex_error("ct_send() failed");
		return ret;
	}

	if ((ret = handle_results(cmd)) != CS_SUCCEED)
	{
		ex_error("handle_results() failed");
		return ret;
	}

	/*
	** Deallocate the dynamic statement.
	*/
	printf("\nCalling  ct_dynamic(CS_DEALLOC)\n");
	if ((ret = ct_dynamic(cmd, CS_DEALLOC, "my_dyn_stmt", CS_NULLTERM,
			NULL, CS_UNUSED)) != CS_SUCCEED)
	{
		ex_error("ct_dynamic(CS_DEALLOC) failed");
		return ret;
	}

	if ((ret = ct_send(cmd)) != CS_SUCCEED)
	{
		ex_error("ct_send() failed");
		return ret;
	}

	if ((ret = handle_results(cmd)) != CS_SUCCEED)
	{
		ex_error("handle_results() failed");
		return ret;
	}
		

	/*
	** Clean up and exit.
	** Drop the table.
	*/
	if ((ret = drop_table(connection)) != CS_SUCCEED)
	{
		ex_error("Could not drop table!");
		return ret;
	}

	/*
	** Drop the command structure.
	*/
	if ((ret = ct_cmd_drop(cmd)) != CS_SUCCEED)
	{
		ex_error("ct_cmd_drop failed");
		return ret;
	}

	/*
	** Close the connection and drop its control structure.
	*/
	if ((ret = ct_close(connection, CS_UNUSED)) != CS_SUCCEED)
	{
		ex_error("ct_close failed");
		return ret;
	}

	if ((ret = ct_con_drop(connection)) != CS_SUCCEED)
	{
		ex_error("ct_con_drop failed");
		return ret;
	}

	/*
	** ct_exit tells Client-Library that we are done.
	*/
	if ((ret = ct_exit(context, CS_UNUSED)) != CS_SUCCEED)
	{
		ex_error("ct_exit failed");
		return ret;
	}

	/*
	** Drop the context structure.
	*/
	if ((ret = cs_ctx_drop(context)) != CS_SUCCEED)
	{
		ex_error("cs_ctx_drop failed");
		return ret;
	}

	/*
	** Normal exit to the operating system.
	*/
	return EX_EXIT_SUCCEED;
}


/*
** Handler for server messages. Client-Library will call this
** routine when it receives a message from the server.
*/
CS_RETCODE CS_PUBLIC
servermsg_callback(CS_CONTEXT *cp, CS_CONNECTION *chp, CS_SERVERMSG *msgp)
{

	/*
	** Print the message info.
	*/
	fprintf(ERR_CH, "Server message:\n\t");
	fprintf(ERR_CH, "number(%ld) severity(%ld) state(%ld) line(%ld)\n",
		(long)msgp->msgnumber, (long)msgp->severity,
		(long)msgp->state, (long)msgp->line);

	/*
	** Print the server name if one was supplied.
	*/
	if (msgp->svrnlen > 0)
		fprintf(ERR_CH, "\tServer name: %s\n", msgp->svrname);

	/*
	** Print the procedure name if one was supplied.
	*/
	if (msgp->proclen > 0)
		fprintf(ERR_CH, "\tProcedure name: %s\n", msgp->proc);

	/*
	** Print the null terminated message.
	*/
	fprintf(ERR_CH, "\t%s\n", msgp->text);

	/*
	** Server message callbacks must return CS_SUCCEED.
	*/
	return CS_SUCCEED;
}

/*
**  Client-Library error handler. This function will be invoked
**  when a Client-Library has detected an error. Before Client-
**  Library routines return CS_FAIL, this handler will be called
**  with additional error information.
*/
CS_RETCODE CS_PUBLIC
clientmsg_callback(CS_CONTEXT *context, CS_CONNECTION *conn, CS_CLIENTMSG *emsgp)
{
	/*
	** Error number: Print the error's severity, number, origin, and
	** layer. These four numbers uniquely identify the error.
	*/
	fprintf(ERR_CH, "Client Library error:\n\t");
	fprintf(ERR_CH, "severity(%ld) number(%ld) origin(%ld) layer(%ld)\n",
		(long)CS_SEVERITY(emsgp->severity),
		(long)CS_NUMBER(emsgp->msgnumber),
		(long)CS_ORIGIN(emsgp->msgnumber),
		(long)CS_LAYER(emsgp->msgnumber));

	/*
	** Error text: Print the error text.
	*/
	fprintf(ERR_CH, "\t%s\n", emsgp->msgstring);

	/*
	** Operating system error information: Some errors, such as
	** network errors, may have an operating system error associated
	** with them. If there was an operating system error, this code
	** prints the error message text.
	*/
	if (emsgp->osstringlen > 0)
	{
		fprintf(ERR_CH,
			"Operating system error number(%ld):\n",
			(long)emsgp->osnumber);
		fprintf(ERR_CH, "\t%s\n", emsgp->osstring);
	}

	/*
	** If we return CS_FAIL, Client-Library marks the connection as
	** dead. This means that it cannot be used anymore. If we return
	** CS_SUCCEED, the connection remains alive if it was not already
	** dead.
	*/
	return CS_SUCCEED;
}

/*
**  CS-Library error handler. This function will be invoked
**  when CS-Library has detected an error.
*/
CS_RETCODE CS_PUBLIC
csmsg_callback(CS_CONTEXT *context, CS_CLIENTMSG *emsgp)
{

	/*
	** Print the error number and message.
	*/
	fprintf(ERR_CH, "CS-Library error:\n");
	fprintf(ERR_CH, "\tseverity(%ld) layer(%ld) origin(%ld) number(%ld)",
		(long)CS_SEVERITY(emsgp->msgnumber),
		(long)CS_LAYER(emsgp->msgnumber),
		(long)CS_ORIGIN(emsgp->msgnumber),
		(long)CS_NUMBER(emsgp->msgnumber));

	fprintf(ERR_CH, "\t%s\n", emsgp->msgstring);

	/*
	** Print any operating system error information.
	*/
	if (emsgp->osstringlen > 0)
	{
		fprintf(ERR_CH, "Operating System Error: %s\n",
			emsgp->osstring);
	}

	return CS_SUCCEED;
}

/*
** create_table
** This routine sets up a table for the LOB dynamic operation
*/
CS_RETCODE CS_PUBLIC
create_table(CS_CONNECTION *connection)
{
	CS_RETCODE      retcode;
	CS_CHAR         *cmdbuf;

	/* 
	** Allocate the buffer for the command string.
	*/
	cmdbuf = (CS_CHAR *)malloc(MAXBUF);
	if (cmdbuf == NULL)
	{
		ex_error("create_table: malloc() failed");
		return CS_MEM_ERROR;
	}

	/*
	** If the table exists, drop it.
	*/
	if ((retcode = drop_table(connection)) != CS_SUCCEED)
	{
		free (cmdbuf);
		ex_error("create_table: drop_table() failed");
		return retcode;
	}

	/*
	** Set up and send the command to create the table.  
	*/
	sprintf(cmdbuf, "create table %s (intcol int, textcol text)",
				Ex_tabname);
	if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
	{
		free (cmdbuf);
		ex_error("create_table: ex_execute_cmd(create table) failed");
		return retcode;
	}

	/*
	** Insert some rows.
	*/
	sprintf(cmdbuf, "insert %s values(1, %s) \
		insert %s values(2, %s)",
		Ex_tabname, text1, Ex_tabname, text2);
	if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
	{
		free (cmdbuf);
		ex_error("create_table: ex_execute_cmd(create table) failed");
		return retcode;
	}

	free(cmdbuf);
	return retcode;
}

/*
** drop_table
** This routine drops the table that was created.
*/
CS_RETCODE CS_PUBLIC
drop_table(CS_CONNECTION *connection)
{
	CS_RETCODE      retcode;
	CS_CHAR         *cmdbuf;

	/* 
	** Allocate the buffer for the command string.
	*/
	cmdbuf = (CS_CHAR *)malloc(MAXBUF);
	if (cmdbuf == NULL)
	{
		ex_error("drop_table: malloc() failed");
		return CS_MEM_ERROR;
	}

	/*
	** If the table exists, drop it.
	*/
	sprintf(cmdbuf, "if exists (select name from sysobjects \
		where name = \"%s\") drop table %s", Ex_tabname, Ex_tabname);
	if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
	{
		free (cmdbuf);
		ex_error("drop_table: ex_execute_cmd(drop table) failed");
		return retcode;
	}
		
	free(cmdbuf);
	return retcode;
}

CS_RETCODE CS_PUBLIC
handle_results(CS_COMMAND *cmd)
{
	CS_INT          res_type;
	CS_RETCODE      ret;
	CS_DATAFMT      fmt;
	CS_CHAR         charbuf[EX_BUFSIZE + 1];
	CS_INT          count;
	CS_INT          i;
	CS_DATAFMT      *fmtp;
	CS_INT          nodynparams;

	fmtp = NULL;

	/*
	** process results
	*/
	while ((ret = ct_results(cmd, &res_type)) == CS_SUCCEED)
	{
		switch ((int)res_type)
		{
		  case CS_CURSOR_RESULT:
			/*
			** Bind the columns here.
			*/
			memset(&fmt, 0, sizeof(CS_DATAFMT));
			fmt.datatype = CS_CHAR_TYPE;
			fmt.format   = CS_FMT_NULLTERM;
			fmt.count    = 1;
			fmt.maxlength = CS_MAX_CHAR;
			if (ct_bind(cmd, 1, &fmt, charbuf, NULL, NULL)
			    != CS_SUCCEED)
			{
				return CS_FAIL;
			}

			/*
			** Now fetch the rows.
			*/
			while(((ret = ct_fetch(cmd, CS_UNUSED, CS_UNUSED,
					CS_UNUSED, &count)) == CS_SUCCEED) ||
			      (ret == CS_ROW_FAIL))
			{
				if (ret == CS_ROW_FAIL) 
				{
					return CS_FAIL;
				}
			}

			/*
			** We're done processing rows. Let's check the
			** final return value of ct_fetch().
			*/
			if (ret != CS_END_DATA)
			{
				return CS_FAIL;
			}

			break;

		  case CS_DESCRIBE_RESULT:
			if (ct_res_info(cmd, CS_NUMDATA, &nodynparams,
					sizeof(nodynparams), NULL)
			    != CS_SUCCEED)
			{
				return CS_FAIL;
			}

			if (nodynparams > 0)
			{
				printf("CS_DESCRIBE_RESULT: Number of parameters: %d\n", 
					nodynparams);
				if ((fmtp = (CS_DATAFMT *)
				  malloc(nodynparams * sizeof(CS_DATAFMT))) == NULL)
				{
					return CS_FAIL;
				}
			}

			for (i = 0; i < nodynparams; i++)
			{
				if (ct_describe(cmd, (i+1), &(fmtp[i])) != CS_SUCCEED)
				{
					free(fmtp);
					return CS_FAIL;
				}

				printf("Parameter number %d:\n", i+1);
				printf("  Datatype: %d\n", fmtp[i].datatype);
				printf("  maxlength: %d\n", fmtp[i].maxlength);
			}

			/*
			** Free the memory, only if there are parameters.
			*/
			if (nodynparams > 0)
			{
				free(fmtp);
			}
			break;

		  case CS_ROW_RESULT:
		  case CS_COMPUTE_RESULT:
		  case CS_PARAM_RESULT:
		  case CS_STATUS_RESULT:
			/*
			** Bind just the 2nd column here.
			*/
			memset(&fmt, 0, sizeof(fmt));
			fmt.datatype = CS_CHAR_TYPE;
			fmt.format   = CS_FMT_NULLTERM;
			fmt.count    = 1;
			fmt.maxlength = EX_BUFSIZE;
			if (ct_bind(cmd, 1, &fmt, charbuf,
					NULL, NULL) != CS_SUCCEED)
			{
				return CS_FAIL;
			}

			while ((ret = ct_fetch (cmd, CS_UNUSED, CS_UNUSED,
			CS_UNUSED, NULL)) != CS_END_DATA)
			{
				if (ret == CS_FAIL || ret == CS_CANCELED)
				{
					return CS_FAIL;
				}
			
				/*
				** Print the data retrieved from the server.
				*/
				printf("Data retrieved: \n%s\n", charbuf);
			}

			if (ret != CS_END_DATA)
			{
				return CS_FAIL;
			}
			break;

		  case CS_CMD_DONE:
		  case CS_SUCCEED:
		  case CS_CMD_SUCCEED:
			break;

		  case CS_CMD_FAIL:
			return CS_FAIL;

		  default:
			return CS_FAIL;
		}
	}

	/*
	** Let us check the return val from ct_results
	*/
	switch ((int)ret)
	{
	  case CS_END_RESULTS:
		/*
		** Everything went fine
		*/
		break;

	  case CS_FAIL:
		/*
		** it should not happen
		*/
		return CS_FAIL;

	  default:
		/*
		** Got an Unexpected return value
		*/
		return CS_FAIL;
	}

	return  CS_SUCCEED;
}


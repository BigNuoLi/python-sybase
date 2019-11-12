/*
** Lob Locator Example Program
** ---------------------------
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
**	This example program demonstrates the use of LOB locators.
**    	A lob locator value is retrieved from the server, inluding some
**    	pre-fetch data from the entire LOB value in the server.
**    	Operations on the LOB in the server are being performed using the
**    	retrieved locator value.
**
** Routines Used
** -------------
**	All the required routines required for establishing and closing
**	a connection to a server, sending a language command to a
**	server, and processing row results.
**
**	In addition, the following routines were used to demonstrate
**	the use of locators:
**
**	cs_locator_alloc()
**	cs_locator()
**	cs_locator_drop()
**
** Inputs
** ------
**    None.
**
** Server Tables
** -------------
**    This program requires the pubs2 database.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctpublic.h>
#include "example.h"

#define MAXCOLUMNS	2
#define MAXSTRING	80
#define LOCATORSTRLEN	48 /* ASE (binary) locator value is 24 bytes long */
#define CHARBUFLEN	800
#define ERR_CH stderr
#define OUT_CH stdout

/*
** Username and password are defined in example.h
*/
CS_CHAR *Ex_username = EX_USERNAME;
CS_CHAR *Ex_password = EX_PASSWORD;

/*
** Macro definition that exits if a function return code indicates failure.
*/
#define EXIT_ON_FAIL(context, ret, str) \
  if (ret != CS_SUCCEED) \
  { \
	fprintf(ERR_CH, "Fatal error: %s\n", str); \
	if (context != NULL) \
	{ \
		(CS_VOID)ct_exit(context, CS_FORCE_EXIT); \
		(CS_VOID)cs_ctx_drop(context); \
	} \
	exit(EX_EXIT_FAIL); \
  }

/*
** Globals.
*/
CS_LOCATOR	*Locator;


/*
** Callback routines for library errors and server messages.
*/
CS_RETCODE CS_PUBLIC csmsg_callback(
	CS_CONTEXT     *context,
	CS_CLIENTMSG   *clientmsg);
CS_RETCODE CS_PUBLIC clientmsg_callback(
	CS_CONTEXT     *context,
	CS_CONNECTION  *connection,
	CS_CLIENTMSG   *clientmsg);
CS_RETCODE CS_PUBLIC servermsg_callback(
	CS_CONTEXT     *context,
	CS_CONNECTION  *connection,
	CS_SERVERMSG   *servermsg);
void process_results(CS_CONTEXT *context,
	CS_COMMAND *cmd);

/*
** Main entry point for the program.
*/
int 
main(int argc, char *argv[])
{
	CS_CONTEXT	*context;	/* Context structure     */
	CS_CONNECTION	*connection;	/* Connection structure. */
	CS_COMMAND	*cmd;		/* Command structure.    */

	CS_SMALLINT	indicator;
	CS_RETCODE	ret;
	CS_CHAR		cmdbuf[MAXSTRING];
	CS_BOOL		loblocatorsupport;
	CS_DATAFMT	locatorfmt;
	CS_BOOL		loc;
	CS_INT		prefetchsize;

	/*
	** Allocate a context structure.
	*/
	context = NULL;
	ret = cs_ctx_alloc(EX_CTLIB_VERSION, &context);
	EXIT_ON_FAIL(context, ret, "cs_ctx_alloc failed");

	/*
	** Initialize Client-Library.
	*/
	ret = ct_init(context, EX_CTLIB_VERSION);
	EXIT_ON_FAIL(context, ret, "ct_init failed");

	/*
	** Install a callback function to handle CS-Library errors.
	*/
	ret = cs_config(context, CS_SET, CS_MESSAGE_CB,
			(CS_VOID *)csmsg_callback, CS_UNUSED, NULL);
	EXIT_ON_FAIL(context, ret, "cs_config(CS_MESSAGE_CB) failed");

	/*
	** Install a callback function to handle Client-Library errors.
	** The client message callback receives error or informational
	** messages discovered by Client-Library.
	*/
	ret = ct_callback(context, NULL, CS_SET, CS_CLIENTMSG_CB,
			  (CS_VOID *)clientmsg_callback);
	EXIT_ON_FAIL(context, ret, "ct_callback for client messages failed");

	/*
	** The server message callback receives server messages sent by
	** the server. These are error or inforamational messages.
	*/
	ret = ct_callback(context, NULL, CS_SET, CS_SERVERMSG_CB,
			  (CS_VOID *)servermsg_callback);
	EXIT_ON_FAIL(context, ret, "ct_callback for server messages failed");

	/*
	** Alocate a connection structure.
	*/
	ret = ct_con_alloc(context, &connection);
	EXIT_ON_FAIL(context, ret, "ct_con_alloc() failed");

	/*
	** Set username and password for the connection.
	*/
	ret = ct_con_props(connection, CS_SET, CS_USERNAME,
			   Ex_username, CS_NULLTERM, NULL);
	EXIT_ON_FAIL(context, ret, "Could not set user name");

	ret = ct_con_props(connection, CS_SET, CS_PASSWORD,
			   Ex_password, CS_NULLTERM, NULL);
	EXIT_ON_FAIL(context, ret, "Could not set password");

	/*
	** Initialize the global Locator variable
	*Locator = NULL;
	*/

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

	EXIT_ON_FAIL(context, ret, "Could not connect!");

	/*
	** Check if the use of LOB locators is allowed on the connection.
	** If it is not, leave here.
	*/
	if ((ret = ct_capability(connection, CS_GET, CS_CAP_REQUEST,
		CS_DATA_LOBLOCATOR, &loblocatorsupport)) != CS_SUCCEED)
	{
		EXIT_ON_FAIL(context, ret, "ct_capability() failed");
		return ret;
	}

	if (loblocatorsupport == CS_FALSE)
	{
		fprintf(stdout, "LOB locators are not supported on this connection\n");
		return EX_EXIT_FAIL;
	}

	/*
	** Turn lob locators on.
	** We're turning lob locators on with a ct_options() command. This
	** could also be done by sending language command 'set send_locator on'
	** to the server.
	*/
	loc = CS_TRUE;
	ret = ct_options(connection, CS_SET, CS_OPT_LOBLOCATOR, &loc, CS_UNUSED, NULL);
	EXIT_ON_FAIL(context, ret, "ct_options(CS_SET, CS_OPT_LOBLOCATOR) failed");

	/*
	** When requesting LOB locators, also request for 100 bytes of prefetch data.
	*/
	prefetchsize = 100;
	ret = ct_options(connection, CS_SET, CS_OPT_LOBLOCFETCHSIZE, &prefetchsize,
		CS_UNUSED, NULL);
	EXIT_ON_FAIL(context, ret, "ct_options(CS_SET, CS_OPT_LOBLOCFETCHSIZE) failed");

	/*
	** Allocator memory for the global Locator variable.
	*/
	ret = cs_locator_alloc(context, &Locator);
	EXIT_ON_FAIL(context, ret, "cs_locator_alloc() failed");

	/*
	** Send a command to the server.
	** First allocate a command structure.
	*/
	ret = ct_cmd_alloc(connection, &cmd);
	EXIT_ON_FAIL(context, ret, "ct_cmd_alloc() failed");

	/*
	** Lob locators are only valid within a transaction. 
	** Let's start a transaction here.
	*/
	strcpy(cmdbuf, "begin transaction ");
	ret = ct_command(cmd, CS_LANG_CMD, cmdbuf, CS_NULLTERM, CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "ct_command() failed");

	printf("Sending 'begin transaction'\n"); 
	ret = ct_send(cmd);
	EXIT_ON_FAIL(context, ret, "ct_send() failed");

	process_results(context, cmd);

	/*
	** Send a language command to the server, to obtain the locator for the
	** 'copy' text field.
	*/
	strcpy(cmdbuf, "select au_id, copy from pubs2..blurbs where au_id like '486-29-%'");
	ret = ct_command(cmd, CS_LANG_CMD, cmdbuf, CS_NULLTERM, CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "ct_command() failed");

	/*
	** Send the command to the server.
	*/
	printf("Sending query: %s\n", cmdbuf);
	ret = ct_send(cmd);
	EXIT_ON_FAIL(context, ret, "ct_send() failed");

	/*
	** Process results.
	*/
	process_results(context, cmd);

	/*
	** Now use the retrieved LOB locator value to send a subsequent query
	** to the server.
	** Send language command with the CS_LOCATOR as parameter. The 
	** previously retrieved lob locator was saved in the global CS_LOCATOR
	** variable 'Locator'.
	** Use this locator to do further queries. For example to select 
	** a substring from the server's LOB.
	*/	
	strcpy(cmdbuf, "select 'substring test', return_lob(text, substring(@myparam, 4, 17))");
	printf("\nSending language command with CS_TEXTLOCATOR_TYPE parameter:\n  %s\n", cmdbuf);
	
	ret = ct_command(cmd, CS_LANG_CMD, cmdbuf, CS_NULLTERM, CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "ct_command() failed");

	/*
	** Set the parameter and send the command to the server
	*/
	strcpy(locatorfmt.name, "@myparam");
	locatorfmt.namelen = CS_NULLTERM;
	locatorfmt.datatype = CS_TEXTLOCATOR_TYPE;
	locatorfmt.usertype = 0;
	locatorfmt.maxlength = CS_UNUSED;
	locatorfmt.format = CS_FMT_UNUSED;
	locatorfmt.locale = NULL;
	locatorfmt.status = CS_INPUTVALUE;

	indicator = 0;
	ret = ct_param(cmd, &locatorfmt, (CS_VOID *)Locator, CS_UNUSED, indicator);
	EXIT_ON_FAIL(context, ret, "ct_param() failed");

	ret = ct_send(cmd);
	EXIT_ON_FAIL(context, ret, "ct_send() failed");

	process_results(context, cmd);

	/*
	** Or select the actual LOB data for this locator.
	*/
	strcpy(cmdbuf, "select 'full text test', return_lob(text, @myparam)");
	printf("\nSending language command with CS_TEXTLOCATOR_TYPE parameter:\n  %s\n", cmdbuf);
	
	ret = ct_command(cmd, CS_LANG_CMD, cmdbuf, CS_NULLTERM, CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "ct_command() failed");

	ret = ct_param(cmd, &locatorfmt, (CS_VOID *)Locator, CS_UNUSED, indicator);
	EXIT_ON_FAIL(context, ret, "ct_param() failed");

	ret = ct_send(cmd);
	EXIT_ON_FAIL(context, ret, "ct_send() failed");

	process_results(context, cmd);

	/*
	** End the transaction.
	*/
	strcpy(cmdbuf, "commit transaction ");
	ret = ct_command(cmd, CS_LANG_CMD, cmdbuf, CS_NULLTERM, CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "ct_command() failed");

	ret = ct_send(cmd);
	EXIT_ON_FAIL(context, ret, "ct_send() failed");

	process_results(context, cmd);

	/*
	** Clean up and exit.
	** Drop the command structure.
	*/
	ret = ct_cmd_drop(cmd);
	EXIT_ON_FAIL(context, ret, "ct_cmd_drop failed");

	/*
	** Close the connection and drop its control structure.
	*/
	ret = ct_close(connection, CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "ct_close failed");

	ret = ct_con_drop(connection);
	EXIT_ON_FAIL(context, ret, "ct_con_drop failed");

	/*
	** ct_exit tells Client-Library that we are done.
	*/
	ret = ct_exit(context, CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "ct_exit failed");

	/*
	** Drop the context structure.
	*/
	ret = cs_ctx_drop(context);
	EXIT_ON_FAIL(context, ret, "cs_ctx_drop failed");

	/*
	** Drop the global locator.
	*/
	ret = cs_locator_drop(context, Locator);
	EXIT_ON_FAIL(context, ret, "cs_ctx_drop failed");

	/*
	** Normal exit to the operating system.
	*/
	return EX_EXIT_SUCCEED;
}

void
process_results(CS_CONTEXT *context, CS_COMMAND *cmd)
{
	CS_INT		datalength[MAXCOLUMNS];
	CS_SMALLINT	indicator[MAXCOLUMNS];
	CS_INT		count;
	CS_RETCODE	ret;
	CS_RETCODE	results_ret;
	CS_CHAR		charvar[MAXSTRING];
	CS_LOCATOR	*loblocator;
	CS_INT		num_cols;
	CS_DATAFMT	datafmt[MAXCOLUMNS];	
	int		i;
	CS_CHAR		charbuf[CHARBUFLEN];
	CS_CHAR		locatorbuf[LOCATORSTRLEN + 1];
	CS_INT		outlen;
	CS_BIGINT	totalloblen;
	CS_INT		prefetchlen;
	CS_INT		locatortype;
	CS_INT		result_type;

	/*
	** Initialize
	*/
	loblocator = NULL;
	memset(charvar, 0, MAXSTRING);
	memset(charbuf, 0, CHARBUFLEN);
	memset(locatorbuf, 0, LOCATORSTRLEN + 1);

	/*
	** Process the results of the command.
	*/
	while ((results_ret = ct_results(cmd, &result_type)) == CS_SUCCEED)
	{
		/*
		** ct_results sets result_type to indicate when data is
		** available and to indicate command status codes.
		*/
		switch ((int)result_type)
		{
		  case CS_ROW_RESULT:

			/*
			** Find out how many columns we have
			*/
			ret = ct_res_info(cmd, CS_NUMDATA, &num_cols, CS_UNUSED, NULL);
			EXIT_ON_FAIL(context, ret, "ct_res_info() failed");

			/*
			** Get the description of each column
			*/

			for (i = 0; i < num_cols; i++)
			{
				ret =  ct_describe(cmd, (i + 1), &datafmt[i]);
				EXIT_ON_FAIL(context, ret, "ct_describe() failed");
			}

			/*
			** This result_type value indicates that the rows
			** returned by the query have arrived. We bind and
			** fetch the rows.
			*/
			ret = ct_bind(cmd, 1, &datafmt[0], &charvar, &datalength[0],
				      &indicator[0]);
			EXIT_ON_FAIL(context, ret, "ct_bind() for cola failed");

			/*
			** For the query send, we expect either a text locator
			** or text data back. The global Locator will contain the 
			** locator value in case we have a locator result.
			*/
			switch (datafmt[1].datatype)
			{
			  case CS_TEXTLOCATOR_TYPE:
				datafmt[1].maxlength = MAXSTRING;
				ret = ct_bind(cmd, 2, &datafmt[1], Locator, &datalength[1],
					      &indicator[1]);
				break;

			  case CS_TEXT_TYPE:
				ret = ct_bind(cmd, 2, &datafmt[1], charbuf, &datalength[1],
					      &indicator[1]);
				break;

			  default:
				printf("Unexpeced datatype: %d\n",(int)datafmt[1].datatype);
				ret = CS_FAIL;
				break;
			}

			EXIT_ON_FAIL(context, ret, "ct_bind() for colb failed");

			/*
			** Fetch and print the rows.
			*/
			while (((ret = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED,
				&count)) == CS_SUCCEED) || (ret == CS_ROW_FAIL))
			{
				/*
				** Check if we hit a recoverable error.
				*/
				if (ret == CS_ROW_FAIL)
				{
					fprintf(ERR_CH, "Error on row %ld.\n", (long)(count + 1));
				}

				/*
				** Print author id first.
				*/
				printf("au_id: %s\n", charvar);
	
				/*
				** If we have a lob locator, let's print out some of
				** the data contained in that locator.
				*/
				if (datafmt[1].datatype == CS_TEXTLOCATOR_TYPE)
				{
					/*
					** If we received a LOB locator from the server,
					** then let's read and print the information that
					** that is containined in the CS_LOCATOR variable.
					**
					** Get the ASCII representation of the binary
					** locator value. 
					*/
					cs_locator(context, CS_GET, Locator,
						CS_LCTR_LOCATOR, (CS_VOID *)locatorbuf,
						sizeof(locatorbuf), &outlen);

					/*
					** Print the string representation of the locator.
					*/
					printf("--- We have a locator result ---\n");
					printf("-------------------------------------");
					printf("-------------------------------------\n");
					printf("locator value: %s\n\n", locatorbuf);

					/*
					** Get the prefetch datalength.
					*/
					cs_locator(context, CS_GET, Locator,
						CS_LCTR_PREFETCHLEN, (CS_VOID *)&prefetchlen,
						sizeof(prefetchlen), &outlen);

					printf("Prefetch datalen: %d\n\n", (int)prefetchlen);

					/*
					** Get the prefetch data.
					*/
					cs_locator(context, CS_GET, Locator,
						CS_LCTR_PREFETCHDATA, (CS_VOID *)charbuf,
						sizeof(charbuf), &outlen);

					printf("Prefetch data:\n%s\n\n", charbuf);

					/*
					** Get the total length of the LOB data in the server.
					*/
					cs_locator(context, CS_GET, Locator,
						CS_LCTR_LOBLEN, (CS_VOID *)&totalloblen,
						sizeof(totalloblen), &outlen);

					printf("Total LOB length in server: %ld\n\n", 
						(long)totalloblen);

					/*
					** Get the data type of the locator.
					*/
					cs_locator(context, CS_GET, Locator,
						CS_LCTR_DATATYPE, (CS_VOID *)&locatortype,
						sizeof(locatortype), &outlen);

					printf("Locator datatype: %d\n\n", (int)locatortype);

					printf("-------------------------------------");
					printf("-------------------------------------\n");
				}
				else
				{
					/*
					** We have no LOB locator, but the actual data.
					*/
					printf("\nText:\n%s\n", charbuf);
				}
			}

			/*
			** We're finished processing rows, so check ct_fetch's final
			** return value to see if an error occurred. The final return
			** code should be CS_END_DATA.
			*/
			if (ret == CS_END_DATA)
			{
				fprintf(OUT_CH, "\nAll done processing rows.\n");
			}
			else
			{
				EXIT_ON_FAIL(context, CS_FAIL, "ct_fetch failed");
			}

			/*
			** All done with this result set.
			*/
			break;

		  case CS_CMD_SUCCEED:
			/*
			** We executed a command that never returns rows.
			*/
			fprintf(OUT_CH, "No rows returned.\n");
			break;

		  case CS_CMD_FAIL:
			/*
			** The server encountered an error while
			** processing our command. These errors will be
			** displayed by the server-message callback that
			** we installed earlier.
			*/
			break;

		  case CS_CMD_DONE:
			/*
			** The logical command has been completely
			** processed.
			*/
			break;

		default:
			/*
			** We got something unexpected.
			*/
			EXIT_ON_FAIL(context, CS_FAIL,
				"ct_results returned unexpected result type");
			break;
		}
	}

	/*
	** We've finished processing results. Check the return value of
	** ct_results() to see if everything went okay.
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
		** Something terrible happened.
		*/
		EXIT_ON_FAIL(context, CS_FAIL,
			     "ct_results() returned CS_FAIL.");
		break;

	  default:
		/*
		** We got an unexpected return value.
		*/
		EXIT_ON_FAIL(context, CS_FAIL,
			   "ct_results returned unexpected result type");
		break;
	}
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
	fprintf(ERR_CH,
		"Client Library error:\n\t");
	fprintf(ERR_CH,
		"severity(%ld) number(%ld) origin(%ld) layer(%ld)\n",
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
	fprintf(ERR_CH,
		"CS-Library error:\n");
	fprintf(ERR_CH,
		"\tseverity(%ld) layer(%ld) origin(%ld) number(%ld)",
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

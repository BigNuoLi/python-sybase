/*
** RPC COMMAND EXAMPLE PROGRAM WITH TEXT AND CHUNKED LOB PARAMETERS
** ----------------------------------------------------------------
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
**	This example program demonstrates sending a RPC command 
**	to a Server containing a mixture of (LOB)  parameters.
**
**	The example uses standard ANSI C for input/output and
**	memory management.
**
**	All work is performed synchronously.
**
** Routines Used
** -------------
**	All the routines required for establishing and closing
**	a connection to a server, sending a language command to a
**	server, and processing row results.
**
**	In addition, the following routines were used to demonstrate
**	sending and processing a RPC command:
** 
**	ct_param()
**	ct_bind()
**	ct_send_data()
**	ct_res_info()
**	ct_command()
** 
** Input
** -----
**	This example uses an input file that should be available
**	in the current directory: lopsum.txt
**
** Output
** ------
**	The example program displays the row results
**	status results, parameter results, and server message
**	returned by the remote procedure.
**
** Server Dependencies
** -------------------
**	If connecting to an Open Server, the Open Server must be able 
**	to handle language and RPC commands intended for ASE.
**
** Algorithm
** ----------
**	- Initialize Client-Library.
**	- Install message handling callbacks.
**	- Establish a connection.
**	- Create a database.
**	- Create a stored procedure.
**	- Execute the stored procedure.
**	- Retrieve and display the results returned from the stored
**	  proocedure.
**	- Perform cleanup by dropping the database and the connection,
**	  deallocating memory allocated for commands, connections, and
**	  contexts, and exiting Client-Library.
**
*/

#include <stdio.h>
#include <fcntl.h>
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
CS_CHAR *Ex_appname  = "lobrpc";
CS_CHAR	*Ex_dbname   = "sampledb";
CS_CHAR *Ex_server   = EX_SERVER;
CS_CHAR *Ex_username = EX_USERNAME;
CS_CHAR *Ex_password = EX_PASSWORD;

#define MYTEXTDATA "Just some small piece of text for the first text parameter"
#define MYIMAGEDATA "Some readable data, inserted into an image array"
#define MYLOBDATA1 "This is (readable) data for the first streamed IMAGE parameter"
#define MYCHARDATA "Some character data"
#define MYCHARDATA2 "This data string should be returned to the client again"

#define CHUNKSIZE1 18
#define CHUNKSIZE2 10
#define CHUNKSIZE3 200

/*
** Prototypes for routines in the example code.
*/
CS_STATIC CS_RETCODE create_procedure(CS_CONNECTION *connection);
CS_STATIC CS_RETCODE installnulls(CS_CONTEXT *context);
CS_STATIC CS_RETCODE do_rpc(CS_CONNECTION *connection);

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
*/
int
main(int argc, char *argv[])
{
	CS_CONTEXT	*context;
	CS_CONNECTION	*connection;
	CS_RETCODE	retcode;
	CS_BOOL		lobparamsupport;
	
	fprintf(stdout, "RPC Example using LOB parameters.\n");

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
	if (retcode != CS_SUCCEED)
	{
		ex_error("Connect failed\n");
		return EX_EXIT_FAIL;
	}

	/*
	** Check if the use of LOB parameters is allowed on the connection.
	** If it is not, leave here.
	*/
	retcode = ct_capability(connection, CS_GET, CS_CAP_REQUEST,
		CS_RPCPARAM_LOB, &lobparamsupport);
	if (retcode != CS_SUCCEED)
	{
		ex_error("ct_capability() failed");
		return EX_EXIT_FAIL;
	}

	if (lobparamsupport == CS_FALSE)
	{
		fprintf(stdout, "LOB parameters are not supported on this connection\n");
		return EX_EXIT_FAIL;
	}

	/*
	** Create a database for the sample program and change to it.
	*/
	retcode = ex_create_db(connection, Ex_dbname);

	/*
	** Create a stored procedure to execute
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = create_procedure(connection);
	}

	/*
	** Install our null values to display
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = installnulls(context);
	}

	/*
	** Execute the the newly created RPC
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = do_rpc(connection);

		/*
		** Remove the database that was created.  The error
		** code (if not succeed), will be used in the cleanup
		** routines.
		*/
		if (retcode == CS_SUCCEED)
		{
			retcode = ex_remove_db(connection, Ex_dbname);
		}
		else
		{
			(void)ex_remove_db(connection, Ex_dbname);
		}
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
** create_procedure()
**
** Type of function:
** 	rpc program internal api
**
** Purpose:
** 	Create a stored procedure in the server.
**
** Parameters:
** 	connection	- Pointer to CS_CONNECTION structure.
**
** Return:
**	CS_SUCCEED if rpc was created.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE 
create_procedure(CS_CONNECTION *connection)
{
	CS_RETCODE	retcode;
	CS_CHAR		*cmdbuf;
	
	if ((retcode = ex_use_db(connection, Ex_dbname)) != CS_SUCCEED)
	{
		ex_error("create_procedure: ex_use_db() failed");
		return retcode;
	}

	/* 
	** Allocate
	*/
	cmdbuf = (CS_CHAR *)malloc(EX_BUFSIZE);
	if (cmdbuf == NULL)
	{
		ex_error("CreateTable: malloc() failed");
		return CS_MEM_ERROR;
	}

	/* 
	** Build the command for creating the stored procedure.
	** First, drop the stored procedure if it already exits.
	*/
	strcpy(cmdbuf, "if exists (select name from sysobjects \
			where type = \"P\" and name = \"sample_LOBrpc\") \
			Begin	\
				drop proc sample_LOBrpc	\
			End ");
	if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
	{
		ex_error("CreateTable: ex_execute_cmd(drop table) failed");
		free (cmdbuf);
		return retcode;
	}
		
	/* 
	** Define the procedure with parameters.
	*/
	strcpy(cmdbuf, "create proc sample_LOBrpc (@intparam int, @textparam text, \
		@imageparam1 image, @imageparam2 image, @charparam varchar(50), \
		@textparam2 text, @unitextparam unitext, @returncharparam varchar(50) output) \
		as ");

	/* 
	** Define queries to return row results, assign parameter values,
	** and return a message result.
	*/
	strcat(cmdbuf, "select @intparam, @textparam, @textparam2");
	if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
	{
		ex_error("create_procedure: ex_execute_cmd(drop table) failed");
	}
		
	free (cmdbuf);
	return retcode;
}

/*
** installnulls()
**
** Type of function:
** 	rpc program internal api
**
** Purpose:
** 	Install the character string "NULL" as the default null value
**	when binding to character columns.
**
** Parameters:
** 	context	- Pointer to CS_CONTEXT structure.
**
** Return:
**	CS_SUCCEED if null was installed.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE 
installnulls(CS_CONTEXT *context)
{
	CS_DATAFMT	datafmt;

	memset(&datafmt, 0, sizeof(datafmt));
	datafmt.datatype = CS_CHAR_TYPE;
	datafmt.status = CS_FMT_UNUSED;
	datafmt.locale = NULL;
	
	return cs_setnull(context, &datafmt, "NULL", strlen("NULL"));
}

/*
** do_rpc()
**
** Type of function:
** 	rpc program internal api
**
** Purpose:
**	This routine passes the parameters and runs 
**	the sample RPC on the server.
**
** Parameters:
** 	connection	- Pointer to CS_CONNECTION structure.
**
** Return:
**	CS_SUCCEED if rpc was executed.
**	Otherwise a Client-Library failure code.
** 
*/
CS_STATIC CS_RETCODE 
do_rpc(CS_CONNECTION *connection)
{
	CS_RETCODE	retcode;
	CS_CONTEXT	*ctx;
	CS_COMMAND	*cmd;
	CS_DATAFMT	paramfmt;
	CS_DATAFMT	srcfmt;
	CS_DATAFMT	dstfmt;
	CS_INT		intvar;
	CS_CHAR		charbuf[EX_MAXSTRINGLEN];
	CS_CHAR		textbuf[EX_BUFSIZE];
	CS_IMAGE	imagebuf[EX_BUFSIZE];
	CS_CHAR		lobbuf[EX_BUFSIZE];
	CS_UNITEXT	unibuf[2*EX_BUFSIZE];
	CS_CHAR		rpc_name[CS_MAX_NAME];
	CS_SMALLINT	indicator;
	CS_INT		datalen;
	CS_CHAR		*bufp;
	CS_INT		bytesleft;
	CS_INT		sendbytes;
	FILE	 	*fp;
	CS_INT		numbytes;

	/*
	** Initialize
	*/
	strcpy(rpc_name, "sample_LOBrpc");
	indicator = 0;

	/*
	** Use the new database
	*/
	if ((retcode = ex_use_db(connection, Ex_dbname)) != CS_SUCCEED)
	{
		ex_error("do_rpc: ex_use_db() failed");
		return retcode;
	}

	/*
	** Allocate a command structure
	*/
	if ((retcode = ct_cmd_alloc(connection, &cmd)) != CS_SUCCEED)
	{
		ex_error("do_rpc: ct_cmd_alloc() failed");
		return retcode;
	}

	/*
	** Initiate an RPC command
	*/
	if ((retcode = ct_command(cmd, CS_RPC_CMD, rpc_name, CS_NULLTERM,
			CS_NO_RECOMPILE)) != CS_SUCCEED)
	{
		ex_error("do_rpc: ct_command() failed");
		return retcode;
	}

	/*
	** Set up the CS_DATAFMT structure and pass each of the
	** parameters for the RPC
	**
	** First parameter:
	*/
	memset(&paramfmt, 0, sizeof(paramfmt));
	strcpy(paramfmt.name, "@intparam");
	paramfmt.namelen = CS_NULLTERM;
	paramfmt.datatype = CS_INT_TYPE;
	paramfmt.maxlength = CS_UNUSED;
	paramfmt.status = CS_INPUTVALUE;
	paramfmt.locale = NULL;

	intvar = 2;
	if ((retcode = ct_param(cmd, &paramfmt, (CS_VOID *)&intvar,
			sizeof(CS_INT), indicator)) != CS_SUCCEED)
	{
		ex_error("do_rpc: ct_param(int) failed");
		return retcode;
	}

	/*
	** Second parameter, some small text.
	*/
	strcpy(paramfmt.name, "@textparam");
	paramfmt.namelen = CS_NULLTERM;
	paramfmt.datatype = CS_TEXT_TYPE;
	paramfmt.maxlength = 5000;
	paramfmt.status = CS_INPUTVALUE;
	paramfmt.locale = NULL;

	strcpy(textbuf, MYTEXTDATA);
	datalen = sizeof(MYTEXTDATA);

	if ((retcode = ct_param(cmd, &paramfmt, (CS_VOID *)&textbuf,
			datalen, indicator)) != CS_SUCCEED)
	{
		ex_error("do_rpc: ct_param(text) failed");
		return retcode;
	}

	/*
	** Third parameter, image (send some readable character for convenience).
	*/
	strcpy(paramfmt.name, "@imageparam1");
	paramfmt.namelen = CS_NULLTERM;
	paramfmt.datatype = CS_IMAGE_TYPE;
	paramfmt.maxlength = 5000;
	paramfmt.status = CS_INPUTVALUE;
	paramfmt.locale = NULL;

	memcpy(imagebuf, MYIMAGEDATA, sizeof(imagebuf));
	datalen = sizeof(MYIMAGEDATA);

	if ((retcode = ct_param(cmd, &paramfmt, (CS_VOID *)&imagebuf,
			datalen, indicator)) != CS_SUCCEED)
	{
		ex_error("do_rpc: ct_param(text) failed");
		return retcode;
	}

	/*
	** Fourth parameter, an image sent as streamed LOB data
	*/
	strcpy(paramfmt.name, "@imageparam2");
	paramfmt.namelen = CS_NULLTERM;
	paramfmt.datatype = CS_IMAGE_TYPE;
	paramfmt.maxlength = CS_UNUSED;
	paramfmt.status = CS_INPUTVALUE;
	paramfmt.locale = NULL;
	if ((retcode = ct_param(cmd, &paramfmt, NULL, 0, 0)) != CS_SUCCEED)
	{
		ex_error("do_rpc: ct_param(blob) failed");
		return retcode;
	}

	/*
	** Fifth parameter, char.
	*/
	strcpy(paramfmt.name, "@charparam");
	paramfmt.namelen = CS_NULLTERM;
	paramfmt.datatype = CS_CHAR_TYPE;
	paramfmt.maxlength = EX_MAXSTRINGLEN;
	paramfmt.status = CS_INPUTVALUE;
	paramfmt.locale = NULL;

	strcpy(charbuf, MYCHARDATA);
	datalen = sizeof(MYCHARDATA);

	if ((retcode = ct_param(cmd, &paramfmt, (CS_VOID *)&charbuf,
			datalen, indicator)) != CS_SUCCEED)
	{
		ex_error("do_rpc: ct_param(char) failed");
		return retcode;
	}

	/*
	** Sixth parameter, some streamed text data.
	*/
	strcpy(paramfmt.name, "@textparam2");
	paramfmt.namelen = CS_NULLTERM;
	paramfmt.datatype = CS_TEXT_TYPE;
	paramfmt.maxlength = CS_UNUSED;
	paramfmt.status = CS_INPUTVALUE;
	paramfmt.locale = NULL;
	if ((retcode = ct_param(cmd, &paramfmt, NULL, 0, 0)) != CS_SUCCEED)
	{
		ex_error("do_rpc: ct_param(blob) failed");
		return retcode;
	}

	/*
	** Seventh parameter, unitext sent as a stream.
	*/
	strcpy(paramfmt.name, "@unitextparam");
	paramfmt.namelen = CS_NULLTERM;
	paramfmt.datatype = CS_UNITEXT_TYPE;
	paramfmt.maxlength = CS_UNUSED;
	paramfmt.status = CS_INPUTVALUE;
	paramfmt.locale = NULL;
	if ((retcode = ct_param(cmd, &paramfmt, NULL, 0, 0)) != CS_SUCCEED)
	{
		ex_error("do_rpc: ct_param(blob) failed");
		return retcode;
	}

	/*
	** And the last parameter, char, a return parameter.
	*/
	strcpy(paramfmt.name, "@returncharparam");
	paramfmt.namelen = CS_NULLTERM;
	paramfmt.datatype = CS_CHAR_TYPE;
	paramfmt.maxlength = EX_MAXSTRINGLEN;
	paramfmt.status = CS_RETURN;
	paramfmt.locale = NULL;

	strcpy(charbuf, MYCHARDATA2);
	datalen = sizeof(MYCHARDATA2);

	if ((retcode = ct_param(cmd, &paramfmt, (CS_VOID *)&charbuf,
			datalen, indicator)) != CS_SUCCEED)
	{
		ex_error("do_rpc: ct_param(char) failed");
		return retcode;
	}

	/*
	** Call ct_send_data() in a loop to send every LOB parameter with
	** unknown length to the server.
	*/
	strcpy(lobbuf, MYLOBDATA1);
	datalen = sizeof(lobbuf);
	bytesleft = sizeof(lobbuf);
	bufp = lobbuf;
	printf("Sending CS_IMAGE_TYPE parameter in chunks of %d bytes\n", CHUNKSIZE1);
	do
	{
		sendbytes = MIN(bytesleft, CHUNKSIZE1);
		ct_send_data(cmd, (CS_VOID *)bufp, sendbytes);
		bufp += sendbytes;	
		bytesleft -= sendbytes;
	}
	while (bytesleft > 0);

	/*
	** End with sending 0 bytes to indicate the end of data.
	*/
	ct_send_data(cmd, (CS_VOID *)bufp, 0);

	/*
	** Second streamed LOB, read a file from disk and send it in a stream.
	*/
	bufp = lobbuf;
	fp = fopen("lopsum.txt","r");
	if (fp == NULL)
	{
		ex_error("do_rpc: open() failed");
		return CS_FAIL;
	}
	
	printf("Sending CS_TEXT_TYPE parameter in chunks of %d bytes\n", CHUNKSIZE2);
	do
	{
		numbytes = fread(bufp, 1, CHUNKSIZE2, fp);
		ct_send_data(cmd, (CS_VOID *)bufp, numbytes);
	}
	while (numbytes != 0);

	fclose(fp);

	/*
	** The third streamed LOB, read the same file from disk and convert it
	** to unitext (utf16) before sending.
	** Grab the context, required for cs_convert().
	*/
	cs_ctx_global(EX_CTLIB_VERSION, &ctx);
	bufp = lobbuf;
	fp = fopen("lopsum.txt", "r");
	if (fp == NULL)
	{
		ex_error("do_rpc: open() failed");
		return CS_FAIL;
	}
	
	printf("Sending CS_UNITEXT_TYPE parameter in chunks of %d bytes\n", 2*CHUNKSIZE3);
	do
	{
		/*
		** Read a part of the file.
		*/
		numbytes = fread(bufp, 1, CHUNKSIZE3, fp);

		/*
		** convert it to utf16
		*/
		srcfmt.datatype = CS_TEXT_TYPE;
		srcfmt.format = CS_FMT_UNUSED;
		srcfmt.maxlength = numbytes;
		srcfmt.locale = NULL;

		dstfmt.datatype = CS_UNITEXT_TYPE;
		dstfmt.format = CS_FMT_UNUSED;
		dstfmt.maxlength = EX_BUFSIZE;
		dstfmt.locale = NULL;

		if (numbytes > 0)
		{
			cs_convert(ctx, &srcfmt, bufp, &dstfmt, unibuf, &numbytes);
		}

		/*
		** And send the unitext chunk to the server.
		*/
		ct_send_data(cmd, (CS_VOID *)unibuf, numbytes);
	}
	while (numbytes != 0);

	/*
	** Make sure everything is sent.
	*/
	if (ct_send(cmd) != CS_SUCCEED)
	{
		ex_error("do_rpc: ct_send() failed");
		return retcode;
	}

	fclose(fp);

	return ex_handle_results(cmd);
}

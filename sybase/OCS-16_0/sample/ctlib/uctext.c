/*
** CT_GET_DATA/CT_SEND_DATA with UPDATETEXT EXAMPLE PROGRAM
** --------------------------------------------------------
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
**	This example program how to perform partial text updates.
**	The text is retrieved from an ASE.  ASE doesn't support
**	partial updates of text so, we send a request to an open
**	server that does support it.  The open server program 
**	provides the server part of the example.  In a more realistic
**	scenario, the open server would send the updatetext
**	(partial update) command to a server that does support the
**	functionality (e.g., MSSQL).  In this sample, the open server
**	sends a message back to the client indicating the text received.
**
**	The example uses standard ANSI C library routines for input/output
**	and memory management.
**
**	All work is performed synchronously.
**
** Routines Used
** -------------
**	All the required routines required for establishing and closing
**	a connection to a server, sending a language command to a
**	server, and processing row results.
**
**	In addition, the following routines were used to demonstrate
**	text/image data manipulation:
** 
**	ct_get_data()
**	ct_data_info()
**	ct_send_data()
**	ct_describe()
** 
** Input
** -----
**	No input is required. Information normally required
**	from the user is retrieved from global variables defined
**	in the example header files.
**
** Output
** ------
**	The example program displays the text data it retrieved
**	from ASE and prints the message from the open server
** 
**	Messages from the server and error messages from the
**	Server and Client-Library are directed to stderr.
**
** Server Dependencies
** -------------------
**	One ASE, one openserver.
**
** Server Tables
** -------------
**	This example will create a table with the following 
**	definition in a database created by the example:
**
**	create table sampletext (i1 int, t text, f float, i2 int)
**
**	The example will drop the database it created.
**
** Algorithm
** ----------
**	Initialize Client-Library.
**
**	install message handling callbacks.
**
**	Establish two connections. One connection will be used for
**	retrieving data while the other connection will be used to do
**	the send the partial update command to the open server.
**
**	Create the database and the table, and initialize the table.
**
**	Retrieve the text data from the newly created table and retrieve
**	the timestamp for the text data.
**
**	Display the data and validate that it is the expected value.
**
**	Do the following for various partial update examples:
**	Send a partial update to the open server.
**
**	Display open server messages.
**
**	Perform cleanup by dropping the database and the connection,
**	deallocating memory allocated for commands, connections, and 
**	contexts, and exiting Client-Library.
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
CS_CHAR *Ex_appname = "uctext_example";	
CS_CHAR *Ex_dbname = "sampledb";
CS_CHAR *Ex_tabname = "sampletext";
CS_CHAR *Ex_server   = EX_SERVER;
CS_CHAR *Open_server;
CS_CHAR *Ex_username = EX_USERNAME;
CS_CHAR *Ex_password = EX_PASSWORD;


/*
** Define what text values that we will manipulate
*/
#define	EX_TXT_INIT_VALUE	"This is the initial value of the text field"
#define	EX_TXT_UPD1_VALUE	"Here is the some different text."

#define	EX_TXT_UPD2_VALUE	"More different text here."

/*
** The maximum length of text that this example program may use
*/
#define EX_MAX_TEXT		255

/*
** Define a structure used in managing the text data
*/

typedef struct _text_data
{
	CS_IODESC	iodesc;		/* iodesc associated with text value */
	CS_TEXT		textbuf[EX_MAX_TEXT];	/* holds the value */
	CS_INT		textlen;	/* number of bytes in textbuf */
} TEXT_DATA;

/*
** Prototypes for routines in the example code.
*/
CS_STATIC CS_RETCODE CreateDatabase(
	CS_CONNECTION *connection1,
	CS_CONNECTION *connection2
	);
CS_STATIC CS_RETCODE RemoveDatabase(
	CS_CONNECTION *connection1,
	CS_CONNECTION *connection2
	);
CS_STATIC CS_RETCODE CreateTable(
	CS_CONNECTION *connection
	);
CS_STATIC CS_INT DoGetSend(
	CS_CONNECTION *connection1,
	CS_CONNECTION *connection2
	);
CS_STATIC CS_RETCODE RetrieveData(
	CS_CONNECTION *connection,
	TEXT_DATA *textdata
	);
CS_STATIC CS_RETCODE FetchResults(
	CS_COMMAND *cmd,
	TEXT_DATA *textdata
	);
CS_STATIC CS_RETCODE UpdateTextData(
	CS_CONNECTION *connection,
	TEXT_DATA *textdata,
	char *newdata
	);
CS_STATIC CS_RETCODE ProcessTimestamp(
	CS_COMMAND *cmd,
	TEXT_DATA *textdata
	);

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
	CS_CONNECTION	*connection1 = NULL;
	CS_CONNECTION	*connection2 = NULL;
	CS_RETCODE	retcode;
	
	fprintf(stdout,"Partial text update Example\n");
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
	** Establish two connections. Connection1 is used to 
	** select data. Connection2 is used for doing updates.
	** Allocate the connection structure, set their properties, and  
	** establish the connections.
	*/
	retcode = ex_connect(context, &connection1, Ex_appname,
				Ex_username, Ex_password, Ex_server);
	/*
	** Use DSLISTEN env variable for the Open_server name
	*/
	if ((Open_server = getenv("DSLISTEN")) == NULL)
	{
		ex_error("uctext: getenv(DSLISTEN) failed\n");
	}

	if (retcode == CS_SUCCEED)
	{
		retcode = ex_connect(context, &connection2, Ex_appname,
					Ex_username, Ex_password, Open_server);
		if (retcode != CS_SUCCEED)
		{
			(CS_VOID)ct_con_drop(connection2);
			connection2 = NULL;
			ex_error("uctext: ex_connect() to open server failed.\n");
			return retcode;
		}
	}

	/*
	** Create a database for the sample program and change to it. The
	** routine will use the global variable Ex_dbname, which is defined in
	** the sample header file.
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = CreateDatabase(connection1, connection2);
		if (retcode != CS_SUCCEED)
		{
			ex_error("getsend: ex_create_db() failed");
		}
	}

	/*
	** Create a table for the example.
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = CreateTable(connection1);
	}

	/*
	** Execute the routines for the uctext sample
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = DoGetSend(connection1, connection2);
	}

	/*
	** Remove the sample database.
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = RemoveDatabase(connection1, connection2);
	}

	/*
	** Deallocate the allocated structures, close the connection,
	** and exit Client-Library.
	*/
	if (connection1 != NULL)
	{
		retcode = ex_con_cleanup(connection1, retcode);
	}
	if (connection2 != NULL)
	{
		retcode = ex_con_cleanup(connection2, retcode);
	}
	
	if (context != NULL)
	{
		retcode = ex_ctx_cleanup(context, retcode);
	}

	return (retcode == CS_SUCCEED) ? EX_EXIT_SUCCEED : EX_EXIT_FAIL;
}

/*
** CreateDatabase()
**
** Type of function:
** 	uctext function
**
** Purpose:
** 	This routine uses ex_create_db() to create a database. It then
** 	does a ex_use_db() to attach to it.
**
** Parameters:
** 	connection1	- Pointer to CS_CONNECTION structure.
** 	connection2	- Pointer to CS_CONNECTION structure.
**
** Return:
**	CS_SUCCEED if database was created.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE 
CreateDatabase(CS_CONNECTION *connection1, CS_CONNECTION *connection2)
{
	CS_RETCODE	retcode;

	/*
	** create database
	*/
	retcode = ex_create_db(connection1, Ex_dbname);
	if (retcode != CS_SUCCEED)
	{
		ex_error("CreateDatabase: ex_create_db() failed");
		return retcode;
	}

	/*
	** point both connections to the new database.
	*/
	if ((retcode = ex_use_db(connection1, Ex_dbname)) != CS_SUCCEED)
	{
		ex_error("CreateDatabase: ex_use_db(Ex_dbname) failed");
		return retcode;
	}

	/*
	if ((retcode = ex_use_db(connection2, Ex_dbname)) != CS_SUCCEED)
	{
		ex_error("CreateDatabase: ex_use_db(Ex_dbname) failed");
		return retcode;
	}
	*/

	return retcode;
}

/*
** RemoveDatabase()
**
** Type of function:
** 	uctext function
**
** Purpose:
** 	This routine uses ex_remove_db() to remove a database.
**
**	Note: The server will only let us drop a database if there are no
**	current users logged in and using it. Since multiple connections
**	have been established and are using the sampledb database, we need
**	to have them use master before the drop. This could be optimized by
**	first closing connection2, then do the remove of the database on
**	connection1.
**
** Parameters:
** 	connection1	- Pointer to CS_CONNECTION structure.
** 	connection2	- Pointer to CS_CONNECTION structure.
**
** Return:
**	CS_SUCCEED if database was removed.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE 
RemoveDatabase(CS_CONNECTION *connection1, CS_CONNECTION *connection2)
{
	CS_RETCODE	retcode;

	if ((retcode = ex_use_db(connection1, "master")) != CS_SUCCEED)
	{
		ex_error("RemoveDatabase: ex_use_db(master) failed");
		return retcode;
	}

	/*
	if ((retcode = ex_use_db(connection2, "master")) != CS_SUCCEED)
	{
		ex_error("RemoveDatabase: ex_use_db(master) failed");
		return retcode;
	}
	*/

	retcode = ex_remove_db(connection1, Ex_dbname);
	if (retcode != CS_SUCCEED)
	{
		ex_error("getsend: ex_remove_db() failed");
		return retcode;
	}

	return retcode;
}

/*
** CreateTable()
**
** Type of function:
** 	uctext function
**
** Purpose:
** 	This routine sets up the table for the getdata operation
**
** Parameters:
** 	connection	- Pointer to CS_CONNECTION structure.
**
** Return:
**	CS_SUCCEED if table was created.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE 
CreateTable(CS_CONNECTION *connection)
{

	CS_RETCODE	retcode;
	CS_CHAR		*cmdbuf;
	
	/* 
	** Allocate the buffer for the command string.
	*/
	cmdbuf = (CS_CHAR *) malloc(EX_BUFSIZE);
	if (cmdbuf == NULL)
	{
		ex_error("CreateTable: malloc() failed");
		return CS_MEM_ERROR;
	}

	/*
	** If the table exists, drop it.
	*/
	sprintf(cmdbuf, "if exists (select name from sysobjects \
			where name = \"%s\") drop table %s",
					Ex_tabname, Ex_tabname);
	if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
	{
		ex_error("CreateTable: ex_execute_cmd(drop table) failed");
		free (cmdbuf);
		return retcode;
	}
		
	/*
	** Set up and send the command to create the table.  
	*/
	sprintf(cmdbuf, "create table %s (i1 int, t text, f float, i2 int)",
				Ex_tabname);
	if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
	{
		ex_error("CreateTable: ex_execute_cmd(drop table) failed");
		free (cmdbuf);
		return retcode;
	}

	/*
	** Insert an initial value into the table
	*/
	sprintf(cmdbuf, "insert %s values (35, '%s', 20.3, 50)",
			Ex_tabname, EX_TXT_INIT_VALUE);
	if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
	{
		ex_error("CreateTable: ex_execute_cmd(insert table) failed");
	}
	free(cmdbuf);	
	return retcode;
}

/*
** DoGetSend()
**
** Type of function:
** 	uctext function
**
** Purpose:
** 	This routine is the main driver for doing the getdata operation.
** 	It assumes that tha database and tables have been set up.
**
** Parameters:
** 	connection	- Pointer to CS_CONNECTION structure.
** 	connection	- Pointer to CS_CONNECTION structure.
**
** Return:
*/
CS_STATIC CS_RETCODE 
DoGetSend(CS_CONNECTION *connection1, CS_CONNECTION *connection2)
{
	CS_RETCODE	retcode;
	TEXT_DATA	textdata;

	/* 
	** Retrieve the data initially in the table and
	** get the descriptor for the text data.
	*/
	if ((retcode = RetrieveData(connection1, &textdata)) != CS_SUCCEED)
	{
		ex_error("DoGetSend: RetrieveData failed");
		return retcode;
	}

	/*
	** Set iodesc values.
	*/
	textdata.iodesc.log_on_update = CS_TRUE;
	textdata.iodesc.iotype = CS_IOPARTIAL;
	textdata.iodesc.offset = CS_NULLDATA;
	textdata.iodesc.delete_length = CS_NULLDATA;

	/*
	** update the table with new text, validate that it's ok.
	*/
	retcode = UpdateTextData(connection2, &textdata, EX_TXT_UPD1_VALUE);
	if (retcode != CS_SUCCEED)
	{
		ex_error("DoGetSend: UpdateTextData failed");
		return retcode;
	}

	/*
	** Try different offset/delete_length and text.
	*/
	textdata.iodesc.offset = 10;
	textdata.iodesc.delete_length = 3;
	retcode = UpdateTextData(connection2, &textdata, EX_TXT_UPD2_VALUE);
	if (retcode != CS_SUCCEED)
	{
		ex_error("DoGetSend: UpdateTextData failed");
		return retcode;
	}

	/*
	** Try different no data.
	*/
	textdata.iodesc.offset = 4;
	textdata.iodesc.delete_length = 0;
	retcode = UpdateTextData(connection2, &textdata, "");
	if (retcode != CS_SUCCEED)
	{
		ex_error("DoGetSend: UpdateTextData failed");
		return retcode;
	}

	return retcode;
}

/*
** RetrieveData()
**
** Type of function:
** 	uctext function
** 
** Purpose:
**	This function queries the database for all columns and rows in the
**	table defined in EX_tabname.
**
** Parameters:
**	connection	- Pointer to CS_CONNECTION structure.
**	textdata	- Pointer to a TEXT_DATA structure to fill.
**
** Return:
**	CS_SUCCEED if text was retrieved correctly.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE
RetrieveData(CS_CONNECTION *connection, TEXT_DATA *textdata)
{
	CS_RETCODE	retcode;
	CS_INT		res_type;
	CS_CHAR         *cmdbuf;
	CS_COMMAND	*cmd;

	/* 
	** Allocate the buffer for the command string.
	*/
	cmdbuf = (CS_CHAR *) malloc(EX_BUFSIZE);
	if (cmdbuf == NULL)
	{
		ex_error("RetrieveData: malloc() failed");
		return CS_FAIL;
	}

	/*
	** Allocate a command handle to send the select query with
	*/
	if ((retcode = ct_cmd_alloc(connection, &cmd)) != CS_SUCCEED)
	{
		ex_error("RetrieveData: ct_cmd_alloc() failed");
		free(cmdbuf);
		return retcode;
	}

	/*
	** Set up the command to select from the table.  
	*/
	sprintf(cmdbuf, "select t from %s", Ex_tabname);
	retcode = ct_command(cmd, CS_LANG_CMD, cmdbuf, CS_NULLTERM, CS_UNUSED);
	if (retcode != CS_SUCCEED)
	{
		ex_error("RetrieveData: ct_command() failed");
		free(cmdbuf);
		ct_cmd_drop(cmd);
		return retcode;
	}
	free(cmdbuf);

	/*
	** Send the command to the server 
	*/
	if (ct_send(cmd) != CS_SUCCEED)
	{
		ex_error("RetrieveData: ct_send() failed");
		ct_cmd_drop(cmd);
		return retcode;
	}
					
	/*
	** Process the results of the command
	*/
	while ((retcode = ct_results(cmd, &res_type)) == CS_SUCCEED)
	{
		switch ((int)res_type)
		{
		    case CS_ROW_RESULT:
			/* 
			** Handle the expected row results.
			*/
			retcode = FetchResults(cmd, textdata);
			if (retcode != CS_SUCCEED)
			{
				ex_error("RetrieveData: FetchResults() failed");
				/*
				** Something failed so cancel all results.
				*/
				ct_cancel(NULL, cmd, CS_CANCEL_ALL);
				ct_cmd_drop(cmd);
				return retcode;
			}
			break;

		    case CS_CMD_SUCCEED:
		    case CS_CMD_DONE:
			/*
			** This means that the command succeeded or is
			** finished.
			*/
			break;

		    case CS_CMD_FAIL:
			/*
			** The server encountered an error while
			** processing our command.
			*/
			ex_error("RetrieveData: ct_result() returned CS_CMD_FAIL");
			ct_cmd_drop(cmd);
			return CS_FAIL;

		   default:
			/*
			** We got something unexpected.
			*/
			ex_error("RetrieveData: ct_results() returned unexpected result typ");
			/*
			** Cancel all results.
			*/
			ct_cancel(NULL, cmd, CS_CANCEL_ALL);
			break;
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
			retcode = CS_SUCCEED;
			break;

		case CS_FAIL:
			/*
			** ct_results() call failed.
			*/
			ex_error("RetrieveData: ct_results() failed");
			break;

		default:
			/*
			** We got an unexpected return value.
			*/
			ex_error("RetrieveData: ct_results() returned unexpected result");
			break;
	}
	
	/*
	** Drop our command structure.
	*/
	if ((retcode = ct_cmd_drop(cmd)) != CS_SUCCEED)
	{
		ex_error("RetrieveData: ct_cmd_drop() failed");
		return retcode;
	}

	return retcode;
}

/*
** FetchResults()
**
** Type of function:
** 	uctext function
** 
** Purpose:
**	This function retrieves the results expected
**	for this example. It demonstrates retrieving
**	a text column intermixed with columns of 
**	other data types. 
**
** Parameters:
** 	cmd		- Pointer to CS_COMMAND structure.
**	textdata	- Pointer to a TEXT_DATA structure to fill.
**
** Return:
**	CS_SUCCEED if text was fetched correctly.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE
FetchResults(CS_COMMAND *cmd, TEXT_DATA *textdata)
{
	CS_RETCODE	retcode;
	CS_TEXT		*txtptr;
	CS_INT		count;
	CS_INT		len;

	/*
	** Retrieve and display the results.
	*/
	while(((retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED,
		&count)) == CS_SUCCEED) || (retcode == CS_ROW_FAIL))
	{
		/*
		** Check for a recoverable error.
		*/
		if (retcode == CS_ROW_FAIL)
		{
			ex_error("FetchResults: ct_fetch() returned CS_ROW_FAIL");
			continue;
		}
		
		/*
		** Get the text data item in the second column.
		** Loop until we have all the data for this item.
		** The text used for this example could be retrieved
		** in one ct_get_data call, but data could be too large
		** for this to be the case. Instead, the data would have to
		** be retrieved in chunks. The example will retrieve
		** the text in 5 byte increments to demonstrate retrieving
		** data items in chunks.
		*/
		txtptr = textdata->textbuf;
		textdata->textlen = 0;
		do
		{
			retcode = ct_get_data(cmd, 1, txtptr, 5, &len);
			textdata->textlen += len;
			/*
			** Protect against overflowing the string buffer.
			*/
			if ((textdata->textlen + 5) > (EX_MAX_TEXT - 1))
			{
				break;
			}
			txtptr += len;
		} while (retcode == CS_SUCCEED);
		
		if (retcode != CS_END_DATA)
		{
			ex_error("FetchResults: ct_get_data() failed");
			return retcode;
		}
		
		/* 
		** Retrieve the descriptor of the text data. It is  
		** available while retrieving results of a select query.
		** The information will be needed for later updates.
		*/
		retcode = ct_data_info(cmd, CS_GET,  1, &textdata->iodesc);
		if (retcode != CS_SUCCEED)
		{
			ex_error("FetchResults: cs_data_info() failed");
			return retcode;
		}
		
	}
	
	/*
	** We're done processing rows. Check the
	** final return value of ct_fetch().
	*/
	if (retcode == CS_END_DATA)
	{
		/*
		** Succeeded in retrieving data.
		*/
		retcode = CS_SUCCEED;
	}
	else 
	{
		/*	
		** Something unexpected happened. 
		*/
		ex_error("FetchResults: ct_fetch() failed");
	}

	return retcode;
}

/*
** UpdateTextData()
**
** Type of function:
** 	uctext function
** 
** Purpose:
** 	This function updates the text column
**	with the specified text, and calls ProcessResults()
**	to retrieve the new timestamp for the updated
**	text column.
** 
** Parameters:
**	connection	- Pointer to CS_CONNECTION structure.
**	textdata	- Pointer to a TEXT_DATA structure to fill.
**	newdata		- Pointer to new data to send.
**
** Returns:
**	CS_SUCCEED if text was updated correctly.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE
UpdateTextData(CS_CONNECTION *connection, TEXT_DATA *textdata, char *newdata)
{
	CS_RETCODE	retcode;
	CS_INT		res_type;
	CS_COMMAND	*cmd;
	CS_INT		i;
	CS_TEXT		*txtptr;
	CS_INT		txtlen;

	/*
	** Allocate a command handle to send the text with
	*/
	if ((retcode = ct_cmd_alloc(connection, &cmd)) != CS_SUCCEED)
	{
		ex_error("UpdateTextData: ct_cmd_alloc() failed");
		return retcode;
	}

	/*
	** Inform Client-Library the next data sent will be 
	** used for a text or image update.
	*/
	if ((retcode = ct_command(cmd, CS_SEND_DATA_CMD, NULL, CS_UNUSED,
			CS_COLUMN_DATA)) != CS_SUCCEED)
	{
		ex_error("UpdateTextData: ct_command() failed");
		ct_cmd_drop(cmd);
		return retcode;
	}
		
	/*
	** Fill in the description information for the update
	** and send it to Client-Library.
	*/
	txtptr = (CS_TEXT *)newdata;
	txtlen = strlen(newdata);
	textdata->iodesc.total_txtlen = txtlen;

	retcode = ct_data_info(cmd, CS_SET, CS_UNUSED, &textdata->iodesc);
	if (retcode != CS_SUCCEED)
	{
		ex_error("UpdateTextData: ct_data_info() failed");
		ct_cmd_drop(cmd);
		return retcode;
	}

	/*
	** If txtlen == 0 do a 0 byte send.
	*/
	if (txtlen == 0)
	{
		retcode = ct_send_data(cmd, txtptr, (CS_INT)0);
		if (retcode != CS_SUCCEED)
		{
			ex_error("UpdateTextData: ct_send_data() failed");
			ct_cmd_drop(cmd);
			return retcode;
		}
	}

	/*
	** Send the text one byte at a time. This is not the best thing to do 
	** for performance reasons, but does demonstrate the ct_send_data()
	** can handle arbitrary amounts of data.
	*/
	for (i = 0; i < txtlen; i++, txtptr++)
	{
		retcode = ct_send_data(cmd, txtptr, (CS_INT)1);
		if (retcode != CS_SUCCEED)
		{
			ex_error("UpdateTextData: ct_send_data() failed");
			ct_cmd_drop(cmd);
			return retcode;
		}
	}

	/*
	** ct_send_data() does writes to internal network buffers. To insure
	** that all the data is flushed to the server, a ct_send() is done.
	*/
	if ((retcode = ct_send(cmd)) != CS_SUCCEED)
	{
		ex_error("UpdateTextData: ct_send() failed");
		ct_cmd_drop(cmd);
		return retcode;
	}

	/*
	** Process the results of the command
	*/
	while ((retcode = ct_results(cmd, &res_type)) == CS_SUCCEED)
	{
		switch ((int)res_type)
		{
		    case CS_PARAM_RESULT:
			/*
			** Retrieve a description of the parameter data.
			** Only timestamp data is expected in this example.
			*/
			retcode = ProcessTimestamp(cmd, textdata);
			if (retcode != CS_SUCCEED)
			{
				ex_error("UpdateTextData: ProcessTimestamp() failed");
				/*
				** Something failed so cancel all results.
				*/
				ct_cancel(NULL, cmd, CS_CANCEL_ALL);
				ct_cmd_drop(cmd);
				return retcode;
			}
			break;

		    case CS_STATUS_RESULT:
			/*	
			** Not expecting CS_STATUS_RESULT in this example, 
			** but if received results will be pending. Therefore, 
			** cancel the current result set.
			*/
			retcode = ct_cancel(NULL, cmd, CS_CANCEL_CURRENT);
			if (retcode != CS_SUCCEED)
			{
				ex_error("UpdateTextData: ct_cancel() failed");
				ct_cmd_drop(cmd);
				return retcode;
			}
			break;

		    case CS_CMD_SUCCEED:
		    case CS_CMD_DONE:
			/*
			** This means that the command succeeded or is
			** finished.
			*/
			break;

		    case CS_CMD_FAIL:
			/*
			** The server encountered an error while
			** processing our command.
			*/
			ex_error("UpdateTextData: ct_results() returned CS_CMD_FAIL");
			break;

		   default:
			/*
			** We got something unexpected.
			*/
			ex_error("UpdateTextData: ct_results() returned unexpected result typ");
			/*
			** Cancel all results.
			*/
			ct_cancel(NULL, cmd, CS_CANCEL_ALL);
			break;
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
			retcode = CS_SUCCEED;
			break;

		case CS_FAIL:
			/*
			** ct_results() call failed.
			*/
			ex_error("UpdateTextData: ct_results() failed");
			break;

		default:
			/*
			** We got an unexpected return value.
			*/
			ex_error("UpdateTextData: ct_results() returned unexpected result");
			break;
	}
	
	/*
	** Drop our command structure.
	*/
	if ((retcode = ct_cmd_drop(cmd)) != CS_SUCCEED)
	{
		ex_error("UpdateTextData: ct_cmd_drop() failed");
		return retcode;
	}

	return retcode;
}

/*
** ProcessTimestamp()
** 
** Purpose:
**	This function retrieves the new timestamp for
**	the updated text column into the CS_IODESC 
**	structure.
** 
** Parameters:
**	cmd		- Pointer to a CS_COMMAND structure.
**	textdata	- Pointer to a TEXT_DATA structure to fill.
**
** Returns:
**	CS_SUCCEED if text was updated correctly.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE
ProcessTimestamp(CS_COMMAND *cmd, TEXT_DATA *textdata)
{
	CS_RETCODE	retcode;
	CS_INT		count;
	CS_DATAFMT	datafmt;

	retcode = ct_describe(cmd, 1, &datafmt);
	if (retcode != CS_SUCCEED)
	{
		ex_error("ProcessTimestamp: ct_describe() failed");
		return retcode;
	}
		
	/*
	** Check if the data is a timestamp. If so, save it
	** to the CS_IODESC structure for future text updates.
	*/
	if (!(datafmt.status & CS_TIMESTAMP))
	{
		/* 
		** Unexpected parameter data was received.
		*/
		ex_error("ProcessTimestamp: unexpected parameter data received");
		return CS_FAIL;
	}

	/*
	** Bind the timestamp field of the io descriptor
	** to assign the new timestamp from the parameter
	** results.
	*/
	datafmt.maxlength = sizeof(textdata->iodesc.timestamp);
	datafmt.format     = CS_FMT_UNUSED;
	if ((retcode = ct_bind(cmd, 1, &datafmt,
		(CS_VOID *)textdata->iodesc.timestamp,
		&textdata->iodesc.timestamplen,
		NULL)) != CS_SUCCEED)
	{
		ex_error("ProcessTimestamp: ct_bind() failed");
		return retcode;
	}

	/*
	** Retrieve the parameter result containing the timestamp.
	*/
	retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, &count);
	if (retcode != CS_SUCCEED)
	{
		ex_error("ProcessTimestamp: ct_fetch() failed");
		return retcode;
	}

	/*
	** The timestamp was retrieved, so cancel the 
	** rest of the result set.
	*/
	retcode = ct_cancel(NULL, cmd, CS_CANCEL_CURRENT);
	if (retcode != CS_SUCCEED)
	{
		ex_error("ProcessTimestamp: ct_cancel() failed");
	}
	return retcode;
}

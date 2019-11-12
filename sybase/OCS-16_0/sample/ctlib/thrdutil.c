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
** 	This example file contains utility routines which are used by 
** 	multithreaded sample program. It demonstrates how an application can 
**	hide some of the implementation details of CT-Lib from higher level 
**	programs.
**
** Routines Used
** -------------
** 	cs_ctx_alloc	
** 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctpublic.h>
#include "example.h"
#include "thrdutil.h"
#include "thrdfunc.h"

CS_BOOL	display_server_address = CS_FALSE;

/*****************************************************************************
** 
** display functions 
** 
*****************************************************************************/

/*
** ex_display_dlen()
**
** Type of function:
** example program api
**
** Purpose:
**
** Returns:
** Nothing.
**
** Side Effects:
** None
*/

CS_INT CS_PUBLIC
ex_display_dlen(CS_DATAFMT *column)
{
	CS_INT		len;

	switch ((int) column->datatype)
	{
		case CS_CHAR_TYPE:
		case CS_VARCHAR_TYPE:
		case CS_TEXT_TYPE:
		case CS_IMAGE_TYPE:
			len = MIN(column->maxlength, MAX_CHAR_BUF);
			break;

		case CS_BINARY_TYPE:
		case CS_VARBINARY_TYPE:
			len = MIN((2 * column->maxlength) + 2, MAX_CHAR_BUF);
			break;

		case CS_BIT_TYPE:
		case CS_TINYINT_TYPE:
			len = 3;
			break;

		case CS_SMALLINT_TYPE:
			len = 6;
			break;

		case CS_INT_TYPE:
			len = 11;
			break;

		case CS_REAL_TYPE:
		case CS_FLOAT_TYPE:
			len = 20;
			break;

		case CS_MONEY_TYPE:
		case CS_MONEY4_TYPE:
			len = 24;
			break;

		case CS_DATETIME_TYPE:
		case CS_DATETIME4_TYPE:
			len = 30;
			break;

		case CS_NUMERIC_TYPE:
		case CS_DECIMAL_TYPE:
			len = (CS_MAX_PREC + 2);
			break;

		default:
			len = 12;
			break;
	}

	return MAX((CS_INT)strlen(column->name) + 1, len);
}

/*
** ex_display_header()
**
** Type of function:
** example program api
**
** Purpose:
**	Displays header. Caller should protect this using global mutex
**	to avoid output from different threads being mixed up.
**
** Returns:
** Nothing.
**
** Side Effects:
** None
*/

CS_RETCODE CS_PUBLIC
ex_display_header(CS_INT threadnum, CS_INT numcols, CS_DATAFMT columns[])
{
	CS_INT		i;
	CS_INT		l;
	CS_INT		j;
	CS_INT		disp_len;

	fprintf(stdout, "\nThread_%d:", threadnum);
	for (i = 0; i < numcols; i++)
	{
		disp_len = ex_display_dlen(&columns[i]);
		fprintf(stdout, "%s", columns[i].name);
		fflush(stdout);
		l = disp_len - strlen(columns[i].name);
		for (j = 0; j < l; j++)
		{
			fputc(' ', stdout);
			fflush(stdout);
		}
	}
	fprintf(stdout, "\nThread_%d:", threadnum);
	fflush(stdout);
	for (i = 0; i < numcols; i++)
	{
		disp_len = ex_display_dlen(&columns[i]);
		l = disp_len - 1;
		for (j = 0; j < l; j++)
		{
			fputc('-', stdout);
		}
		fputc(' ', stdout);
	}
	fputc('\n', stdout);

	return CS_SUCCEED;
}

/*
** ex_display_column()
**
** Type of function:
** example program api
**
** Purpose:
**	Displays a column. Caller should protect this using global mutex
**	to avoid output from different threads being mixed up.
**
** Returns:
** Nothing.
**
** Side Effects:
** None
*/

CS_RETCODE CS_PUBLIC
ex_display_column(CS_INT threadnum, CS_CONTEXT *context, CS_DATAFMT *colfmt, 
		  CS_VOID *data, CS_INT datalength, CS_INT indicator)
{
	char		*null = "NULL";
	char		*nc   = "NO CONVERT";
	char		*cf   = "CONVERT FAILED";
	CS_DATAFMT	srcfmt;
	CS_DATAFMT	destfmt;
	CS_INT		olen;
	CS_CHAR		wbuf[MAX_CHAR_BUF];
	CS_BOOL		res;
	CS_INT		i;
	CS_INT		disp_len;

	if (indicator == CS_NULLDATA)
	{
		olen = strlen(null);
		strcpy(wbuf, null);
	}
	else
	{
		cs_will_convert(context, colfmt->datatype, CS_CHAR_TYPE, &res);
		if (res != CS_TRUE)
		{
			olen = strlen(nc);
			strcpy(wbuf, nc);
		}
		else
		{
			srcfmt.datatype  = colfmt->datatype;
			srcfmt.format    = colfmt->format;
			srcfmt.locale    = colfmt->locale;
			srcfmt.maxlength = datalength;

			memset(&destfmt, 0, sizeof(destfmt));

			destfmt.maxlength = MAX_CHAR_BUF;
			destfmt.datatype  = CS_CHAR_TYPE;
			destfmt.format    = CS_FMT_NULLTERM;
			destfmt.locale    = NULL;

			if (cs_convert(context, &srcfmt, data, &destfmt,
					wbuf, &olen) != CS_SUCCEED)
			{
				olen = strlen(cf);
				strcpy(wbuf, cf);
			}
			else
			{
				/*
				** output length include null
				** termination
				*/
				olen -= 1;
			}
		}
	}
	fprintf(stdout, "%s", wbuf);
	fflush(stdout);

	disp_len = ex_display_dlen(colfmt);
	for (i = 0; i < (disp_len - olen); i++)
	{
		fputc(' ', stdout);
	}
	fflush(stdout);
	
	return CS_SUCCEED;
}

/*
** ex_display_rows()
**
** Type of function:
** example program api
**
** Purpose:
**	Display a set of row data. It also displays the header first.
**
** Returns:
**	CS_SUCCEED always 
**
** Side Effects:
**	This acquires global mutex to display the rows as a single
**	undivided sequence.
**
*/
 
CS_RETCODE CS_PUBLIC
ex_display_rows(CS_INT threadnum, CS_INT numrows, CS_INT numcols, 
		CS_DATAFMT *colfmt, EX_COLUMN_DATA *coldata)
{
	CS_INT row, col, j;
	CS_INT	disp_len;

	if (ex_take_global_mutex() != CS_SUCCEED)
	{
		fprintf(stdout, "Thread_%d:ex_take_global_mutex() failed.\n",
			threadnum);
		return CS_FAIL;
	}

	/* display header first */
	(void)ex_display_header(threadnum, numcols, colfmt);	

        for (row = 0; row < numrows; row++)
        {
                 /*
                 ** We have a row.  Loop through the columns
                 ** displaying the column values.
                 */
                 for (col = 0; col < numcols; col++)
                 {
                         /*
                         ** Display the column value
                         */
                         if (col == 0)
                         {
                                 fprintf(stdout, "Thread_%d:",
                                         threadnum);
                         }

                         fprintf(stdout, "%s", 
				EX_GET_COLUMN_VALUE(coldata, row, 
							col, colfmt));
                         fflush(stdout);

                         /*
                         ** If not last column, Print out spaces
                         ** between this column and next one.
                         */
                         if (col != numcols - 1)
                         {
                                 disp_len = ex_display_dlen(&colfmt[col]);
                                 disp_len -= coldata[col].valuelen[row] - 1;
                                 for (j = 0; j < disp_len; j++)
                                 {
                                         fputc(' ', stdout);
                                 }
                         }
                 }
                 fprintf(stdout, "\n");
                 fflush(stdout);
         }
	(void)ex_release_global_mutex();
	return (CS_SUCCEED);
}


/*****************************************************************************
** 
** error functions 
** 
*****************************************************************************/

/*
** ex_panic()
**
** Type of function:
** 	example program utility api
**
** Purpose:
** 	Reports a string message to EX_ERROR_OUT, and exits program.
**
** Returns:
** 	nothing
**
** Side Effects:
** 	Terminates program
*/

CS_VOID CS_PUBLIC
ex_panic(CS_INT threadnum, char *msg)
{
	EX_GLOBAL_PROTECT(fprintf(EX_ERROR_OUT,
			"Thread_%d:ex_panic: FATAL ERROR: %s\n",
	 		threadnum, msg));
	fflush(EX_ERROR_OUT);
	exit(EX_EXIT_FAIL);
}

/*
** ex_error()
**
** Type of function:
** 	example program utility api
**
** Purpose:
** 	Reports a string message to EX_ERROR_OUT.
**
** Returns:
** 	nothing
**
** Side Effects:
** 	none.
*/

CS_VOID CS_PUBLIC
ex_error(CS_INT threadnum, char *msg)
{
	EX_GLOBAL_PROTECT(fprintf(EX_ERROR_OUT, 
		"Thread_%d:ERROR: %s\n", threadnum, msg));
	fflush(EX_ERROR_OUT);
}

/*****************************************************************************
** 
** callback functions 
** 
*****************************************************************************/

/*
** ex_clientmsg_cb()
**
** Type of function:
** 	example program client message handler
**
** Purpose:
** 	Installed as a callback into Open Client.
**
** Returns:
** 	CS_SUCCEED
**
** Side Effects:
** 	None
*/

CS_RETCODE CS_PUBLIC
ex_clientmsg_cb(CS_CONTEXT *context, CS_CONNECTION *connection, CS_CLIENTMSG *errmsg)
{
	CS_INT	threadnum = 0;

	/*                            
	** Get Connection specific data - get thread number from the   
	** user data stored in connection structure
	*/
	(void)ex_get_threadnum(context, connection, &threadnum); 

	/* take global lock to print error msg. undivided */
        if (ex_take_global_mutex() != CS_SUCCEED)
        {
                fprintf(stdout, "Thread_%d:ex_take_global_mutex() failed.\n",
                        threadnum);
                return CS_FAIL;
        }
	
	fprintf(EX_ERROR_OUT, "\nThread_%d:Open Client Message:\n", threadnum);
	fprintf(EX_ERROR_OUT, 
		"Thread_%d:Message number: LAYER = (%d) ORIGIN = (%d) ",
		threadnum,
		CS_LAYER(errmsg->msgnumber), CS_ORIGIN(errmsg->msgnumber));
	fprintf(EX_ERROR_OUT, "SEVERITY = (%d) NUMBER = (%d)\n",
		CS_SEVERITY(errmsg->msgnumber), CS_NUMBER(errmsg->msgnumber));
	fprintf(EX_ERROR_OUT, "Thread_%d:Message String: %s\n", threadnum, 
			errmsg->msgstring);
	if (errmsg->osstringlen > 0)
	{
		fprintf(EX_ERROR_OUT, "Thread_%d:Operating System Error: %s\n",
			threadnum, errmsg->osstring);
	}
	fflush(EX_ERROR_OUT);

	(void)ex_release_global_mutex();

	return CS_SUCCEED;
}

/*
** ex_servermsg_cb()
**
** Type of function:
** 	example program server message handler
**
** Purpose:
** 	Installed as a callback into Open Client.
**
** Returns:
** 	CS_SUCCEED
**
** Side Effects:
** 	None
*/
CS_RETCODE CS_PUBLIC
ex_servermsg_cb(CS_CONTEXT *context, CS_CONNECTION *connection, CS_SERVERMSG *srvmsg)
{
        CS_INT  threadnum = 0;
 
        /*               
        ** Get Connection specific data - get thread number from the
        ** user data stored in connection structure
        */
        (void)ex_get_threadnum(context, connection, &threadnum); 
 
	/* take global lock to print error msg. undivided */
        if (ex_take_global_mutex() != CS_SUCCEED)
        {
                fprintf(stdout, "Thread_%d:ex_take_global_mutex() failed.\n",
                        threadnum);
                return CS_FAIL;
        }

	fprintf(EX_ERROR_OUT, "\nThread_%d:Server message:\n", threadnum);
	fprintf(EX_ERROR_OUT, "Thread_%d:Message number: %d, Severity %d, ",
		threadnum, srvmsg->msgnumber, srvmsg->severity);
	fprintf(EX_ERROR_OUT, "State %d, Line %d\n",
		srvmsg->state, srvmsg->line);
	
	if (srvmsg->svrnlen > 0)
	{
		fprintf(EX_ERROR_OUT, "Thread_%d:Server '%s'\n",
			threadnum, srvmsg->svrname);
	}
	
	if (srvmsg->proclen > 0)
	{
		fprintf(EX_ERROR_OUT, "Thread_%d: Procedure '%s'\n",
			threadnum, srvmsg->proc);
	}

	fprintf(EX_ERROR_OUT, "Thread_%d:Message String: %s\n",
			threadnum, srvmsg->text);
	fflush(EX_ERROR_OUT);

	(void)ex_release_global_mutex();

	return CS_SUCCEED;
}

/*****************************************************************************
** 
** utility functions 
** 
*****************************************************************************/

/*
** ex_init()
**
** Type of function:
** 	example program utility api
**
** Purpose:
** 	This function allocates the context structure, initializes
**	Client-Library, and installs the default callbacks. If this function
**	should fail, it will deallocated all memory allocations it has done.
**
**	The callbacks are installed at the context level. Other applications
**	may need to install callbacks at the connection level.
**
** Parameters:
** 	context		- Pointer to A Pointer to CS_CONTEXT structure.
**
** Returns:
** 	Result of initialization functions from CT-Lib.
**
*/

CS_RETCODE CS_PUBLIC
ex_init(CS_INT threadnum, CS_CONTEXT **context)
{
	CS_RETCODE	retcode;
	CS_INT		netio_type = CS_SYNC_IO;

	/*
	** Get a context handle to use.
	*/
	retcode = cs_ctx_alloc(EX_CTLIB_VERSION, context);
	if (retcode != CS_SUCCEED)
	{
		ex_error(threadnum, "ex_init: cs_ctx_alloc() failed");
		return retcode;
	}

        /*
        ** Put this thread number in context user data for later use.
        */
        retcode = cs_config(*context, CS_SET, CS_USERDATA, &threadnum,
                                CS_SIZEOF(CS_INT), NULL);
 
        if (retcode != CS_SUCCEED)
        {
                ex_error(threadnum, "cs_config(CS_SET CS_USERDATA) failed");
                return retcode;
        }

	/*
	** Initialize Open Client.
	*/
	retcode = ct_init(*context, EX_CTLIB_VERSION);
	if (retcode != CS_SUCCEED)
	{
		ex_error(threadnum, "ex_init: ct_init() failed");
		cs_ctx_drop(*context);
		*context = NULL;
		return retcode;
	}

#ifdef EX_API_DEBUG
	/*
	** ct_debug stuff. Enable this function right before any call to
	** OC-Lib that is returning failure.
	*/
	retcode = ct_debug(*context, NULL, CS_SET_FLAG, CS_DBG_API_STATES,
				NULL, CS_UNUSED);
	if (retcode != CS_SUCCEED)
	{
		ex_error(threadnum, "ex_init: ct_debug() failed");
	}
#endif

	/*
	** Install client and server message handlers.
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = ct_callback(*context, NULL, CS_SET, CS_CLIENTMSG_CB,
					(CS_VOID *)ex_clientmsg_cb);
		if (retcode != CS_SUCCEED)
		{
			ex_error(threadnum,
				"ex_init: ct_callback(clientmsg) failed");
		}
	}

	if (retcode == CS_SUCCEED)
	{
		retcode = ct_callback(*context, NULL, CS_SET, CS_SERVERMSG_CB,
				(CS_VOID *)ex_servermsg_cb);
		if (retcode != CS_SUCCEED)
		{
			ex_error(threadnum, 
				"ex_init: ct_callback(servermsg) failed");
		}
	}

	/* 
	** This is an synchronous example so set the input/output type
	** to synchronous (This is the default setting, but show an
	** example anyway).
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = ct_config(*context, CS_SET, CS_NETIO, &netio_type, 
						CS_UNUSED, NULL);
		if (retcode != CS_SUCCEED)
		{
			ex_error(threadnum, 
					"ex_init: ct_config(netio) failed");
		}
	}

	if (retcode != CS_SUCCEED)
	{
		ct_exit(*context, CS_FORCE_EXIT);
		cs_ctx_drop(*context);
		*context = NULL;
	}

	return retcode;
}

/*
** ex_connect()
**
** Type of function:
** 	example program utility api
**
** Purpose:
** 	This routine establishes a connection to the server identified
** 	in example.h and sets the CS_USER, CS_PASSWORD, and 
** 	CS_APPNAME properties for the connection. 
**
** 	If a connection property is NULL, it is not set.
**
** 	If this function should fail, it will deallocated all memory 
** 	allocations it has done.
**
** Parameters:
**	threadnum	- Id of the thread that invokes this call 
** 	context		- Pointer to CS_CONTEXT structure.
** 	connection 	- Pointer to CS_CONNECTION pointer.
**	appname		- Name of application calling this routine.
**	username	- user name to use when connecting.
**	password	- password to use when connecting.
**	server		- server to connect to.
**
** Return:
** 	Result of function calls from CT-Lib.
*/

CS_RETCODE CS_PUBLIC
ex_connect(CS_INT threadnum, CS_CONTEXT *context, CS_CONNECTION **connection, 
	   CS_CHAR *appname, CS_CHAR *username, CS_CHAR *password, CS_CHAR *server)
{
        CS_INT		len;
	CS_RETCODE	retcode;
	CS_TRANADDR	serveraddr;

	/* 
	** Allocate a connection structure. 
	*/
	retcode = ct_con_alloc(context, connection);
	if (retcode != CS_SUCCEED)
	{
		ex_error(threadnum, "ct_con_alloc failed");
		return retcode;
	}

	/*
	** Put this thread number in connection user data for later use. 
	*/
	retcode = ct_con_props(*connection, CS_SET, CS_USERDATA, &threadnum,
				CS_SIZEOF(CS_INT), NULL);

	if (retcode != CS_SUCCEED)
	{
		ex_error(threadnum, "ct_con_props(CS_SET CS_USERDATA) failed");
		return retcode;
	}
				
	/*	
	** If a username is defined, set the CS_USERNAME property.
	*/
	if (retcode == CS_SUCCEED && username != NULL)
	{
		if ((retcode = ct_con_props(*connection, CS_SET, CS_USERNAME, 
				username, CS_NULLTERM, NULL)) != CS_SUCCEED)
		{
			ex_error(threadnum, "ct_con_props(username) failed");
		}
	}

	/*	
	** If a password is defined, set the CS_PASSWORD property.
	*/
	if (retcode == CS_SUCCEED && password != NULL)
	{
		if ((retcode = ct_con_props(*connection, CS_SET, CS_PASSWORD, 
				password, CS_NULLTERM, NULL)) != CS_SUCCEED)
		{
			ex_error(threadnum, "ct_con_props(password) failed");
		}
	}

	/*	
	** Set the CS_APPNAME property.
	*/
	if (retcode == CS_SUCCEED && appname != NULL)
	{
		if ((retcode = ct_con_props(*connection, CS_SET, CS_APPNAME, 
				appname, CS_NULLTERM, NULL)) != CS_SUCCEED)
		{
			ex_error(threadnum, "ct_con_props(appname) failed");
		}
	}

	/*	
	** Open a Server connection.
	*/
	if (retcode == CS_SUCCEED)
	{
		len = (server == NULL) ? 0 : CS_NULLTERM;
		retcode = ct_connect(*connection, server, len);
		if (retcode != CS_SUCCEED)
		{
			ex_error(threadnum, "ct_connect failed");
		}

		if (display_server_address == CS_TRUE)
		{
			retcode = ct_con_props(*connection, CS_GET,
					CS_CONNECTED_ADDR, &serveraddr,
					sizeof (serveraddr), NULL);
			if (retcode == CS_SUCCEED)
			{
				EX_GLOBAL_PROTECT(
				  fprintf(EX_ERROR_OUT,
					"Thread_%d:INFO: %s %s\n",
					threadnum,
					serveraddr.addr_trantype.str_buffer,
					serveraddr.addr_tranaddress.str_buffer));
				fflush(stdout);
			}
		}
	}

	if (retcode != CS_SUCCEED)
	{
		ct_con_drop(*connection);
		*connection = NULL;
	}
	return retcode;
}

/*
** ex_con_cleanup()
**
** Type of function:
** 	example program utility api
**
** Purpose:
** 	The routine closes a connection and deallocates the
**	CS_CONNECTION structure.
**
** Parameters:
** 	connection	- Pointer to connection structure.
** 	status		- status of last interaction with this Client-Library
**			  If not ok, this routine will perform a force close.
**
** Returns:
** 	Result of function calls from CT-Lib.
*/

CS_RETCODE CS_PUBLIC
ex_con_cleanup(CS_CONNECTION *connection, CS_RETCODE status)
{
	CS_RETCODE	retcode;
	CS_INT		close_option;
        CS_INT  	threadnum = 0;
 
        /*               
        ** Get Connection specific data - get thread number from the
        ** user data stored in connection structure
        */
        (void)ex_get_threadnum(NULL, connection, &threadnum); 
 
	close_option = (status != CS_SUCCEED) ? CS_FORCE_CLOSE : CS_UNUSED;
	retcode = ct_close(connection, close_option);
	if (retcode != CS_SUCCEED)
	{
		ex_error(threadnum, "ex_con_cleanup: ct_close() failed");
		return retcode;
	}
	retcode = ct_con_drop(connection);
	if (retcode != CS_SUCCEED)
	{
		ex_error(threadnum, "ex_con_cleanup: ct_con_drop() failed");
		return retcode;
	}

	return retcode;
}

/*
** ex_ctx_cleanup()
**
** Type of function:
** 	example program utility api
**
** Purpose:
** 	The routine exits Client-Library deallocates the
**	CS_CONTEXT structure.
**
** Parameters:
** 	context		- Pointer to context structure.
** 	status		- status of last interaction with Client-Library.
**			  If not ok, this routine will perform a force exit.
**
** Returns:
** 	Result of function calls from CT-Lib.
*/

CS_RETCODE CS_PUBLIC
ex_ctx_cleanup(CS_CONTEXT *context, CS_RETCODE status)
{
	CS_RETCODE	retcode;
	CS_INT		exit_option;
        CS_INT  	threadnum = 0;
 
        /*               
        ** Get Connection specific data - get thread number from the
        ** user data stored in connection structure
        */
        (void)ex_get_threadnum(context, NULL, &threadnum); 

	exit_option = (status != CS_SUCCEED) ? CS_FORCE_EXIT : CS_UNUSED;
	retcode = ct_exit(context, exit_option);
	if (retcode != CS_SUCCEED)
	{
		ex_error(threadnum, "ex_ctx_cleanup: ct_exit() failed");
		return retcode;
	}
	retcode = cs_ctx_drop(context);
	if (retcode != CS_SUCCEED)
	{
		ex_error(threadnum, "ex_ctx_cleanup: cs_ctx_drop() failed");
		return retcode;
	}
	return retcode;
}

/*
** ex_execute_cmd()
**
** Type of function:   
**      example program utility api
**
** Purpose:
**      This routine sends a language command to the server. It expects no
**	rows or parameters to be returned from the server.
**
** Parameters:   
**      connection       - Pointer to CS_COMMAND structure.
**      cmdbuf          - The buffer containing the command.
**
** Return:
**      Result of functions called in CT-Lib
*/

CS_RETCODE CS_PUBLIC
ex_execute_cmd(CS_CONNECTION *connection, CS_CHAR *cmdbuf)
{
        CS_RETCODE      retcode;
        CS_INT          restype;
        CS_COMMAND      *cmd;
        CS_RETCODE      query_code;
        CS_INT  	threadnum = 0;
 
        /*               
        ** Get Connection specific data - get thread number from the
        ** user data stored in connection structure
        */
        (void)ex_get_threadnum(NULL, connection, &threadnum); 


        /*
        ** Get a command handle, store the command string in it, and
        ** send it to the server.
        */
        if ((retcode = ct_cmd_alloc(connection, &cmd)) != CS_SUCCEED)
        {
                ex_error(threadnum, "ex_execute_cmd: ct_cmd_alloc() failed");
                return retcode;
        }

        if ((retcode = ct_command(cmd, CS_LANG_CMD, cmdbuf, CS_NULLTERM,
                        CS_UNUSED)) != CS_SUCCEED)
        {
                ex_error(threadnum, "ex_execute_cmd: ct_command() failed");
                (void)ct_cmd_drop(cmd);
                return retcode;
        }

        if ((retcode = ct_send(cmd)) != CS_SUCCEED)
        {
                ex_error(threadnum, "ex_execute_cmd: ct_send() failed");
                (void)ct_cmd_drop(cmd);
                return retcode;
	}

        /*
        ** Examine the results coming back. If any errors are seen, the query
        ** result code (which we will return from this function) will be
        ** set to FAIL.
        */
        query_code = CS_SUCCEED;
        while ((retcode = ct_results(cmd, &restype)) == CS_SUCCEED)
        {
                switch((int)restype)
                {
                    case CS_CMD_SUCCEED:
                    case CS_CMD_DONE:
                        break;

                    case CS_CMD_FAIL:
                        query_code = CS_FAIL;
                        break;

                    case CS_STATUS_RESULT:
                        retcode = ct_cancel(NULL, cmd, CS_CANCEL_CURRENT);
                        if (retcode != CS_SUCCEED)
                        {
                                ex_error(threadnum, 
					"ex_execute_cmd: ct_cancel() failed");
                                query_code = CS_FAIL;
                        }
                        break;

                    default:
                        /*
                        ** Unexpected result type.
                        */
                        query_code = CS_FAIL;
                        break;
                }
                if (query_code == CS_FAIL)
                {
                        /*
                        ** Terminate results processing and break out of
                        ** the results loop
                        */
                        retcode = ct_cancel(NULL, cmd, CS_CANCEL_ALL);
                        if (retcode != CS_SUCCEED)
                        {
                                ex_error(threadnum,
					"ex_execute_cmd: ct_cancel() failed");
                        }
                        break;
                }
        }

        /*
        ** Clean up the command handle used
        */
        if (retcode == CS_END_RESULTS)
        {
                retcode = ct_cmd_drop(cmd);
                if (retcode != CS_SUCCEED)
                {
                        query_code = CS_FAIL;
                }
        }
        else
        {
                (void)ct_cmd_drop(cmd);
                query_code = CS_FAIL;
        }

        return query_code;
}

/*
** ex_fetch_data()
**
** Type of function:
** 	example program utility api
** 
** Purpose:
** 	This function processes fetchable results sets. The results include:
**
**		CS_ROW_RESULT
**		CS_CURSOR_RESULT 
**		CS_PARAM_RESULT 
**		CS_STATUS_RESULT 
**		CS_COMPUTE_RESULT 
**
**	Since the Client-Library result model has been unified, the same
**	apis are used for each of the above result types.
**
**	One caveat is the processing of CS_COMPUTE_RESULTs. The name field
**	sent from the server is typically zero length. To display a meaningful
**	header, the aggregate compute operator name should be found for the
**	column, and that name used instead. The compute example program has
**	code which demonstrates this.
**
** Parameters:
**	cmd - Pointer to command structure
**
** Return:
**	CS_MEM_ERROR	If an memory allocation failed.
**	CS_SUCCEED	If the data was displayed.
**	CS_FAIL		If no columns were present.
** 	<retcode>	Result of the Client-Library function if a failure was
**			returned.
**
*/

CS_RETCODE CS_PUBLIC
ex_fetch_data(CS_COMMAND *cmd)
{
	CS_RETCODE		retcode;
	CS_INT			num_cols;
	CS_INT			i;
	CS_INT			j;
	CS_INT			rows_read;
	CS_DATAFMT		*datafmt;
	EX_COLUMN_DATA		*coldata;
        CS_INT  		threadnum;
	CS_CONNECTION		*connection;
	CS_INT			rowsize;
	CS_INT			num_rows;	
				/* number of rows to get in a single fetch */
	CS_INT			row_count = 0;

	connection =(CS_CONNECTION *)NULL;
	rowsize = 0;
	threadnum = 0;

	/*
	** Get parent connection 
	*/
	retcode = ct_cmd_props(cmd, CS_GET, CS_PARENT_HANDLE, &connection,
				CS_UNUSED, NULL);
	if (retcode != CS_SUCCEED)
	{
		ex_error(threadnum, 
			"ex_fetch_data: ct_cmd_props(CS_PARENT_HANDLE) failed");
		return retcode;
	}
				 
        /*               
        ** Get Connection specific data - get thread number from the
        ** user data stored in connection structure
        */
        (void)ex_get_threadnum(NULL, connection, &threadnum); 

	/*
	** Find out how many columns there are in this result set.
	*/
	retcode = ct_res_info(cmd, CS_NUMDATA, &num_cols, CS_UNUSED, NULL);
	if (retcode != CS_SUCCEED)
	{
		ex_error(threadnum, "ex_fetch_data: ct_res_info() failed");
		return retcode;
	}

	/*
	** Make sure we have at least one column
	*/
	if (num_cols <= 0)
	{
		ex_error(threadnum, 
			"ex_fetch_data: ct_res_info() returned zero columns");
		return CS_FAIL;
	}

	/*
	** Our program variable, called 'coldata', is an array of 
	** EX_COLUMN_DATA structures. Each array element represents
	** one column.  Each array element will re-used for each row set.
	** In coldata, we allocate an array of values and thus use
	** array binding.
	**
	** First, allocate memory for the data element to process.
	*/
	coldata = (EX_COLUMN_DATA *)malloc(num_cols * sizeof (EX_COLUMN_DATA));
	if (coldata == NULL)
	{
		ex_error(threadnum, "ex_fetch_data: malloc() failed");
		return CS_MEM_ERROR;
	}

	datafmt = (CS_DATAFMT *)malloc(num_cols * sizeof (CS_DATAFMT));
	if (datafmt == NULL)
	{
		ex_error(threadnum, "ex_fetch_data: malloc() failed");
		free(coldata);
		return CS_MEM_ERROR;
	}

	/*
	** Loop through the columns getting a description of each one
	** and binding each one to a program variable.
	**
	** We're going to bind each column to a character string; 
	** this will show how conversions from server native datatypes
	** to strings can occur via bind.
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
			ex_error(threadnum,
				"ex_fetch_data: ct_describe() failed");
			break;
		}

		/*
		** update the datafmt structure to indicate that we want the
		** results in a null terminated character string.
		**
		** First, update datafmt.maxlength to contain the maximum
		** possible length of the column. To do this, call
		** ex_display_len() to determine the number of bytes needed
		** for the character string representation, given the
		** datatype described above.  Add one for the null
		** termination character.
		*/
		datafmt[i].maxlength = ex_display_dlen(&datafmt[i]) + 1;
		
		/*
		** Set datatype and format to tell bind we want things
		** converted to null terminated strings
		*/
		datafmt[i].datatype = CS_CHAR_TYPE;
		datafmt[i].format   = CS_FMT_NULLTERM;

		rowsize += MEM_ALIGN_SIZE(datafmt[i].maxlength);
	}

        if (retcode != CS_SUCCEED)
        {
                free(coldata);
                free(datafmt);
                return retcode;
        }

	rowsize += (MEM_ALIGN_SIZE(sizeof(CS_INT)) + 
			MEM_ALIGN_SIZE(sizeof(CS_SMALLINT))) * num_cols; 

	/* 
	** Find max. number of rows that we want to obtain in a single
	** fetch (Why? - just to limit memory being used).
	*/
	num_rows = MAX_MEM_BLOCK_SIZE / rowsize;

	for (i = 0; i < num_cols; i++)
	{
		coldata[i].value = (CS_CHAR *)NULL;
		coldata[i].valuelen = (CS_INT *)NULL;
		coldata[i].indicator = (CS_SMALLINT *)NULL;

                /*
                ** Allocate memory for the column string
                */
                coldata[i].value = (CS_CHAR *)malloc(
					datafmt[i].maxlength * num_rows);
                if (coldata[i].value == NULL)
                {
                        ex_error(threadnum, "ex_fetch_data: malloc() failed");
                        retcode = CS_MEM_ERROR;
                        break;
                }

                /*
                ** Allocate memory for the valuelen 
                */
                coldata[i].valuelen = (CS_INT *)malloc(
					sizeof(CS_INT) * num_rows);
                if (coldata[i].valuelen == NULL)
                {
                        ex_error(threadnum, "ex_fetch_data: malloc() failed");
                        retcode = CS_MEM_ERROR;
                        break;
                }

                /*
                ** Allocate memory for the indicator
                */
                coldata[i].indicator = (CS_SMALLINT *)malloc(
					sizeof(CS_SMALLINT) * num_rows);
                if (coldata[i].indicator == NULL)
                {
                        ex_error(threadnum, "ex_fetch_data: malloc() failed");
                        retcode = CS_MEM_ERROR;
                        break;
                }


		/*
		** Now do an array bind.
		*/
		datafmt[i].count = num_rows;
		retcode = ct_bind(cmd, (i + 1), &datafmt[i],
				coldata[i].value, coldata[i].valuelen,
				coldata[i].indicator);
		if (retcode != CS_SUCCEED)
		{
			ex_error(threadnum, "ex_fetch_data: ct_bind() failed");
			break;
		}
	}
	if (retcode != CS_SUCCEED)
	{
		for (j = 0; j <= i; j++)
		{
			free(coldata[j].value);
                        free(coldata[j].valuelen);
                        free(coldata[j].indicator);
		}
		free(coldata);
		free(datafmt);
		return retcode;
	}

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
			EX_GLOBAL_PROTECT(fprintf(stdout, 
					"Thread_%d:Error on row %d.\n",
					threadnum, row_count));
			fflush(stdout);
		}

		ex_display_rows(threadnum, rows_read, num_cols, datafmt, 
				coldata);
	}

	/*
	** Free allocated space.
	*/
	for (i = 0; i < num_cols; i++)
	{
		free(coldata[i].value);
                free(coldata[i].valuelen);
                free(coldata[i].indicator);
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
			EX_GLOBAL_PROTECT(fprintf(stdout, 
			"Thread_%d:All done processing rows - total %d.\n",
				threadnum, row_count));
			fflush(stdout);
			retcode = CS_SUCCEED;
			break;

		case CS_FAIL:
			/*
			** Something terrible happened.
			*/
			ex_error(threadnum, "ex_fetch_data: ct_fetch() failed");
			return retcode;
			/*NOTREACHED*/
			break;

		default:
			/*
			** We got an unexpected return value.
			*/
			ex_error(threadnum, 
		"ex_fetch_data: ct_fetch() returned an unexpected retcode");
			return retcode;
			/*NOTREACHED*/
			break;

	}

	return retcode;
}

/*****************************************************************************
** 
** sql based functions 
** 
*****************************************************************************/

/*
** ex_create_db()
**
** Type of function:
**      example program utility api
**
** Purpose:
**      This routine creates a database and opens it. It first checks
**      that the database does not already exists. If it does exist
**      the database is dropped before creating a new one.
**
** Parameters:
**      connection      - Pointer to CS_CONNECTION structure.
**      dbname          - The name to be used for the created database.
**
** Return:
**      Result of functions called in CT-Lib.
**
*/

CS_RETCODE CS_PUBLIC
ex_create_db(CS_CONNECTION *connection, char *dbname)
{
        CS_RETCODE      retcode;
        CS_CHAR         *cmdbuf;
        CS_INT  	threadnum = 0;
 
        /*               
        ** Get Connection specific data - get thread number from the
        ** user data stored in connection structure
        */
        (void)ex_get_threadnum(NULL, connection, &threadnum); 

        /*
        ** If the database already exists, drop it.
        */
        if ((retcode = ex_remove_db(connection, dbname)) != CS_SUCCEED)
        {
                ex_error(threadnum, "ex_create_db: ex_remove_db() failed");
        }

        /*
        ** Allocate the buffer for the command string.
        */
        cmdbuf = (CS_CHAR *) malloc(EX_BUFSIZE);
        if (cmdbuf == (CS_CHAR *)NULL)
        {
                ex_error(threadnum, "ex_create_db: malloc() failed");
                return CS_FAIL;
        }

        /*
        ** Set up and send the command to create the database.
        */
        sprintf(cmdbuf, "create database %s", dbname);
        if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
        {
                ex_error(threadnum, 
			"ex_create_db: ex_execute_cmd(create db) failed");
        }
	free(cmdbuf);
	return retcode;
}

/*
** ex_remove_db()
**
** Type of function:
**      example program utility api
**
** Purpose:
**      This routine removes a database. It first checks that
**      the database exists, and if so, removes it.
**
** Parameters:
**      connection      - Pointer to CS_CONNECTION structure.
**      dbname          - The name of the database to remove.
**
** Return:
**      Result of functions called in CT-Lib or CS_FAIL if a malloc failure
**      occurred.
*/

CS_RETCODE
ex_remove_db(CS_CONNECTION *connection, char *dbname)
{
        CS_RETCODE      retcode;
        CS_CHAR         *cmdbuf;
        CS_INT  	threadnum = 0;
 
        /*               
        ** Get Connection specific data - get thread number from the
        ** user data stored in connection structure
        */
        (void)ex_get_threadnum(NULL, connection, &threadnum); 

        /*
        ** Connect to the master database in order to
        ** remove the specified database.
        */
        if ((retcode = ex_use_db(connection, "master")) != CS_SUCCEED)
        {
                ex_error(threadnum, "ex_remove_db: ex_use_db(master) failed");
                return retcode;
        }

        /*
        ** Allocate the buffer for the command string.
        */
        cmdbuf = (CS_CHAR *) malloc(EX_BUFSIZE);
        if (cmdbuf == (CS_CHAR *)NULL)
        {
                ex_error(threadnum, "ex_remove_db: malloc() failed");
                return CS_FAIL;
        }

        /*
        ** Set up and send the command to check for and drop the
        ** database if it exists.
        */
        sprintf(cmdbuf,
                "if exists (select name from sysdatabases where name = \"%s\") \
                drop database %s", dbname, dbname);
        if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
        {
                ex_error(threadnum, 
			"ex_remove_db: ex_execute_cmd(drop db) failed");
        }
        free(cmdbuf);
        return retcode;
}

/*
** ex_use_db()
**
** Type of function:
**      example program utility api
**
** Purpose:
**      This routine changes the current database to the named db passed in.
**
** Parameters:
**      connection      - Pointer to CS_CONNECTION structure.
**      dbname          - The name of the database to use.
**
** Return:
**      Result of functions called in CT-Lib or CS_FAIL if a malloc failure
**      occured.
*/

CS_RETCODE
ex_use_db(CS_CONNECTION *connection, char *dbname)
{
        CS_RETCODE      retcode;
        CS_CHAR         *cmdbuf;
        CS_INT  threadnum = 0;
 
        /*               
        ** Get Connection specific data - get thread number from the
        ** user data stored in connection structure
        */
        (void)ex_get_threadnum(NULL, connection, &threadnum); 

        /*
        ** Allocate the buffer for the command string.
        */
        cmdbuf = (CS_CHAR *) malloc(EX_BUFSIZE);
        if (cmdbuf == (CS_CHAR *)NULL)
        {
                ex_error(threadnum, "ex_use_db: malloc() failed");
                return CS_FAIL;
        }

        /*
        ** Set up and send the command to use the database
        */
	sprintf(cmdbuf, "use %s\n", dbname);
        if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
        {
                ex_error(threadnum, "ex_use_db: ex_execute_cmd(use db) failed");
        }
        free(cmdbuf);
        return retcode;
}

/*
** ex_get_threadnum()
**
** Type of function:
**      example program utility api
**
** Purpose:
**	Retrieve thread number from connection or context level user data.
**
**	NOTE that, application can store thread level data into connection or
**	context user data (as in this sample) for use in callbacks. For
**	other purposes, Operating System provided primitives for thread 
**	specific data (also known as thread local data) may also be used. 
**
** Parameters:
**	context		- Pointer to CS_CONTEXT structure
**      connection      - Pointer to CS_CONNECTION structure.
**	threadnum	- Pointer to CS_INT - that receives the user data
**
** Return:
**      Result of functions called in CT-Lib 
*/
 
CS_RETCODE
ex_get_threadnum(CS_CONTEXT *context, CS_CONNECTION *connection, CS_INT *threadnum)
{
        CS_RETCODE      retcode;
 
        if (connection != (CS_CONNECTION *)NULL)
        {
                retcode = ct_con_props(connection, CS_GET, CS_USERDATA,
				threadnum, CS_SIZEOF(CS_INT), (CS_INT *)NULL);
		if (retcode != CS_SUCCEED)
		{
			ex_error(0,
		    "Thread_unknown:ct_con_props(CS_GET CS_USERDATA) failed\n");
		}
        }
        else
        {
                retcode = cs_config(context, CS_GET, CS_USERDATA, threadnum,
                                        CS_SIZEOF(CS_INT), (CS_INT *)NULL);
		if (retcode != CS_SUCCEED)
		{
			ex_error(0,
		    "Thread_unknown:cs_config(CS_GET CS_USERDATA) failed\n");
		}
        }
	return(retcode);
}


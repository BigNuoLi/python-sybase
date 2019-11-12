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
** 	Test program for the array binding utility routines in ctrowbuf.c.
**
** 	This program is a simple, clumsily coded interactive shell. It's sole
** 	use is to test the ctrowbuf.c routines.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctpublic.h>
#include "dbtoctex.h"
#include "ctrowbuf.h"
#include <exutils.h>

/*
** Global names used in this module
*/
CS_CHAR *Ex_appname  = "Test ctrowbuf.";
CS_CHAR	*Ex_dbname   = "pubs2";
CS_CHAR *Ex_server   = ((CS_CHAR *)NULL);
CS_CHAR *Ex_username = "sa";
CS_CHAR *Ex_password = "";

/*
** Global context structure.
*/
CS_CONTEXT	*Ex_context;

/*
** Prototypes for private routines in this file.
*/ 
int main PROTOTYPE ((
	void
	));

CS_STATIC CS_RETCODE CS_INTERNAL DoCompute PROTOTYPE ((
	CS_CONNECTION	*connection,
	CS_CHAR		*sqlbuf
	));
CS_STATIC CS_RETCODE CS_INTERNAL GetRows PROTOTYPE ((
	CS_COMMAND	        *cmd
	));

int
main()
{
	CS_CONNECTION	*connection;
	CS_RETCODE	retcode;
	CS_CHAR         sqlbuf[1024];

	EX_SCREEN_INIT();
	
	fprintf(stdout, "Test ctrowbuf routines.\n");
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


	retcode = ex_use_db(connection, "pubs2");
	if (retcode != CS_SUCCEED)
	{
		ex_panic("use pubs2 failed.");
	}

	while ( (retcode == CS_SUCCEED) )
	{

		fprintf(stdout, "Enter a SELECT query: \n");

		fgets(sqlbuf, 1024, stdin);

		if (strncmp(sqlbuf, "quit", strlen("quit")) == 0)
		{
			break; /* while */
		}

		retcode = DoCompute(connection, sqlbuf);

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

	return (retcode == CS_SUCCEED) ? NORMAL_EXIT : ERROR_EXIT;
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
DoCompute(CS_CONNECTION *connection, CS_CHAR *sqlbuf)
{

	CS_RETCODE	retcode;
	CS_COMMAND	*cmd;
	CS_INT		res_type;	/* result type from ct_results */

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
	retcode = ct_command(cmd, CS_LANG_CMD, sqlbuf, CS_NULLTERM, CS_UNUSED);
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
			fprintf(stdout, "<CMD_SUCCEED>\n");
			break;

		  case CS_CMD_DONE:
			/*
			** This means we're done with one result set.
			*/
			fprintf(stdout, "<CMD_DONE>\n");
			break;

		  case CS_CMD_FAIL:
			/*
			** This means that the server encountered an error 
			** while processing our command.
			*/
			ex_error("DoCompute: ct_results() returned CMD_FAIL");
			break;

		  case CS_ROW_RESULT:
		  case CS_COMPUTE_RESULT:
		  case CS_PARAM_RESULT:
		  case CS_STATUS_RESULT:
			retcode = GetRows(cmd);
			if (retcode != CS_SUCCEED)
			{
				ex_panic("GetRows() failed");
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

CS_STATIC CS_RETCODE CS_INTERNAL
GetRows(CS_COMMAND *cmd)
{
	CS_RETCODE	retcode;
	CS_INT		nrows, ncols, rownum, currow;
	ROW_BUFFER	*rowbuf;
	CS_INT		loop_state;
	char		numstring[64];


	/*
	** Initialization
	*/
	rownum = 0;

	retcode = rb_createbuf(cmd, 8, &ncols, &rowbuf);
	if (retcode != CS_SUCCEED)
	{
		ex_error("GetRows: rb_createbuf() failed");
		return retcode;
	}

#define STATE_STOP 0
#define STATE_CONTINUE 1
#define STATE_SCROLL 2
#define STATE_SCROLLNEXT 6
#define STATE_SKIP 3
#define STATE_SHOWNEXT 4
#define STATE_SHOWROW 5

	loop_state = STATE_SHOWNEXT;
	while ((loop_state != STATE_STOP )
		&& (retcode != CS_END_DATA) 
		&& (((retcode = rb_fillbuf(cmd, rowbuf, &nrows)) == CS_SUCCEED)
	            || (retcode == CS_END_DATA)))
	{
		if (nrows == 0)
		{
			loop_state = STATE_STOP;
		}
		if (loop_state == STATE_SHOWNEXT)
		{
			loop_state = STATE_SHOWROW;
		}
		else
		{
			loop_state = STATE_SCROLL;
			rownum = 0;
		}

		/*
		** Print rows at the user's discretion, or for scrolling, print
		** all rows in order.
		*/
		while(loop_state != STATE_SCROLLNEXT 
		      && loop_state != STATE_SHOWNEXT
		      && loop_state != STATE_STOP)
		{
			if (loop_state == STATE_SKIP)
			{
				loop_state = STATE_SHOWROW;
			}

			if (loop_state == STATE_SCROLL)
			{
				/*
				** Scrolling. Increment the row number,
				** if we've printed all the rows in
				**this buffer, go on to the next.
				*/
				rownum++;
				if (rownum > nrows) 
				{
					loop_state = STATE_SCROLLNEXT;
				}
			}
			else
			{
				/*
				** Ask for a row number. 0, -1, -2, etc.
				** cause state transitions.
				*/
				assert(loop_state == STATE_SHOWROW);
				fprintf(stdout,
					"\n%ld rows available. ", (long)nrows);
				fprintf(stdout, 
					"Enter row number (0 next buf, -1 cancel, -2 scroll all)\n");
				fprintf(stdout, "Row: ");
				fgets(numstring, CS_SIZEOF(numstring), stdin);
				fprintf(stdout, "\n");
				currow = rownum;
				rownum = atoi(numstring);
				if (rownum == 0) 
				{
					loop_state = STATE_SHOWROW;
					rownum = currow + 1;
					if (rownum > nrows) 
					{
						fprintf(stdout,
							"Not that many rows in buffer.\n");
						loop_state = STATE_SKIP;
					}
				}
				else if (rownum == -1) 
				{
					loop_state = STATE_STOP;
					retcode = ct_cancel(NULL, cmd,
							    CS_CANCEL_CURRENT);
					if (retcode != CS_SUCCEED)
					{
						ex_error("GetRows: ct_cancel(CS_CANCEL_CURRENT) failed.\n");
					}
				}
				else if (rownum == -2)
				{
					rownum = 1;
					loop_state = STATE_SCROLL;
				}
				else if (rownum > nrows) 
				{
					fprintf(stdout,
						"Not that many rows in buffer.\n");
					loop_state = STATE_SKIP;
				}
			} /* else */

			if (loop_state == STATE_SHOWROW)
			{
				fprintf(stdout,
					"--- Row %ld -----------------------------\n", 
					(long)rownum);
				retcode = rb_printrow(cmd, rowbuf, rownum,
						      stdout);
				if (retcode != CS_SUCCEED)
				{
					retcode = rb_dropbuf(rowbuf);
					return CS_FAIL; 
				}
			}
			else if (loop_state == STATE_SCROLL)
			{
				retcode = rb_printrow(cmd, rowbuf, rownum,
						      stdout);
				if (retcode != CS_SUCCEED)
				{
					retcode = rb_dropbuf(rowbuf);
					return CS_FAIL; 
				}
			}
		}
	} 

	if (retcode != CS_END_DATA && (loop_state != STATE_STOP))
	{
		retcode = rb_dropbuf(rowbuf);
		return CS_FAIL;
	}
		
	retcode = rb_dropbuf(rowbuf);
	if (retcode != CS_SUCCEED)
	{
		ex_error("rb_dropbuf() failed.");
	}
	return CS_SUCCEED;

}

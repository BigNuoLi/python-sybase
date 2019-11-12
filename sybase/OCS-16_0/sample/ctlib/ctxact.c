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
**	This file contains functions for Two Phase Commit Application. 
**
*/

#include	<stdio.h>
#include	<string.h>
#include	"ctxact.h"

/* 
** The name that will show up in sysprocesses 
*/
#define PROCESSNAME 	"Commit"	
#define DEF_COMMITNAME 	"DSCOMMIT"

CS_STATIC CS_RETCODE 	handle_results PROTOTYPE ((
			CS_COMMAND	*cmd
			));

/*
** open_commit()
**
** Purpose:
** 	Open_commit establishs a network connection for use 
** 	by other commit service calls.
**
** Parameters
**	conn		Connection structure initialized with the username,
**			password etc.
**	service		name to look up commit service in interfaces file
**			if NULL, use "DSCOMMIT"
**	
** Returns
**	CS_SUCCEED | CS_FAIL
**	
**
** Side Effects
**
**	On failure, the connection structure passed in is NOT deallocated;
**	this is left to the caller.
*/
CS_RETCODE
open_commit(CS_CONNECTION *conn, CS_CHAR *service, CS_INT service_len)
{
	CS_RETCODE	ret;

	if ((service == (CS_CHAR *)NULL) || (service_len == 0))
	{
		service = DEF_COMMITNAME;
		service_len = CS_NULLTERM;
	}

	/* Set the application name to 'Commit'				*/
	if (ct_con_props(conn, CS_SET, CS_APPNAME, (CS_VOID *)PROCESSNAME,
		CS_NULLTERM, (CS_INT *)NULL) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	/* Connect to the commit server					*/
	ret = ct_connect(conn, service, service_len);

	return (ret);
}

/*
** close_commit()
**
** Purpose:
** 	Close_commit shuts down network connection previous opened with 
**	open_commit.
**
** Parameters
**	conn		CS_CONNECTION for connection to be shut down
**	
**
** Returns
**	CS_SUCCEED | CS_FAIL
**	
**
** Side Effects
**	Closes a network connection.
**
**	The connection structure is not dropped; this is left to the user.
**
*/
CS_RETCODE
close_commit(CS_CONNECTION *conn)
{
	CS_RETCODE	ret;

	ret = ct_close(conn, CS_FORCE_CLOSE);

	return (ret);
}

/*
** build_xact_string()
**
** Purpose:
** 	Build_xact_string builds a string of the form:
**
**		"commid:service.xact_name"
**
** Parameters
**	xact_name   		Transaction name
**	xact_len		Length of the transaction name
**	service   		Server name
**	service_len		Length of the server name
**	commid   		The commit id returned by start_xact
**	result   		The buffer to put the string in.
**	result_len  		The length of the result buffer.
**	outlen			The length written to the result buffer
**
** Returns
**	none.
**
** Side Effects
**	none.
**
*/
CS_RETCODE
build_xact_string(CS_CHAR *xact_name, CS_INT xact_len, CS_CHAR *service, CS_INT service_len, 
		  CS_INT commid, CS_CHAR *result, CS_INT result_len, CS_INT *outlen)
{
	CS_INT		len;
	CS_CHAR		tmp_xact[CS_MAX_NAME +1];
	CS_CHAR		tmp_service[CS_MAX_NAME +1];

	/* Initialization						*/
	len = 0;

	if ((service == (CS_CHAR *)NULL) || (service_len == 0))
	{
		service = DEF_COMMITNAME;
		service_len = CS_NULLTERM;
	}

	/* Null terminate the service name				*/
	if (service_len == CS_NULLTERM)
	{
		len += strlen(service); 
	}
	else if (service_len > CS_MAX_NAME)
	{
		(CS_VOID)fprintf(stderr, "Service name too long...\n");
		return (CS_FAIL);
	}
	else
	{
		strncpy(tmp_service, service, service_len);
		tmp_service[service_len] = '\0';
		service = tmp_service;
		len += service_len;
	}

	/* Null terminate the transaction name				*/
	if (xact_len == CS_NULLTERM)
	{
		len += strlen(xact_name); 
	}
	else if (xact_len > CS_MAX_NAME)
	{
		(CS_VOID)fprintf(stderr, "Transaction name too long...\n");
		return (CS_FAIL);
	}
	else
	{
		strncpy(tmp_xact, xact_name, xact_len);
		tmp_xact[xact_len] = '\0';
		xact_name = tmp_xact;
		len += xact_len;
	}

	/* Make sure there is room in the result buffer			*/
	if (result_len <= (len + 8))
	{
		(CS_VOID)fprintf(stderr, 
			"Result buffer too small...(need %d)\n", (len + 9));
		return (CS_FAIL);
	}

	(CS_VOID)sprintf(result, "%d:%s.%s", commid, service, xact_name);

	if (outlen != (CS_INT *)NULL)
	{
		*outlen = strlen(result);
	}

	return (CS_SUCCEED);
}

/*
** start_xact()
**
** Purpose:
**	Start_xact creates an entry for this transaction in the commit
**	service table. Mark the entry as state BEGIN, with 'count'
**	sites participating. Return an id 'commid' used for identifying
**	the transaction when communicating with the service. The application
**	and xact names are not essential but are used for debugging or
**	monitoring the state of transactions.
**
** Parameters
**	conn		The CS_CONNECTION used to communicate via ctlib
**	app_name	The application name
**	app_len		The length of the application name
**	xact_name	The transaction name
**	xact_len	The length of the transaction name
**	count		Number of sites participating
**
** Returns
**	commid		id used to identify xact to service
**			or zero if failure.
**
** Side Effects
**	Changes commit table.
**
*/
CS_INT
start_xact(CS_CONNECTION *conn, CS_CHAR *app_name, CS_INT app_len, 
	   CS_CHAR *xact_name, CS_INT xact_len, CS_INT count)
{
	CS_INT		ret;
	CS_INT		commid;
	CS_RETCODE	retcode;
	CS_DATAFMT	fmt;
	CS_COMMAND	*cmd;

	/* Initialization						*/
	commid = 0;
	(CS_VOID)memset((char *)&fmt, 0, sizeof(CS_DATAFMT));

	/* Allocate a command handle 					*/ 
	if (ct_cmd_alloc(conn, &cmd) != CS_SUCCEED)
	{
		return (0);
	}

	if (ct_command(cmd, CS_RPC_CMD, "sp_start_xact", CS_NULLTERM,
		CS_UNUSED) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (0);
	}

	/* Prepare the application name parameter			*/
	strcpy(fmt.name, "@applname");
	fmt.namelen = strlen(fmt.name);
	fmt.datatype = CS_CHAR_TYPE;
	fmt.maxlength = 30;
	fmt.status = CS_INPUTVALUE;

	if (ct_param(cmd, &fmt, (CS_VOID *)app_name, app_len, 0) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (0);
	}

	/* Prepare the xact parameter					*/
	strcpy(fmt.name, "@xactname");
	fmt.namelen = strlen(fmt.name);
	fmt.datatype = CS_CHAR_TYPE;
	fmt.maxlength = 30;
	fmt.status = CS_INPUTVALUE;

	if (ct_param(cmd, &fmt, (CS_VOID *)xact_name, xact_len, 0) 
		!= CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (0);
	}

	/* Prepare the count parameter					*/
	strcpy(fmt.name, "@count");
	fmt.namelen = strlen(fmt.name);
	fmt.datatype = CS_INT_TYPE;
	fmt.maxlength = CS_SIZEOF(CS_INT);
	fmt.status = CS_INPUTVALUE;

	if (ct_param(cmd, &fmt, (CS_VOID *)&count, CS_UNUSED, 0) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (0);
	}

	/* Send to server 						*/
	if (ct_send(cmd) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (0);
	}

	/* 
	** Get the result (a smallint commid) 
	**
	** Someday, this sproc will return a 4-byte commid. Until then, we
	** have to do a little dance.
	*/
	while ((retcode = ct_results(cmd, &ret)) == CS_SUCCEED)
	{
		switch ((int)ret)
		{
		case CS_ROW_RESULT:
			/*
			** Could do a ct_describe() to ensure proper
			** data type/num columns being returned.
			*/
			fmt.namelen = 0;
			fmt.datatype = CS_INT_TYPE;
			fmt.maxlength = CS_SIZEOF(CS_INT);
			fmt.status = 0;
			if (ct_bind(cmd, 1, &fmt, (CS_VOID *)&commid,
				(CS_INT *)NULL, (CS_SMALLINT *)NULL)
				!= CS_SUCCEED)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (0);
			}

			while ((retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, 
				CS_UNUSED, NULL)) == CS_SUCCEED)
			{
				continue;
			}

			if (retcode != CS_END_DATA)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (0);
			}
			break;

		case CS_CMD_FAIL:
			(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
				cmd, CS_CANCEL_ALL);
			(CS_VOID)ct_cmd_drop(cmd);
			return (0);

		case CS_STATUS_RESULT:
		default:
			if (ct_cancel((CS_CONNECTION *)NULL, cmd, 
				CS_CANCEL_CURRENT) != CS_SUCCEED)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (0);
			}
			break;
		}
	}

	if (retcode != CS_END_RESULTS)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (0);
	}

	/* Free the command buffer					*/
	if (ct_cmd_drop(cmd) != CS_SUCCEED)
	{
		return (0);
	}


	return (commid);
}

/*
** commit_xact()
**
** Purpose:
**	Mark this transaction as committed in the commit table so any
**	subsequent call to probe_xact() for this commid will return
**	'commit'.
**
** Parameters
**	conn		The CS_CONNECTION used to communicate
**	commid		id used to identify xact to service
**
** Returns
**	CS_SUCCEED or CS_FAIL
**	
**
** Side Effects
**	Changes commit table.
**
*/
CS_RETCODE
commit_xact(CS_CONNECTION *conn, CS_INT commid)
{
	CS_INT		ret;
	CS_RETCODE	retcode;
	CS_DATAFMT	fmt;
	CS_COMMAND	*cmd;
	CS_INT		p_result;

	/* Initialization						*/
	p_result  = -1;
	(CS_VOID)memset((char *)&fmt, 0, sizeof(CS_DATAFMT));

	/* Allocate a command handle 					*/ 
	if (ct_cmd_alloc(conn, &cmd) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	if (ct_command(cmd, CS_RPC_CMD, "sp_commit_xact", CS_NULLTERM,
		CS_UNUSED) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	/* Prepare the count parameter					*/
	strcpy(fmt.name, "@commid");
	fmt.namelen = strlen(fmt.name);
	fmt.datatype = CS_INT_TYPE;
	fmt.maxlength = CS_SIZEOF(CS_INT);
	fmt.status = CS_INPUTVALUE;

	if (ct_param(cmd, &fmt, (CS_VOID *)&commid, CS_UNUSED, 0) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	/* Send to server 						*/
	if (ct_send(cmd) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}


	/* Process the result from the commit				*/
	while ((retcode = ct_results(cmd, &ret)) == CS_SUCCEED)
	{
		switch ((int)ret)
		{
		case CS_ROW_RESULT:
			/*
			** Could do a ct_describe() to ensure proper
			** data type/num columns being returned.
			*/
			fmt.namelen = 0;
			fmt.datatype = CS_INT_TYPE;
			fmt.maxlength = CS_SIZEOF(CS_INT);
			fmt.status = 0;
			if (ct_bind(cmd, 1, &fmt, (CS_VOID *)&p_result,
				(CS_INT *)NULL, (CS_SMALLINT *)NULL)
				!= CS_SUCCEED)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (CS_FAIL);
			}

			while ((retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, 
				CS_UNUSED, NULL)) == CS_SUCCEED)
			{
				continue;
			}

			if (retcode != CS_END_DATA)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (CS_FAIL);
			}
			break;

		case CS_CMD_FAIL:
			(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
				cmd, CS_CANCEL_ALL);
			(CS_VOID)ct_cmd_drop(cmd);
			return (CS_FAIL);

		case CS_STATUS_RESULT:
		default:
			if (ct_cancel((CS_CONNECTION *)NULL, cmd, 
				CS_CANCEL_CURRENT) != CS_SUCCEED)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (CS_FAIL);
			}
			break;
		}
	}

	if (retcode != CS_END_RESULTS)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}


	/* Free the command buffer					*/
	if (ct_cmd_drop(cmd) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	return ((p_result == 0) ? CS_SUCCEED : CS_FAIL);
}

/*
** abort_xact()
**
** Purpose:
**	Mark this transaction as aborted in the commit table so any
**	subsequent call to probe_xact() for this commid will return
**	'abort'.
**
** Parameters
**	conn		The CS_CONNECTION used to communicate
**	commid		id used to identify xact to service
**
** Returns
**	CS_SUCCEED or CS_FAIL
**	
**
** Side Effects
**	Changes commit table.
**
*/
CS_RETCODE 
abort_xact(CS_CONNECTION *conn, CS_INT commid)
{
	CS_INT 		ret;
	CS_DATAFMT	fmt;
	CS_COMMAND	*cmd;

	/* Initialization						*/
	(CS_VOID)memset((char *)&fmt, 0, sizeof(CS_DATAFMT));

	/* Allocate a command handle 					*/ 
	if (ct_cmd_alloc(conn, &cmd) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	if (ct_command(cmd, CS_RPC_CMD, "sp_abort_xact", CS_NULLTERM,
		CS_UNUSED) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	/* Prepare the count parameter					*/
	strcpy(fmt.name, "@commid");
	fmt.namelen = strlen(fmt.name);
	fmt.datatype = CS_INT_TYPE;
	fmt.maxlength = CS_SIZEOF(CS_INT);
	fmt.status = CS_INPUTVALUE;

	if (ct_param(cmd, &fmt, (CS_VOID *)&commid, CS_UNUSED, 0) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	/* Send to server 						*/
	if (ct_send(cmd) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	ret = handle_results(cmd);

	/* Free the command buffer					*/
	if (ct_cmd_drop(cmd) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	return (ret);
}

/*
** remove_xact()
**
** Purpose:
**	Decrement the count of sites participating in the transaction by N.
**	Delete the commit table entry entirely if the count reaches zero.
**
** Parameters
**	conn		The CS_CONNECTION used to communicate
**	commid		id used to identify xact to service
**	n		Number of sites to remove
**
** Returns
**	CS_SUCCEED or CS_FAIL
**	
**
** Side Effects
**	Changes commit table.
**
*/
CS_RETCODE 
remove_xact(CS_CONNECTION *conn, CS_INT commid, CS_INT n)
{
	CS_INT		ret;
	CS_DATAFMT	fmt;
	CS_COMMAND	*cmd;

	/* Initialization						*/
	(CS_VOID)memset((char *)&fmt, 0, sizeof(CS_DATAFMT));

	/* Allocate a command handle 					*/ 
	if (ct_cmd_alloc(conn, &cmd) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	if (ct_command(cmd, CS_RPC_CMD, "sp_remove_xact", CS_NULLTERM,
		CS_UNUSED) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	/* Prepare the count parameter					*/
	strcpy(fmt.name, "@commid");
	fmt.namelen = strlen(fmt.name);
	fmt.datatype = CS_INT_TYPE;
	fmt.maxlength = CS_SIZEOF(CS_INT);
	fmt.status = CS_INPUTVALUE;

	if (ct_param(cmd, &fmt, (CS_VOID *)&commid, CS_UNUSED, 0) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	/* Prepare the count parameter					*/
	strcpy(fmt.name, "@count");
	fmt.namelen = strlen(fmt.name);
	fmt.datatype = CS_INT_TYPE;
	fmt.maxlength = CS_SIZEOF(CS_INT);
	fmt.status = CS_INPUTVALUE;

	if (ct_param(cmd, &fmt, (CS_VOID *)&n, CS_UNUSED, 0) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	/* Send to server 						*/
	if (ct_send(cmd) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	/* Process the results/status (no rows expected) 		*/
	ret = handle_results(cmd);

	/* Free the command buffer					*/
	if (ct_cmd_drop(cmd) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	return (ret);
}

/*
** scan_xact()
**
** Purpose:
** 	Scan_xact is a debugging routine used to retrieve the commit
** 	service row for a given commid. If commid is -1, scan_xact
** 	can be called repeatedly (while scan_xact(..))
** 	to return all entries in the commit service.
**
** Parameters
**	conn		The CS_CONNECTION used to communicate
**	commid		id used to identify xact to service
**
**
** Returns
**	CS_SUCCEED or CS_FAIL
**	
**
** Side Effects
**
*/ 
CS_RETCODE 
scan_xact(CS_CONNECTION *conn, CS_INT commid)
{
	CS_INT		ret;
	CS_RETCODE	retcode;
	CS_DATAFMT	fmt;
	CS_COMMAND	*cmd;

	/* Initialization						*/
	(CS_VOID)memset((char *)&fmt, 0, sizeof(CS_DATAFMT));

	/* Allocate a command handle 					*/ 
	if (ct_cmd_alloc(conn, &cmd) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	if (ct_command(cmd, CS_RPC_CMD, "sp_scan_xact", CS_NULLTERM,
		CS_UNUSED) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	/* Prepare the count parameter					*/
	strcpy(fmt.name, "@commid");
	fmt.namelen = strlen(fmt.name);
	fmt.datatype = CS_INT_TYPE;
	fmt.maxlength = CS_SIZEOF(CS_INT);
	fmt.status = CS_INPUTVALUE;

	if (ct_param(cmd, &fmt, (CS_VOID *)&commid, CS_UNUSED, 0) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	/* Send to server 						*/
	if (ct_send(cmd) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	while ((retcode = ct_results(cmd, &ret)) == CS_SUCCEED)
	{
		switch ((int)ret)
		{
		case CS_ROW_RESULT:
		case CS_COMPUTE_RESULT:
			if (pr_head(cmd) != CS_SUCCEED)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (CS_FAIL);
			}

			if (pr_line(cmd) != CS_SUCCEED)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (CS_FAIL);
			}

			if (pr_row(cmd) != CS_SUCCEED)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (CS_FAIL);
			}
			break;

		case CS_STATUS_RESULT:
			while ((retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, 
				CS_UNUSED, NULL)) == CS_SUCCEED)
			{
				continue;
			}

			if (retcode != CS_END_DATA)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (CS_FAIL);
			}
			break;

		case CS_CMD_FAIL:
			(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, 
				CS_CANCEL_ALL);
			(CS_VOID)ct_cmd_drop(cmd);
			return (CS_FAIL);

		default:
			break;
		}
	}

	if (retcode != CS_END_RESULTS)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	/* Free the command buffer					*/
	if (ct_cmd_drop(cmd) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	return (CS_SUCCEED);
}

/*
** stat_xact()
**
** Purpose:
** 	Stat_xact returns the status of the transaction identified by 'commid'.
**
** Parameters
**	conn		The connection to use.
**	commid		id used to indentify the transaction.
**
** Returns
**	'a' (abort), 'b' (begin), 'c' (commit), 'u' (unknown), or -1 (fail).
**	
**
** Side Effects
**
*/
CS_INT
stat_xact(CS_CONNECTION *conn, CS_INT commid)
{
	CS_INT		ret;
	CS_RETCODE	retcode;
	CS_CHAR		commitchar;
	CS_DATAFMT	fmt;
	CS_COMMAND	*cmd;

	/* Initialization						*/
	commitchar = (CS_CHAR)-1;

	/* Allocate a command handle 					*/ 
	if (ct_cmd_alloc(conn, &cmd) != CS_SUCCEED)
	{
		return (-1);
	}

	if (ct_command(cmd, CS_RPC_CMD, "sp_stat_xact", CS_NULLTERM,
		CS_UNUSED) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (-1);
	}

	/* Prepare the count parameter					*/
	strcpy(fmt.name, "@commid");
	fmt.namelen = strlen(fmt.name);
	fmt.datatype = CS_INT_TYPE;
	fmt.maxlength = CS_SIZEOF(CS_INT);
	fmt.status = CS_INPUTVALUE;

	if (ct_param(cmd, &fmt, (CS_VOID *)&commid, CS_UNUSED, 0) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	/* Send to server 						*/
	if (ct_send(cmd) != CS_SUCCEED)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (-1);
	}

	/* Process the result from the commit				*/
	while ((retcode = ct_results(cmd, &ret)) == CS_SUCCEED)
	{
		switch ((int)ret)
		{
		case CS_ROW_RESULT:
			/*
			** Could do a ct_describe() to ensure proper
			** data type/num columns being returned.
			*/
			fmt.namelen = 0;
			fmt.datatype = CS_CHAR_TYPE;
			fmt.maxlength = 1;
			fmt.status = 0;
			if (ct_bind(cmd, 1, &fmt, (CS_VOID *)&commitchar,
				(CS_INT *)NULL, (CS_SMALLINT *)NULL)
				!= CS_SUCCEED)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (CS_FAIL);
			}

			while ((retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, 
				CS_UNUSED, NULL)) == CS_SUCCEED)
			{
				continue;
			}

			if (retcode != CS_END_DATA)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (CS_FAIL);
			}
			break;

		case CS_CMD_FAIL:
			(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
				cmd, CS_CANCEL_ALL);
			(CS_VOID)ct_cmd_drop(cmd);
			return (CS_FAIL);

		case CS_STATUS_RESULT:
		default:
			if (ct_cancel((CS_CONNECTION *)NULL, cmd, 
				CS_CANCEL_CURRENT) != CS_SUCCEED)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				(CS_VOID)ct_cmd_drop(cmd);
				return (CS_FAIL);
			}
			break;
		}
	}

	if (retcode != CS_END_RESULTS)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		(CS_VOID)ct_cmd_drop(cmd);
		return (CS_FAIL);
	}

	/* Free the command buffer					*/
	if (ct_cmd_drop(cmd) != CS_SUCCEED)
	{
		return (-1);
	}

	return ((CS_INT)commitchar);
}

/*
** handle_results()
**
** Purpose:
**	Process the results of a command.  No results are expected.
**
** Parameters
**	cmd		The CS_COMMAND used to communicate
**
** Returns
**	CS_SUCCEED or CS_FAIL
**	
** Side Effects
**	none.
*/
CS_STATIC CS_RETCODE
handle_results(CS_COMMAND *cmd)
{
	CS_INT		ret;
	CS_RETCODE	retcode;

	while ((retcode = ct_results(cmd, &ret)) == CS_SUCCEED)
	{
		switch ((int)ret)
		{
		case CS_CMD_FAIL:
			(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, 
				CS_CANCEL_ALL);
			return (CS_FAIL);

		case CS_ROW_RESULT:
		case CS_STATUS_RESULT:
		default:
			if (ct_cancel((CS_CONNECTION *)NULL, cmd, 
				CS_CANCEL_CURRENT) != CS_SUCCEED)
			{
				(CS_VOID)ct_cancel((CS_CONNECTION *)NULL,
					cmd, CS_CANCEL_ALL);
				return (CS_FAIL);
			}
			break;
		}
	}

	if (retcode != CS_END_RESULTS)
	{
		(CS_VOID)ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
		return (CS_FAIL);
	}

	return (CS_SUCCEED);
}

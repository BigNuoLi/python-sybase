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
**	This example demonstrates using external config file to connect to a
**	server at run time. It connects to "server1" which is defined in 
**	${SYBASE}/${SYBASE_OCS}/config/ocs.cfg. User can change the server
**	name prior to this example.
**
** 	The following statement is a sample of ocs.cfg that you can put
**	to ${SYBASE}/${SYBASE_OCS}/config in Unix or %SYBASE%\%SYBASE_OCS%\ini
**	on  Windows to run this example:
**
**		[DEFAULT]
**			; This default section should always be present
**		[server1]
**			CS_SERVERNAME = connect50
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
**	ct_close()
**	ct_con_drop()
**	ct_exit()
**	ct_ctx_drop()
**
** Input
** -----
**	This example uses a ocs.cfg and gets the ASE name from
**	its server1 section.
**
** Output
** ------
**	None.
**	
** Server Dependencies
** -------------------
**	None.
**
** Server Tables
** -------------
**	None.
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
CS_CHAR *Ex_appname  = "exconfig";
CS_CHAR	*Ex_dbname   = "pubs2";
CS_CHAR *Ex_server   = "server1";	/* server1, section defined in ocs.cfg*/
CS_CHAR *Ex_username = EX_USERNAME;
CS_CHAR *Ex_password = EX_PASSWORD;


/*
** Global context structure
*/
CS_CONTEXT	*Ex_context;


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
        CS_INT		len;
	CS_INT		prpvalue;


	EX_SCREEN_INIT();
	
	fprintf(stdout, "Exconfig Example\n");
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
	** Allocate a connection structure. 
	*/
	retcode = ct_con_alloc(Ex_context, &connection);
	if (retcode != CS_SUCCEED)
	{
		ex_error("ct_con_alloc failed");
		return retcode;
	}

	/*	
	** set property to use external configuration file.
	*/
	prpvalue = CS_TRUE;
	if ((retcode = ct_con_props(connection, CS_SET, CS_EXTERNAL_CONFIG,
			&prpvalue, CS_UNUSED, NULL)) != CS_SUCCEED)
	{
		ex_error("ct_con_props(CS__EXTERNALCONFIG) failed");
	}

	/*	
	** If a username is defined, set the CS_USERNAME property.
	*/
	if (retcode == CS_SUCCEED && Ex_username != NULL)
	{
		if ((retcode = ct_con_props(connection, CS_SET, CS_USERNAME, 
				Ex_username, CS_NULLTERM, NULL)) != CS_SUCCEED)
		{
			ex_error("ct_con_props(Ex_username) failed");
		}
	}


	/*	
	** If a password is defined, set the CS_PASSWORD property.
	*/
	if (retcode == CS_SUCCEED && Ex_password != NULL)
	{
		if ((retcode = ct_con_props(connection, CS_SET, CS_PASSWORD, 
				Ex_password, CS_NULLTERM, NULL)) != CS_SUCCEED)
		{
			ex_error("ct_con_props(Ex_password) failed");
		}
	}

	/*	
	** Set the CS_APPNAME property.
	*/
	if (retcode == CS_SUCCEED && Ex_appname != NULL)
	{
		if ((retcode = ct_con_props(connection, CS_SET, CS_APPNAME, 
				Ex_appname, CS_NULLTERM, NULL)) != CS_SUCCEED)
		{
			ex_error("ct_con_props(Ex_appname) failed");
		}
	}

	/*	
	** set property to get connecting server from the default external
	** configuration file, ${SYBASE}/${SYBASE_OCS}/config/ocs.cfg
	** (%SYBASE%\%SYBASE_OCS%\ini\ocs.cfg on Windows)
	*/
	if ((retcode = ct_con_props(connection, CS_SET, CS_CONFIG_BY_SERVERNAME,
			&prpvalue, CS_UNUSED, NULL)) != CS_SUCCEED)
	{
		ex_error("ct_con_props(CS_CONFIG_BY_SERVERNAME) failed");
	}

	/*	
	** Open a Server connection.
	*/
	if (retcode == CS_SUCCEED)
	{
		len = (Ex_server == NULL) ? 0 : CS_NULLTERM;
		retcode = ct_connect(connection, Ex_server, len);
		if (retcode != CS_SUCCEED)
		{
			ex_error("ct_connect failed");
		}
		else
		{
			fprintf(stdout, "Connect to server1 using ocs.cfg succeed.\n");
			fflush(stdout);
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
	
	if (Ex_context != NULL)
	{
		retcode = ex_ctx_cleanup(Ex_context, retcode);
	}

	return (retcode == CS_SUCCEED) ? EX_EXIT_SUCCEED : EX_EXIT_FAIL;
}


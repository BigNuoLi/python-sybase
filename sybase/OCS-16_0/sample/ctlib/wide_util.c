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
**	This file contains generic routines which are used by the wide_*
**	sample programs. 
*/

#include <stdio.h>
#include <ctpublic.h>
#include "example.h"
#include "exutils.h"
#include "wide_example.h"

char	row_buffer[MAX_COLUMN][MAX_COLSIZE];

/* 
** init_db()
** 
** Type of function:
**	initialize the library and callback.
**
** Purpose:
**	This function allocates the context and initializes the library.
**	It also installs the callback routines and is called at the very
**	beginning of several sample programs.
**
** Parameters:
**	cntx_ptr - Pointer to the contex structure.
**
** Return:
**	CS_SUCCEED if everything goes fine
**	otherwise a failure message will be displayed and failure code will
**	be returned.
*/
CS_RETCODE
init_db(CS_CONTEXT **cntx_ptr)
{
	CS_RETCODE      retcode;

	/* 
	** allocate a context 
	*/
	retcode = cs_ctx_alloc(EX_CTLIB_VERSION, cntx_ptr);
	RETURN_IF(retcode, "cs_ctx_alloc ");

	/* 	
	** initialize the library 
	*/
	retcode = ct_init(*cntx_ptr, EX_CTLIB_VERSION);
	RETURN_IF(retcode, "ct_init ");

	/* 
	** install the server message callback 
	*/
	retcode = ct_callback(*cntx_ptr, NULL, CS_SET, CS_SERVERMSG_CB,
		              (CS_VOID *)ex_servermsg_cb) ;
	RETURN_IF(retcode, "ct_callback ");

	/* 
	** install the client message callback 
	*/
	retcode = ct_callback(*cntx_ptr, NULL, CS_SET, CS_CLIENTMSG_CB,
		              (CS_VOID *)ex_clientmsg_cb);
	RETURN_IF(retcode, "ct_callback: CLIENTMSG ");

	return retcode ;
}

/*
** cleanup_db()
**
** Type of function:
**      Close the connection and drop the context
**
** Purpose:
**      This function closes the connection to the server and cleans up
**      the contex structure. This function is called at the end of the
**      wide_curupd and wide_dynamic samples.
**
** Parameters:
**      cntx_ptr - Pointer to the contex structure.
**      status   - To specify type of exit.
**
** Return:
**      CS_SUCCEED if everything goes fine
**      otherwise a failure message will be displayed and failure code will
**      be returned.
*/
CS_RETCODE
cleanup_db(CS_CONTEXT *cntx_ptr, CS_RETCODE status)
{
	CS_RETCODE      retcode;
	CS_INT		exit_type;

	exit_type = (status != CS_SUCCEED) ? CS_FORCE_EXIT : CS_UNUSED ;

	/* 
	** close and cleanup connection to the server 
	*/
	retcode = ct_exit(cntx_ptr, exit_type);
	RETURN_IF(retcode, "db_cleanup:ct_exit ");

	/* 
	** drop the context 
	*/
	retcode = cs_ctx_drop(cntx_ptr) ;
	RETURN_IF(retcode, "db_cleanup:cs_drop ");

	return retcode ;
}

/*
** connect_db()
**
** Type of function:
**      connect to the server
**
** Purpose:
**	This function makes the connection to the server. It sets the
**	appropriate user name and password.
**
** Parameters:
**      cntx_ptr - Pointer to the contex structure.
**      conn_ptr - Pointer to the connection structure.
**	user_name- User name.
**	password - Password.
**
** Return:
**      CS_SUCCEED if everything goes fine
**      otherwise a failure message will be displayed and failure code will
**      be returned.
*/
CS_RETCODE
connect_db(CS_CONTEXT *cntx_ptr, CS_CONNECTION **conn_ptr, 
	   CS_CHAR *user_name, CS_CHAR *password)
{
	CS_RETCODE      retcode;

	/* 
	** Allocate a connection pointer 
	*/
	retcode = ct_con_alloc (cntx_ptr, conn_ptr);
	RETURN_IF (retcode, "connect_db: ct_con_alloc ");

	/* 
	** Set the username and password properties 
	*/
	retcode = ct_con_props(*conn_ptr, CS_SET, CS_USERNAME, user_name,
	                       CS_NULLTERM, NULL);
	RETURN_IF (retcode, "connect_db: ct_con_props: username ");

	retcode = ct_con_props(*conn_ptr, CS_SET, CS_PASSWORD, password,
		               CS_NULLTERM, NULL);
	RETURN_IF(retcode, "connect_db: ct_con_props: password");

	/* 
	** connect to the server 
	*/
	retcode = ct_connect(*conn_ptr, NULL, 0);
	RETURN_IF(retcode, "connect_db: ct_connect ");

	return  CS_SUCCEED ;
}

/*
** handle_returns()
**
** Type of function:
**      process the return result type.
**
** Purpose:
**
** Parameters:
**      cmd_ptr - Pointer to the command structure.
**
** Return:
**      CS_SUCCEED if everything goes fine
**      otherwise a failure message will be displayed and failure code will
**      be returned.
*/
CS_RETCODE
handle_returns(CS_COMMAND *cmd_ptr)
{
	CS_RETCODE	results_ok;
	CS_INT		result_type;

	/* 
	** Process all returned result types 
	*/
	while ((results_ok = ct_results (cmd_ptr, &result_type)) == CS_SUCCEED)
	{
		switch ((int)result_type)
		{
		       case CS_ROW_RESULT:
		                 printf("TYPE: ROW RESULT \n");
		                 results_ok = bind_columns (cmd_ptr);
		                 RETURN_IF (results_ok, "bind columns");
		                 results_ok = fetch_n_print (cmd_ptr);
		                 RETURN_IF (results_ok, "fetch_n_print");
		                 break ;

		       case CS_CMD_SUCCEED:
		                 printf("TYPE: CMD SUCCEEDED \n");
		                 break ;

		       case CS_CMD_DONE   :
		                 printf("TYPE : CMD DONE \n");
		                 break ;

		       case CS_CMD_FAIL   :
		                 printf("TYPE: CMD FAIL \n");
		                 break ;

			case CS_PARAM_RESULT :
		                 printf("TYPE: PARAM RESULT \n");
		                 results_ok = bind_columns (cmd_ptr);
		                 RETURN_IF (results_ok, "bind columns");
		                 results_ok = fetch_n_print (cmd_ptr);
		                 RETURN_IF (results_ok, "fetch_n_print");
		                 break ;

		       case CS_CURSOR_RESULT :
		                 printf("TYPE: CURSOR RESULT ");
		                 results_ok = bind_columns (cmd_ptr);
		                 RETURN_IF(results_ok, "bind columns");
		                 results_ok = fetch_n_print(cmd_ptr);
		                 RETURN_IF (results_ok, "fetch n print");
		                 break ;

		       default    :
		                 RETURN_IF (CS_FAIL, "unknown results \n");
		                 break ;
		};
	}
	if (results_ok == CS_END_RESULTS)
	{
		return CS_SUCCEED; 
	}
	return CS_FAIL;
}

/*
** fetch_n_print()
**
** Type of function:
**	fetch the bound data into host variable.
**
** Purpose:
**
** Parameters:
**      cmd_ptr - Pointer to the command structure.
**
** Return:
**      CS_SUCCEED if everything goes fine
**      otherwise a failure message will be displayed and failure code will
**      be returned.
*/
CS_RETCODE
fetch_n_print(CS_COMMAND *cmd_ptr)
{
	CS_RETCODE      retcode ;
	CS_INT		i, num_cols ;

	retcode = ct_res_info(cmd_ptr, CS_NUMDATA, &num_cols, CS_UNUSED, NULL);
	RETURN_IF(retcode, "fetch_n_print: ct_res_info ");

	/* 
	** Fetch the bound data into host variables 
	*/
	while (((retcode = ct_fetch(cmd_ptr, CS_UNUSED, CS_UNUSED, CS_UNUSED,
				    NULL)) == CS_SUCCEED) ||
	     			    (retcode == CS_ROW_FAIL))
	{
		if (retcode == CS_ROW_FAIL)
		{
		       printf("ct_fetch returned row fail \n");
		       continue ;
		}

		for (i = 1 ; i <= num_cols ; i++)
		{
		       printf(" %s \t ", row_buffer[i-1]);
		}
		printf("\n");
		single_row(cmd_ptr);
	}

	if (retcode != CS_END_DATA)
	{
		RETURN_IF (retcode, "fetch_n_print ");
	}

	return CS_SUCCEED ;
}

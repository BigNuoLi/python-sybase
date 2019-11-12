/* 
** SAMPLE PROGRAM FOR XNL FEATURE
** ------------------------------
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
**	This program uses a cursor to retrieve data from the
** 	publishers table in the pubs2 database. It retrieves
**	data row by row and prompts the user to input new values
**	for the column 'state' in the publishers table.
**
** Inputs
** ------
**	value for the input parameter ('state' column from the
**	'publishers' table ) for the UPDATE. Create a publishers3
**	table as shown below before running the sample program:
**
** 	use pubs2
** 	go
** 	drop table publishers3
** 	go
** 	create table publishers3 (pub_id char(4) not null, 
**	pub_name varchar(400) null, city varchar(20) null,
**  	state char(2) null)
** 	go
** 	insert into publishers3 select * from publishers
** 	go
** 	create unique index pubind on publishers3(pub_id)
** 	go
**
** References
** ----------
** 	Open Client Reference Manual pages for ct_cursor and ct_param.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctpublic.h>
#include "example.h"
#include "exutils.h"
#include "wide_example.h"

/*
** main()
**
** Purpose:
**      Entry point for example program.
**
** Parameters:
**      None, argc and argv will not be used.
**
**
*/

int main(int argc, char *argv[])
{
	CS_CONTEXT      *cntx_ptr ;
	CS_CONNECTION   *conn_ptr ;
	CS_COMMAND      *cmd_ptr ;
	CS_RETCODE      ret ;

	/* 
	** Allocate a context and initialize client-library 
	*/
	ret = init_db(&cntx_ptr);
	EXIT_IF(ret);

	/* 
	** Establish a connection to the server 
	*/
	ret = connect_db (cntx_ptr, &conn_ptr, EX_USERNAME, EX_PASSWORD);
	EXIT_IF(ret);

	/* 
	** Allocate a command structure 
	*/
	ret = ct_cmd_alloc (conn_ptr, &cmd_ptr );
	EXIT_IF(ret);

	 /* Perform cursor operations */
	ret = open_cursor ( cmd_ptr );
	EXIT_IF(ret);

	/* 
	** Drop the command structure 
	*/
	ret = ct_cmd_drop ( cmd_ptr );
	EXIT_IF(ret);

	/* 
	** Close connection to the server 
	*/
	ret = ct_close ( conn_ptr, CS_UNUSED ) ;
	EXIT_IF(ret);

	/* 
	** Drop the context and perform cleanup 
	*/
	ret = cleanup_db (cntx_ptr, ret );
	EXIT_IF(ret);

	printf("\n End of program run! \n");
	return(0);
}

/*
** bind_columns()
**
** Type of function:
**      bind the return data to host variable.
**
** Purpose:
**
** Parameters:
**      cmd_ptr - Pointer to the commanad structure.
**
** Return:
**      CS_SUCCEED if everything goes fine
**      otherwise a failure message will be displayed and failure code will
**      be returned.
*/

CS_RETCODE
bind_columns(CS_COMMAND *cmd_ptr )
{
	CS_RETCODE retcode;
	CS_INT	   i, num_cols;
	CS_DATAFMT target_format;

	retcode = ct_res_info(cmd_ptr, CS_NUMDATA, &num_cols,
			      CS_UNUSED, NULL);
	RETURN_IF(retcode, "ct_bind : ct_res_info ");

	for (i = 1 ; i <= num_cols ; i++)
	{
		memset (&target_format, 0, CS_SIZEOF(target_format));

		target_format.datatype = CS_CHAR_TYPE;
		target_format.maxlength = MAX_COLSIZE;
		target_format.count = 1;
		target_format.format = CS_FMT_NULLTERM;

		/* 
		** Bind returned data to host variables 
		*/
		retcode = ct_bind(cmd_ptr, i, &target_format, row_buffer[i-1],
				  NULL, NULL ) ;

		RETURN_IF(retcode, "bind_columns: ct_bind ");
	}
	return CS_SUCCEED ;
}

/*
** open_cursor()
**
** Type of function:
**      open a cursor.
**
** Purpose:
**	Here we send a cursor statement to the server. After receiving the
**	result from the server we close the cursor.
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
open_cursor(CS_COMMAND *cmd_ptr)
{
	CS_RETCODE      ret ;

	ret = ct_cursor(cmd_ptr, CS_CURSOR_DECLARE, "browse_cursor",
			CS_NULLTERM, "select * from pubs2.dbo.publishers3",
			CS_NULLTERM, CS_FOR_UPDATE );
	RETURN_IF(ret, "ct_cursor: declare ");

	ret = ct_cursor(cmd_ptr, CS_CURSOR_OPEN, NULL, CS_UNUSED, NULL,
			CS_UNUSED, CS_UNUSED);
	RETURN_IF(ret, "ct_cursor: open ");

	ret = ct_send (cmd_ptr);
	RETURN_IF(ret, "ct_send : cursors ");

	ret = handle_returns(cmd_ptr) ;
	RETURN_IF(ret, "ct_results : cursors ");

	ret = ct_cursor(cmd_ptr, CS_CURSOR_CLOSE, NULL, CS_UNUSED,
			NULL, CS_UNUSED, CS_DEALLOC);
	RETURN_IF(ret, "ct_cursor:close ");

	ret = ct_send (cmd_ptr );
	RETURN_IF(ret, "ct_send : cursor close ");

	ret = handle_returns(cmd_ptr ) ;
	RETURN_IF(ret, "ct_results : cursor close");

	return CS_SUCCEED ;
}

/*
** single_row()
**
** Type of function:
**	update the cursor.
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
single_row(CS_COMMAND *cmd_ptr)
{
	CS_RETCODE      retcode ;
	CS_DATAFMT      datafmt ;
	CS_CHAR	 sqlstmt[70] , state[3];

	printf("New state: ");
	fgets(state, 3, stdin);
	if ( strlen(state) < 1 )
	{
		/* 
		** If the user hit 'enter' for the new input value,
		** don't bother updating.Return to the calling function.
		*/
		return CS_SUCCEED;
	}

	strcpy(sqlstmt,"update pubs2.dbo.publishers3 set state = @state ");
	retcode = ct_cursor(cmd_ptr, CS_CURSOR_UPDATE,
			  "pubs2.dbo.publishers3", CS_NULLTERM,
			   sqlstmt, CS_NULLTERM, CS_UNUSED ) ;
	RETURN_IF ( retcode, "ct_cursor; update ");

	/* 
	** Set up the parameters for the update 
	*/
	memset(&datafmt, 0, sizeof (datafmt));
	strcpy(datafmt.name, "@state");
	datafmt.namelen = CS_NULLTERM;
	datafmt.datatype = CS_CHAR_TYPE;
	datafmt.maxlength = 1024;
	datafmt.status = CS_INPUTVALUE;
	datafmt.locale = NULL;

	/* 
	** If an input value was entered, do the update. 
	*/
	retcode = ct_param(cmd_ptr, &datafmt, (CS_VOID *)state,
		strlen(state), 0) ;
		RETURN_IF( retcode, "ct_param ");

	retcode = ct_send (cmd_ptr) ;
	RETURN_IF(retcode, "ct_cursor:ct_send ");

	retcode = handle_returns ( cmd_ptr );
	RETURN_IF ( retcode, "ct_cursor:ct_results ");

	return CS_SUCCEED ;
}

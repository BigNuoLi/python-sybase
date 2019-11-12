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
**	This program uses Dynamic SQL to retrieve values 
**	from the 'titles' table in the 'tempdb' database.
**	The select statement, which contains placeholders with  
**	identifiers, is sent to the server to be partially compiled 
**	and stored. Therefore, every time we call the select, 
**	we only pass new values for the key value which 
**	determines the row to be retrieved. The behaviour is similar 
**	to passing input parameters to stored procedures.
**
** References
** ----------
**	Open Client-Library/C reference manual. Refer to the sections
**		  on Dynamic SQL , ct_dynamic.
**
*/

#include <stdio.h>
#include <ctype.h>
#include <ctpublic.h>
#include <cstypes.h>
#include <string.h>
#include <stdlib.h>
#include "example.h"
#include "exutils.h"
#include "wide_example.h"

#define CREATETBL "create table tempdb..titles \
		   (title_id varchar(6) not null, \
		   price money null, \
		   type char(290) null)"

#define INSERT1	"insert into tempdb..titles values(\"BU1111\",$20.00,replicate('a',275))"
#define INSERT2	"insert into tempdb..titles values(\"BU2222\",$40.00,replicate('b',285))"
#define DUMPTBL	"select title_id from tempdb..titles"

CS_RETCODE open_dynamic(CS_COMMAND *cmd_ptr);
CS_RETCODE bind_columns(CS_COMMAND *cmd_ptr);
CS_RETCODE single_row(CS_COMMAND *cmd_ptr);


/*
** main()
**
** Purpose:
**      Entry point for example program.
**
** Parameters:
**      None, argc and argv will not be used.
**
*/
int main(int argc, char *argv[])
{
	CS_CONTEXT      *cntx_ptr;     /* common context area pointer */
	CS_CONNECTION   *conn_ptr;     /* handle for this server connection */
	CS_COMMAND      *cmd_ptr;      /* handle for this SQL command batch */
	CS_RETCODE      ret;	       /* to hold all CT-Lib function returns */

	/* 
	** setup context of database connections 
	*/
	ret = init_db(&cntx_ptr); 
	EXIT_IF(ret);
	
	/* 
	** connect to the server.
	*/     
	ret = connect_db(cntx_ptr, &conn_ptr, EX_USERNAME, EX_PASSWORD);
	EXIT_IF(ret);

	/* 
	** alloc a command struct (controls SQL sent) 
	*/
	ret = ct_cmd_alloc(conn_ptr, &cmd_ptr);
	EXIT_IF(ret);

	/*
	** Drop the table titles from tempdb
	*/
	ret = ct_command(cmd_ptr, CS_LANG_CMD, "drop table tempdb..titles", 
			 CS_NULLTERM, CS_UNUSED);
	EXIT_IF(ret);

	/* 
	** Send the command to the server 
	*/
	ret = ct_send(cmd_ptr);
	EXIT_IF(ret);

	/* 
	** Process the results. Loop while ct_results() returns CS_SUCCEED.
	*/
	ret = handle_returns(cmd_ptr);
	EXIT_IF(ret);

	/* 
	** create the titles table in the tempdb database. 
	*/
	ret = ct_command(cmd_ptr, CS_LANG_CMD, CREATETBL, CS_NULLTERM,
			 CS_UNUSED);
	EXIT_IF(ret);

	/* 
	** Send the command to the server 
	*/
	ret = ct_send(cmd_ptr);
	EXIT_IF(ret);

	/* 
	** Process the results. Loop while ct_results() returns CS_SUCCEED.
	*/
	ret = handle_returns(cmd_ptr);
	EXIT_IF(ret);

	/* 
	** Insert data into the table. 
	*/
	ret = ct_command(cmd_ptr, CS_LANG_CMD, INSERT1, CS_NULLTERM,
			 CS_UNUSED);
	EXIT_IF(ret);

	/* 
	** Send the command to the server 
	*/
	ret = ct_send(cmd_ptr);
	EXIT_IF(ret);

	/* 
	** Process the results. Loop while ct_results() returns CS_SUCCEED.
	*/
	ret = handle_returns(cmd_ptr);
	EXIT_IF(ret);

	/* 
	** Insert data into the table. 
	*/
	ret = ct_command(cmd_ptr, CS_LANG_CMD, INSERT2, CS_NULLTERM,
			 CS_UNUSED);
	EXIT_IF(ret);

	/* 
	** Send the command to the server 
	*/
	ret = ct_send(cmd_ptr);
	EXIT_IF(ret);

	/* 
	** Process the results. Loop while ct_results() returns CS_SUCCEED.
	*/
	ret = handle_returns(cmd_ptr);
	EXIT_IF(ret);

	/*
	** List the available information in the table.
	*/
	printf("Available titles to query are:\n");
	ret = ct_command(cmd_ptr, CS_LANG_CMD, DUMPTBL, CS_NULLTERM, CS_UNUSED);
	EXIT_IF(ret);

	/* 
	** Send the command to the server 
	*/
	ret = ct_send(cmd_ptr);
	EXIT_IF(ret);

	/* 
	** Process the results. Loop while ct_results() returns CS_SUCCEED.
	*/
	ret = handle_returns(cmd_ptr);
	EXIT_IF(ret);

	/* 
	** Begin prepared statement operations 
	*/
	ret = open_dynamic(cmd_ptr);
	EXIT_IF(ret);

	/* 
	** close the connection to the server 
	*/
	ret = ct_close(conn_ptr, CS_UNUSED);
	EXIT_IF(ret);

	/*
	** cleanup at both Client & Server
	*/
	ret = cleanup_db(cntx_ptr, ret);
	EXIT_IF(ret);

	printf("Program completed successfully!\n");
	return 0;
}


/*
** check_title_input
**
** Type of function:
**	Input validation routine.
**
** Purpose:
**	Validate that entered string is valid to pass as
**	a parameter. This means that the new line is stripped
**	off and that the resultant length is > 0.
**
** Return:
**	0 if there is a problem with the string.
**	length of string otherwise.
*/
int
check_title_input(char *title_id, int title_size)
{
	char	*cp;
	int	 i;

	cp = title_id;
	i = 0;

	while (i < title_size)
	{
		switch (*cp)
		{
		  case '\n':
			*cp = 0;
			return i;

		  case 0:
			return i;
		}

		cp++;
		i++;
	}

	/*
	** Ooops. This shouldn't ever happen since fgets will supply
	** a terminating 0.
	*/
	return 0;
}


/*
** open_dynamic()
**
** Type of function:
**      open a prepared statement.
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
open_dynamic(CS_COMMAND *cmd)
{  

	CS_DATAFMT	data_format;
	CS_RETCODE	retcode;
	char		title_id[8], statement[100];

	/*
	** Initialize
	*/
	memset(title_id, 0, sizeof(title_id));

	/* 
	** Prepare the sql statement for the first cursor 
	*/
	sprintf(statement, "select title_id, type, price from tempdb..titles \
		where title_id like (?) ");
	retcode = ct_dynamic(cmd, CS_PREPARE, "myquery", CS_NULLTERM, 
			     statement, CS_NULLTERM);
	RETURN_IF(retcode, "open_dynamic: ct_dynamic prepare");

	/* 
	** Send the prepared statement to the server 
	*/
	retcode = ct_send(cmd);
	RETURN_IF(retcode, "open_dynamic: ct_dynamic send");

	/* 
	** See if the prepared statement was successful 
	*/
	retcode = handle_returns(cmd);
	RETURN_IF(retcode, "open_dynamic: ct_dynamic results");

	/* 
	** We need to get values for title_id 
	*/
	printf("Enter title id value - enter an X if you wish to stop: \n");
	while (toupper(title_id[0]) != 'X') 
	{
		/*
		** Get the record to run the query on.
		*/
		do {
			printf("Retrieve detail record for title id: ?");
			fgets(title_id, sizeof (title_id),  stdin);
		} while (check_title_input(title_id, sizeof (title_id)) == 0);

		if (toupper(title_id[0]) == 'X')
		{
			break;
		}

		/*
		** Set the command structure up to execute the prepared
		** statement.
		*/
		retcode = ct_dynamic(cmd, CS_EXECUTE, "myquery", CS_NULLTERM,
				NULL, CS_UNUSED);
		RETURN_IF(retcode, "open_dynamic: ct_dynamic prepare");

		/* 
		** Define the input parameter 
		*/
		memset(&data_format, 0, sizeof(data_format)) ;
		data_format.status = CS_INPUTVALUE;
		data_format.namelen = CS_NULLTERM ;
		data_format.datatype = CS_CHAR_TYPE;
		data_format.format = CS_FMT_NULLTERM;
   		data_format.maxlength = CS_MAX_CHAR;
		data_format.count = 1;
 
		retcode = ct_param(cmd, &data_format, (CS_VOID *)title_id,
				   strlen(title_id), 0);
		RETURN_IF(retcode, "open_dynamic: missing param ");
 
		/* 
		** cursor open and params command 
		*/
		retcode = ct_send(cmd); 
		RETURN_IF(retcode, "open_dynamic: param send");

		retcode = handle_returns(cmd);
		RETURN_IF(retcode, "open_dynamic: param results");
	} 

	/* 
	** Now, deallocate the prepared statement 
	*/
	retcode = ct_dynamic(cmd, CS_DEALLOC, "myquery", CS_NULLTERM, 
			     NULL, CS_UNUSED);
	RETURN_IF(retcode, "ct_dynamic dealloc");

	/* 
	** Send the dealloc statement to the server 
	*/
	retcode = ct_send(cmd);
	RETURN_IF(retcode, "ct_dynamic dealloc send");

	/* 
	** Process results from above commend 
	*/
	retcode = handle_returns(cmd);
	RETURN_IF(retcode, "ct_dynamic dealloc results");

  	return CS_SUCCEED ;
}

/*
** bind_columns()
**
** Type of function:
**      bind the return data to host variable.
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
bind_columns(CS_COMMAND *cmd_ptr)
{
	CS_RETCODE	retcode ;
	CS_INT		i, num_cols;
	CS_DATAFMT	target_format ;

	retcode = ct_res_info(cmd_ptr, CS_NUMDATA, &num_cols, CS_UNUSED, NULL) ;
	RETURN_IF (retcode, "ct_bind : ct_res_info ");

	for (i = 1 ; i <= num_cols ; i++)
	{
		memset(&target_format, 0, sizeof(target_format)) ;

		target_format.datatype = CS_LONGCHAR_TYPE ;
   		target_format.maxlength = 300;
		target_format.count    = 1 ;
		target_format.format   =  CS_FMT_NULLTERM;
		target_format.locale   = NULL;

		retcode = ct_bind(cmd_ptr, i, &target_format, row_buffer[i-1],
				  NULL, NULL) ;
		RETURN_IF(retcode, "bind_columns: ct_bind ");

	}

	return CS_SUCCEED ;
}

CS_RETCODE
single_row(CS_COMMAND *cmd_ptr)
{
	/* 
	** Just a dummy function. We need it to make fetch_n_print() a 
	** common function.
	*/ 
	return CS_SUCCEED ;
}

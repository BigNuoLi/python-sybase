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
**  	utility routines for handling text/image data with Client-Library.
*/

/*
** C libraries.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/*
** Client-Library header file.
*/
#include <ctpublic.h>

#include "txtimg.h"

/*
** Macro for return code error checking. This is
** used here for brevity, and a real program would 
** want to cleanup a connection or command structure 
** (cancel if necessary, drop command structures if necessary, 
** etc) before returning CS_FAIL.
*/
#if !defined(ERR_CH)
#define ERR_CH stderr
#endif
#define FAIL_ON_FAIL(ret, str) \
{ if (ret != CS_SUCCEED) { \
  { \
    fprintf(ERR_CH, "ERROR(%ld): txtimg.c line %ld: %s\n",\
            (long)ret,(long)__LINE__,str); \
    return CS_FAIL; \
  } \
} }

/*
** In System 10, the ct_fetch() routine has 3 unused parameters.
*/
#define CT10_FETCH(cmd, p_rows_read) \
  ct_fetch((cmd), CS_UNUSED, CS_UNUSED, CS_UNUSED, (p_rows_read))

/* 
** GetTextValue() -- Get the IO-Descriptor (CS_IODESC) required to update 
**    a text/image column with a Client-Library send-data command. 
**    Optionally, a buffer can be supplied to receive the existing 
**    text/image value.
**
** PARAMETERS
**  conn -- An open connection. The connection can have no pending
**          results.
**  iodesc -- The address of an IO-Descriptor (CS_IODESC) structure. 
**          The structure fields will be filled in with the necessary
**          information for a text update to the column with ct_send_data. 
**  select_stmt -- A null-terminated character (CS_CHAR) string
**          containing a SQL SELECT statement that selects the
**          text/image column in the row of interest.
**          
**          The statement should return one regular row containing the 
**          text/image column and no other fetchable results.
**
**  col_num -- The column number that the text/image column will have in
**          the rows returned by the SELECT statement. The first
**          column is 1.
**  value_buf -- The address (cast to CS_VOID *) of a buffer to hold the
**          value. If the text/image column is text, buffer should be 
**          CS_TEXT or CS_CHAR array. If the text/image column is image, 
**          buffer should be a CS_IMAGE or CS_BINARY array. Text values
**          are not null-terminated.
**  buf_len -- The length, in bytes, of value_buf. If buf_len is 0 or
**          value_buf is NULL, both are ignored. Only the IO-Descriptor
**          is returned in *iodesc in this case.
**  out_len -- The address of a CS_INT variable to receive the length of
**          the value written to buffer.
**
** RETURNS
**   CS_SUCCEED -- success.
**   CS_FAIL -- failure.
*/
CS_RETCODE
GetTextValue(CS_CONNECTION *conn, CS_IODESC *iodesc, CS_CHAR *select_stmt,
	     CS_INT col_num, CS_VOID *val_buf, CS_INT buf_len, CS_INT *out_len)
{
	CS_COMMAND *cmd;
	CS_RETCODE ret;
	CS_INT     result_type;
	CS_RETCODE fetch_ret;
	CS_DATAFMT datafmt;
	CS_BYTE    bogus_buf;


	/*
	** Set *out_len here in case it does not get set 
	** below.
	*/
	if (out_len != (CS_INT *) NULL)
	  *out_len = 0;

	/*
	** Allocate a CS_COMMAND structure.
	*/
	ret = ct_cmd_alloc(conn, &cmd);
	FAIL_ON_FAIL(ret, "GetTextValue: ct_cmd_alloc() failed.");

	/*
	** We are doing two things at once in this function:
	**  1. We retrieve the text/image value.
	**  2. We retrieve the IO-Descriptor for the
	**     text/image column in case the caller needs it
	**     for later updates to the text/image column.
	** We can do 1 without doing 2, but we can not do 2 without
	** doing 1.
	**
	** The algorithm for retrieving a text/image value and 
	** getting an IO-Descriptor for later updates:
	**
	** A: With a language command, select the column containing the 
	**    text/image value of interest.
	**
	** B: Process the results. In this program, we expect the SELECT
	**    to return one regular row containing the text/image column.
	**    B1. Call ct_get_data to retrieve all, some, or none of the
	**       text/image value. ct_get_data must be called to "prime"
	**       Client-Library for later retrieval of the IO-Descriptor.
	**    B2. Call ct_data_info to retrieve the IO-Descriptor structure
	*/     

	/*
	** Step A:
	** Build and send the SELECT language command.
	*/
	ret = ct_command(cmd, CS_LANG_CMD, select_stmt, strlen(select_stmt),
		   CS_UNUSED);
	FAIL_ON_FAIL(ret, "GetTextValue: ct_command for select failed");
	ret = ct_send(cmd);
	FAIL_ON_FAIL(ret, "GetTextValue: ct_send failed.");
	
	/*
	** Step B:
	** Process the results. We expect the row containing the text value
	** to be a regular row.
	*/
	while ((ret = ct_results(cmd, &result_type)) == CS_SUCCEED)
	{
	  switch ((int) result_type)
	  {
	  case CS_ROW_RESULT:

	    /*
	    ** The row containing the text column should be in this
	    ** result set.
	    */

	    /* Make sure that col_num represents a text/image column. */
	    ret = ct_describe(cmd, col_num, &datafmt);
	    FAIL_ON_FAIL(ret, "GetTextValue: ct_describe failed.");
	    if (datafmt.datatype != CS_TEXT_TYPE 
	  && datafmt.datatype != CS_IMAGE_TYPE)
	    { 
	      FAIL_ON_FAIL(CS_FAIL, 
		     "GetTextValue: col_num is not text/image.");
	    }

	    /* 
	    ** Fetch the first row. We expect that only one row is returned,
	    ** but we will deal with any remaining rows later.
	    */
	    fetch_ret = CT10_FETCH(cmd, (CS_INT *) NULL);
	    /* We require CS_SUCCEED to continue. */
	    FAIL_ON_FAIL(fetch_ret, 
		   "GetTextValue: ct_fetch() failed or no rows returned.");

	    /* 
	    ** Step B1:
	    **
	    ** Call ct_get_data() on the text column. We must do this before we 
	    ** can get the IO-Descriptor with ct_data_info().
	    **
	    ** If the buffer is not zero-length, we also get the data value. In 
	    ** this case, we expect that the entire value will fit in the buffer 
	    ** and ct_get_data will return CS_END_DATA or CS_END_ITEM. CS_SUCCEED
	    ** in this case indicates that only part of the value was copied.
	    **
	    ** If the buffer is zero-length, we assume that the caller just wants
	    ** the IO-Descriptor and does not care about the value.
	    */
	    if ( (buf_len == (CS_INT) 0) || (val_buf == (CS_VOID *) NULL) ) 
	    {
	/* 
	      ** Don't care about the value, but still must call ct_get_data()
	** here so that we can get the IO-Descriptor below. 
	*/
	ret = ct_get_data(cmd, col_num, (CS_VOID *) &bogus_buf, 0, out_len);
	    }
	    else
	    {	
	      /* Copy the value. */
	ret = ct_get_data(cmd, col_num, val_buf, buf_len, out_len);
	    }

	    /* 
	    ** Fail if ct_get_data() did not return CS_END_DATA or CS_END_ITEM, 
	    ** or if it returned CS_SUCCEED and our buffer length is not 0.
	    */
	    if (ret == CS_SUCCEED && buf_len != 0)
	    {
	      /*
	      ** CS_SUCCEED indicates we only got part of the value.
	      */
	FAIL_ON_FAIL(ret, 
		     "GetTextValue: buffer is too small to hold value.\n");
	    }
	    else if (ret != CS_END_ITEM && ret != CS_END_DATA)
	    {
	/* ct_get_data failed. */
	FAIL_ON_FAIL(ret, "GetTextValue: ct_get_data failed.");
	    }

	    /*
	    ** Step B2:
	    ** 
	    ** Retrieve the IO-Descriptor for later updates to the text/image
	    ** column.
	    */
	    if (iodesc != (CS_IODESC *) NULL)
	    {
	ret = ct_data_info(cmd, CS_GET, col_num, iodesc);
	FAIL_ON_FAIL(ret, "GetTextValue: ct_data_info(CS_GET) failed.");
	    }

	    /*
	    ** Now discard any remaining rows.
	    */
	    ret = ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_CURRENT);
	    FAIL_ON_FAIL(ret, "GetTextValue: ct_cancel(CURRENT) failed.");

	    break;

	  case CS_CMD_DONE:
	  case CS_CMD_SUCCEED:
	    /*
	    ** These result_type values require no action here.
	    */
	    break;

	  case CS_CMD_FAIL:
	    /* 
	    ** The command failed on the server. Return a fail after canceling
	    ** all the remaining results. We don't check the ct_cancel() return
	    ** code because we are going to fail anyway.
	    */
	    (CS_VOID) ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
	    FAIL_ON_FAIL(CS_FAIL, "GetTextValue: Server command failed.");
	    
	  default:
	    /* 
	    ** Received an unexpected result type. Return a fail after canceling
	    ** all the remaining results. We don't check the ct_cancel() return
	    ** code because we are going to fail anyway.
	    */
	    (CS_VOID) ct_cancel((CS_CONNECTION *)NULL, cmd, CS_CANCEL_ALL);
	    FAIL_ON_FAIL(CS_FAIL, "GetTextValue: Unexpected result type.");
	  } /* switch result_type */
	} /* while results */


	/*
	** Clean up.
	*/
	ret = ct_cmd_drop(cmd);
	FAIL_ON_FAIL(ret, "ct_cmd_drop() failed.");

	return CS_SUCCEED;

} /* GetTextValue() */

/*
** UpdateTextCol() -- Update a text or image column with a
**   new value.
**
** PARAMETERS
**  conn -- An open connection. The connection can have no pending
**          results.
**  iodesc -- The address of an IO-Descriptor (CS_IODESC) structure. 
**          On input, the fields should be set up to describe the text/
**          image column to be updated. GetTextValue() will set the
**          timestamp, textprt, and other fields. The caller
**          should set the total_txtlen and log_on_update fields to
**          suit it's needs.
**          On output, the timestamp field will contain the post-
**          update timestamp for the text/image column.
**  value -- The address (cast to CS_VOID *) of a buffer containing the
**          new value. If the text/image column is text, buffer should be 
**          CS_TEXT or CS_CHAR array. If the text/image column is image, 
**          buffer should be a CS_IMAGE or CS_BINARY array.
**  value_len -- The length, in bytes, of the value.
**
** RETURNS
**   CS_SUCCEED -- success.
**   CS_FAIL -- failure.
*/
CS_RETCODE UpdateTextCol(conn, iodesc, value, value_len)
CS_CONNECTION *conn;
CS_IODESC     *iodesc;
CS_VOID       *value;
CS_INT        value_len;
{
	CS_COMMAND *cmd;
	CS_RETCODE ret;

	ret = ct_cmd_alloc(conn, &cmd);
	FAIL_ON_FAIL(ret, "UpdateTextCol: ct_cmd_alloc() failed.");

	/*
	** To send text/image values with Client-Library, we do:
	**   a. Call ct_command(CS_SEND_DATA_CMD).
	**   b. Call ct_data_info(CS_SET) to refresh Client-Library's internal
	**      IO-Descriptor.
	**   c. Call ct_send_data() and ct_send() to send the new value to the 
	**      server.
	**   d. Process the results and retrieve the new text timestamp value
	**      sent by the server.
	*/

	/*
	** a. Call ct_command(CS_SEND_DATA_CMD) to initiate the command to 
	**    send a text or image value. The actual value is given by calling 
	**    ct_send_data().
	*/
	ret = ct_command(cmd, CS_SEND_DATA_CMD, (CS_VOID *) NULL, CS_UNUSED,
		   CS_COLUMN_DATA);
	FAIL_ON_FAIL(ret, "UpdateTextCol: ct_command() failed");

	/*
	** b. Call ct_data_info(CS_SET) to refresh Client-Library's internal
	**    IO-Descriptor.
	*/
	iodesc->total_txtlen = value_len;
	ret = ct_data_info(cmd, CS_SET, CS_UNUSED, iodesc);
	FAIL_ON_FAIL(ret, "UpdateTextCol: ct_data_info() failed.");

	/*
	** c. Call ct_send_data() and ct_send() to send the new value to 
	**    the server.
	**
	** To pass large text/image values, programs often call ct_send_data() in 
	** a loop to pass the value in chunks. This can be useful when the
	** value is being read from a file via buffered input -- ct_send_data 
	** can send the value a bufferful at a time. Here, we have the entire value
	** in memory already, so we just call ct_send_data() once.
	*/
	ret = ct_send_data(cmd, (CS_VOID *)value, value_len);
	FAIL_ON_FAIL(ret, "UpdateTextCol: ct_send_data() failed.");

	/*
	** When sending a text/image value, the last ct_send_data() 
	** call must always be followed by ct_send(). 
	*/
	ret = ct_send(cmd);
	FAIL_ON_FAIL(ret, "UpdateTextCol: ct_send_data() failed.");
 
	/*
	** d. Process the results and retrieve the new text timestamp sent
	**    by the server.
	*/
	ret = TextUpdateResults(cmd, iodesc);
	FAIL_ON_FAIL(ret, "UpdateTextCol: TextUpdateResults() failed.");

	ret = ct_cmd_drop(cmd);
	FAIL_ON_FAIL(ret, "UpdateTextCol: ct_cmd_drop() failed.");

	return CS_SUCCEED;  
} /* UpdateTextCol() */


/*
** TextUpdateResults()
**   Process the results of a Client-Library SEND_DATA command and
**   retrieve the new text timestamp value sent by the server.
**
** Parameters:
**   cmd -- A pointer to the command structure.
**   iodesc -- A pointer to the CS_IODESC structure to 
**       receive the new text timestamp. Only the iodesc->
**       textptr and iodesc->textptrlen fields are updated.
**       No other field is affected.
**
** Returns:
**   CS_SUCCEED: Success. The IO-Descriptor has been updated
**               with the new text timestamp
**   CS_FAIL: Failure.
*/
CS_RETCODE
TextUpdateResults(cmd, iodesc)
CS_COMMAND *cmd;
CS_IODESC  *iodesc;
{
	CS_RETCODE ret;
	CS_RETCODE results_ret;
	CS_INT     result_type;
	CS_DATAFMT datafmt;

	/*
	** In the results of a successful send-data command, the server
	** sends a new text/image timestamp for the column that was 
	** updated. The timestamp is sent as a parameter result type 
	** (CS_PARAM_RESULT).
	*/

	while ((results_ret = ct_results(cmd, &result_type)) == CS_SUCCEED)
	{
		switch ((int) result_type)
		{
		  case CS_PARAM_RESULT:
			/*
			** After a Client-Library text/image update, the server
			** sends the new text timestamp as a parameter result.
			**
			** If we weren't given an IO-Descriptor structure to
			** update, we just do ct_cancel(CS_CANCEL_CURRENT).
			*/
			if (iodesc != (CS_IODESC *) NULL)
			{
				/*
				** We can verify that we have received the
				** timestamp by calling ct_describe and
				** checking the datafmt.status field.
				*/
				ret = ct_describe(cmd, 1, &datafmt);
				FAIL_ON_FAIL(ret,
					     "TextUpdateResults: ct_describe failed.");

				/* 
				** status should be CS_TIMESTAMP.
				*/
				if (!(datafmt.status & CS_TIMESTAMP))
				{
					FAIL_ON_FAIL(CS_FAIL, 
						     "TextUpdateResults: unexpected parameter data.");
				}

				/* 
				** Bind and fetch the timestamp.
				*/
				datafmt.maxlength = CS_SIZEOF(iodesc->timestamp);
				datafmt.format = CS_FMT_UNUSED;
				ret = ct_bind(cmd, 1, &datafmt,
					      (CS_VOID *)&(iodesc->timestamp),
					      &(iodesc->timestamplen),
					      (CS_SMALLINT *) NULL);
				FAIL_ON_FAIL(ret,
					     "TextUpdateResults: ct_bind() failed.");

				ret = CT10_FETCH(cmd, (CS_INT *)NULL);
				FAIL_ON_FAIL(ret,
					     "TextUpdateResults: ct_fetch() failed");
			}

			/*
			** If ct_fetch() returned CS_SUCCEED, we got the
			** timestamp. Cancel any remaining rows
			** (there should be none, but play it safe).
			*/
			ret = ct_cancel(NULL, cmd, CS_CANCEL_CURRENT);
			FAIL_ON_FAIL(ret,
				     "TextUpdateResults: Cancel current failed.");

			break;

		  case CS_CMD_DONE:
		  case CS_CMD_SUCCEED:
			/*
			** These result_type values require no action here.
			*/
			break;

		  case CS_CMD_FAIL:
			/* 
			** The command failed on the server. Return a fail
			** after canceling all the remaining results.
			** We don't check the ct_cancel() return
			** code because we are going to fail anyway.
			*/
			(CS_VOID) ct_cancel((CS_CONNECTION *)NULL, cmd,
					    CS_CANCEL_ALL);
			FAIL_ON_FAIL(CS_FAIL,
				     "GetTextValue: Server command failed.");
    
		  default:
			/* 
			** Received an unexpected result type. Return a fail
			** after canceling all the remaining results.
			** We don't check the ct_cancel() return
			** code because we are going to fail anyway.
			*/
			(CS_VOID) ct_cancel((CS_CONNECTION *)NULL, cmd,
					    CS_CANCEL_ALL);
			FAIL_ON_FAIL(CS_FAIL,
				     "GetTextValue: Unexpected result type.");
		}
	}

	/*
	** Check the final return code from ct_results.
	*/
	switch((int) results_ret)
	{
	  case CS_END_RESULTS:
		/* We're ok. */
		break;
	  case CS_FAIL:    
		FAIL_ON_FAIL(CS_FAIL,
			     "TextUpdateResults: ct_results() failed.");
	  case CS_CANCELED:
		/* Don't expect CS_CANCELED here. */
		/* 
		** FALL THROUGH
		*/
	  default:
		FAIL_ON_FAIL(CS_FAIL, 
			     "TextUpdateResults: ct_results() returned unexpected retcode.");
	}

	return CS_SUCCEED;
} /* TextUpdateResults() */ 

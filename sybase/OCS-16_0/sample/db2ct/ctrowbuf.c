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
** 	Client-Library utility routines that use array binding to buffer rows.
**
** INTRODUCTION
** ------------
**  	The utilility routines in this file allow a synchronous-mode 
**  	Client-Library program to buffer fetched rows. The routines form 
**  	a simple layer over the usual ct_res_info(), ct_describe(), 
**  	ct_bind(), and ct_fetch() calls.
**
**  	The routines are listed below. For detailed descriptions of
**  	a routine's parameters and return codes, see the block comment above
**  	the routine's source code.
**
**   	rb_createbuf() -- Creates a row buffer with a given length.
**     	  The number of columns in the result set is returned as
**     	  an output parameter. The row buffer is allocated memory
**     	  which must be freed with rb_dropbuf().
**
**     	  rb_createbuf() should be called after ct_results() indicates
**     	  a fetchable result type. rb_createbuf() allocates arrays for 
**     	  each column and binds to them. The arrays store the data as 
**     	  generic CS_BYTE * pointers. Each column is retrieved in its raw 
**     	  format.
**
**        rb_createbuf() takes an nrows parameter that determines the
**        number of rows that can be buffered. A size of 1 can be useful
**        for situations where only one row is expected (such as for
**        output parameter or compute row results). 
**
**        Larger sizes are useful for regular or cursor row results. It is 
**        possible to use these routines with cursor results, but any cursor 
**        updates require use of Client-Library's ct_key_data() mechanism to
**        direct the update to the desired row.
**
**        Please understand that once rb_createbuf() is called on a 
**        fetchable result set, the routines in this file "own" that 
**        result set. Do not directly call ct_bind() or ct_fetch() on the
**        same result set.
**
**   	rb_dropbuf() -- Deinitialize the row buffer and free it's 
**     	  memory. A buffer returned by rb_createbuf() can be dropped
**        at any time. Do not use a row buffer pointer after you've
**        dropped the row buffer. 
** 
**        rb_dropbuf() does not cancel the result set or clear bindings.
**        So, if you drop a row buffer before rb_fillbuf() returns 
**        CS_END_DATA or CS_FAIL, then you will have results pending
**        on your connection and active binds which point at freed memory.
**        In this situation, a ct_results() call will fail, and a ct_fetch()
**        call will likely cause a memory fault. Please cancel results if 
**        you must drop the row buffer before all the rows have been retrieved. 
**
**   	rb_fillbuf() -- Fetches rows into the row buffer, overwriting any
**        previous contents. rb_fillbuf() should be called in a loop
**        similar to a typical ct_fetch() loop.
** 
**        Each time rb_fillbuf() returns CS_SUCCEED, CS_ROW_FAIL, or 
**        CS_END_DATA, the number of rows fetched is returned as
**        an output parameter. The application can access the row data
**        with rb_describe(), rb_datalen(), and rb_dataptr().
**
**   	rb_describe() -- Get format information for a column's data
**        value. Copies a CS_DATAFMT structure that describes the
**        column. The CS_DATAFMT settings are equivalent to those set
**        by a ct_describe() call on the result set column. However, 
**        rb_describe() can be called after all the result set's rows 
**        have been fetched.
**
**        rb_describe(), rb_datalen(), and rb_dataptr() can be used 
**        together with cs_convert(), as the following pseudocode shows:
**      
**         ret = rb_describe(rowbuf, col, &srcfmt);
**         srcfmt.len = rb_datalen(rowbuf, row, col);
**         ret = cs_convert(context, &srcfmt,
**                          (CS_VOID *)rb_dataptr(rowbuf, row, col),
**                          &your_destfmt, &your_buffer, &outlen); 
**
**   	rb_dataptr() -- Returns a pointer to a column value in a row.
**
**   	rb_datalen() -- Returns the length, in bytes, of a column value
**        in a row.
**
**   	rb_printrow() -- Prints a row's data to a specified file in a
**        default format. Intended for testing and debugging use only.
**  
** BASIC USAGE
** -----------
**   	The basic algorithm for using these routines follows. This algorithm
**   	assumes that you want to use one row buffer to process a buffer's 
**   	worth of data at a time. For an algorithm that saves multiple buffers
**   	of data, see "CREATING MULTIPLE BUFFERS" below.
**
**      ... ct_results() has indicated a fetchable result type ...
**      ret = rb_createbuf()
**      stop_fetching = false
**      while not stop_fetching
**        fill_ret = rb_fillbuf()
**        if fill_ret in { CS_SUCCEED, CS_END_DATA, CS_ROW_FAIL }
**          ... your code to process the buffer's data ...
**        end if
**        if fill_ret in { CS_END_DATA, CS_SUCCEED }
**          stop_fetching = true
**        end if
**      end while
**      ret = rb_dropbuf()
**  
** CREATING MULTIPLE BUFFERS
** ------------------------- 
**   	rb_createbuf() can be called more than once on the same result set.
**   	So, for instance, if a program wanted to save all the rows in a 
**   	result set, it could save them using an array (or linked list or
**   	whatever) of row buffer pointers. The pseudocode for this would
**   	be:
**        stop_fetching = false
**        while not stop_fetching
**          ret = rb_createbuf()
**          fill_ret = rb_fillbuf()
**          if fill_ret in { CS_SUCCEED, CS_END_DATA, CS_ROW_FAIL }
**            ... your code to save the row buffer and the number
**                of valid rows it contains ...
**          end if
**          if fill_ret in { CS_END_DATA, CS_FAIL }
**            stop_fetching = true
**          end if
**        end while
**
**   	Of course, you would need to free all the row buffers with 
**   	rb_dropbuf() at some later time.
**
** RESTRICTIONS
** ------------
** 	1. These routines are coded for a synchronous connection. They will
**    	   fail miserably if used on an asynchronous connection.
**
** 	2. Columns whose type is text, image, CS_LONGCHAR, or
**    	   CS_LONGBINARY are not retrieved. For these columns, the 
**    	   datafmt->maxlength field set by rb_describe() will be 
**    	   0. It is an error to call rb_datalen() or rb_dataptr() on 
**    	   columns whose datatype is not supported.
**   
**    	Use rb_describe() and the SUPPORTED_TYPE() macro to see if
**    	data is available for a column in the row buffer.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctpublic.h>

#include "ctrowbuf.h"

/*
** We call dbct_prtype() from dbctutil.c
*/
#include "dbctutil.h"

#if !defined(ERR_CH)
#define ERR_CH stderr
#endif
#if !defined(OUT_CH)
#define OUT_CH stdout
#endif

/*
** Private prototypes.
*/
CS_STATIC CS_RETCODE CreateColDataAreas PROTOTYPE ((
	CS_COMMAND	*cmd,
	CS_INT		nrows,
	CS_INT		*ncols,
	COLUMN_DATA	**col_bufs
	));
CS_STATIC CS_RETCODE DropColDataAreas PROTOTYPE ((
	CS_INT		nrows,
	CS_INT		ncols,
	COLUMN_DATA	*col_bufs
	));
CS_STATIC CS_RETCODE GetParentCtx PROTOTYPE ((
	CS_COMMAND	*cmd,
	CS_CONNECTION	*conn,
	CS_CONTEXT	**ctxptr
	));
CS_STATIC CS_RETCODE GetParentConn PROTOTYPE ((
	CS_COMMAND	*cmd,
	CS_CONNECTION	**conn
	));

/*
** Prototype the CS-Library error handler used by this example.
** Applications which use CS-Library should install a CS-Library
** error handler. CS-Library errors are NOT reported through the
** Client-Library error handler.
*/
CS_STATIC CS_RETCODE CS_PUBLIC cslib_err_handler    PROTOTYPE((
	CS_CONTEXT	*cp,
	CS_CLIENTMSG	*msg
	));

/*
** Macros for checking a row buffer pointer passed by calling
** code. rb_createbuf() calls SET_ROWBUF_MAGIC() to put a 
** magic number into the row buffer that it returns. All the other
** public routines here use CHECK_ROWBUF_MAGIC() on the row buffer
** that they receive as a parameter.
*/

#define ROWBUF_MAGIC ((CS_INT) 0x401DEC)

#define SET_ROWBUF_MAGIC(rowbuf) \
	( (rowbuf)->magic = ROWBUF_MAGIC )

#define CHECK_ROWBUF_MAGIC(rowbuf) \
	( ( (rowbuf) != (ROW_BUFFER *) NULL ) \
	  && ( (rowbuf)->magic == ROWBUF_MAGIC ) )

/*
** Macros for return code error checking.
*/

#define ERROR_OUT(err_string) \
 fprintf(ERR_CH, "Error (line %d): %s\n", (int) __LINE__, (char *) err_string);

#define FAIL_ON_FAIL(ret, err_string) \
{ if (ret != CS_SUCCEED) \
	{ \
	  ERROR_OUT(err_string); \
	  return CS_FAIL; \
} }

/*
** rb_createbuf() -- Create the row buffer for a result set.
**   
** Parameters
**   cmd -- the CS_COMMAND structure. This code expects that the 
**       caller is processing the results and ct_results has just
**       returned a result_type that indicates fetchable data.
**   nrows -- The length of the row buffer. This determines the
**       size of the row buffer and how many rows will be available
**       after each rb_fillbuf() call.
**   *p_ncols -- (output) The number of columns in the result set.
**   *p_rowbuf -- (output) The row buffer. The row buffer should be
**        freed by calling rb_dropbuf().
**
** Side effects
**   Bindings on the command structure.
**
** Returns
**   CS_SUCCEED
**   CS_FAIL
*/
CS_RETCODE CS_PUBLIC
rb_createbuf(CS_COMMAND *cmd, CS_INT nrows, CS_INT *p_ncols,
	     ROW_BUFFER **p_rowbuf)
{
	CS_RETCODE	ret;

	/*
	** Check for incoming rudeness.
	*/
	if (p_ncols == (CS_INT *)NULL)
	{
		FAIL_ON_FAIL(CS_FAIL,
			     "rb_createbuf: received NULL ncols (3rd) parameter.");
	}
	*p_ncols = 0;
	if (p_rowbuf == (ROW_BUFFER **) NULL)
	{
		FAIL_ON_FAIL(CS_FAIL, 
			     "rb_createbuf: received NULL rowbuf (4th) parameter.");
	}

	*p_rowbuf = (ROW_BUFFER *) malloc ( sizeof (ROW_BUFFER) );
	if (*p_rowbuf == (ROW_BUFFER *)NULL)
	{
		FAIL_ON_FAIL(CS_FAIL,
			     "rb_createbuf: Out of memory for row buffer.");
	}

	SET_ROWBUF_MAGIC(*p_rowbuf);

	/*
	** CreateColDataAreas() gets column descriptions and creates the array 
	** of COLUMN_DATA structures.
	*/
	ret = CreateColDataAreas(cmd, nrows, p_ncols, &((*p_rowbuf)->columns));
	if (ret != CS_SUCCEED)
	{
		free((CS_VOID *) (*p_rowbuf));
		*p_ncols = 0;
		FAIL_ON_FAIL(CS_FAIL,
			     "rb_createbuf: CreateColDataAreas() failed.");
	}

	(*p_rowbuf)->nrows = nrows;
	(*p_rowbuf)->nfetched = 0;
	(*p_rowbuf)->ncols = *p_ncols;
	
	return CS_SUCCEED;

} /* rb_createbuf() */


/*
** rb_dropbuf() -- Free a row buffer's memory.
**
** Parameters
**   rowbuf -- The row buffer created by rb_createbuf().
**
** Notes
**   rb_dropbuf() does not clear any active binds to the
**   row buffer's memory.
**
**   If the result set on which the row buffer was created
**   has not been completely retrieved (rb_fillbuf has not
**   returned CS_END_DATA), then any call to ct_fetch() will
**   copy data into freed memory. Please cancel the result
**   set if it has not been completely processed.
**
** Returns
**   CS_SUCCEED
**   CS_FAIL
*/
CS_RETCODE CS_PUBLIC
rb_dropbuf(ROW_BUFFER *rowbuf)
{
	CS_RETCODE	ret;
	CS_INT		nrows;
	CS_INT		ncols;

	if ((rowbuf == (ROW_BUFFER *) NULL) 
	    || (! CHECK_ROWBUF_MAGIC(rowbuf)))
	{
		FAIL_ON_FAIL(CS_FAIL,
			     "rb_dropbuf: Received bad row buffer pointer.");
	}

	nrows = rowbuf->nrows;
	ncols = rowbuf->ncols;

	ret = DropColDataAreas(nrows, ncols, rowbuf->columns);
	if (ret != CS_SUCCEED)
	{
		return CS_FAIL;
	}

	free ((CS_VOID *) rowbuf);

	return CS_SUCCEED;
	
}

/*
** rb_fillbuf() -- Fetch rows into a row buffer created by rb_createbuf().
**
** Parameters
**  cmd -- The CS_COMMAND structure that rb_createbuf() succeeded with.
**  *rowbuf -- The row buffer created by rb_createbuf().
**  *p_nfetched -- (output) The number of valid rows placed into the
**      buffer. The last fetch on a result set may return fewer rows
**      than the buffer length.
**
** RETURNS
**  CS_SUCCEED -- The buffer has been filled, and additional results 
**       remain to be retrieved. The command structure will have
**       results pending until this routine returns CS_END_DATA or
**       the caller cancels the result set.
**  CS_END_DATA -- The last of the queries rows have been fetched.
**       *p_nfetched tells the number of valid rows in the buffer.
**       It is possible for *p_nfetched to be 0.
**  CS_ROW_FAIL -- Recoverable error occurred while fetching the last 
**       row in the buffer (*p_nfetched). The last row contains invalid
**       data. The rows before it are fine. There is still data to be 
**       read in the row set sent by the server.
**  CS_FAIL -- Failure occurred.
*/
CS_RETCODE CS_PUBLIC
rb_fillbuf(CS_COMMAND *cmd, ROW_BUFFER *rowbuf, CS_INT *p_nfetched)
{
	CS_RETCODE	ret;

	/*
	** Check for incoming rudeness.
	*/
	if (p_nfetched == (CS_INT *)NULL)
	{
		FAIL_ON_FAIL(CS_FAIL, 
			     "rb_fillbuf: received NULL nfetched (3rd) parameter.");
	}
	*p_nfetched = 0;
	if ((rowbuf == (ROW_BUFFER *) NULL) 
	    || (! CHECK_ROWBUF_MAGIC(rowbuf) ) )
	{
		FAIL_ON_FAIL(CS_FAIL,
			     "rb_fillbuf: Received bad row buffer pointer.");
	}

	ret = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, p_nfetched);
	if (ret != CS_SUCCEED && ret != CS_END_DATA && ret != CS_ROW_FAIL)
	{
		ERROR_OUT("rb_fillbuf: ct_fetch() failed.");
		*p_nfetched = 0;
		ret = CS_FAIL;
	}

	rowbuf->nfetched = *p_nfetched;
	return ret;

} /* rb_fillbuf() */

/*
** rb_describe() -- Set the fields of a CS_DATAFMT to describe the format
**   of a column in the row buffer.
**
** Parameters
**    rowbuf -- Pointer to the row buffer.
**    colnum -- The column number. The first column is 1.
**    p_datafmt -- Pointer to the CS_DATAFMT structure. The CS_DATAFMT
**       fields are set as in a call to ct_describe().
**
** Returns
**  CS_SUCCEED -- success.
**  CS_FAIL -- error occurred.
*/
CS_RETCODE CS_PUBLIC
rb_describe (ROW_BUFFER *rowbuf, CS_INT col, CS_DATAFMT *p_datafmt)
{
	CS_DATAFMT	*p_colfmt;
	COLUMN_DATA	*p_column;

	/*
	** Make sure the row buffer is ours.
	*/
	if ((rowbuf == (ROW_BUFFER *) NULL) 
	    || (! CHECK_ROWBUF_MAGIC(rowbuf) ) )
	{
		FAIL_ON_FAIL(CS_FAIL,
			     "rb_describe: Received bad row buffer pointer.");
	}

	/*
	** Check for null CS_DATAFMT pointer.
	*/
	if (p_datafmt == (CS_DATAFMT *) NULL)
	{
		FAIL_ON_FAIL(CS_FAIL,
			     "rb_describe: Received NULL CS_DATAFMT pointer.");
	}

	/*
	** Range column number.
	*/
	if (col < 0 || col > rowbuf->ncols)
	{
		ERROR_OUT("rb_describe: Bad column number.");
		return (CS_FAIL);
	}
 
	p_column = (rowbuf->columns) + (col - 1);
	p_colfmt = &(p_column->datafmt);

	/*
	** Copy the fields.
	*/
	(CS_VOID) memcpy(p_datafmt, p_colfmt, sizeof(CS_DATAFMT));

	/*
	** Set the count to 1 to match ct_describe's output. 
	** We also set the format to CS_FMT_UNUSED. ct_describe
	** leaves the format field as is.
	*/
	p_colfmt->count = 1;
	p_colfmt->format = CS_FMT_UNUSED;

	return CS_SUCCEED;

} /* rb_describe() */

/*
** rb_datalen() -- Returns the length in bytes of a column value in
**    the row buffer. 
**
**    This function returns the length in bytes of a column value in a
**    specific row. Use rb_describe() to get a generic description of
**    the column, including the maximum length.
**
** Parameters
**    rowbuf -- Pointer to the row buffer.
**    row  -- The row number. The first row is 1. It is an
**      error if rownum > the number of rows fetched by the last
**      rb_fillbuf() call.
**    col -- The column number. The first column is 1.
**
** Returns
**    n >= 0 -- The length, in bytes, of the column data.
**    0 is returned on error.
*/
CS_INT CS_PUBLIC
rb_datalen (ROW_BUFFER *rowbuf, CS_INT row, CS_INT col)
{
	CS_DATAFMT	*p_datafmt;
	CS_INT		*p_datalen;
	COLUMN_DATA	*p_column;
	CS_CHAR		message[256];

	/*
	** Make sure the row buffer is ours.
	*/
	if ((rowbuf == (ROW_BUFFER *) NULL) 
	    || (! CHECK_ROWBUF_MAGIC(rowbuf) ) )
	{
		ERROR_OUT("rb_datalen: Received bad row buffer pointer.");
		return ((CS_INT) 0);
	}

	/*
	** Range check row and column numbers.
	*/
	if (col < 0 || col > rowbuf->ncols)
	{
		ERROR_OUT("rb_datalen: Bad column number.");
		return ((CS_INT) 0);
	}
	if (row < 0 || row > rowbuf->nfetched)
	{
		ERROR_OUT("rb_datalen: Bad row number.");
		return ((CS_INT) 0);
	}

	p_column = (rowbuf->columns) + (col - 1);
	p_datafmt = &(p_column->datafmt);

	/*
	** If the datatype is unsupported, the lengths pointer will
	** be NULL.
	*/
	if (p_column->lengths == (CS_INT *)NULL)
	{
		sprintf(message, 
			"rb_datalen: Attempt to get the value of unsupported datatype %s.",
			dbct_prtype(p_datafmt->datatype));
		ERROR_OUT(message);
		return ((CS_INT) 0);
	}

	p_datalen = (p_column->lengths) + (row - 1);

	return (*p_datalen);

} /* rb_datalen() */

/*
** rb_dataptr() -- returns a pointer to a column's value
**    in the row buffer.
**
** Parameters
**    rowbuf -- Pointer to the row buffer.
**    row  -- The row number. The first row is 1. It is an
**      error if rownum > the number of rows fetched by the last
**      rb_fillbuf() call.
**    col -- The column number. The first column is 1.
**
** Returns
**    ptr -- A pointer to the column data.
**    NULL is returned on error.
*/
CS_BYTE * CS_PUBLIC 
rb_dataptr (ROW_BUFFER	*rowbuf, CS_INT row, CS_INT col)
{
	CS_DATAFMT	*p_datafmt;
	CS_BYTE		*p_datavalue;
	CS_INT		maxlen;
	COLUMN_DATA	*p_column;
	CS_CHAR		message[128];

	/*
	** Make sure the row buffer is ours.
	*/
	if ((rowbuf == (ROW_BUFFER *) NULL) 
	    || (! CHECK_ROWBUF_MAGIC(rowbuf) ) )
	{
		ERROR_OUT("rb_dataptr: Received bad row buffer pointer.");
		return ((CS_BYTE *) NULL);
	}

	/*
	** Range check row and column numbers.
	*/
	if (col < 0 || col > rowbuf->ncols)
	{
		ERROR_OUT("rb_dataptr: Bad column number.");
		return ((CS_BYTE *) NULL);
	}
	if (row < 0 || row > rowbuf->nfetched)
	{
		ERROR_OUT("rb_dataptr: Bad row number.");
		return ((CS_BYTE *) NULL);
	}
 
	p_column = (rowbuf->columns) + (col - 1);
	p_datafmt = &(p_column->datafmt);
	maxlen = p_datafmt->maxlength;

	/*
	** If the datatype is unsupported, the value_array pointer will
	** be NULL.
	*/
	if (p_column->value_array == (CS_BYTE *)NULL)
	{
		sprintf(message, 
			"rb_dataptr: Attempt to get the value of unsupported datatype %s.",
			dbct_prtype(p_datafmt->datatype));
		ERROR_OUT(message);
		return ((CS_BYTE *) NULL);
	}

	p_datavalue = (p_column->value_array) + ((row - 1) * maxlen);

	return (p_datavalue);

} /* rb_dataptr() */

/*
** rb_printrow() -- Print a row from the row buffer to the specified 
**    character device.
**
** Parameters
**   cmd -- A CS_COMMAND structure. The structure's grandmother 
**     context structure is used to convert the data to char.
**   rowbuf -- The row buffer created by rb_createbuf().
**   row_num -- The number of the row to print. The first row number is 1.
**   outfile -- File descriptor to write to. The file should be open for
**     writing.
**
** Returns
**    CS_SUCCEED -- Success.
**    CS_FAIL -- Conversion error occurred.
*/
CS_RETCODE CS_PUBLIC
rb_printrow (CS_COMMAND *cmd, ROW_BUFFER *rowbuf, CS_INT row_num, FILE *outfile)
{ 
	CS_RETCODE	ret;
	CS_CONTEXT	*context;
	CS_INT		ncols;
	CS_INT		col_num;
	CS_DATAFMT	outfmt;
	CS_INT		conv_len;
	CS_DATAFMT	srcdatafmt;
	CS_BYTE		*srcdata;
	CS_CHAR		outbuf[512];
	CS_VOID		*old_errhandler;

	/*
	** Make sure the row buffer is ours.
	*/
	if ((rowbuf == (ROW_BUFFER *) NULL) 
	    || (! CHECK_ROWBUF_MAGIC(rowbuf) ) )
	{
		FAIL_ON_FAIL(CS_FAIL,
			     "rb_printrow: Received bad row buffer pointer.");
	}

	/*
	** Make sure that row_num is in range.
	*/
	if (row_num < 1 || row_num > (rowbuf->nfetched))
	{
		FAIL_ON_FAIL(CS_FAIL,
			     "rb_printrow: Received out-of-range row number.");
	}

	ncols = rowbuf->ncols;

	/*
	** We will need the command structure's grandmother context structure
	** to call cs_convert().
	*/
	ret = GetParentCtx(cmd, (CS_CONNECTION *)NULL, &context);
	FAIL_ON_FAIL(ret, "rb_printrow: GetContext() failed.");

	/*
	** Get the existing CS-Library error handler, if any. From this
	** point on, we need to be careful to re-install before returning.
	*/
	ret = cs_config(context, CS_GET, CS_MESSAGE_CB, 
			(CS_VOID *) &old_errhandler,
			CS_UNUSED, NULL);
	FAIL_ON_FAIL(ret, "rb_printrows: Could not get CS-Lib error handler.");

	/* 
	** Install our CS-Library error handler.
	*/
	ret = cs_config(context, CS_SET, CS_MESSAGE_CB, 
			(CS_VOID *)cslib_err_handler,
			CS_UNUSED, NULL);
	if (ret != CS_SUCCEED)
	{
		ERROR_OUT("rb_printrows: Could not install CS-Lib error handler.");
	}

	/* 
	** Set up the destination CS_DATAFMT. This format will be used for
	** all conversions.
	*/
	outfmt.datatype = CS_CHAR_TYPE;
	outfmt.maxlength = 512;
	outfmt.format = CS_FMT_NULLTERM;
	outfmt.locale = (CS_LOCALE *) NULL;     

	/*
	** Go through the columns, convert each value to char, and display it.
	*/
	for (col_num = 1; (ret == CS_SUCCEED) && (col_num <= ncols); ++col_num)
	{
		ret = rb_describe(rowbuf, col_num, &srcdatafmt);
		FAIL_ON_FAIL(ret, "rb_printrows: rb_describe() failed.");

		outfmt.precision = srcdatafmt.precision;
		outfmt.scale = srcdatafmt.scale;

		/*
		** Get a pointer to the value and get its length in bytes.
		*/
		if (SUPPORTED_TYPE(srcdatafmt.datatype))
		{
			srcdata = rb_dataptr(rowbuf, row_num, col_num);
			srcdatafmt.maxlength = rb_datalen(rowbuf, row_num,
							  col_num);
		}
		else
		{
			srcdata = (CS_BYTE *) NULL;
			srcdatafmt.maxlength = 0;
		}

		/*
		** If the column's datatype is supported by the routines in this
		** file, then call cs_convert() to get a printable value.
		** Otherwise, we print a dummy value.
		*/
		if (SUPPORTED_TYPE(srcdatafmt.datatype))
		{
			/*
			** Convert the data to char.
			*/
			ret = cs_convert(context, &srcdatafmt,
					 (CS_VOID *) srcdata, &outfmt,
					 (CS_VOID *) outbuf, &conv_len);
			if (ret != CS_SUCCEED)
			{
				ERROR_OUT("rb_printrow: cs_convert() failed.");
			}
		}
		else
		{
			/*
			** Print a substitute value for columns where
			** we did not get the data.
			*/
			sprintf(outbuf, "** (Unsupported datatype %s)",
				dbct_prtype(srcdatafmt.datatype));
			conv_len = strlen(outbuf) + 1;
		}

		/*
		** Print out the column's name and value.
		*/
		if (ret == CS_SUCCEED)
		{
			/*
			** Print the column's name.
			*/
			if (srcdatafmt.namelen <= 0)
			{
				fprintf(outfile, "(unnamed)");
			}
			else
			{
				fprintf(outfile, "%s", srcdatafmt.name);
			}
			fprintf(outfile, ": ");
			fprintf(outfile, "%s\n", outbuf);
		} /* if */
 
	} /* for */

	/*
	** Re-install the caller's CS-Library error handler.
	*/
	if (old_errhandler == (CS_VOID *) NULL)
	{ 
		if (cs_config(context, CS_CLEAR, CS_MESSAGE_CB, 
			      (CS_VOID *) NULL, CS_UNUSED, NULL)
		    != CS_SUCCEED)
		{
			ERROR_OUT("rb_printrow: Could not clear CS-Lib error handler.");
		}
	}
	else if (cs_config(context, CS_SET, CS_MESSAGE_CB, old_errhandler,
			   CS_UNUSED, NULL)
		 != CS_SUCCEED)
	{
		ERROR_OUT("rb_printrow: Could not re-install CS-Lib error handler.");
	}

	if (ret != CS_SUCCEED)
	{
		return CS_FAIL;
	}
	
	return CS_SUCCEED;

} /* rb_printrow() */

/*
** DropColDataAreas() -- Free an array of COLUMN_DATA structures.
**
** Parameters
**   nrows -- the length of the row buffer.
**   ncols -- The number of columns in the result set.
**   *p_col_bufs -- The address of the array of COLUMN_DATA structures.
**
** Returns
**   CS_SUCCEED (always in this implementation).
*/
CS_STATIC CS_RETCODE
DropColDataAreas (CS_INT nrows, CS_INT ncols, COLUMN_DATA *p_col_bufs)
{
	CS_INT		col_num;
	COLUMN_DATA	*p_col;;

	/*
	** Free the COLUMN_DATA members
	** which were allocated.
	*/
	for (col_num = 0; col_num < ncols; col_num++)
	{
		p_col = p_col_bufs + col_num;

		if (p_col->value_array != (CS_BYTE *) NULL)
		{
			free ((CS_VOID *) (p_col->value_array));
		}
		if (p_col->lengths != (CS_INT *) NULL)   
		{
			free ((CS_VOID *) (p_col->lengths) );
		}
	}
	  
	/*
	** Free the array of COLUMN_DATA structures.
	*/
	free ((CS_VOID *) (p_col_bufs));

	return CS_SUCCEED;

} /* DropColDataAreas() */

/*
** CreateColDataAreas() -- Allocate the column areas for
**   a result set.
**
** Parameters
**   cmd -- the CS_COMMAND structure. This code expects that the 
**       caller is processing the results and ct_results has just
**       returned a result_type that indicates fetchable data.
**   nrows -- the length of the row buffer.
**   *p_ncols -- (output) The number of columns in the result set.
**   *p_col_bufs -- (output) An array of COLUMN_DATA structures.
**        The *p_col_bufs array should be freed by calling
**        DropColDataAreas().
**
** Side effects
**   Bindings on the command structure.
**
** Returns
**   CS_SUCCEED -- success.
**   CS_FAIL -- failure.
*/
CS_STATIC CS_RETCODE
CreateColDataAreas(CS_COMMAND *cmd, CS_INT nrows, CS_INT *p_ncols,
		   COLUMN_DATA **p_col_bufs)
{
	CS_RETCODE	ret;
	CS_INT		col_num;
	COLUMN_DATA	*p_col;

	*p_col_bufs = (COLUMN_DATA *)NULL;

	/*
	** How many columns?
	*/
	ret = ct_res_info(cmd, CS_NUMDATA, p_ncols, CS_UNUSED, (CS_INT *)NULL);
	if (ret != CS_SUCCEED)
	{
		*p_ncols = 0;
		FAIL_ON_FAIL(CS_FAIL,
			     "CreateColDataAreas: ct_res_info() failed.");
	}

	/*
	** Allocate an array of COLUMN_DATA structures.
	*/
	*p_col_bufs = (COLUMN_DATA *)malloc( sizeof(COLUMN_DATA) * (*p_ncols) );
	if (*p_col_bufs == (COLUMN_DATA *)NULL)
	{
		*p_ncols = 0;
		FAIL_ON_FAIL(CS_FAIL,
			     "CreateColDataAreas: Not enough memory for row buffer.");
	}

	/*
	** Loop through the result columns.
	** For each column, we'll get a description and allocate the neccessary
	** space to retrieve the data.
	*/
	ret = CS_SUCCEED;
	for (col_num = 0; (col_num < *p_ncols) && (ret == CS_SUCCEED);
	     ++col_num)
	{

		p_col = (*p_col_bufs + col_num);

		  /*
		  ** Get the column's description.
		  */
		  ret = ct_describe(cmd, col_num + 1, &(p_col->datafmt));
		  if (ret != CS_SUCCEED)
		  {
			    ERROR_OUT("ct_describe failed.");
			    ret = CS_FAIL;
		  }
	  
		/*
		** Set the datafmt.count so we get array binding later.
		** Make sure that the format field is CS_FMT_UNUSED.
		*/
		if (ret == CS_SUCCEED)
		{
			(p_col->datafmt).count = nrows;
			(p_col->datafmt).format = CS_FMT_UNUSED;
		}

		/*
		** We won't bind to unsupported datatypes, so set the
		** length to 0 for these.
		*/
		if ((ret == CS_SUCCEED)
		    && (! SUPPORTED_TYPE((p_col->datafmt).datatype)) )
		{
			(p_col->datafmt).maxlength = 0;
			p_col->value_array = (CS_BYTE *) NULL;
			p_col->lengths = (CS_INT *) NULL;
		}

		/*
		** Now allocate space for the value array and bind
		** the column to it. If the column length was set to 0 above,
		** we do nothing.
		*/
		if ( ret == CS_SUCCEED && ((p_col->datafmt).maxlength > 0) )
		{
			/*
			** Allocate space for the values.
			*/
			p_col->value_array = (CS_BYTE *) malloc(sizeof(CS_BYTE) * nrows * ((p_col->datafmt).maxlength) );
			if (p_col->value_array == (CS_BYTE *) NULL)
			{
				ret = CS_FAIL;
			}

			/*
			** Allocate an array to hold the length of each value.
			*/
			if (ret == CS_SUCCEED)
			{
				p_col->lengths = (CS_INT *)malloc(sizeof(CS_INT) * nrows );
				if (p_col->lengths == (CS_INT *) NULL)
				{
					ret = CS_FAIL;
				}
			}
			if (ret != CS_SUCCEED)
			{
				ERROR_OUT("CreateColDataAreas: Out of memory for data areas.");
			}
	    
			/*
			** Bind to the value array.
			*/
			if (ret == CS_SUCCEED)
			{
				ret = ct_bind(cmd, col_num + 1,
					      &(p_col->datafmt),
					      (CS_VOID *) (p_col->value_array),
					      (p_col->lengths),
					      (CS_SMALLINT *)NULL);
				if (ret != CS_SUCCEED)
				{
					ERROR_OUT("CreateColDataAreas: ct_bind failed.");
					ret = CS_FAIL;
				}
			}
		} /* if */

	} /* for */

	/* 
	** Clean up and fail if an error occurred.
	*/
	if (ret != CS_SUCCEED)
	{
		/*
		** Starting with the last allocated, free the COLUMN_DATA
		** members which were allocated.
		*/
		ERROR_OUT("CreateColDataAreas: Begin cleanup ...");
		for (--col_num; col_num >= 0; --col_num)
		{
			p_col = *p_col_bufs + col_num;

			/*
			** Lengths and value_array are null if we decided that
			** the column's datatype is not supported.
			*/
			if (p_col->value_array != (CS_BYTE *) NULL)
			{
				free ((CS_VOID *) (p_col->value_array) );
			}
			if (p_col->lengths != (CS_INT *) NULL)
			{
				free ((CS_VOID *) (p_col->lengths) );
			}
		}
	  
		/*
		** Free the array of COLUMN_DATA structures.
		*/
		free ((CS_VOID *) (*p_col_bufs));
		*p_col_bufs = (COLUMN_DATA *)NULL;
		*p_ncols = 0;

		/*
		** Clear all bindings so our caller does not get any surprises
		** if she calls ct_fetch() for some reason.
		*/
		(CS_VOID) ct_bind(cmd, CS_UNUSED, (CS_DATAFMT *)NULL, 
				  (CS_VOID *)NULL, (CS_INT *)NULL,
				  (CS_SMALLINT *)NULL);

		return CS_FAIL;
	} /* if */

	return CS_SUCCEED;

} /* CreateColDataAreas() */

/*
** GetParentCtx() --
**    Gets the parent context of a connection or the grandparent 
**    context of a command structure. 
**    
**    If the connection handle is non-null, gets the parent of 
**    the connection. Otherwise, gets the parent of the 
**    command-handle's parent connection.
**
** Returns
**   CS_SUCCEED or CS_FAIL.
*/
CS_STATIC CS_RETCODE
GetParentCtx (CS_COMMAND *cmd, CS_CONNECTION *conn, CS_CONTEXT **ctxptr)
{
	CS_RETCODE	retcode;

	*ctxptr = (CS_CONTEXT *) NULL;

	if (conn != (CS_CONNECTION *) NULL)
	{
		retcode = ct_con_props(conn, CS_GET, CS_PARENT_HANDLE,
				       (CS_VOID*) ctxptr, CS_UNUSED,
				       (CS_INT *)NULL);
		if (retcode != CS_SUCCEED)
		{
			return CS_FAIL;
		}
		else
		{
			return CS_SUCCEED;
		}
	}
	else
	{
		/* 
		** Get the command handle's parent connection.
		*/
		retcode = GetParentConn(cmd, &conn);
		if (retcode != CS_SUCCEED)
		{
			return retcode;
		}

		/*
		** Recurse to get connection's parent context.
		*/
		return GetParentCtx((CS_COMMAND *) NULL, conn, ctxptr);
	}
} /* GetParentCtx() */

/*
** GetParentConn() --
**   Gets the parent context of a connection.  
**
** Returns:
**   CS_SUCCEED or CS_FAIL.
*/
CS_STATIC CS_RETCODE
GetParentConn (CS_COMMAND *cmd, CS_CONNECTION **connptr)
{
	CS_RETCODE retcode;
	retcode = ct_cmd_props(cmd, CS_GET, CS_PARENT_HANDLE,
			       (CS_VOID *) connptr, CS_UNUSED, (CS_INT *)NULL);
	if (retcode != CS_SUCCEED)
	{
		return CS_FAIL;
	}
	return CS_SUCCEED;
} /* GetParentConn() */

/*
**  cslib_err_handler() - CS-Library error handler.
**
**   This routine is the CS-Library error handler used by this
**   application. It is called by CS-Library whenever an error
**   occurs. Here, we simply print the error and return.
**
**  Parameters:
**      context         A pointer to the context handle for context
**       on which the error occurred.
**
**      error_msg       The structure containing information about the
**      error.
**
**  Returns:
**      CS_SUCCEED
*/
CS_STATIC CS_RETCODE CS_PUBLIC
cslib_err_handler(CS_CONTEXT *context, CS_CLIENTMSG  *errmsg)
{
	/*
	** Print the error details.
	*/
	fprintf(ERR_CH, "CS-Library error: ");
	fprintf(ERR_CH, "LAYER = (%ld) ORIGIN = (%ld) ",
		(long)CS_LAYER(errmsg->msgnumber), 
		(long)CS_ORIGIN(errmsg->msgnumber) );
	fprintf(ERR_CH, "SEVERITY = (%ld) NUMBER = (%ld)\n",
		(long)CS_SEVERITY(errmsg->msgnumber), 
		(long)CS_NUMBER(errmsg->msgnumber) ); 
	fprintf(ERR_CH, "\t%s\n", errmsg->msgstring);

	/*
	** Print any operating system error information.
	*/
	if( errmsg->osstringlen > 0 )
	{
		fprintf(ERR_CH, "CS-Library OS error %ld - %s.\n",
			(long) errmsg->osnumber, errmsg->osstring);
	}

	/*
	** All done.
	*/
	return (CS_SUCCEED);

} /* cslib_err_handler() */

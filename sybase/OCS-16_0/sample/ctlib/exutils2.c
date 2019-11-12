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
** 	This example file contains utility routines which are used only by the
**	scrollable cursor sample programs. This file is used with the 
**	csr_disp_scrollable and csr_disp_scrollable2 examples. It uses an array 
**	of index values to simulate a user driving the ct_scroll_fetch() API. 
**	The values are chosen by passing a numerical list (sequence of valid 
**	integers) to generate a pattern of commands retrieving random data off 
**	a read-only scrollable cursor. 
**
**	The csr_disp_scrollable example uses a section out of this file based
**	on program variables only, prefetch count is set to 1. 
**	The csr_disp_scrollable2 example uses arrays plus array binding. 
**	These examples are illustrative.
**
** Routines Used
** -------------
** 	ct_scroll_fetch()
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctpublic.h>
#include "example.h"
#include "exutils.h"

/*
** Arrays for binding columns
** See exutils.h for the array of columns definition
*/
static COLUMN_ARRAY s_col1, s_col2, s_col3;

/*
** 2D array for selecting a scroll type for an array of allowable
** values. This is called as follows:
**
** 	keyval = scroll_index(array[index], scroll_arrmap);
**	
**	with index taking values of [0..5], keyval then corresponds
**	with index, e.g. 0 -> CS_FIRST etc.
*/
CS_STATIC CT_SCROLL_INDEX 
scroll_arrmap[] =
{
	{ 0,        	CS_FIRST    },
	{ 1,      	CS_NEXT     },
	{ 2,       	CS_PREV     },
	{ 3,      	CS_RELATIVE },
	{ 4,      	CS_ABSOLUTE },
	{ 5,    	CS_LAST     },
	{ EX_BADVAL,	EX_BADVAL }
};

CS_INT CS_PUBLIC
scroll_index(CS_INT index, CT_SCROLL_INDEX *map)
{
	if (index == EX_BADVAL)
	{
		return EX_BADVAL;
	}
	else
	{
		return (map[index].scrolltype);
	}
}

/*
** Data to feed ex_scroll_fetch_data(), e.g. feed it the position
** request plus relevant offset or  scrolling terminator. This is
** used to simulate a user typing cursor scroll commands without 
** having a user present.
*/

/*
** Below testlists represent a sample, mixed usage of driving 
** ct_scroll_fetch(). As an example, the first list (active 
** list #0) below issues the following type of requests on 
** ct_scroll_fetch():  
**
** 	CS_FIRST, CS_NEXT, CS_NEXT, CS_FIRST, CS_PREV, CS_PREV, 
** 	EX_BADVAL (-1), e.g. terminate. 
**
** If EX_BADVAL is returned we pass CS_FALSE into the option 
** argument to ct_scroll_fetch() and call it one more time. 
** Note that for type and offset CS_UNUSED is passed in. Also the 
** list for type _and_ offset should have correct and corresponding 
** values, e.g. if type is CS_ABSOLUTE, offset can take an unsigned 
** integer value, if type is e.g. CS_PREV, offset must be CS_UNUSED. 
** See the CT-Lib reference manuals for more information.
*/

CS_STATIC CS_INT type_list0[EX_MAX_ARR] = 
				{0,1,1,0,2,2,EX_BADVAL};

CS_STATIC CS_INT offset_list0[EX_MAX_ARR] = 
				{CS_UNUSED, CS_UNUSED, CS_UNUSED, 
				 CS_UNUSED, CS_UNUSED, CS_UNUSED, 
				 EX_BADVAL};


/*****************************************************************************
** 
** utility functions 
** 
*****************************************************************************/

/*
** ex_scroll_fetch_1()
**
** Type of function:
** 	example program utility api
** 
** Purpose:
** 	This function processes fetchable cursor results sets. The results 
**	set of interest is:
**
**		CS_CURSOR_RESULT 
**
** Parameters:
**	cmd    - Pointer to command structure
**
** Return:
**	CS_MEM_ERROR		If an memory allocation failed.
**	CS_SUCCEED		If the data was displayed.
**	CS_FAIL			If no columns were present.
**	CS_SCROLL_CURSOR_ENDS	If user signalled end of fetching.
** 	<retcode>		Result of the Client-Library function if a 
**				failure was returned.
**
*/
CS_RETCODE CS_PUBLIC
ex_scroll_fetch_1(CS_COMMAND *cmd)
{
	CS_RETCODE		retcode;
	CS_INT			num_cols;
	CS_INT			i;
	CS_INT			j;
	CS_INT			k;
	CS_INT			row_count = 0;
	CS_INT			rows_read;
	CS_INT			disp_len;
	CS_INT			sc_type;
	CS_INT			sc_offset;
	CS_INT			sc_option;
	CS_DATAFMT		*datafmt;
	EX_COLUMN_DATA		*coldata;

	/*
	** Find out how many columns there are in this result set.
	*/
	retcode = ct_res_info(cmd, CS_NUMDATA, &num_cols, CS_UNUSED, NULL);
	if (retcode != CS_SUCCEED)
	{
		ex_error("ex_scroll_fetch_data: ct_res_info() failed");
		return retcode;
	}

	/*
	** Make sure we have at least one column
	*/
	if (num_cols <= 0)
	{
		ex_error("ex_scroll_fetch_data: ct_res_info() returned zero columns");
		return CS_FAIL;
	}

	/*
	** Our program variable, called 'coldata', is an array of 
	** EX_COLUMN_DATA structures. Each array element represents
	** one column.  Each array element will re-used for each row.
	**
	** First, allocate memory for the data element to process.
	*/
	coldata = (EX_COLUMN_DATA *)malloc(num_cols * sizeof (EX_COLUMN_DATA));
	if (coldata == NULL)
	{
		ex_error("ex_scroll_fetch_data: malloc() failed");
		return CS_MEM_ERROR;
	}

	datafmt = (CS_DATAFMT *)malloc(num_cols * sizeof (CS_DATAFMT));
	if (datafmt == NULL)
	{
		ex_error("ex_scroll_fetch_data: malloc() failed");
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
			ex_error("ex_scroll_fetch_data: ct_describe() failed");
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

		/*
		** Allocate memory for the column string
		*/
		coldata[i].value = (CS_CHAR *)malloc(datafmt[i].maxlength);
		if (coldata[i].value == NULL)
		{
			ex_error("ex_scroll_fetch_data: malloc() failed");
			retcode = CS_MEM_ERROR;
			break;
		}

		/*
		** Now bind.
		*/
		retcode = ct_bind(cmd, (i + 1), &datafmt[i],
				coldata[i].value, &coldata[i].valuelen,
				(CS_SMALLINT *)&coldata[i].indicator);
		if (retcode != CS_SUCCEED)
		{
			ex_error("ex_scroll_fetch_data: ct_bind() failed");
			break;
		}
	}
	if (retcode != CS_SUCCEED)
	{
		for (j = 0; j < i; j++)
		{
			free(coldata[j].value);
		}
		free(coldata);
		free(datafmt);
		return retcode;
	}

	/*
	** Display column header
	*/
	ex_display_header(num_cols, datafmt);

	/*
	** Fetch the rows.  Call ct_scroll_fetch() as long as it returns 
	** CS_SUCCEED, CS_ROW_FAIL, CS_CURSOR_BEFORE_FIRST or 
	** CS_CURSOR_AFTER_LAST.
	** These are recoverable or "row" producing conditions, e.g. non-fatal.
	** All other terminate the loop, either by error or choice. 
	*/
	for (i = 0; i < EX_MAX_ARR; i++)
	{
		sc_type = scroll_index(type_list0[i], scroll_arrmap);
		sc_offset = offset_list0[i];

		if (type_list0[i] != EX_BADVAL)
		{
			sc_option = CS_TRUE;
		}
		else
		{
			/*
			** Since EX_BADVAL is not valid to pass into
			** either sc_type or sc_offset we set these
			** to CS_UNUSED respectively.
			*/
			sc_type = CS_UNUSED;
			sc_offset = CS_UNUSED;
			sc_option = CS_FALSE;
		}

		retcode = ct_scroll_fetch(cmd, sc_type, sc_offset, sc_option, &rows_read);

		switch ((int)retcode)
		{
			case CS_ROW_FAIL:

				fprintf(stdout, "Error on row %d.\n", row_count);
				fflush(stdout);
				break;

			case CS_CURSOR_BEFORE_FIRST:

				fprintf(stdout, " Cursor before first row\n");
				fflush(stdout);
				break;

			case CS_CURSOR_AFTER_LAST:
				fprintf(stdout, " Cursor after last row\n");
				fflush(stdout);
				break;

			case CS_SUCCEED:

				/*
				** Increment our row count by the number of 
				** rows just fetched.
				*/
				row_count = row_count + rows_read;

				/*
				** Assume we have a row. Loop through the 
				** columns displaying the column values.
				*/
				for (k = 0; k < num_cols; k++)
				{	  
					/*
					** Display the column value
					*/
					fprintf(stdout, "%s", coldata[k].value);
					fflush(stdout);
		
					/*
					** If not last column, Print out spaces between
					** this column and next one. 
					*/
					if (k != num_cols - 1)
					{
						disp_len = ex_display_dlen(&datafmt[k]);
						disp_len -= coldata[k].valuelen - 1;
						for (j = 0; j < disp_len; j++)
						{
							fputc(' ', stdout);
						}
					}
				} 
				fprintf(stdout, "\n");
				fflush(stdout);
				break;

			case CS_FAIL:
	
				/*
				** Free allocated space.
				*/
				for (k = 0; k < num_cols; k++)
				{
					free(coldata[k].value);
				}
				free(coldata);
				free(datafmt);
				return retcode;

			case CS_SCROLL_CURSOR_ENDS:

				/*
				** User signalled ct_scroll_fetch() to stop
				** scrolling, we are done with this result set.
				** Free allocated space.
				*/
				for (k = 0; k < num_cols; k++)
				{
					free(coldata[k].value);
				}
				free(coldata);
				free(datafmt);
				return retcode;

			default:
			
				fprintf(stdout, "Hit default, this should not happen. Exiting program.\n");
				fflush(stdout);
				exit(0);
		} /* end switch */
	} /* end for */
	return CS_SUCCEED;
}

/*
** ex_scroll_fetch_2()
**
** Type of function:
** 	example program utility api
** 
** Purpose:
** 	This function processes fetchable cursor results sets. The results 
**	set of interest is:
**
**		CS_CURSOR_RESULT 
**
** Parameters:
**	cmd    - Pointer to command structure
**
** Return:
**	CS_MEM_ERROR		If an memory allocation failed.
**	CS_SUCCEED		If the data was displayed.
**	CS_FAIL			If no columns were present.
**	CS_SCROLL_CURSOR_ENDS	If user signalled end of fetching.
** 	<retcode>		Result of the Client-Library function if a 
**				failure was returned.
**
*/
CS_RETCODE CS_PUBLIC
ex_scroll_fetch_2(CS_COMMAND *cmd)
{
	CS_RETCODE		retcode;
	CS_INT			num_cols;
	CS_INT			i;
	CS_INT			j;
	CS_INT			k;
	CS_INT			row_count = 0;
	CS_INT			rows_read;
	CS_INT			sc_type;
	CS_INT			sc_offset;
	CS_INT			sc_option;
	CS_DATAFMT		*dfmt;

	/*
	** Find out how many columns there are in this result set.
	*/
	retcode = ct_res_info(cmd, CS_NUMDATA, &num_cols, CS_UNUSED, NULL);
	if (retcode != CS_SUCCEED)
	{
		ex_error("ex_scroll_fetch_arrbind: ct_res_info() failed");
		return retcode;
	}

	/*
	** Make sure we have at least one column
	*/
	if (num_cols <= 0)
	{
		ex_error("ex_scroll_fetch_arrbind: ct_res_info() returned zero columns");
		return CS_FAIL;
	}

	if (num_cols > 3)
	{
		ex_error("ex_scroll_fetch_arrbind: ct_res_info() returned too many columns");
		return CS_FAIL;
	}

	/*
	** Our static program variables, called 's_col[1-3]', are arrays of 
	** COLUMN_ARRAY structures. Each array element represents
	** one column with ARRAY_BND_LEN 'rows'. In this example,
	** memory for the arrays are allocated by the compiler, no dynamic
	** allocation is done.
	*/

	/*
	** Bind to first column 's_col1'
	*/
	dfmt = &(s_col1.datafmt);
	strcpy(dfmt->name, "s_col1");
	dfmt->namelen = strlen(dfmt->name);
	dfmt->datatype = CS_CHAR_TYPE;
	dfmt->maxlength = MAX_COLUMN_LEN;
	dfmt->format = CS_FMT_NULLTERM;
	dfmt->count = ARRAY_BND_LEN;
	dfmt->locale = NULL;

	retcode = ct_bind(cmd, 1, dfmt,
			  (CS_VOID *)(s_col1.value), (s_col1.valuelen),
			  (s_col1.indicator));
	if (retcode != CS_SUCCEED)
	{
		ex_error("ex_ct_scroll_fetch: ct_bind(1) failed");
		return CS_FAIL;
	}

	/*
	** Bind to second column 's_col2'
	*/
	dfmt = &(s_col2.datafmt);
	strcpy(dfmt->name, "s_col2");
	dfmt->namelen = strlen(dfmt->name);
	dfmt->datatype = CS_CHAR_TYPE;
	dfmt->maxlength = MAX_COLUMN_LEN;
	dfmt->format = CS_FMT_NULLTERM;
	dfmt->count = ARRAY_BND_LEN;
	dfmt->locale = NULL;

	retcode = ct_bind(cmd, 2, dfmt,
			  (CS_VOID *)(s_col2.value), (s_col2.valuelen),
			  (s_col2.indicator));
	if (retcode != CS_SUCCEED)
	{
		ex_error("ex_ct_scroll_fetch: ct_bind(2) failed");
		return CS_FAIL;
	}

	/*
	** Bind to third column ('s_col3')
	*/
	dfmt = &(s_col3.datafmt);
	strcpy(dfmt->name, "s_col3");
	dfmt->namelen = strlen(dfmt->name);
	dfmt->datatype = CS_CHAR_TYPE;
	dfmt->maxlength = MAX_COLUMN_LEN;
	dfmt->format = CS_FMT_NULLTERM;
	dfmt->count = ARRAY_BND_LEN;
	dfmt->locale = NULL;

	retcode = ct_bind(cmd, 3, dfmt,
			  (CS_VOID *)(s_col3.value), (s_col3.valuelen),
			  (s_col3.indicator));
	if (retcode != CS_SUCCEED)
	{
		ex_error("ex_ct_scroll_fetch: ct_bind(3) failed");
		return CS_FAIL;
	}

	/*
	** Fetch the rows.  Call ct_scroll_fetch() as long as it returns 
	** CS_SUCCEED, CS_ROW_FAIL, CS_CURSOR_BEFORE_FIRST or CS_CURSOR_AFTER_LAST.
	** These are recoverable or "row" producing conditions, e.g. non-fatal.
	** All other terminate the loop, either by error or choice. 
	*/
	for (i = 0; i < EX_MAX_ARR; i++)
	{
		sc_type = scroll_index(type_list0[i], scroll_arrmap);
		sc_offset = offset_list0[i];

		if (type_list0[i] != EX_BADVAL)
		{
			sc_option = CS_TRUE;
		}
		else
		{
			/*
			** Since EX_BADVAL is not valid to pass into
			** either sc_type or sc_offset we set these
			** to CS_UNUSED respectively.
			*/
			sc_type = CS_UNUSED;
			sc_offset = CS_UNUSED;
			sc_option = CS_FALSE;
		}

		retcode = ct_scroll_fetch(cmd, sc_type, sc_offset, sc_option, &rows_read);

		switch ((int)retcode)
		{
			case CS_ROW_FAIL:

				fprintf(stdout, "Error on row %d.\n", row_count);
				fflush(stdout);
				break;

			case CS_CURSOR_BEFORE_FIRST:

				fprintf(stdout, "Cursor before first row\n");
				fflush(stdout);
				break;

			case CS_CURSOR_AFTER_LAST:
				fprintf(stdout, "Cursor after last row\n");
				fflush(stdout);
				break;

			case CS_SUCCEED:

				/*
				** Increment our row count by the number of 
				** rows just fetched.
				*/
				row_count = row_count + rows_read;

				for (j = 0; j < rows_read; j++)
				{
					/*
					** Value for column 's_col1'
					*/
					if (s_col1.indicator[j] <= 0)
					{
						/* Good data or null data */
						for (k = 0; k < s_col1.valuelen[j]; k++)
							fputc(s_col1.value[j][k], stdout);
						for (; k < s_col1.datafmt.maxlength; k++)
							fputc(' ', stdout);
					}

					/*
					** Value for column 's_col2'
					*/
					if (s_col2.indicator[j] <= 0)
					{
						/* Good data or null data */
						for (k = 0; k < s_col2.valuelen[j]; k++)
						fputc(s_col2.value[j][k], stdout);
						for (; k < s_col2.datafmt.maxlength; k++)
							fputc(' ', stdout);
					}

					/*
					** Value for column 's_col3'
					*/
					if (s_col3.indicator[j] <= 0)
					{
						/* Good data or null data */
						for (k = 0; k < s_col3.valuelen[j]; k++)
						fputc(s_col3.value[j][k], stdout);
						for (; k < s_col3.datafmt.maxlength; k++)
							fputc(' ', stdout);
					}
					fprintf(stdout, "\n");

				}
				fprintf(stdout, "%d rows read\n", rows_read);
				fprintf(stdout, "\n");
				fflush(stdout);
				break;

			case CS_FAIL:

				fprintf(stdout, "ct_scroll_fetch() returned FAIL\n");	
				return retcode;

			case CS_SCROLL_CURSOR_ENDS:

				/*
				** User signalled ct_scroll_fetch() to stop
				** scrolling, we are done with this result set.
				*/
				return retcode;

			default:
			
				fprintf(stdout, "Hit default, this should not happen. Exiting program.\n");
				fflush(stdout);
				exit(0);
		} /* end switch */
	} /* end for */
	return CS_SUCCEED;
}

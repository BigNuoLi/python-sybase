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
**	SAP OCS Print Functions
**
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"ctxact.h"

/*
** Some default options.
*/
#define PRLINELEN       	80

#define PRLINE       		"-"
 
#define PRLINESEP       	"\n"
#define PRLINESEPLEN       	1
#define PRCOLSEP        	" "
#define PRCOLSEPLEN        	1
#define PRPAD           	" "
#define PRPADLEN           	1
  
#define MAXCOLWIDTH		512
#define TABWIDTH		8

#define MAX(a,b)		(a > b ? a : b)
#define MIN(a,b)		(a > b ? b : a)

/* 
** Maximum printing lengths for fixed-length data 
*/
#define PRINT4  		11
#define PRINT2  		6
#define PRINT1  		3
#define PRFLT8  		20
#define PRFLT4  		20
#define PRBIT   		1
#define PRMONEY   		24
#define PRMONEY4  		24
#define PRDATETIME  		30
#define PRDATETIME4  		30

CS_STATIC CS_INT 	pr_len 		PROTOTYPE((
			CS_INT		type,
			CS_INT		len
			));
CS_STATIC CS_INT 	visual_len 	PROTOTYPE((
			CS_CHAR		*str,
			CS_INT		stringlen
			));

/*
** pr_head()
**
**	Print column headings
**
** Parameters:
**	cmd		The command handle.
**
** Returns:
**	CS_SUCCEED	Function completed without error.
**	CS_FAIL		Failure occurred.
*/
CS_RETCODE
pr_head(CS_COMMAND *cmd)
{
	CS_CHAR		charval;
	CS_INT		num;
	CS_INT		colsep_visual_len;
	CS_INT		colwidth;
	CS_INT		i,j;
	CS_INT		count;
	CS_INT		colprlen;
	CS_INT		linesep_visual_len;
	CS_DATAFMT	descfmt;

	/* Initialization						*/
	colwidth = 0;
	colsep_visual_len = visual_len(PRCOLSEP, PRCOLSEPLEN);
	linesep_visual_len = visual_len(PRLINESEP, PRLINESEPLEN);

	/* Determine the number of columns in the results rows.         */
	if (ct_res_info(cmd, CS_NUMDATA, (CS_VOID *)&num, CS_UNUSED,
		(CS_INT *)NULL) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	/*
	** Go through columns and print column headings.
	*/
	for (i = 0; i < num; i++)
	{
		/* Get a description of the column format.              */
		if (ct_describe(cmd, (i + 1), &descfmt) != CS_SUCCEED)
			return (CS_FAIL);

		colprlen = MAX(descfmt.namelen, 
				pr_len(descfmt.datatype, descfmt.maxlength));

		/* check for wrap-around 				*/
		if ((colwidth + colsep_visual_len + colprlen) >= PRLINELEN)
		{
			(CS_VOID)fprintf(stdout, "%s", PRLINESEP);
			colwidth = linesep_visual_len;
		}
		else
		{
			(CS_VOID)fprintf(stdout, "%s", PRCOLSEP);
			colwidth += colsep_visual_len;
		}

		colwidth += colprlen;

		/* print the column name */
		count = 0;
		for (j = 0; j < descfmt.namelen && j <= colprlen; j++)
		{
			/* Check for wrap around */
			if ( count++ >= PRLINELEN)
			{
				(CS_VOID)fprintf(stdout, "%s", PRLINESEP);
				count = 1;
			}

			charval = descfmt.name[j];
			(CS_VOID)fprintf(stdout, "%c", charval);
		}

		/* 
		** fill with blanks, if necessary, but don't bother to
		** print the thousands of blanks that may be required
		** for TEXT columns.
		*/
		for (j = descfmt.namelen; j < colprlen && j < MAXCOLWIDTH; j++)
		{
			(CS_VOID)fprintf(stdout, "%s", PRPAD);
		}
	}

	(CS_VOID)fprintf(stdout, "%s", PRLINESEP);

	return (CS_SUCCEED);
}

/*
** PR_LINE
**
**	Print out the line separator for the row.
**
** Parameters:
**	cmd		The command handle.
**
** Returns:
**	CS_SUCCEED	Function completed without error.
**	CS_FAIL		Failure occurred.
**
*/
CS_RETCODE
pr_line(CS_COMMAND *cmd)
{
	CS_INT		i, j;
	CS_INT		num;
	CS_INT		colsep_visual_len;
	CS_INT		colwidth;
	CS_INT		colprlen;
	CS_INT		linesep_visual_len;
	CS_DATAFMT	descfmt;

	/* Initialization						*/
	colwidth = 0;
	colsep_visual_len = visual_len(PRCOLSEP, PRCOLSEPLEN);
	linesep_visual_len = visual_len(PRLINESEP, PRLINESEPLEN);

	/* Determine the number of columns in the results rows.         */
	if (ct_res_info(cmd, CS_NUMDATA, (CS_VOID *)&num, CS_SIZEOF(num), 
		(CS_INT *)NULL) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	/*
	** Go through columns and print column lines.
	*/
	for (i = 0; i < num; i++)
	{
		/* Get a description of the column format.              */
		if (ct_describe(cmd, (i + 1), &descfmt) != CS_SUCCEED)
			return (CS_FAIL);

		colprlen = MAX(descfmt.namelen,
			       pr_len(descfmt.datatype, descfmt.maxlength));

		/* check for wrap-around */
		if ((colwidth + colsep_visual_len + colprlen) >= PRLINELEN)
		{
			(CS_VOID)fprintf(stdout, "%s", PRLINESEP);
			colwidth = linesep_visual_len;
		}
		else
		{
			(CS_VOID)fprintf(stdout, "%s", PRCOLSEP);
			colwidth += colsep_visual_len;
		}

		colwidth += colprlen;

		/* 
		** print the column line, but don't bother to print the
		** thousands of line-characters that may be required by
		** a TEXT column.
		*/
		for (j = 0; (j < colprlen) && (j < MAXCOLWIDTH); j++)
		{
			(CS_VOID)fprintf(stdout, "%s", PRLINE);
		}
	}

	(CS_VOID)fprintf(stdout, "%s", PRLINESEP);

	return (CS_SUCCEED);
}

/*
** PR_ROW
**
**	Print out the rows in a default format to the buffer provided.
**
** Parameters:
**	cmd		The command handle.
**
** Returns:
**	CS_SUCCEED	Function completed without error.
**	CS_FAIL		Failure occurred.
**
*/
CS_RETCODE 
pr_row(CS_COMMAND *cmd)
{
	CS_RETCODE	ret;
	CS_INT		num;
	CS_INT		i,j;
	CS_INT		colwidth;
	CS_INT		colprlen;
	CS_INT		colsep_visual_len;
	CS_INT		linesep_visual_len;
	CS_DATAFMT	*descfmt;
	CS_DATAFMT	bindfmt;
	CS_INT		*len;
	CS_CHAR		**data;

	/* Initialization						*/
	colwidth = 0;
	len = (CS_INT *)NULL;
	data = (CS_CHAR **)NULL;
	descfmt = (CS_DATAFMT *)NULL;
	colsep_visual_len = visual_len(PRCOLSEP, PRCOLSEPLEN);
	linesep_visual_len = visual_len(PRLINESEP, PRLINESEPLEN);

	memset((char *)&bindfmt, 0, sizeof(CS_DATAFMT));

	/* Set up the bind format					*/
	bindfmt.datatype = CS_CHAR_TYPE;
	bindfmt.format = (CS_FMT_NULLTERM | CS_FMT_PADBLANK);

	/* Determine the number of columns in the results rows.         */
	if (ct_res_info(cmd, CS_NUMDATA, (CS_VOID *)&num, CS_SIZEOF(num), 
		(CS_INT *)NULL) != CS_SUCCEED)
	{
		return (CS_FAIL);
	}

	/* Rows to process?						*/
	if (num <= 0)
		return (CS_FAIL);
	
	/* Get space for the data and descriptors			*/
	if ((descfmt = (CS_DATAFMT *)malloc(CS_SIZEOF(CS_DATAFMT) * num))
		== (CS_DATAFMT *)NULL)
	{
		return (CS_FAIL);
	}
	if ((len = (CS_INT *)malloc(CS_SIZEOF(CS_INT) * num)) == (CS_INT *)NULL)
	{
		free((char *)descfmt);
		return (CS_FAIL);
	}
	if ((data = (CS_CHAR **)malloc(CS_SIZEOF(CS_CHAR *) * num))
		== (CS_CHAR **)NULL)
	{
		free((char *)len);
		free((char *)descfmt);
		return (CS_FAIL);
	}

	/*
	** Go through the columns and allocate and bind to the data.
	*/
	for (i = 0; i < num; i++)
	{
		/* Get a description of the column format.              */
		if (ct_describe(cmd, (i + 1), &descfmt[i]) != CS_SUCCEED)
			return (CS_FAIL);

		colprlen = MAX(descfmt[i].namelen,
			       pr_len(descfmt[i].datatype, 
				      descfmt[i].maxlength));

		bindfmt.maxlength = colprlen +1;

		/* Allocate space for the data				*/
		if ((data[i] = (CS_CHAR *)malloc(bindfmt.maxlength +1))
			== (CS_CHAR *)NULL)
		{
			for (j = 0; j < i; j++)
				free(data[j]);

			free((char *)len);
			free((char *)data);
			free((char *)descfmt);
			return (CS_FAIL);
		}


		/* Bind to the data space				*/
		if (ct_bind(cmd, i+1, &bindfmt, (CS_VOID *)data[i], &len[i],
			NULL) != CS_SUCCEED)
		{
			for (j = 0; j < i; j++)
				free(data[j]);

			free((char *)len);
			free((char *)data);
			free((char *)descfmt);
			return (CS_FAIL);
		}
	}

	/*
	** Fetch and print the column data...
	*/
	while ((ret = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, NULL))
		== CS_SUCCEED)
	{
		for (i = 0; i < num; i++)
		{
			colprlen = MAX(descfmt[i].namelen,
				       pr_len(descfmt[i].datatype, 
					      descfmt[i].maxlength));

			/* check for wrap-around			*/
			if ((colwidth + colsep_visual_len + colprlen) 
				>= PRLINELEN)
			{
				(CS_VOID)fprintf(stdout, "%s", PRLINESEP);
				colwidth = linesep_visual_len;
			}
			else
			{
				(CS_VOID)fprintf(stdout, "%s", PRCOLSEP);
				colwidth += colsep_visual_len;
			}

			colwidth += colprlen;

			(CS_VOID)fprintf(stdout, "%s", data[i]);
		}
	}

	(CS_VOID)fprintf(stdout, "%s", PRLINESEP);

	for (i = 0; i < num; i++)
		free(data[i]);

	free((char *)len);
	free((char *)data);
	free((char *)descfmt);

	if (ret != CS_END_DATA)
		return (CS_FAIL);

	return (CS_SUCCEED);
}

/*
** visual_len()
**
**	This routine returns the visual length of a string, taking into account
**	tabstobs and linefeeds.
**
** Arguments:
**	str 		A pointer to the string to examine
**	stringlen 	length of the string.
**
** Returns:
**	The number of characters that woud be displayed
**
*/
CS_STATIC CS_INT
visual_len(CS_CHAR *str, CS_INT stringlen)
{
	CS_INT	i;
	CS_INT	len;

	len = 0;

	for (i = 0; i < stringlen; i++)
	{
		switch((int)*(str++))
		{
		  case '\n':
			len = 0;
			break;
		  case '\t':
			len += TABWIDTH;
			break;
		  default:
			len++;
			break;
		}
	}

	return(len);
}

/*
** pr_len()
**
**      Calculate the character string length needed for a partyicular server
**      datatype.
**
** Parameters:
**	type		- Server column type
**	len		- Server column length
**
** Returns:
**	Length in bytes or -1 on error.
**
*/
CS_STATIC CS_INT 
pr_len(CS_INT type, CS_INT len)
{
	switch ((int)type)
	{
	case CS_INT_TYPE:
		return (PRINT4);

	case CS_SMALLINT_TYPE:
		return (PRINT2);

	case CS_TINYINT_TYPE:
		return (PRINT1);

	case CS_REAL_TYPE:
		return (PRFLT4);

	case CS_FLOAT_TYPE:
	case CS_NUMERIC_TYPE:
	case CS_DECIMAL_TYPE:
		return (PRFLT8);

	case CS_MONEY_TYPE:
		return (PRMONEY);

	case CS_MONEY4_TYPE:
		return (PRMONEY4);

	case CS_DATETIME_TYPE:
		return (PRDATETIME);

	case CS_DATETIME4_TYPE:
		return (PRDATETIME4);

	case CS_BINARY_TYPE:
	case CS_VARBINARY_TYPE:
	case CS_LONGBINARY_TYPE:
		return (MIN(MAXCOLWIDTH, (len * 2) + 2));

	default:
		return (MIN(MAXCOLWIDTH, len));
	}
}

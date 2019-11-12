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
**	This example demonstrates some of the international features that
**	are available in Client Library. It installs a custom conversion
**	routine to handle multiple monetary formats. When processing
**	results, any conversions that are needed during a fetch of a row
**	will automatically call the installed routine when needed.
**
**	To demonstrate the fetch processing, the example sends user SQL
**	commands to the server via a language command. It processes the
**	results using the standard ct_results() while loop. It binds the
**	column values to program variables. It then fetches and displays
**	the rows in the standard ct_fetch() while loop.
**
**	The example also demonstrate the use of cs_locale() to set
**	localization, and supports a simplistic method of displaying
**	messages to the user based on language.
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
**	ct_cmd_alloc()
**	ct_send()
**	ct_results()
**	ct_res_info()
**	ct_describe()
**	ct_bind()
**	ct_fetch()
**	ct_cmd_drop()
**	ct_close()
**	ct_con_drop()
**	ct_exit()
**	ct_ctx_drop()
**	cs_loc_alloc()
**	cs_locale()
**	cs_config()
**	cs_set_convert()
**	cs_setnull()
**	cs_loc_drop()
**	cs_convert()
**
** Input
** -----
**	This example can use a hard-coded query of the titles table in
**	the pubs2 database.  The query is defined by EX_SELECT. If
**	EX_USE_SELECT is not defined, stdin is used to prompt the user.
**
** Output
** ------
**	This example displays the rows as they are returned from the server.
**
** Server Dependencies
** -------------------
**	To have server messages in the defined language requires that
**	langinstall has been run against the server for the language.
**
** Server Tables
** -------------
**	This example relies on the pubs2 database and the titles table.
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
CS_CHAR *Ex_appname  = "i18n";
CS_CHAR	*Ex_dbname   = "pubs2";
CS_CHAR *Ex_server   = EX_SERVER;
CS_CHAR *Ex_username = EX_USERNAME;
CS_CHAR *Ex_password = EX_PASSWORD;

/*
** Global context structure
*/
CS_CONTEXT	*Ex_context;

/*
** Uncomment the following if a hard coded select statment is desired for
** this example program. Otherwise, we will prompt for a query using
** stdin.
**
** #define EX_USE_SELECT	1
*/
#define EX_SELECT	"select title_id, total_sales, price from titles \
				where type like \"%cook\""

/*
** If a hard coded locale is desired, uncomment the defined below.
** Otherwise, the example program will use the environment for the
** locale. For a complete list of possible values for EX_LOCALE, please
** refer to the file $SYBASE/locales/locales.dat for valid entries for
** your particular platform.
**
** #define EX_USE_LOCALE	1
*/
#define EX_LOCALE	"fr"

/*
** Define an index for the custom conversion routine that
** we will install in this example.
*/
#define MY_MONEY_CHAR_TYPE	(CS_USER_TYPE + 1)

/*
** The following defines a very simplistic method of managing a
** set of messages which can work with multiple languages. The
** SetupLocale() function will initialize the Ex_language variable
** to the correct language.
**
** Normally, these messages would be in an external resource file.
*/ 

#define M_UNKNOWN	(CS_INT)-1
#define M_ENGLISH	(CS_INT)0
#define M_GERMAN	(CS_INT)1
#define M_FRENCH	(CS_INT)2
#define M_MAX_LANG	(CS_INT)(M_FRENCH + 1)

#define MSG_STARTUP	(CS_INT)0
#define MSG_QUERY	(CS_INT)1
#define MSG_ROW_DONE	(CS_INT)2
#define MSG_CMD_FAIL	(CS_INT)3
#define MSG_MAX_MSG	(CS_INT)(MSG_CMD_FAIL + 1)

CS_CHAR	*Ex_messages[][MSG_MAX_MSG] = {
{ 
  "International Example Program\n",
  "\nEnter query to send or type quit: ",
  "\nAll done processing rows.\n",
  "\nSQL command failed. Check the Server messages for possible errors.\n",
},
{ 
  "DM: International Example Program\n",
  "\nDM: Enter query to send or type quit: ",
  "\nDM: All done processing rows.\n",
  "\nDM: SQL command failed. Check the Server messages for possible errors.\n",
},
{ 
  "FF: International Example Program\n",
  "\nFF: Enter query to send or type quit: ",
  "\nFF: All done processing rows.\n",
  "\nFF: SQL command failed. Check the Server messages for possible errors.\n",
},
};

CS_INT	Ex_language = M_ENGLISH;

#define MSG(X)		 Ex_messages[Ex_language][(X)]

/*
** Prototypes for routines in sample code
*/ 
CS_STATIC CS_RETCODE CS_INTERNAL SetupLocale(CS_CONTEXT *context);
CS_STATIC CS_RETCODE CS_INTERNAL DoCommand(CS_CONNECTION *connection,
	CS_BOOL *p_domore);
CS_STATIC CS_RETCODE CS_INTERNAL FetchData(CS_COMMAND *cmd);
CS_STATIC CS_RETCODE CS_INTERNAL MyMoneyToChar(CS_CONTEXT *context,
	CS_DATAFMT *srcfmt, CS_VOID *srcdata, CS_DATAFMT *destfmt,
	CS_VOID *destdata, CS_INT *destlen);
CS_STATIC CS_INT CS_INTERNAL LookupLanguage(CS_CHAR *lang);


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
	CS_BOOL		anothercmd = CS_TRUE;
	
	/* 
	** Allocate a context structure and initialize Client-Library 
	*/
	retcode = ex_init(&Ex_context);
	if (retcode != CS_SUCCEED)
	{
		ex_panic("ex_init failed");
	}

	/*
	** Setup up our locale for desired language and charset
	*/
	retcode = SetupLocale(Ex_context);

	fprintf(stdout, "%s", MSG(MSG_STARTUP));
	fflush(stdout);

	/* 
	** Allocate a connection structure, set its properties, and
	** establish a connection. Since the locale has already been
	** defined in the context structure, the connection will inherit
	** the context localization.
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = ex_connect(Ex_context, &connection, Ex_appname,
				Ex_username, Ex_password, Ex_server);
	}

	/*
	** Use the pubs2 database
	*/
	if (retcode == CS_SUCCEED)
	{
		if ((retcode = ex_use_db(connection, Ex_dbname)) != CS_SUCCEED)
		{
			ex_error("ex_use_db(pubs2) failed");
		}
	}

	/*
	** In a loop, execute the routines for the international example
	** program until anothercmd is set to CS_FALSE or a failure was
	** detected.
	*/
	while (retcode == CS_SUCCEED && anothercmd == CS_TRUE)
	{
		retcode = DoCommand(connection, &anothercmd);
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

/*
** SetupLocale(context)
**
** Type of function:
** 	International example program internal api
**
** Purpose:
**	This function setups up locale related information for the
**	example program. This includes configuring the language, and
**	installing custom conversion routines.
**	
** Parameters:
**	context - Pointer to context structure
**
** Return:
**	CS_SUCCEED	If locale was setup up correctly.
** 	<retcode>	Result of the Client-Library function if a failure
**			was returned.
**
*/

CS_STATIC CS_RETCODE CS_INTERNAL
SetupLocale(CS_CONTEXT *context)
{
	CS_RETCODE	retcode;
	CS_LOCALE	*locale;
	CS_DATAFMT	tmpfmt;
	CS_CONV_FUNC	myfunc;
	CS_CHAR		locbuf[EX_MAXSTRINGLEN];

	/*
	** Get a locale handle.
	*/
	retcode = cs_loc_alloc(context, &locale);
	if (retcode != CS_SUCCEED)
	{
		ex_error("SetupLocale: cs_loc_alloc() failed");
		return retcode;
	}

#if EX_USE_LOCALE
	/*
	** Set locale to the language/charset that we want
	*/
	retcode = cs_locale(context, CS_SET, locale, CS_LC_ALL, EX_LOCALE, 
				CS_NULLTERM, NULL);
	if (retcode != CS_SUCCEED)
	{
		ex_error("SetupLocale: cs_locale(CS_SET) failed");
		return retcode;
	}

	/*
	** set context locale to the one we just defined
	*/
	retcode = cs_config(context, CS_SET, CS_LOC_PROP, locale,
			CS_UNUSED, NULL);
	if (retcode != CS_SUCCEED)
	{
		ex_error("SetupLocale: cs_config(CS_LOC_PROP) failed");
		return retcode;
	}
#endif /* EX_USE_LOCALE */

	/*
	** In order for our application program to work with the
	** multitude of locales that could be defined, we will get the
	** SAP representation of the language and standardize on that.
	*/
	retcode = cs_locale(context, CS_GET, locale, CS_SYB_LANG,
				locbuf, CS_SIZEOF(locbuf), NULL);
	if (retcode != CS_SUCCEED)
	{
		ex_error("SetupLocale: cs_locale(CS_GET) failed");
		return retcode;
	}

	/*
	** Based on locale name, Set up global messages
	*/
	Ex_language = LookupLanguage(locbuf);
	if (Ex_language == M_UNKNOWN)
	{
		fprintf(stdout,
		    "SetupLocale: Unknown language '%s', using English\n",
		    locbuf);
		fflush(stdout);
		Ex_language = M_ENGLISH;
	}

	/*
	** Install a custom conversion routine that we can use whenever
	** we want to display money values. cs_set_convert() can also be
	** used to override SAP conversion routines (e.g. the SAP
	** CS_MONEY_TYPE to CS_CHAR_TYPE conversion routine could be
	** replaced with MyMoneyToChar()). Since cs_set_convert()
	** requires a pointer to a pointer to a function, we use the temp
	** variable "myfunc" to hold the location of the routine.
	*/
	myfunc = MyMoneyToChar;
	retcode = cs_set_convert(context, CS_SET, CS_MONEY_TYPE,
				MY_MONEY_CHAR_TYPE, (CS_VOID *)&myfunc); 
	if (retcode != CS_SUCCEED)
	{
		ex_error("SetupLocale: cs_set_convert() failed");
		return retcode;
	}

	/*
	** When a NULL value is received from the server, the conversion
	** routine is not called. Instead, a default null value is placed
	** in the destination buffer. In our case, we will display the
	** string "NULL" when this occurs.
	*/
	memset(&tmpfmt, 0, sizeof(tmpfmt));
	tmpfmt.datatype = MY_MONEY_CHAR_TYPE;
	retcode = cs_setnull(context, &tmpfmt, "NULL", strlen("NULL"));
	if (retcode != CS_SUCCEED)
	{
		ex_error("SetupLocale: cs_set_convert() failed");
		return retcode;
	}

	/*
	** drop locale handle.
	*/
	retcode = cs_loc_drop(context, locale);
	if (retcode != CS_SUCCEED)
	{
		ex_error("SetupLocale: cs_loc_drop() failed");
		return retcode;
	}

	return retcode;
}

/*
** DoCommand(connection, p_domore)
**
** Type of function:
** 	International example program internal api
**
** Purpose:
**	This function is the core of the international example. It sends
**	a sql statement to the server. It then processes the results in
**	the standard ct_results() loop, calling FetchData() when the
**	type of result is "fetchable".
**	
** Parameters:
**	connection	- Pointer to connection structure
**	p_domore	- Pointer to a bool which is set to CS_TRUE if
**			  another command should be issued.
**
** Return:
**	CS_SUCCEED	If the result set was processed correctly.
** 	<retcode>	Result of the Client-Library function if a failure
**			was returned.
*/
CS_STATIC CS_RETCODE CS_INTERNAL
DoCommand(CS_CONNECTION *connection, CS_BOOL *p_domore)
{

	CS_RETCODE	retcode;
	CS_COMMAND	*cmd;
	CS_INT		res_type;	/* result type from ct_results */
	CS_CHAR		buf[MAX_CHAR_BUF];

	/*
	** Allocate a command handle to send the query with
	*/
	if ((retcode = ct_cmd_alloc(connection, &cmd)) != CS_SUCCEED)
	{
		ex_error("DoCommand: ct_cmd_alloc() failed");
		return retcode;
	}

	/*
	** Define a language command that contains a sql statement to
	** execute. EX_SELECT is a select statment which may be defined
	** above in this file. This routine will set p_domore to CS_FALSE
	** if a select was defined, or the user wants to exit the
	** application.
	*/
#if EX_USE_SELECT
	strcpy(buf, EX_SELECT);
	*p_domore = CS_FALSE;
#else
	fprintf(stdout, "%s", MSG(MSG_QUERY));
	fflush(stdout);
	fgets(buf, sizeof (buf), stdin);
	*p_domore = CS_TRUE;
#endif /* EX_USE_SELECT */
	if (strncmp(buf, "quit", 4) == 0)
	{
		*p_domore = CS_FALSE;
		return CS_SUCCEED;
	}


	retcode = ct_command(cmd, CS_LANG_CMD, buf, CS_NULLTERM, CS_UNUSED);
	if (retcode != CS_SUCCEED)
	{
		ex_error("DoCommand: ct_command() failed");
		return retcode;
	}

	/*
	** Send the command to the server 
	*/
	if (ct_send(cmd) != CS_SUCCEED)
	{
		ex_error("DoCommand: ct_send() failed");
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
			break;

		  case CS_CMD_DONE:
			/*
			** This means we're done with one result set.
			*/
			break;

		  case CS_CMD_FAIL:
			/*
			** This means that the server encountered an error
			** while processing our command.
			*/
			fprintf(stdout, "%s", MSG(MSG_CMD_FAIL));
			fflush(stdout);
			break;

		  case CS_ROW_RESULT:
		  case CS_PARAM_RESULT:
		  case CS_STATUS_RESULT:
		  case CS_COMPUTE_RESULT:
			retcode = FetchData(cmd);
			if (retcode != CS_SUCCEED)
			{
				ex_error("DoCommand: FetchData() failed");
				return retcode;
			}
			break;

		  default:
			/*
			** We got an unexpected result type.
			*/
			ex_error("DoCommand: ct_results() returned unexpected result type");
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
		ex_error("DoCommand: ct_results() failed");
		return retcode;

	  default:
		/*
		** We got an unexpected return value.
		*/
		ex_error("DoCommand: ct_results() returned unexpected result code");
		return retcode;
	}

	/*
	** Drop our command structure
	*/
	if ((retcode = ct_cmd_drop(cmd)) != CS_SUCCEED)
	{
		ex_error("DoCommand: ct_cmd_drop() failed");
		return retcode;
	}

	return retcode;
}

/*
** FetchData()
**
** Type of function:
** 	International example program internal api
** 
** Purpose:
** 	This function processes fetchable results sets. The results include:
**
**		CS_ROW_RESULT
**		CS_CURSOR_RESULT 
**		CS_PARAM_RESULT 
**		CS_STATUS_RESULT 
**		CS_COMPUTE_RESULT 
**
**	Since the Client-Library result model has been unified, the same
**	apis are used for each of the above result types.
**
**	One caveat is the processing of CS_COMPUTE_RESULTs. The name field
**	sent from the server is typically zero length. To display a meaningful
**	header, the aggregate compute operator name should be found for the
**	column, and that name used instead. The compute example program has
**	code which demonstrates this.
**
**	This routine is essentially the same as the utility function
**	ex_fetch_data(), with modifications to support MY_MONEY_CHAR_TYPE
**	datatypes. If the user installed function had replaced the SAP
**	money to char conversion routine, this function would not be
**	needed.
**
** Parameters:
**	cmd - Pointer to command structure
**
** Return:
**	CS_MEM_ERROR	If an memory allocation failed.
**	CS_SUCCEED	If the data was displayed.
**	CS_FAIL		If no columns were present.
** 	<retcode>	Result of the Client-Library function if a failure was
**			returned.
**
*/
CS_STATIC CS_RETCODE CS_INTERNAL
FetchData(CS_COMMAND *cmd)
{
	CS_RETCODE		retcode;
	CS_INT			num_cols;
	CS_INT			i;
	CS_INT			j;
	CS_INT			row_count = 0;
	CS_INT			rows_read;
	CS_INT			disp_len;
	CS_DATAFMT		*datafmt;
	EX_COLUMN_DATA		*coldata;

	/*
	** Find out how many columns there are in this result set.
	*/
	retcode = ct_res_info(cmd, CS_NUMDATA, &num_cols, CS_UNUSED, NULL);
	if (retcode != CS_SUCCEED)
	{
		ex_error("FetchData: ct_res_info() failed");
		return retcode;
	}

	/*
	** Make sure we have at least one column
	*/
	if (num_cols <= 0)
	{
		ex_error("FetchData: ct_res_info() returned zero columns");
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
		ex_error("FetchData: malloc() failed");
		return CS_MEM_ERROR;
	}

	datafmt = (CS_DATAFMT *)malloc(num_cols * sizeof (CS_DATAFMT));
	if (datafmt == NULL)
	{
		ex_error("FetchData: malloc() failed");
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
			ex_error("FetchData: ct_describe() failed");
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
		
		/*
		** Set datatype and format to tell bind we want things
		** converted to null terminated strings
		*/
		if (datafmt[i].datatype == CS_MONEY_TYPE)
		{
			datafmt[i].maxlength = 30;
			datafmt[i].datatype = MY_MONEY_CHAR_TYPE;
			datafmt[i].format   = CS_FMT_NULLTERM;
		}
		else
		{
			datafmt[i].maxlength = ex_display_dlen(&datafmt[i]) + 1;
			datafmt[i].datatype = CS_CHAR_TYPE;
			datafmt[i].format   = CS_FMT_NULLTERM;
		}

		/*
		** Allocate memory for the column string
		*/
		coldata[i].value = (CS_CHAR *)malloc(datafmt[i].maxlength);
		if (coldata[i].value == NULL)
		{
			ex_error("FetchData: malloc() failed");
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
			ex_error("FetchData: ct_bind() failed");
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
	** Fetch the rows.  Loop while ct_fetch() returns CS_SUCCEED or 
	** CS_ROW_FAIL
	*/
	while (((retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED,
			&rows_read)) == CS_SUCCEED) || (retcode == CS_ROW_FAIL))
	{
		/*
		** Increment our row count by the number of rows just fetched.
		*/
		row_count = row_count + rows_read;

		/*
		** Check if we hit a recoverable error.
		*/
		if (retcode == CS_ROW_FAIL)
		{
			fprintf(stdout, "Error on row %d.\n", row_count);
			fflush(stdout);
		}

/*
** NOTE: There is a bug with cs_setnull() and user defined types (btsid
**	 45825) To work around this bug, code has been added to display
**	 a value of "NULL" whenever the column indicator is set to
**	 CS_NULLDATA. Since this example should demonstrate the
**	 cs_setnull() functionality that is available, the code has been
**	 conditionally #if'd with the following define. Once the bug has
**	 been fixed, this example should be updated.
*/
#define EX_BTSID_45825	1

		/*
		** We have a row.  Loop through the columns displaying the
		** column values.
		*/
		for (i = 0; i < num_cols; i++)
		{
			/*
			** Display the column value.
			**
			** NOTE: See EX_BTSID_45825 note above
			*/
#if EX_BTSID_45825
			if (coldata[i].indicator == CS_NULLDATA)
			{
				fprintf(stdout, "NULL");
				disp_len = datafmt[i].maxlength;
				disp_len -= strlen("NULL");
			}
			else
			{
				fprintf(stdout, "%s", coldata[i].value);
				disp_len = datafmt[i].maxlength;
				disp_len -= coldata[i].valuelen - 1;
			}
			fflush(stdout);

			/*
			** If not last column, Print out spaces between this
			** column and next one. 
			*/
			if (i != num_cols - 1)
			{
				for (j = 0; j < disp_len; j++)
				{
					fputc(' ', stdout);
				}
			}
#else /* EX_BTSID_45825 */
			fprintf(stdout, "%s", coldata[i].value);
			fflush(stdout);

			/*
			** If not last column, Print out spaces between this
			** column and next one. 
			*/
			if (i != num_cols - 1)
			{
				disp_len = ex_display_dlen(&datafmt[i]);
				disp_len -= coldata[i].valuelen - 1;
				for (j = 0; j < disp_len; j++)
				{
					fputc(' ', stdout);
				}
			}
#endif /* EX_BTSID_45825 */
		} 
		fprintf(stdout, "\n");
		fflush(stdout);
	}

	/*
	** Free allocated space.
	*/
	for (i = 0; i < num_cols; i++)
	{
		free(coldata[i].value);
	}
	free(coldata);
	free(datafmt);

	/*
	** We're done processing rows.  Let's check the final return
	** value of ct_fetch().
	*/
	switch ((int)retcode)
	{
	  case CS_END_DATA:
		/*
		** Everything went fine.
		*/
		fprintf(stdout, "%s", MSG(MSG_ROW_DONE));
		fflush(stdout);
		retcode = CS_SUCCEED;
		break;

	  case CS_FAIL:
		/*
		** Something terrible happened.
		*/
		ex_error("FetchData: ct_fetch() failed");
		break;

	  default:
		/*
		** We got an unexpected return value.
		*/
		ex_error("FetchData: ct_fetch() returned an expected retcode");
		break;

	}
	return retcode;
}

/*
** MyMoneyToChar()
**
** Type of function:
** 	Installed conversion function for international example program
**
** Purpose:
**	This function does a crude attempt at supporting multiple monetary
**	formats. 
**	
**	Note that this routine may be called from interrupt level. This
**	means that the any code within this routine must be interrupt safe.
**
** Parameters:
**	context - Pointer to context structure
**	srcfmt	- Pointer to source datafmt structure.
**	srcdata	- Pointer to source data buffer.
**	destfmt	- Pointer to destination datafmt structure.
**	destdata - Pointer to destination data buffer.
**	destlen	- Pointer to length variable (might be NULL).
**
** Return:
**	CS_SUCCEED	If conversion was successful.
**	CS_EOVERFLOW	If destination buffer is too small.
** 	<retcode>	Result of the Client-Library function if a failure
**			was returned.
**
*/

CS_STATIC CS_RETCODE CS_INTERNAL
MyMoneyToChar(CS_CONTEXT *context, CS_DATAFMT *srcfmt, CS_VOID *srcdata, 
	      CS_DATAFMT *destfmt, CS_VOID *destdata, CS_INT *destlen)
{
	CS_RETCODE	retcode;
	CS_RETCODE	convcode;
	CS_DATAFMT	tmpfmt;
	CS_FLOAT	flt;
	CS_CHAR		locbuf[EX_MAXSTRINGLEN];
	CS_CHAR		tmpbuf[EX_MAXSTRINGLEN];

	/*
	** First, convert it to float. Be careful when calling conversion
	** routines from within a conversion routine, since recursion
	** might occur (e.g. float->int calling float->char and
	** float->char calling float->int would spin forever).
	*/
	memset(&tmpfmt, 0, sizeof(tmpfmt));
	tmpfmt.datatype = CS_FLOAT_TYPE;
	retcode = cs_convert(context, srcfmt, srcdata, &tmpfmt, &flt, NULL);
	if (retcode != CS_SUCCEED)
	{
		ex_error("MyMoneyToChar: cs_convert() failed");
		return retcode;
	}

	/*
	** Get the language defined for the destination locale. Using
	** this approach allows multiple locales to be supported within
	** one application.
	**
	** Another approach that can be used is to store multiple
	** monetary values within one table, differentiating them with a
	** unique usertype field in the database. They can then be
	** displayed based on the usertype. To determine the usertype for
	** the column in the database, look at the srcfmt "usertype"
	** field.
	*/
	retcode = cs_locale(context, CS_GET, destfmt->locale, CS_SYB_LANG,
				locbuf, CS_SIZEOF(locbuf), NULL);
	if (retcode != CS_SUCCEED)
	{
		ex_error("MyMoneyToChar: cs_locale(CS_GET) failed");
		return retcode;
	}

	/*
	** Based on SAP language name, do conversion.
	*/
	switch ((int)LookupLanguage(locbuf))
	{
	  case M_FRENCH:
		sprintf(tmpbuf, "FF %-d,%02d", (int)flt,
			(int)((flt - (int)flt + .005) * 100));
		break;

	  case M_GERMAN:
		sprintf(tmpbuf, "DM %-d,%02d", (int)flt,
			(int)((flt - (int)flt + .005) * 100));
		break;

	  case M_ENGLISH:
		sprintf(tmpbuf, "$ %-.2f", flt);
		break;

	  case M_UNKNOWN:
		sprintf(tmpbuf, "?? %-.2f", flt);
		break;
	}

	/*
	** Fill in dest if there is room. Note that the destlen value
	** should contain all the data written to the buffer, including
	** the null terminator.
	*/
	if (destfmt->maxlength > (CS_INT)strlen(tmpbuf))
	{
		strcpy(destdata, tmpbuf);
		if (destlen != NULL)
		{
			*destlen = strlen(tmpbuf) + 1;
		}
		convcode = CS_SUCCEED;
	}
	else
	{
		convcode = CS_EOVERFLOW;
	}

	return convcode;
}

/*
** LookupLanguage(lang)
**
** Type of function:
** 	International example program internal api
**
** Purpose:
**	This function will return a language define based on the
**	character string passed in. It assumes the input string is a
**	SAP language string found in locales.dat.
**	
** Parameters:
**	lang	- Pointer to string
**
** Return:
**	M_ENGLISH	- If english is to be used
**	M_GERMAN	- If german is to be used
**	M_FRENCH	- If french is to be used
**	M_UNKNOWN	- If language is not known
*/

CS_STATIC CS_INT CS_INTERNAL
LookupLanguage(CS_CHAR *lang)
{
	CS_INT		retlang;

	if (strcmp(lang, "french") == 0)
	{
		retlang = M_FRENCH;
	}
	else if (strcmp(lang, "german") == 0)
	{
		retlang = M_GERMAN;
	}
	else if (strcmp(lang, "us_english") == 0)
	{
		retlang = M_ENGLISH;
	}
	else
	{
		retlang = M_UNKNOWN;
	}

	return retlang;
}

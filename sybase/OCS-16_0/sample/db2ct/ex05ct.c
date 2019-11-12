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
** 	ex05ct.c is converted from DB-Library's example5.c to use 
**	Client-Library and CS-Library.
**
** 	This example illustrates conversion of dbconvert() calls to 
** 	cs_convert() calls.  It converts a number of constants to strings, 
** 	a number of strings to numerical or binary quantities, and a number 
** 	of numerical quantities to other numerical types.
**
** 	Where DB-Library code has been replaced with Client-Library code,
** 	the DB-Library code is commented out and preceded with "--".
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*
-- #include <sybfront.h>
-- #include <sybdb.h>
*/
#include <ctpublic.h>

/*
-- #include "sybdbex.h"
*/
#include "dbtoctex.h"
#include "dbctutil.h"

/*
** dbctutil.c provides some useful utility routines for converting 
** DB-Library code. We use the dbct_datename() function in this
** program.
*/
#include "dbctutil.h"

#define ARRAY_LEN	20

/*
** Prototype the CS-Library error handler used by this example.
** Applications which use CS-Library should install a CS-Library
** error handler. CS-Library errors are NOT reported through the
** Client-Library error handler.
*/
CS_RETCODE CS_PUBLIC cslib_err_handler    PROTOTYPE((
	CS_CONTEXT    *cp,
	CS_CLIENTMSG  *msg
	));

int
main(int argc, char *argv[])
{
	/* These variables will hold the results of data conversions. */

	/*
	-- static DBBINARY   my_binary_array[ARRAY_LEN]
	--                    = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
	-- DBFLT8            my_flt8;
	-- DBINT             my_int4;
	-- DBMONEY           my_money;
	-- DBCHAR            my_string[ARRAY_LEN];
	-- DBDATETIME        my_date;
	*/

	static CS_BINARY	my_binary_array[ARRAY_LEN] = {0x12, 0x34, 0x56,
							      0x78, 0x9a, 0xbc,
							      0xde, 0xf0};
	CS_FLOAT	my_flt8;
	CS_INT		my_int4;
	CS_MONEY	my_money;
	CS_CHAR		my_string[ARRAY_LEN * 2 + 2];
	CS_DATETIME	my_date;
	CS_RETCODE	ret;
	CS_CONTEXT	*context = (CS_CONTEXT *) NULL;
	CS_DATAFMT	srcfmt;
	CS_DATAFMT	destfmt;
	CS_INT		outlen;
	int		i;

	/*
	** Setup screen output.
	*/
	EX_SCREEN_INIT();

	/*
	--  * Initialize DB-Library. *
	-- if (dbinit() == FAIL)
	-- {
	--   exit(ERREXIT);
	-- }
	*/

	/*
	** Initialize CS-Library and get a CS_CONTEXT.
	*/
	ret = cs_ctx_alloc(CS_VERSION_100, &context);
	EXIT_ON_FAIL(NULL, ret, "Context allocation failed.");

	/*
	--  * Install the user-supplied error-handling and message-handling
	--  * routines. They are defined at the bottom of this source file.
	--  *
	-- dberrhandle((EHANDLEFUNC)err_handler);
	-- dbmsghandle((MHANDLEFUNC)msg_handler);
	*/

	/* 
	** Install a CS-Library error handler. Since we are not making any
	** Client-Library calls here, we don't call ct_init() or install
	** client- and server-message callbacks here.
	*/
	ret = cs_config(context, CS_SET, CS_MESSAGE_CB, 
			(CS_VOID *)cslib_err_handler, CS_UNUSED, NULL);
	EXIT_ON_FAIL(context, ret, "Could not install CS-Lib error handler.");

	/*
	** CS-Library and Client-Library have the concept of a "locale", 
	** which contains character set & language information for 
	** character data.
	**
	** All of the conversions will use the default locale that was
	** set up by cs_ctx_alloc().
	*/
	srcfmt.locale = (CS_LOCALE *) NULL;
	destfmt.locale = (CS_LOCALE *) NULL;

	/* 
	** Convert numerical and binary constants to strings.
	**
	** Convert the contents of my_binary_array (CS_BINARY) to 
	** character data.
	*/

	/*
	-- dbconvert((DBPROCESS *)NULL, SYBBINARY, my_binary_array, (DBINT)8,
	-- 	     SYBCHAR, (BYTE DBFAR *)my_string, (DBINT)-1);
	*/

	srcfmt.datatype = CS_BINARY_TYPE;
	srcfmt.maxlength = 8;

	destfmt.datatype = CS_CHAR_TYPE;
	destfmt.maxlength = CS_SIZEOF(my_string);
	destfmt.format = CS_FMT_NULLTERM;

	ret = cs_convert(context, &srcfmt, (CS_VOID *) my_binary_array,
			 &destfmt, (CS_VOID *) my_string, &outlen);
	EXIT_ON_FAIL(context, ret, "Conversion (bin to char)  failed.");

	printf("Binary constant 0x123456789abcdef0 converted to string "); 
	printf("\"%s\".\n\n", my_string);

	/*
	** Convert a float to a character string.
	*/

	my_flt8 = 55.555;

	/*
	-- dbconvert((DBPROCESS *)NULL, SYBFLT8, (BYTE *)&my_flt8, (DBINT)-1,
	--	     SYBCHAR, (BYTE *)my_string, (DBINT)-1);
	*/
	
	srcfmt.datatype = CS_FLOAT_TYPE;
	srcfmt.maxlength = CS_SIZEOF(my_flt8);
	
	destfmt.datatype = CS_CHAR_TYPE;
	destfmt.maxlength = CS_SIZEOF(my_string);
	destfmt.format = CS_FMT_NULLTERM;

	ret = cs_convert(context, &srcfmt, (CS_VOID *) &my_flt8,
			 &destfmt, (CS_VOID *) my_string, &outlen);
	EXIT_ON_FAIL(context, ret, "Conversion (float to char)  failed.");

	printf("Floating-pt constant 55.555 converted to string \"%s\".\n\n",
	       my_string);


	/* Convert string constants to numerical and binary quantities. */

	/*
	** Character to 4-byte integer.
	*/

	/*
	--  dbconvert((DBPROCESS *)NULL, SYBCHAR, (BYTE DBFAR * )"123", (DBINT)-1,
	--	      SYBINT4, (BYTE *)&my_int4, (DBINT)-1);
	*/

	strcpy(my_string, "123");

	srcfmt.datatype = CS_CHAR_TYPE;
	srcfmt.maxlength = strlen(my_string);

	destfmt.datatype = CS_INT_TYPE;
	destfmt.maxlength = CS_SIZEOF(my_int4);
	destfmt.format = CS_FMT_UNUSED;

	ret = cs_convert(context, &srcfmt, (CS_VOID *) my_string,
			 &destfmt, (CS_VOID *) &my_int4, &outlen);
	EXIT_ON_FAIL(context, ret, "Conversion (char to int)  failed.");

	printf("String constant \"123\" converted to 4-byte integer %ld.\n\n",
	       (long) my_int4);

	/*
	** Character to binary.
	*/

	/*
	-- dbconvert((DBPROCESS *)NULL, SYBCHAR, (BYTE DBFAR * )"0xfedc",
	--	     (DBINT)-1, SYBBINARY, my_binary_array, (DBINT)ARRAY_LEN);
	*/

	strcpy(my_string, "0xfedc");
	srcfmt.datatype = CS_CHAR_TYPE;
	srcfmt.maxlength = strlen(my_string);

	destfmt.datatype = CS_BINARY_TYPE;
	destfmt.maxlength = CS_SIZEOF(my_binary_array);
	destfmt.format = CS_FMT_UNUSED;

	ret = cs_convert(context, &srcfmt, (CS_VOID *) my_string,
			 &destfmt, (CS_VOID *) my_binary_array, &outlen);
	EXIT_ON_FAIL(context, ret, "Conversion (char to bin)  failed.");
	
	/*
	-- printf("String constant \"0xfedc\" converted to binary sequence ");
	-- printf("%x.\n\n", *((int *)my_binary_array));
	*/

	/*
	** This output does not print the same as DB-Library example, because
	** we don't use the same method to display the binary value. The 
	** DB-Library example cast the binary array to an (int *).
	*/
	printf("String constant \"0xfedc\" converted to binary sequence ");
	for (i = 0; i < outlen; i++)
	{
		printf("%2x", my_binary_array[i]);
	}
	printf(".\n\n");

	/*
	** Character to float.
	*/

	/*
	-- dbconvert((DBPROCESS *)NULL, SYBCHAR, (BYTE DBFAR * )"123.456",
	--	     (DBINT)-1, SYBFLT8, (BYTE *)&my_flt8, (DBINT)-1);
	*/

	strcpy(my_string, "123.456");
	srcfmt.datatype = CS_CHAR_TYPE;
	srcfmt.maxlength = strlen(my_string);

	destfmt.datatype = CS_FLOAT_TYPE;
	destfmt.maxlength = CS_SIZEOF(my_flt8);
	destfmt.format = CS_FMT_UNUSED;

	ret = cs_convert(context, &srcfmt, (CS_VOID *) my_string,
			 &destfmt, (CS_VOID *) &my_flt8, &outlen);
	EXIT_ON_FAIL(context, ret, "Conversion (char to float)  failed.");

	printf("String constant \"123.456\" converted to ");
	printf("floating-pt number %f.\n\n", my_flt8);


	/* Convert numerical types to other numerical types. */

	/*
	** Float to money (and money to char for display of output).
	*/

	my_flt8 = 98.76;

	/*
	-- dbconvert((DBPROCESS *)NULL, SYBFLT8, (BYTE *)&my_flt8, (DBINT)-1,
	--	     SYBMONEY, (BYTE *)&my_money, (DBINT)-1);
	-- dbconvert((DBPROCESS *)NULL, SYBMONEY, (BYTE *)&my_money,
	--	     (DBINT)-1, SYBCHAR, (BYTE *)my_string, (DBINT)-1);
	*/

	srcfmt.datatype = CS_FLOAT_TYPE;
	srcfmt.maxlength = CS_SIZEOF(my_flt8);

	destfmt.datatype = CS_MONEY_TYPE;
	destfmt.maxlength = CS_SIZEOF(my_money);
	destfmt.format = CS_UNUSED;

	ret = cs_convert(context, &srcfmt, (CS_VOID *) &my_flt8,
			 &destfmt, (CS_VOID *) &my_money, &outlen);
	EXIT_ON_FAIL(context, ret, "Conversion (float to money)  failed.");

	/*
	** Note reversal of srcfmt and destfmt for this one.
	*/
	srcfmt.datatype = CS_CHAR_TYPE;
	srcfmt.maxlength = CS_SIZEOF(my_string);
	srcfmt.format = CS_FMT_NULLTERM; 

	ret = cs_convert(context, &destfmt, (CS_VOID *) &my_money,
			 &srcfmt, (CS_VOID *) my_string, &outlen);
	EXIT_ON_FAIL(context, ret, "Conversion (money to char)  failed.");

	printf("floating-pt number %f converted to money value %s.\n\n",
	       my_flt8, my_string);


	/*
	** Money to float. (For display, use the character representation of
	** my_money that was set above).
	*/

	/*
	-- dbconvert((DBPROCESS *)NULL, SYBMONEY, (BYTE *)&my_money, (DBINT)-1,
	--	     SYBFLT8, (BYTE *)&my_flt8, (DBINT)-1);
	*/
	
	srcfmt.datatype = CS_MONEY_TYPE;
	srcfmt.maxlength = CS_SIZEOF(my_money);

	destfmt.datatype = CS_FLOAT_TYPE;
	destfmt.maxlength = CS_SIZEOF(my_flt8);
	destfmt.format = CS_FMT_UNUSED;

	ret = cs_convert(context, &srcfmt, (CS_VOID *) &my_money,
			 &destfmt, (CS_VOID *) &my_flt8, &outlen);
	EXIT_ON_FAIL(context, ret, "Conversion (money to float)  failed.");

	printf("money value %s converted to floating-pt value %f.\n\n",
	       my_string, my_flt8);


	/*
	**  Datetime conversions:  
	*/

	/*
	**  Many formats are acceptable for dates.  For a list
	**  of these formats, see the ASE Reference
	**  datatypes page.
	*/

	/*
	-- dbconvert((DBPROCESS *)NULL, SYBCHAR, (BYTE DBFAR *)"November 11, 1955", 
	--	     (DBINT)-1, SYBDATETIME, (BYTE *)&my_date , (DBINT)-1);
	-- dbconvert((DBPROCESS *)NULL, SYBMONEY, (BYTE *)&my_date, (DBINT)-1,
	--	     SYBCHAR, (BYTE *)my_string, (DBINT)-1);
	*/

	/*
	** DEBUG --
	** For some reason, the example5 program converted char to datetime, 
	** then cast the datetime to money and converted
	** to char. This program just goes char to date to char.
	*/

	strcpy(my_string, "November 11, 1955");
	srcfmt.datatype = CS_CHAR_TYPE;
	srcfmt.maxlength = strlen(my_string);
	
	destfmt.datatype = CS_DATETIME_TYPE;
	destfmt.maxlength = CS_SIZEOF(my_date);
	destfmt.format = CS_FMT_UNUSED;

	ret = cs_convert(context, &srcfmt, (CS_VOID *) my_string,
			 &destfmt, (CS_VOID *) &my_date, &outlen);
	EXIT_ON_FAIL(context, ret, "Conversion (char to datetime)  failed.");

	srcfmt.datatype = CS_DATETIME_TYPE;
	srcfmt.maxlength = CS_SIZEOF(my_date);
	
	destfmt.datatype = CS_CHAR_TYPE;
	destfmt.maxlength = CS_SIZEOF(my_string);
	destfmt.format = CS_FMT_NULLTERM;

	ret = cs_convert(context, &srcfmt, (CS_VOID *) &my_date,
			 &destfmt, (CS_VOID *) my_string, &outlen);
	EXIT_ON_FAIL(context, ret, "Conversion (datetime to char)  failed.");

	printf ("Internal date representation converted ");
	printf ("to string value %s.\n\n", my_string);

	/*
	-- dbdatename((DBPROCESS *)NULL, my_string, DBDATE_DY, &my_date);
	*/

	/*
	** dbct_datename() is defined in dbctutil.c It emulates everything
	** except DBDATE_WK.
	*/
	memset(&my_date, '\0', CS_SIZEOF(my_date));
	ret = dbct_datename(context, my_string, ARRAY_LEN, DBDATE_DY, &my_date);
	if (ret <= 0)
	{
		EXIT_ON_FAIL(context, CS_FAIL,
			     "dbct_datename(DBDATE_DY) failed.");
	}

	printf ("Day-of-year extracted from internal representation:"); 
	printf (" %s.\n\n", my_string);

	/*
	-- dbdatename((DBPROCESS *)NULL, my_string, DBDATE_DW, &my_date);
	*/

	memset(&my_date, '\0', CS_SIZEOF(my_date));
	ret = dbct_datename(context, my_string, ARRAY_LEN, DBDATE_DW, &my_date);
	if (ret <= 0)
	{
		EXIT_ON_FAIL(context, CS_FAIL,
			     "dbct_datename(DBDATE_DW) failed.");
	}

	printf ("Day-of-week extracted from internal representation:"); 
	printf (" %s.\n\n", my_string);

	memset(&my_date, '\0', CS_SIZEOF(my_date));
	ret = dbct_datename(context, my_string, ARRAY_LEN, DBDATE_MM, &my_date);
	if (ret <= 0)
	{
		EXIT_ON_FAIL(context, CS_FAIL,
			     "dbct_datename(DBDATE_DW) failed.");
	}

	printf ("Month-name extracted from internal representation:"); 
	printf (" %s.\n\n", my_string);

	memset(&my_date, '\0', CS_SIZEOF(my_date));
	ret = dbct_datename(context, my_string, ARRAY_LEN, DBDATE_YY, &my_date);
	if (ret <= 0)
	{
		EXIT_ON_FAIL(context, CS_FAIL,
			     "dbct_datename(DBDATE_YY) failed.");
	}

	printf ("Year extracted from internal representation:"); 
	printf (" %s.\n\n", my_string);

	/*
	** All done, so clean up. If we had initialized Client-Library above,
	** we would call ct_exit() here.
	*/

	/*
	dbexit();
	*/
 
	ret = cs_ctx_drop(context);
	EXIT_ON_FAIL(context, ret, "Context drop failed.");

	exit(NORMAL_EXIT);

} /* main */

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
CS_RETCODE CS_PUBLIC
cslib_err_handler(CS_CONTEXT *context, CS_CLIENTMSG *errmsg)
{
	/*
	** Print the error details.
	*/
	fprintf(ERR_CH, "CS-Library error: ");
	fprintf(ERR_CH, "LAYER = (%ld) ORIGIN = (%ld) ",
		(long) CS_LAYER(errmsg->msgnumber), 
		(long) CS_ORIGIN(errmsg->msgnumber) );
	fprintf(ERR_CH, "SEVERITY = (%ld) NUMBER = (%ld)\n",
		(long) CS_SEVERITY(errmsg->msgnumber), 
		(long) CS_NUMBER(errmsg->msgnumber) ); 
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
}

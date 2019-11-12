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
** 	ex03ct.c is converted from DB-Library example3.c to use 
**	Client-Library.
**
** 	This example selects some information from the "pubs2" database.
** 	It illustrates binding of both aggregate and compute results.
**
** 	Note that this example will only work if the "pubs2" database exists
** 	on your ASE, and the user has SELECT permission on its tables. 
**
** 	Where DB-Library code has been replaced with Client-Library code,
** 	the DB-Library code is commented out and preceded with "--".
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* 
** Client-Library header file is ctpublic.h
**
-- #include <sybfront.h>
-- #include <sybdb.h>
*/
#include <ctpublic.h>

/*
-- #include "sybdbex.h"
*/
#include "dbtoctex.h"

/*
** We use the routines in exutils.c to initialize Client-Library,
** install error handlers, open the connection, and perform 
** connection- and context-level cleanup. The handlers
** for server messages and Client-Library errors are defined
** in exutils.c
**
** For an example that shows conversion of these DB-Library
** operations, look at ctfirst.c
**
*/
#include <exutils.h>

#define PLEN 4 
#define DATEPRINT 26
#define MONEYPRINT 12

int
main(int argc, char *argv[])
{
	/* 
	** Declare the handles for the control structures we need.
	** Client-Library control structures are CS_CONTEXT, CS_CONNECTION,
	** and CS_COMMAND.
	**
	--  LOGINREC       *login;
	--  DBPROCESS      *dbproc;
	*/
	CS_CONTEXT	*context = (CS_CONTEXT *) NULL;
	CS_CONNECTION	*conn;
	CS_COMMAND	*cmd;
	CS_CHAR		sql_string[1024];

	/*
	** Declare the datatypes for the columns in the table "titles". 
	--  DBINT          pcount;   
	--  DBINT          sales;     
	--  DBINT          salesavg; 
	--  DBINT          sumsale;  
	--  DBCHAR         date[DATEPRINT+1];
	--  DBCHAR         price[MONEYPRINT+1];
	--  DBCHAR         priceavg[MONEYPRINT+1];
	--  DBCHAR         pubid[PLEN+1];
	*/

	CS_INT		pcount;
	CS_INT		sales;
	CS_INT		salesavg;
	CS_INT		sumsale;     
	CS_CHAR		date[DATEPRINT+1];
	CS_CHAR		price[MONEYPRINT+1]; 
	CS_CHAR		priceavg[MONEYPRINT+1];
	CS_CHAR		pubid[PLEN+1];
	CS_DATAFMT	char_datafmt;
	CS_DATAFMT	int_datafmt;
	CS_BOOL		reg_bound = CS_FALSE;

	/*
	--  RETCODE        result_code; * to hold the results of dbresults(). *
	--  STATUS         row_code;    * to hold the results of dbnextrow(). *
	*/
	CS_RETCODE	ret;         /* for return-code error checking */
	CS_RETCODE	results_ret; /* driver for ct_results() loop   */  
	CS_INT		result_type; /* ct_results() result type description */

	/*
	** Setup screen output.
	*/
	EX_SCREEN_INIT();

	/*
	-- * Initialize DB-Library. *
	-- if (dbinit() == FAIL)
	-- {
	--   exit(ERREXIT);
	-- }
	--
	-- * Install the user-supplied error-handling and message-handling
	-- * routines. They are defined at the bottom of this source file.
	-- *
	-- dberrhandle((EHANDLEFUNC)err_handler);
	-- dbmsghandle((MHANDLEFUNC)msg_handler);
	*/

	/*
	** Initialize CS-Library and Client-Library. We do this with
	** ex_init() from exutils.c
	** 
	** ex_init() allocates a context structure, initializes Client-Library,
	** and installs callbacks for the error and message handlers.
	**
	*/
	ret = ex_init(&context);
	EXIT_ON_FAIL(context, ret, "Client-Library setup failed!");
  
	/*
	-- * Set up the login information. *
	-- login = dblogin();
	-- DBSETLUSER(login, USER);
	-- DBSETLPWD(login, PASSWORD);
	-- DBSETLAPP(login, "example3");
	--
	-- dbproc = dbopen(login, NULL);
	*/

	/*
	** Connect to the server. We use ex_connect() from exutils.c to 
	** do this. ex_connect() allocates a connection structure, sets the 
	** username and password properties, and calls ct_connect().
	**
	*/
	ret = ex_connect(context, &conn, "ex03ct", USER, PASSWORD, NULL);
	EXIT_ON_FAIL(context, ret, "Connection attempt failed.");

	/*
	-- * Send a "use database" command. *
	--
	-- dbuse(dbproc,"pubs2");
	*/

	/*
	** Change to the pubs2 database. ex_use_db() is in exutils.c
	*/
	ret = ex_use_db(conn, "pubs2");
	EXIT_ON_FAIL(context, ret, "SQL 'USE pubs2' failed");

	/*
	** Allocate a CS_COMMAND structure for sending commands and
	** receiveing results.
	*/
	ret = ct_cmd_alloc(conn, &cmd);
	EXIT_ON_FAIL(context, ret, "Could not allocate command structure.");

	/*
	-- * Put the SQL statement into the command buffer. *
	--
	-- dbcmd(dbproc, "select pub_id, pubdate, price, avg(price), total_sales,");
	-- dbcmd(dbproc, " avg(total_sales), sum(total_sales) from titles");
	-- dbcmd(dbproc, " group by pub_id");
	-- dbcmd(dbproc, " order by pub_id");
	-- dbcmd(dbproc, " compute count(pub_id) by pub_id");
	--
	-- * Send the command buffer to ASE for execution. *
	--
	-- dbsqlexec(dbproc);
	*/

	/*
	** Build and send SELECT language command that has a compute clause.
	*/
	strcpy(sql_string,
	       "select pub_id,pubdate,price,avg(price),total_sales,");
	strcat(sql_string, " avg(total_sales), sum(total_sales) from titles");
	strcat(sql_string, " group by pub_id");
	strcat(sql_string, " order by pub_id");
	strcat(sql_string, " compute count(pub_id) by pub_id");

	ret = ct_command(cmd, CS_LANG_CMD, (CS_VOID *)sql_string, CS_NULLTERM,
			 CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "ct_command() failed.");

	ret = ct_send(cmd);
	EXIT_ON_FAIL(context, ret, "ct_send() failed.");

	/*
	--  *
	-- ** Using the aggregates "sum" and "avg" with the COMPUTE clause 
	-- ** necessitates special handling when binding the results. Since each
	-- ** aggregate creates a new column, this is accounted for in the bind.
	-- ** Notice that avg(price) is the fourth column in the select-list,
	-- ** and is also specified as the fourth column in the dbbind() routine
	-- **
	-- ** The COMPUTE clause creates a compute row, which requires a 
	-- ** special bind routine called dbaltbind().
	-- *
	--
	-- while ((result_code = dbresults(dbproc)) != NO_MORE_RESULTS)
	-- {
	--   if (result_code == SUCCEED)
	--   {
	--     dbbind(dbproc, 1, NTBSTRINGBIND, (DBINT)0,
	--				(BYTE DBFAR *)pubid);
	--     dbbind(dbproc, 2, NTBSTRINGBIND, (DBINT)0,
	--				(BYTE DBFAR *)date);
	--     dbbind(dbproc, 3, NTBSTRINGBIND, (DBINT)0,
	--				(BYTE DBFAR *)price);
	--     dbbind(dbproc, 4, NTBSTRINGBIND, (DBINT)0,
	--				(BYTE DBFAR *)priceavg);
	--     dbbind(dbproc, 5, INTBIND, (DBINT)0, (BYTE *)&sales);
	--     dbbind(dbproc, 6, INTBIND, (DBINT)0, (BYTE *)&salesavg);
	--     dbbind(dbproc, 7, INTBIND, (DBINT)0, (BYTE *)&sumsale);
	-- 
	--     * dbaltbind() binds compute columns. *
	--     dbaltbind(dbproc, 1, 1, INTBIND, (DBINT)0, (BYTE *)&pcount);
	-- 
	--     printf("\nAccounts:\n");
	--     printf("---------\n\n");
	--     printf
	--       ("%-5s  %-26s  %-6s  %-10s  %-5s  %-10s  %-10s\n\n",
	--        "pubid", "date", "price", "avg(price)",
	--        "sales", "avg(sales)", "sum(sales)");
	--      *
	--     ** Print out each result row, using different statements
	--     ** depending on whether the row is a regular row or a 
	--     ** compute row.
	--     *
	-- 
	--     while ((row_code = dbnextrow(dbproc)) != NO_MORE_ROWS)
	--     {
	--       if (row_code == REG_ROW)
	--       {
	--         printf
	--           ("%5s  %26s  %6s  %10s  %5ld  %10ld  %10ld\n",
	--            pubid, date, price, priceavg, sales, 
	--            salesavg, sumsale);
	--       }
	--       else
	--         printf("title count:  %ld\n\n",pcount); 
	--     }
	--   }
	-- }
	*/

	/*
	** Here's our results loop. It runs until ct_results() returns a 
	** value other than CS_SUCCEED. Each time it returns CS_SUCCEED,
	** ct_results() sets our result_type variable. If all goes well, 
	** we will see the following result_type values for the SQL batch 
	** that we sent:
	**   CS_ROW_RESULT:
	**      Indicates that we have a set of regular rows returned from the
	**      select to bind and fetch.
	**   CS_COMPUTE_RESULT:
	**      Indicates that we have a set of compute rows returned from the
	**      select to bind and fetch.
	**   CS_CMD_DONE:
	**      Indicates that we've finished the results for the select
	**      statement.
	**
	** If the SQL fails for some reason, say we don't have permission 
	** for what we selected, then we will see CS_CMD_FAIL.
	**
	** After loop exit, we check for errors indicated by the ct_results
	** return code. These will never indicate server-side errors.
	*/
	while ((results_ret = ct_results(cmd, &result_type)) == CS_SUCCEED)
	{
		switch ((int) result_type)
		{
		  case CS_CMD_FAIL:

			/* 
			** Command failed on server. In DB-Library,
			** the application learned this from dbsqlexec's
			** or dbresults' return code.
			*/
			fprintf(ERR_CH, "Command failed on server.\n");
			break;

		  case CS_CMD_SUCCEED:

			/* 
			** A command that returns no rows, or an EXEC command,
			** ran successfully on the server. In DB-Library,
			** the application learned this from dbsqlexec's
			** or dbresults' return code. In this program, no action
			** is required.
			*/
			break;

		  case CS_ROW_RESULT:

			/*
			** A set of regular rows -- this result_type is roughly
			** equivalent to calling dbnextrow() and seeing it
			** return REG_ROW. We bind to the columns and fetch
			** each one with ct_fetch(). 
			** 
			** Print out the column headers for the regular rows and
			** and bind to each column. Our query will return a
			** compute row each time the value of 'pub_id' changes
			** in the selected rows.
			**
			** The regular rows will be interrupted by the compute
			** rows, and we will see a new CS_ROW_RESULT
			** result_type after each compute row. To emulate the
			** DB-Library sample's output, we print only one title
			** line and only bind once. Our reg_bound flag is set
			** here and cleared when result_type is CS_CMD_DONE.
			*/
			if (reg_bound != CS_TRUE)
			{
				/* 
				** ct_bind requires a CS_DATAFMT structure which
				** describes the datatype and formatting
				** information for the destination variable.
				** We will use char_datafmt for character
				** destinations and int_datafmt for integer
				** destinations.
				** With character destinations, we need to
				** specify the length of the destination array
				** with char_datafmt.maxlength. We do this
				** before each ct_bind() call that binds to a
				** character destination.
				*/
				char_datafmt.datatype = CS_CHAR_TYPE;
				char_datafmt.format = CS_FMT_NULLTERM;
				char_datafmt.count = 1;
				char_datafmt.locale = (CS_LOCALE *) NULL;
				char_datafmt.scale = 0;
				char_datafmt.precision = 0;

				int_datafmt.datatype = CS_INT_TYPE;
				int_datafmt.format = CS_FMT_UNUSED;
				int_datafmt.maxlength = CS_UNUSED;
				int_datafmt.count = 1;
				int_datafmt.locale = (CS_LOCALE *) NULL;

				/* 
				** Bind pubid to the 'pub_id' column 
				*/
				char_datafmt.maxlength = PLEN + 1;
				ret = ct_bind(cmd, 1, &char_datafmt,
					      (CS_VOID *)pubid, (CS_INT *)NULL,
					      (CS_SMALLINT *) NULL);
				EXIT_ON_FAIL(context, ret, "bind for 'pub_id'");

				/* 
				** Bind date to the 'pubdate' column 
				*/
				char_datafmt.maxlength = DATEPRINT + 1;
				ret = ct_bind(cmd, 2, &char_datafmt,
					      (CS_VOID *)date, (CS_INT *)NULL,
					      (CS_SMALLINT *) NULL);
				EXIT_ON_FAIL(context, ret,
					     "bind for 'pubdate'");

				/* 
				** Bind price to the 'price' column 
				*/
				char_datafmt.maxlength = MONEYPRINT + 1;
				ret = ct_bind(cmd, 3, &char_datafmt,
					      (CS_VOID *)price, (CS_INT *)NULL,
					      (CS_SMALLINT *) NULL);
				EXIT_ON_FAIL(context, ret, "bind for 'price'");

				/* 
				** Bind priceavg to the 'avg(price)' column 
				*/
				char_datafmt.maxlength = MONEYPRINT + 1;
				ret = ct_bind(cmd, 4, &char_datafmt,
					      (CS_VOID *)priceavg,
					      (CS_INT *)NULL,
					      (CS_SMALLINT *) NULL);
				EXIT_ON_FAIL(context, ret,
					     "bind for 'avg(price)'");


				/* 
				** Bind sales to the 'total_sales' column.
				*/
				ret = ct_bind(cmd, 5, &int_datafmt,
					      (CS_VOID *)&sales,
					      (CS_INT *)NULL,
					      (CS_SMALLINT *) NULL);
				EXIT_ON_FAIL(context, ret,
					     "bind for 'total_sales'");

				/* 
				** Bind salesavg to the 'avg(total_sales)'
				** column 
				*/
				ret = ct_bind(cmd, 6, &int_datafmt,
					      (CS_VOID *)&salesavg,
					      (CS_INT *)NULL,
					      (CS_SMALLINT *) NULL);
				EXIT_ON_FAIL(context, ret,
					     "bind for 'avg(total_sales)'");

				/* 
				** Bind to the 'sum(total_sales)' column 
				*/
				ret = ct_bind(cmd, 7, &int_datafmt,
					      (CS_VOID *)&sumsale,
					      (CS_INT *)NULL,
					      (CS_SMALLINT *) NULL);
				EXIT_ON_FAIL(context, ret,
					     "bind for sum(total_sales)");

				/*
				** Print a title header line for the data.
				*/
				fprintf(OUT_CH, "\nAccounts:\n");
				fprintf(OUT_CH, "---------\n\n");
				fprintf(OUT_CH,
					"%-5s  %-26s  %-6s  %-10s  %-5s  %-10s  %-10s\n\n",
					"pubid", "date", "price", "avg(price)",
					"sales", "avg(sales)", "sum(sales)");

				reg_bound = CS_TRUE;
	
			} /* if (reg_bound != CS_TRUE) */
	   
			/*
			** Now retrieve each row into the bound variables
			** and print the data.
			*/

			while((ret = ct_fetch(cmd, CS_UNUSED, CS_UNUSED,
					      CS_UNUSED, (CS_INT *) NULL))
			      == CS_SUCCEED)
			{
				fprintf(OUT_CH, 
					"%5s  %26s  %6s  %10s  %5ld  %10ld  %10ld\n",
					pubid, date, price, priceavg,
					(long) sales, (long) salesavg,
					(long) sumsale);
			}

			/* 
			** Check for ct_fetch() errors 
			*/
			if (ret != CS_END_DATA)
			{
				EXIT_ON_FAIL(context, CS_FAIL,
					     "ct_fetch() failed");
			}

			break;

		  case CS_COMPUTE_RESULT:

			/*
			** We have one of the compute rows. This result_type is
			** equivalent to calling dbnextrow() and seeing it
			** return a compute-id > 0.
			**
			** In a situation where the query might have multiple
			** COMPUTE BY clauses, we would be interested in
			** getting the compute-id with the Client-Library
			** routine ct_compute_info(), so that we knew which
			** clause generated the compute row.
			** Here, we know there is just one COMPUTE BY clause,
			** so we just bind to the columns and fetch each one
			** with ct_fetch(). 
			**
			** In Client-Library, there is no seperate routine
			** for binding compute columns as in DB-Library's
			** dbaltbind().
			*/
	
			int_datafmt.datatype = CS_INT_TYPE;
			int_datafmt.format = CS_FMT_UNUSED;
			int_datafmt.maxlength = CS_UNUSED;
			int_datafmt.count = 1;
			int_datafmt.locale = (CS_LOCALE *) NULL;

			ret = ct_bind(cmd, 1, &int_datafmt, (CS_VOID *)&pcount, 
				      (CS_INT *)NULL, (CS_SMALLINT *) NULL);
			EXIT_ON_FAIL(context, ret, "bind for 'count(pub_id)'");
    
			fprintf(OUT_CH, "\n");
			while ((ret = ct_fetch(cmd, CS_UNUSED, CS_UNUSED,
					       CS_UNUSED, (CS_INT *) NULL))
			       == CS_SUCCEED)
			{
				fprintf(OUT_CH,
					"title count:  %ld\n\n", (long) pcount); 
			}

			/* 
			** Check for ct_fetch() errors.
			*/
			if (ret != CS_END_DATA)
			{
				EXIT_ON_FAIL(context, CS_FAIL,
					     "ct_fetch() failed");
			}

			break;

		  case CS_CMD_DONE:

			/* 
			** The results of a logical command have been completely
			** processed by this program. In this case, we've seen
			** all the regular and compute rows returned by our
			** SELECT statement. This result_type is equivalent to
			** the end of one iteration of the dbresults() loop
			** in DB-Library.
			** 
			** Here, we clear the flag that tells whether the
			** regular rows are bound.
			*/
			reg_bound = CS_FALSE;
			break;

		} /* switch */

	} /* while */
	
	/*
	** Now check for errors in the return code from ct_results.
	*/
	switch((int) results_ret)
	{
	  case CS_END_RESULTS: 
		/* No error. */
		break;

	  case CS_CANCELED:
		/* No error. */
		break;

	  case CS_FAIL:
		/* Network or other client-side error. */
		EXIT_ON_FAIL(context, CS_FAIL, "ct_results() failed.");

	  default:
		/* Unexpected ct_results() return code. */
		EXIT_ON_FAIL(context, CS_FAIL, "unknown ct_results() status");

	} /* switch */

	/*
	** Cleanup. ex_con_cleanup() and ex_ctx_cleanup() are defined
	** in exutils.c
	**
	-- dbexit();
	*/
	ret = ex_con_cleanup(conn, CS_SUCCEED);
	ret = ex_ctx_cleanup(context, ret);

	exit(NORMAL_EXIT);
}


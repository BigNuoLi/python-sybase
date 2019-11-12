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
**    	ex09ct.c is converted from DB-Library's example9.c to use
**	Client-Library.
**
**      Where DB-Library code has been replaced with Client-Library code,
**      the DB-Library code is commented out and preceded with '--'.
**
**      This example uses the au_pix table in the pubs2 database
**      to show the basic outline of how to use send text/image
**      data to a ASE using Client-Library. This example
**      sends the value as one monolithic "chunk". For an example
**      of code that does a buffered send, look at ex10ct.c
**
**      The format of the au_pix table is:
**        
**        Column          Datatype      Size      Nulls Allowed?
**        =============== ============= ========= ==============
**        au_id           char          11           NO
**        pic             image                      YES
**        format_type     char          11           YES
**        bytesize        int                        YES
**        pixwidth_hor    char          14           YES
**        pixwidth_vert   char          14           YES
**
*/

#include    <stdlib.h>
#include    <stdio.h>
#include    <string.h>

/*
** Client-Library header file is ctpublic.h. Header file for the 
** Client-Library migration examples is dbtoctex.h.
**
-- #include    <sybfront.h>
-- #include    <sybdb.h>
-- #include    "sybdbex.h"
*/
#include <ctpublic.h>
#include "dbtoctex.h"

#define     IMAGE_LENGTH         ((long)301)
#define     AU_ID                ((char*)"000-00-0000")

/*
** We use the routines in exutils.c to initialize Client-Library,
** install error handlers, open the connection, and perform 
** connection- and context-level cleanup. The handlers
** for server messages and Client-Library errors are defined
** in exutils.c
**
*/
#include <exutils.h>

/*
** txtimg.c contains utility routines for text and image handling in 
** Client-Library.
** 
** The txtimg.c routines used here are:
**
**   GetTextValue() -- Get the CS_IODESC structure required to
**     update a text/image column and, optionally, get the 
**     column's value.
**     
**     GetTextValue() is similar to selecting a text/image column
**     in DB-Library and calling dbtxptr(), dbtxtimestamp(), and
**     dbreadtext().
**
**   UpdateTextCol() -- Update a text or image column with
**     a new value. 
**
**     UpdateTextCol() is similar to using DB-Library's dbwritetext()
**     to update a text/image column with one call.
*/
#include "txtimg.h"

/* 
** This program installs it's own server message callback in place of
** the one in exutils.c
*/
CS_RETCODE CS_PUBLIC servermsg_cb PROTOTYPE ((
	CS_CONTEXT     *context,
	CS_CONNECTION  *connection,
	CS_SERVERMSG   *srvmsg
	));

int compare PROTOTYPE ((
	char *src1,
	char *src2,
	int length
	));

void getimage PROTOTYPE ((
	char *ptr
	));

int
main(int argc, char *argv[])
{
	/*
	-- DBPROCESS       *q_dbproc;  * query connection with ASE  *
	-- DBPROCESS       *u_dbproc;  * update connection with ASE *
	-- LOGINREC        *login;     * login information. *
	*/
	CS_CONTEXT	*context;
	CS_CONNECTION	*q_conn;
	CS_CONNECTION	*u_conn;

	/*
	-- RETCODE         result_code;
	*/
	CS_RETCODE	ret;
	
	/*
	** We gave compare() and getimage() full prototypes up above.
	**
	-- int             compare();
	-- void            getimage();
	*/

	/*
	** Here are the variables which will be used to store the
	** data being examined:
	*/
	
	/*  
	-- BYTE            *ret_p;            * to hold image pointer *
	-- DBBINARY        *txptr;            * text pointer *
	-- DBBINARY        *ts_ptr;           * time stamp *
	-- char            data[IMAGE_LENGTH];
	** 
	** data[] holds the image value to be inserted. data_ret[] holds the
	** the image value read from the server. iodesc is a CS_IODESC structure,
	** which contains fields for the text pointer and text timestamp.
	*/

	CS_BINARY	data[IMAGE_LENGTH];      
	CS_BINARY	data_ret[IMAGE_LENGTH];  
	CS_IODESC	iodesc;
	CS_CHAR		sql_string[512];
	CS_INT		intval;

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
	--  *
	-- **  Install the user-supplied error-handling and message-handling
	-- **  routines. They are defined at the bottom of this source file.
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
	*/
	ret = ex_init(&context);
	EXIT_ON_FAIL(context, ret, "ex_init() failed.");

	/*
	** The server message callback installed by ex_init() does not
	** ignore 57xx messages. In order to mimic the behavior of DB-Library's
	** example9, we need to install a handler that does not print
	** these messages.
	**
	** In Client-Library, all user callbacks are installed with
	** ct_callback(). servermsg_cb() is defined at the bottom of
	** this source file.
	*/
	ret = ct_callback(context, (CS_CONNECTION *)NULL, CS_SET, 
			  CS_SERVERMSG_CB, (CS_VOID *)servermsg_cb);
	EXIT_ON_FAIL(context, ret, "ct_callback() for server messages failed.");

	/*
	--  *
	-- **  Get a LOGINREC structure and fill it with the necessary
	-- **  login information.
	-- *
	-- login = dblogin();
	-- DBSETLUSER(login,USER);           * defined in sybdbex.h *
	-- DBSETLPWD(login,PASSWORD);
	-- DBSETLAPP(login, "Example 9");
	--
	-- q_dbproc = dbopen(login, NULL);
	-- u_dbproc = dbopen(login, NULL);
	*/

	/*
	** Connect to the server. We use ex_connect from exutils.c to 
	** do this. ex_connect() sets the login information (with
	** ct_con_props) and calls ct_connect().
	**
	** As with the DB-Library example, the default server name is taken
	** as the contents of the DSQUERY environment variable.
	*/
	ret = ex_connect(context, &q_conn, "ex09ct", USER, PASSWORD, NULL);
	EXIT_ON_FAIL(context, ret, "Connection attempt 1 failed.");
	ret = ex_connect(context, &u_conn, "ex09ct", USER, PASSWORD, NULL);
	EXIT_ON_FAIL(context, ret, "Connection attempt 2 failed.");

	/*
	--  * Send  ``use database'' commands: *
	-- dbuse(q_dbproc,"pubs2");
	-- dbuse(u_dbproc,"pubs2");
	*/

	/*
	** Use the ex_use_db() routine (from exutils.c) to place each connection
	** in the pubs2 database. 
	*/ 
	ret = ex_use_db(q_conn, "pubs2");
	EXIT_ON_FAIL(context, ret, "'USE pubs2' on query connection failed");
	ret = ex_use_db(u_conn, "pubs2");
	EXIT_ON_FAIL(context, ret, "'USE pubs2' on update connection failed");

	/* 
	**  Get the image for insertion
	*/
	getimage((char *)data);

	/*
	** Here are the text/image steps from the DB-Library program with their
	** Client-Library equivalents explained.
	**
	--  *
	-- **  The database column "pic" is an image column.  
	-- **  Example 9 must perform the following steps in
	-- **  order to insert data into this column:
	-- **
	-- **   1. INSERT all data into the row except the image.
	-- **   2. UPDATE the row to create a valid text pointer.
	**
	** Step 1, 2 (Client-Library): 
	** Same, but use Client-Library calls to send the language commands.
	**
	-- **   3. SELECT the row to pull the text pointer and
	-- **      text timestamp into the DBPROCESS.
	-- **   4. Call dbtxtptr() to retrieve the text pointer 
	-- **      from the DBPROCESS.
	-- **   5. Call dbtxtimestamp() to retrieve the text timestamp 
	-- **      from the DBPROCESS.
	**
	** Step 3, 4, and 5 (Client-Library):
	** Select the row, and when we see it in the results loop, call 
	** ct_get_data() on the text column, then call ct_data_info(CS_GET) to 
	** initialize our CS_IODESC structure for the update. 
	**
	** The CS_IODESC has fields for the text pointer, text timestamp,
	** and other items that are passed as arguments to DB-Library's
	** dbwritetext() routine.
	**
	-- **   6. Write the image value to ASE, using dbwritetext().
	-- * 
	**
	** Step 6 (Client-Library):
	** We call UpdateTextValue(), which is defined in txtimg.c. 
	** UpdateTextValue() accepts a CS_IODESC and a pointer to our text/image
	** data, then performs the following:
	** a. Calls ct_command() to initiate a send-data command.
	** b. Updates the log_on_update and total_txtlen fields
	**    in the CS_IODESC, then call ct_data_info(CS_SET). 
	** d. Writes the image to ASE using ct_send_data() and 
	**    ct_send().
	** e. Handles the results of the command, retrieves the new timestamp,
	**    and returns the new timestamp inside the CS_IODESC.
	*/

	/*
	** Step 1: Insert the row. We use the ex_execute_cmd() routine from
	** exutils.c. ex_execute_cmd() executes any language command that does
	** not return results (for example, inserts, updates,
	** create table, etc.)
	**
	--  *
	-- **  Step 1:  INSERT the data:
	-- *
	-- dbfcmd(q_dbproc, "insert au_pix (au_id) values ('%s')",AU_ID);
	-- dbsqlexec(q_dbproc);
	--
	--  *
	-- **  It is good programming practice to always call dbresults
	-- **  in a loop:
	-- *
	-- while (dbresults(q_dbproc) != NO_MORE_RESULTS)
	-- {
	--  continue;
	-- }
	*/
	sprintf(sql_string, "if not exists \
				(select * from au_pix where au_id = '%s') \
			     begin \
				insert au_pix (au_id) values ('%s') \
			     end", AU_ID, AU_ID);
	ret = ex_execute_cmd(q_conn, sql_string);
	EXIT_ON_FAIL(context, ret, "INSERT failed.");

	/*
	** Step 2:  UPDATE the row:
	**
	** We use ex_execute_cmd() for this also. 
	**
	-- dbfcmd(q_dbproc, "update au_pix set pic = null \
	--                   where au_id = '%s'",AU_ID);
	-- dbsqlexec(q_dbproc);
	--
	--  *
	-- **  It is good programming practice to call always call dbresults 
	-- **  in a loop.
	-- *
	-- while (dbresults(q_dbproc) != NO_MORE_RESULTS)
	-- {
	--   continue; 
	-- }
	*/
	sprintf(sql_string,
		"update au_pix set pic = null where au_id = '%s'", AU_ID);
	ret = ex_execute_cmd(q_conn, sql_string);
	EXIT_ON_FAIL(context, ret, "UPDATE failed.");

	/*
	** Steps 3, 4, 5 for DB-Library.
	**
	--  *
	-- ** Step 3: SELECT the row.
	-- * 
	-- dbfcmd(q_dbproc, "select pic from au_pix \
	--                   where au_id = '%s'",AU_ID);
	-- dbsqlexec(q_dbproc);
	--  *
	-- ** It is good programming practice to always call dbresults 
	-- ** in a loop.
	-- *
	-- while ((result_code = dbresults(q_dbproc)) != NO_MORE_RESULTS)
	-- {
	--     if (result_code == SUCCEED) 
	--     {
	--          * One row is returned but we are not interested in it: *
	--         dbcanquery(q_dbproc);
	--     }
	-- }
	--
	--  *
	-- **  Step 4:  dbtxtptr():
	-- *
	--
	--  * Get the text pointer: *
	-- txptr = dbtxptr(q_dbproc, 1);
	-- 
	--  *
	-- **  Step 5:  dbtxtimestamp():
	-- *
	--
	-- * Get a pointer to the text timestamp *
	-- ts_ptr = dbtxtimestamp(q_dbproc, 1);
	*/

	/*
	**  Step 3, 4, 5 (Client-Library):  Select the row and set up the 
	**    I0-Descriptor for updating the text column. The function 
	**    GetTextValue() from txtimg.c does this.
	**
	** GetTextValue() takes as input a SQL command to select the 
	** text/image column of interest. 
	*/
	sprintf(sql_string, "select pic from au_pix where au_id = '%s'",
		AU_ID);

	/*
	** GetTextValue takes pointers to a CS_IODESC and (optionally)
	** to a buffer to receive the value. We don't care about the
	** value, so we pass the buffer (5th) and buffer-length (6th)
	** arguments as NULL and 0, respectively.
	*/
	ret = GetTextValue(q_conn, &iodesc, sql_string, 1, (CS_VOID *) NULL, 
			   0, (CS_INT *) NULL);
	EXIT_ON_FAIL(context, ret, "GetTextValue() failed.");

	/*
	--  *
	-- **  Step 6:  dbwritetext():
	-- *
	--
	--  * Write the image: *
	-- dbwritetext (u_dbproc, "au_pix.pic", txptr, DBTXPLEN, 
	--              ts_ptr, TRUE, IMAGE_LENGTH, data);
	*/

	/*
	** Step 6 (Client-Library): Write the image with UpdateTextValue() 
	**   from txtimg.c
	**
	** We use the IO-Descriptor obtained by GetTextValue() for 
	** the update. But we must set the log_on_update fields and
	** total_txtlen fields ourselves. 
	**   log_on_update is equivalent to dbwritetext's 5th argument ('log')
	**   total_txtlen is equivalent to dbwritetext's 6th argument ('size')
	*/
	iodesc.log_on_update = CS_TRUE;
	iodesc.total_txtlen = IMAGE_LENGTH;
	ret = UpdateTextCol(u_conn, &iodesc, data, IMAGE_LENGTH);
	EXIT_ON_FAIL(context, ret, "UpdateTextCol() failed.");

	/*
	** Now, we want to retrieve the updated column and compare it
	** to the value we sent in the update. We use GetTextValue() 
	** to retrieve the value.
	** 
	--  *
	-- **  Image data has been inserted in to the server
	-- **  and we are about to select it out for compare
	-- **  to see if it got there OK.
	-- *
	-- 
	-- dbfcmd(q_dbproc, "select pic from au_pix \
	--        where au_id = '%s'",AU_ID);
	-- dbsqlexec(q_dbproc);
	--
	--  *
	-- ** It is good programming practice to call dbresults in a loop
	-- ** even if only a single set of results is expected:
	-- *
	--
	-- while ((result_code = dbresults(q_dbproc)) != NO_MORE_RESULTS)
	-- {
	--     if (result_code == SUCCEED) 
	--     {
	--         * We are expecting only a single row: *
	--         while ( dbnextrow(q_dbproc) != NO_MORE_ROWS)
	--         {
	--             ret_p = dbdata(q_dbproc,1);
	--         }
	--     }
	-- }
	*/

	sprintf(sql_string, "select pic from au_pix where au_id = '%s'",
		AU_ID);
	ret = GetTextValue(q_conn, &iodesc, sql_string, 1, (CS_VOID *) data_ret,
			   IMAGE_LENGTH, &intval);
	EXIT_ON_FAIL(context, ret, "GetTextValue() failed.");

	if (intval != IMAGE_LENGTH)
	{
		EXIT_ON_FAIL(context, CS_FAIL, 
			     "Length of written image less than expected.");
	}

	/*
	** Compare the two images for symmetry:
	*/
	if (compare ((char *)data_ret, (char *)data, (int)IMAGE_LENGTH) != 0)
	{
		EXIT_ON_FAIL(context, CS_FAIL,
			     "Image data not symmetric, exiting.");
	}
	else
	{
		fprintf(OUT_CH,
			"\n Image data symmetric, everything OK. All done.\n\n"); 
	}

	/*
	** Now clean up after ourselves. We need to delete the bogus row
	** that we inserted into au_pix. We do this with ex_execute_cmd()
	** from exutils.c
	**
	-- fprintf(OUT_CH,"\nBYE.\n");
	--
	-- dbfcmd(u_dbproc, "delete au_pix where au_id = '%s'",AU_ID);
	-- dbsqlexec(u_dbproc);
	--  *
	-- ** It is good programming practice to call dbresults even if
	-- ** no results are expected:
	-- *
	-- while ((result_code = dbresults(u_dbproc)) != NO_MORE_RESULTS)
	-- {
	--   if (result_code == SUCCEED) 
	--   {
	--      *
	--     ** No return rows are expected, but it is good
	--     ** programming practice to call dbcanquery, just
	--     ** in case someone tinkers with the command batch.
	--     *
	--     dbcanquery(u_dbproc);
	--   }
	-- }
	*/

	sprintf(sql_string, "delete au_pix where au_id = '%s'",AU_ID);
	ret = ex_execute_cmd(u_conn, sql_string);
	EXIT_ON_FAIL(context, ret, "DELETE for au_pix cleanup failed.");

	/*
	** Cleanup. ex_con_cleanup() and ex_ctx_cleanup() are defined
	** in exutils.c
	**
	-- doexit(STDEXIT);
	*/
	ret = ex_con_cleanup(u_conn, CS_SUCCEED);
	ret = ex_con_cleanup(q_conn, ret);
	ret = ex_ctx_cleanup(context, ret);

	exit(NORMAL_EXIT);
}

/*
** servermsg_cb() --
**   Installed as a callback handler to report server messages.
**
** Parameters:
**   As documented in Client-Library reference manual.
**
** Returns:
**   CS_SUCCEED
**
*/
CS_RETCODE CS_PUBLIC
servermsg_cb(CS_CONTEXT *context, CS_CONNECTION *connection,
	     CS_SERVERMSG *srvmsg)
{
	/*
	** Ignore 5701 (changed database), 5703 (changed language),
	** or 5704 (changed client character set) messages.
	*/
	if (srvmsg->msgnumber == 5701
	    || srvmsg->msgnumber == 5703
	    || srvmsg->msgnumber == 5704)
	{
		return CS_SUCCEED;
	}

	fprintf(ERR_CH, "\nServer message:\n");
	fprintf(ERR_CH, "Message number: %ld, Severity %ld, ",
		(long)srvmsg->msgnumber, (long)srvmsg->severity);
	fprintf(ERR_CH, "State %ld, Line %ld\n",
		(long)srvmsg->state, (long)srvmsg->line);
	
	if (srvmsg->svrnlen > 0)
	{
		fprintf(ERR_CH, "Server '%s'\n", srvmsg->svrname);
	}
	
	if (srvmsg->proclen > 0)
	{
		fprintf(ERR_CH, " Procedure '%s'\n", srvmsg->proc);
	}

	fprintf(ERR_CH, "Message String: %s\n", srvmsg->text);
	fflush(ERR_CH);

	return CS_SUCCEED;
}

/* 
** getimage() -- generate some random information 
** to put in the image row
** 
** This code is unchanged from DB-Library's example9.c
*/
void
getimage(char *ptr)
{
	 long	loop;
	       
	 /*
	 **        fill the buffer with ordered data
	 */
	 for( loop=0;loop<IMAGE_LENGTH;loop++ )
	 {
		ptr[loop] = (loop % 255);    /* keep in range of single byte */
	 }
	 return;
} /* getimage() */

/*
** compare()
** A function to compare two arrays of data, 
** equivalent to bcmp or memcmp,
** used in place of either of these to be system independent.
** 
** This code is unchanged from DB-Library's example9.c
*/
int
compare(char *src1, char *src2, int length)
{
	 for (; length > 0; length--, ++src1, ++src2)
	 { 
		if ( (src1 == NULL) || (src2 == NULL) )
		{
			return(-1);
		}
		if (*src1 != *src2)
		{
			return(*src1 < *src2 ? -1 : 1);
		}
	 }
	     
	 return(0);
} /* compare() */

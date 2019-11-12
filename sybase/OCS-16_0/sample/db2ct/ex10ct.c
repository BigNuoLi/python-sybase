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
**    	ex10ct.c is converted from DB-Library's example10.c to use
**	Client-Library.
**
**      Where DB-Library code has been replaced with Client-Library code,
**      the DB-Library code is commented out and preceded with "--".
**
**      This example uses the au_pix table in the pubs2 database
**      to show the basic outline of how to use Client-Library to write
**      text/image data into a ASE database. This example
**      sends the data in 'chunks', using a 1000-byte buffer to
**      read the data from the input file and send each chunk to
**      to the server with a call to ct_send_data().
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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
/* 
** Client-Library header file is ctpublic.h
**
-- #include <sybfront.h>
-- #include <sybdb.h>
-- #include <syberror.h>
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

/*
** txtimg.c contains utility routines for text and image handling in 
** Client-Library.
** 
** The txtimg.c routines used here are:
**
**  GetTextValue() -- Get the CS_IODESC structure required to
**     update a text/image column and, optionally, get the 
**     column's value.
**     
**     GetTextValue() is similar to selecting a text/image column
**     in DB-Library and calling dbtxptr(), dbtxtimestamp(), and
**     dbreadtext().
**
**  TextUpdateResults() -- Process the results of a Client-Library
**     send-data text/image update command and update the text/image
**     column's IO-Descriptor (CS_IODESC) structure with the new
**     text timestamp.
**  
**     TextUpdateResults() is similar to calling DB-Library's
**     dbresults() and dbtxtsnewval() after using dbwritetext() and
**     dbmoretext() to send a text/image value.
*/
#include "txtimg.h"

/* 
** This program installs it's own server message callback in
** place of the one in exutils.c
*/
CS_RETCODE CS_PUBLIC servermsg_cb PROTOTYPE ((
	CS_CONTEXT     *context,
	CS_CONNECTION  *connection,
	CS_SERVERMSG   *srvmsg
	));

int
main(int argc, char *argv[])
{
	/*
	-- DBPROCESS  *q_dbproc;          * connection with server. *
	-- DBPROCESS  *u_dbproc;          * ibid.                   *
	-- LOGINREC   *login;             * login information.      *
	*/
	CS_CONTEXT	*context = (CS_CONTEXT *) NULL;
	CS_CONNECTION	*q_conn;
	CS_CONNECTION	*u_conn;
	CS_COMMAND	*u_cmd;
	CS_IODESC	iodesc;

	CS_CHAR		filename[80];

	/*
	** The sqlcmd variable from example10.c is renamed as sql_buf here.
	*/
	CS_CHAR		sql_buf[SQLBUFLEN];
	CS_CHAR		au_id[12];

	/*
	-- DBBINARY   buf[DBBUFSIZE];     * DBBINARY so 1st bytes   *
	--                                * come out right...       *
	*/
#define IMAGE_BUFSIZE 1000
	CS_IMAGE	image_buf[IMAGE_BUFSIZE];
	
	long		length, num_done;
	FILE		*fp;

	/*
	-- RETCODE    result_code;
	*/
	CS_RETCODE	ret;

	/*
	** Setup screen output.
	*/
	EX_SCREEN_INIT();

	/*
	** Initialization
	*/
	ret = CS_FAIL;

	/*
	**  First, we need to get two pieces of data
	**  from the user: the au_id and the filename:
	*/

	fprintf(OUT_CH, "Please enter an author-id for a new au_pix row: \n");
	fscanf(stdin,"%s", au_id);
	if ( strlen(au_id) >= 12)
	{
		EXIT_ON_FAIL(context, CS_FAIL,
			     "Author-id must be less than 12 digits\n");
	}


	fprintf(OUT_CH, "Please enter the name of a ");
	fprintf(OUT_CH, "file that holds an image: \n");
	fscanf(stdin,"%s",filename);

	/* Open the recipient file: */
	if ((fp = fopen(filename, "rb")) == (FILE *)NULL)
	{
		fprintf(ERR_CH, "Can't open file %s.\n", filename);
		EXIT_ON_FAIL(context, ret, "Can't open specified file!");
	}

	/* Get length of file to read: */
	length = 0;            
	while(feof(fp) == 0)
	{
		length += fread(image_buf, sizeof(CS_IMAGE),
				sizeof(image_buf), fp);
	}
	rewind(fp);

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
	** example10, we need to install a handler that does not print
	** these messages.
	**
	** In Client-Library, all user callbacks are installed with
	** ct_callback().
	** servermsg_cb() is defined at the bottom of this source file.
	*/
	ret = ct_callback(context, (CS_CONNECTION *)NULL, CS_SET, 
			  CS_SERVERMSG_CB, (CS_VOID *)servermsg_cb);
	EXIT_ON_FAIL(context, ret, "ct_callback() for server messages failed.");

	/*
	--   * Get a LOGINREC:    *
	--  login = dblogin();
	--
	--  DBSETLUSER(login, USER);
	--  DBSETLPWD(login, PASSWORD);
	--
	--   *
	--  ** Get two DBPROCESS structures to talk to the ASE.
	--  ** One is for querying and one for updating.          
	--  *
	--
	--  q_dbproc = dbopen(login, NULL);
	--  u_dbproc = dbopen(login, NULL);
	*/

	/*
	** Connect to the server. We use ex_connect from exutils.c to 
	** do this. ex_connect() sets the login information (with
	** ct_con_props) and calls ct_connect().
	**
	** As with the DB-Library example, the default server name is taken
	** as the contents of the DSQUERY environment variable.
	*/
	ret = ex_connect(context, &q_conn, "ex10ct q", USER, PASSWORD, NULL);
	EXIT_ON_FAIL(context, ret, "Connection attempt 1 failed.");
	ret = ex_connect(context, &u_conn, "ex10ct u", USER, PASSWORD, NULL);
	EXIT_ON_FAIL(context, ret, "Connection attempt 2 failed.");

	/*
	-- dbuse(q_dbproc,"pubs2");
	-- dbuse(u_dbproc,"pubs2");
	*/ 

	/*
	** Call ex_use_db() (defined in exutils.c) to place each connection
	** in the pubs2 database. 
	*/
	ret = ex_use_db(q_conn, "pubs2");
	EXIT_ON_FAIL(context, ret, "'USE pubs2' on query connection failed");
	ret = ex_use_db(u_conn, "pubs2");
	EXIT_ON_FAIL(context, ret, "'USE pubs2' on update connection failed");

	/*
	** We will need a command structure to send commands on the
	** u_conn connection below.
	*/
	ret = ct_cmd_alloc(u_conn, &u_cmd);
	EXIT_ON_FAIL(context, ret, "ct_cmd_alloc() failed.");

	/*
	** Here are the steps performed to insert an image column in 
	** chunks. The Client-Library equivalent is explained below
	** each DB-Library step. 
	**
	--  **   1. INSERT all data into the row except the image.
	--  **   2. UPDATE the row to create a valid text pointer.
	** 
	** Steps 1 and 2 are the same in Client Library
	**
	--  **   3. SELECT the row to pull the text pointer and
	--  **      text timestamp into the DBPROCESS.
	--  **   4. Call dbtxtptr() to retrieve the text pointer 
	--  **      from the DBPROCESS.
	--  **   5. Call dbtxtimestamp() to retrieve the text timestamp 
	--  **      from the DBPROCESS.
	**
	** Step 3, 4, and 5 (Client-Library):
	** Select the row, and when we see it in the results loop, call 
	** ct_get_data() on the text column, then call ct_data_info(CS_GET) to 
	** initialize our CS_IODESC structure for the update. 
	** 
	--  **   6. Write the image value to ASE, using dbwritetext().
	**
	** Step 6 (Client-Library):
	** 6a. Call ct_command() to initiate a send-data command. 
	** 6b: Set the log_on_update fields and the total_txtlen fields in the 
	**     CS_IODESC structure, then call ct_data_info(CS_SET) to set up
	**     the ct_send_data() loop.
	**
	** Note that we don't call ct_results() until after all the data has
	** been sent.
	**
	** Step 7 (Client-Library):
	** Read the contents of the input file a chunk at a time and send each
	** chunk to the server with ct_send_data(). After the last chunk is sent,
	** call ct_send() to finalize the send-data command.
	**
	** Step 8 (Client-Library):
	** Handle the results of the update. We call TextUpdateResults() from 
	** txtimg.c to do this. The new text timestamp is returned as a 
	** one-row, one-column CS_PARAM_RESULT result set in the results stream.
	** TextUpdateResults() executes a ct_results() loop that retrieves the
	** the new timestamp and places it in our CS_IODESC structure.
	*/
	
	/*
	--   *
	--  **  Step 1:  INSERT the data:
	--  * 
	--  sprintf(sqlcmd," insert au_pix (au_id) values ('");
	--  strcat(sqlcmd,au_id);
	--  strcat(sqlcmd, "')");
	--
	--  dbcmd(q_dbproc, sqlcmd);
	--  dbsqlexec(q_dbproc);
	--
	--   *
	--  **  It is good programming practice to always call dbresults
	--  **  in a loop:
	--  * 
	--
	--  while (dbresults(q_dbproc) != NO_MORE_RESULTS)
	--  {
	--    continue;
	--  }
	*/
	
	/*
	** Step 1 (Client-Library): 
	** Use ex_execute_cmd(), defined in exutils.c, to send the insert and
	** process the results.
	*/
	sprintf(sql_buf," insert au_pix (au_id) values ('"); 
	strcat(sql_buf,au_id);
	strcat(sql_buf, "')");
	ret = ex_execute_cmd(q_conn, sql_buf);
	if (ret != CS_SUCCEED)
	{
		EXIT_ON_FAIL(context, CS_FAIL, "Row insert failed.");
	}

	/*
	--   *
	--  **  Step 2:  UPDATE the row:
	--  * 
	--  strcpy(sqlcmd," update au_pix set pic = null \
	--  		    where au_id = '");
	--  strcat(sqlcmd,au_id);
	--  strcat(sqlcmd, "'");
	--
	--  dbcmd(q_dbproc, sqlcmd);
	--  dbsqlexec(q_dbproc);
	--
	--   *
	--  **  It is good programming practice to always call dbresults
	--  **  in a loop:
	--  * 
	--
	--  while (dbresults(q_dbproc) != NO_MORE_RESULTS)
	--  {
	--    continue;
	--  }
	*/

	/*
	** Step 2 (Client-Library):
	** Use ex_execute_cmd(), defined in exutils.c, to send the update and
	** process the results.
	*/
	strcpy(sql_buf," update au_pix set pic = null where au_id = '");
	strcat(sql_buf,au_id);
	strcat(sql_buf, "'");
	ret = ex_execute_cmd(q_conn, sql_buf);
	if (ret != CS_SUCCEED)
	{
		EXIT_ON_FAIL(context, CS_FAIL, "Row update failed.");
	}

	/*
	--   *
	--  **  Step 3:  SELECT the row:
	--  * 
	--
	--  strcpy(sqlcmd," select pic from au_pix \
	--                     where au_id = '");
	--  strcat(cmd,au_id);
	--  strcat(cmd, "'");
	--
	--  dbcmd(q_dbproc, sql_buf);
	--  dbsqlexec(q_dbproc);
	--
	--   *
	--  ** It is good programming practice to always call dbresults 
	--  ** in a loop.
	--  * 
	--
	--  while ((result_code = dbresults(q_dbproc)) != NO_MORE_RESULTS)
	--  {
	--    if (result_code == SUCCEED) 
	--    {
	--        * One row is returned but we are not interested in it: * 
	--       dbcanquery(q_dbproc);
	--    }
	--  }
	*/

	/*
	** Steps 3, 4 and 5 (Client-Library): Select the row and get a CS_IODESC
	**   structure containing the necessary information for a text/image
	**   update. We use GetTextValue(), from txtimg.c, to perform these
	**   steps.
	**
	** GetTextValue() takes as input a SQL statement to select the 
	** text/image column of interest.
	*/
	strcpy(sql_buf, "select pic from au_pix where au_id = '");
	strcat(sql_buf, au_id);
	strcat(sql_buf, "'");

	/*
	** GetTextValue() takes pointers to a CS_IODESC and (optionally)
	** to a buffer to receive the value. We don't care about the
	** value here, so we pass the buffer (5th) and buffer-length (6th)
	** arguments as NULL and 0, respectively.
	*/
	ret = GetTextValue(q_conn, &iodesc, sql_buf, 1, (CS_VOID *) NULL, 
			   0, (CS_INT *) NULL);
	EXIT_ON_FAIL(context, ret, "GetTextValue() failed.");

	/*
	--   *
	--  **  Steps 4, 5, and 6:  dbtxtptr(), dbtxtimestamp(),
	--  **  and dbwritetext():
	--  * 
	--  dbwritetext (u_dbproc, "au_pix.pic", dbtxptr(q_dbproc, 1), 
	--           DBTXPLEN, dbtxtimestamp(q_dbproc, 1), 
	--           TRUE, (DBINT)length, NULL);
	--
	--  dbsqlok(u_dbproc);
	--  dbresults(u_dbproc);
	*/

	/*
	** Step 6 (Client-Library).
	** a. Call ct_command() to initiate a send-data command.
	** b. Set the log_on_update and total_txtlen fields in the CS_IODESC,
	**    then
	** c. call ct_data_info(CS_SET) to tell Client-Library how big the
	**    value is and whether the update should be logged.
	*/

	/*
	** Step 6a: Initiate a send-data command.
	*/
	ret = ct_command(u_cmd, CS_SEND_DATA_CMD, NULL, CS_UNUSED,
			 CS_COLUMN_DATA);
	EXIT_ON_FAIL(context, ret, "ct_command(SEND_DATA) failed");

	/*
	** Step 6b: Tell Client-Library how big the column is and whether
	**   the update should be logged.
	** 
	** We use the IO-Descriptor obtained by GetTextValue() for 
	** the update. We must set the log_on_update fields and
	** total_txtlen fields ourselves, and then give the
	** IO-Descriptor to Client-Library with ct_data_info(CS_SET).
	*/
	iodesc.log_on_update = CS_TRUE;
	iodesc.total_txtlen = length;
	ret = ct_data_info(u_cmd, CS_SET, CS_UNUSED, &iodesc);
	EXIT_ON_FAIL(context, ret, "ct_data_info(SET) failed.");

	/*
	--   *
	--  **  More Step 6:  dbmoretext() is being used in conjunction
	--  **  with dbwritetext() to write the image:
	--  * 
	--
	--  num_done = 0;
	--  while((length - num_done) >= DBBUFSIZE) 
	--  {
	--    if (fread( (char *)buf, sizeof(DBBINARY), DBBUFSIZE, fp)
	--        != DBBUFSIZE) 
	--      fprintf(ERR_CH,"Error in reading image after %ld characters!",
	--              length);
	--    dbmoretext(u_dbproc, DBBUFSIZE, buf);
	--    num_done += DBBUFSIZE;
	--  }
	--
	--  if ((length - num_done) > 0)   
	--  {
	--    if ((CS_INT)fread(buf, sizeof(DBBINARY), (int)(length - num_done),
	--			fp) 
	--        != length - num_done)
	--    {
	--       error("Error in reading image...\n!", (char *)NULL);
	--    }
	--
	--    dbmoretext(u_dbproc, (int) (length - num_done), buf);
	--  }
	--
	--  dbsqlok(u_dbproc);
	--  dbresults(u_dbproc);
	*/

	/*
	** Step 7 (Client-Libary):
	** Now we loop, on each iteration reading a buffer's worth of bytes 
	** from the input file and sending them to the server with
	** ct_send_data(). After the last chunk is sent, we finalize
	** the command with ct_send().
	*/
	num_done = 0;
	while((length - num_done) >= sizeof(image_buf))
	{
		if (fread((char *)image_buf, sizeof(CS_IMAGE),
			  sizeof(image_buf), fp)
		    != sizeof(image_buf)) 
		{
			fprintf(ERR_CH,
				"Error in reading image after %ld characters!",
				length);
			EXIT_ON_FAIL(context, CS_FAIL, "File read error.");
		}
 
		/*
		** Now call ct_send_data() to send the buffer's contents to
		** the server.
		*/
		ret = ct_send_data(u_cmd, (CS_VOID *)image_buf,
				   CS_SIZEOF(image_buf));
		EXIT_ON_FAIL(context, ret, "ct_send_data() failed.");

		num_done += (sizeof(image_buf));

	}

	/*
	** Now, if the file size was not an even multiple of the buffer
	** size, we will have some leftovers to send.
	*/
	if ((length - num_done) > 0)   
	{
		if ((int)fread(image_buf, sizeof(CS_IMAGE),
			       (int)(length - num_done), fp) 
		    != length - num_done)
		{
			EXIT_ON_FAIL(context, CS_FAIL,
				     "Error on final file read.");
		}

		/*
		** ct_send_data to send the last piece.
		*/
		ret = ct_send_data(u_cmd, (CS_VOID *)image_buf, 
				   (CS_INT)(length - num_done));
		EXIT_ON_FAIL(context, ret, "Final ct_send_data failed.");

	}

	/*
	** All Client-Library commands, even send-data commands, must always
	** be finalized by a call to ct_send().
	*/  
	ret = ct_send(u_cmd);
	EXIT_ON_FAIL(context, ret, "ct_send() failed.");

	/*
	** Step 8 (Client-Library):
	** Process the results and get the new timestamp for the 
	** updated columnn. After a send-data command, the
	** new timestamp for the updated column is returned in a 
	** parameter result set. We use TextUpdateResults() from
	** txtimg.c to process the results and get the new timestamp
	** (the timestamp is returned inside iodesc).
	*/
	ret = TextUpdateResults(u_cmd, &iodesc);
	EXIT_ON_FAIL(context, ret, "TextUpdateResults() failed.");

	/*
	-- dbexit();
	*/

	/*
	** Cleanup. ex_con_cleanup() and ex_ctx_cleanup() are defined
	** in exutils.c
	*/
	ret = ct_cmd_drop(u_cmd);
	ret = ex_con_cleanup(u_conn, ret);
	ret = ex_con_cleanup(q_conn, ret);
	ret = ex_ctx_cleanup(context, ret);

	exit(NORMAL_EXIT);

} /* main() */

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
		(long) srvmsg->msgnumber, (long) srvmsg->severity);
	fprintf(ERR_CH, "State %ld, Line %ld\n",
		(long) srvmsg->state, (long) srvmsg->line);
	
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
} /* servermsg_cb() */

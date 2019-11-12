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
** 	ex11ct.c is converted from DB-Library's example11.c to use
**	Client-Library.
**
**      Where DB-Library code has been replaced with Client-Library code,
**      the DB-Library code is commented out and preceded with '--'.
**      
**      This example uses the au_pix table in the pubs2 database
**      to show the basic outline of how to use Client-Library
**      to retrieve a text/image column and write it to an
**      operating system file.
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
-- #include "sybdbex.h"
*/
#include <ctpublic.h>
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
** This program installs it's own server message callback in
** place of the one in exutils.c
**
*/
CS_RETCODE CS_PUBLIC servermsg_cb PROTOTYPE ((
	CS_CONTEXT	*context,
	CS_CONNECTION	*connection,
	CS_SERVERMSG	*srvmsg
	));

/*
** Define the value to set the @@TEXTSIZE ASE global to
** using ct_options().
**
** dboptions() takes a character string for the option value. 
** ct_options() takes a pointer, and the datatype depends on 
** which option is being set or retrieved.
**
--#define  IMAGESIZE "60000"
*/
#define IMAGESIZE ((CS_INT) 60000)

int
main(int argc, char *argv[])
{
	/*
	-- DBPROCESS    *q_dbproc;            * connection with server. *
	-- LOGINREC     *login;               * login information.      *
	-- char         sqlcmd[255];
	-- char         au_id[15];
	*/
	CS_CONTEXT	*context;
	CS_CONNECTION	*conn;
	CS_COMMAND	*cmd;
	CS_IODESC	iodesc;
	CS_RETCODE	ret;
	CS_RETCODE	results_ret;
	CS_RETCODE	get_ret;
	CS_INT		result_type;
	CS_CHAR		sqlcmd[255];
	CS_CHAR		au_id[15];

	/*
	** The DB-Library code used dbdata() and a pointer to retrieve the image 
	** value. This implies a 32-kilobyte (or so) dynamic memory allocation. 
	** We'll use a 512-byte buffer and read the data in chunks.
	**
	-- DBBINARY     *image;
	*/
#define IMAGE_BUF_SIZE 512
	CS_IMAGE	image_buf[IMAGE_BUF_SIZE];
	CS_INT		n_read;

	char		filename[80];
	long		length;
	long		n_written;
	FILE		*fp;
	/*
	-- RETCODE      result_code;
	*/

	/*
	** Initialization
	*/
	length = 0;

	/*
	** Setup screen output.
	*/
	EX_SCREEN_INIT();

	/*
	** Start by creating the recipient file
	*/
	fprintf(OUT_CH, "Please enter the author-id ");
	fprintf(OUT_CH, "to retrieve: \n");
	fscanf(stdin,"%s", au_id);
	if (strlen(au_id) >= 15)
	{
		fprintf(OUT_CH,"Author-id must be less than 15 digits\n");
		exit(ERROR_EXIT);
	}

	fprintf(OUT_CH, "Please enter the filename that will ");
	fprintf(OUT_CH, "hold the image: \n");
	fscanf(stdin,"%s",filename);

	if ((fp = fopen(filename, "wb")) == (FILE*)NULL)
	{
		fprintf (ERR_CH,"Can't create %s\n", filename);
		exit(ERROR_EXIT);
	}

	/*
	--  * Initialize DB-Library:  * 
	-- if (dbinit() == FAIL)
	-- {
	--   fprintf (ERR_CH,"Can't initialize dblib...\n");
	--   exit(ERREXIT);
	-- }
	--
	--  *
	-- ** Install the user-supplied error-handling and message-handling
	-- ** routines. They are defined at the bottom of this source file.
	-- * 
	-- dberrhandle((EHANDLEFUNC)err_handler);
	-- dbmsghandle((MHANDLEFUNC)msg_handler);
	*/

	/*
	** Allocate a CS_CONTEXT, initialize CS-Library and Client-Library, and
	** install callbacks for Client-Library errors and server messages.
	** ex_init() is defined in exutils.c.
	*/  
	context = (CS_CONTEXT *) NULL;
	ret = ex_init(&context);
	EXIT_ON_FAIL(context, ret, "Client-Library initialization failed.");

	/*
	** The server message callback installed by ex_init() does not
	** ignore 57xx messages. In order to mimic the behavior of DB-Library's
	** example11, we need to install a handler that does not print
	** these messages.
	**
	** In Client-Library, all user callbacks are installed with
	** ct_callback(). servermsg_cb() is defined at the bottom of this
	** source file.
	*/
	ret = ct_callback(context, (CS_CONNECTION *)NULL, CS_SET, 
			  CS_SERVERMSG_CB, (CS_VOID *)servermsg_cb);
	EXIT_ON_FAIL(context, ret, "ct_callback() for server messages failed.");

	/*
	--  * Get a LOGINREC:  * 
	-- login = dblogin();
	--
	-- DBSETLUSER(login,USER);
	-- DBSETLPWD(login, PASSWORD);
	--  *
	-- **  Get a DBPROCESS structure to talk to ASE:
	-- * 
	-- q_dbproc = dbopen(login, NULL);
	*/

	/*
	** Connect to the server. ex_connect() is defined in exutils.c.
	*/
	ret = ex_connect(context, &conn, "ex11ct", USER, PASSWORD, NULL);
	EXIT_ON_FAIL(context, ret, "Connection attempt 2 failed.");

	/*
	-- dbuse(q_dbproc,"pubs2");
	*/

	/*
	** Call ex_use_db() (from exutils.c) to place the connection
	** in the pubs2 database. 
	*/
	ret = ex_use_db(conn, "pubs2");
	EXIT_ON_FAIL(context, ret, "'USE pubs2' on query connection failed");

	/*
	** We will need a command structure to send commands on the
	** connection.
	*/
	ret = ct_cmd_alloc(conn, &cmd);
	EXIT_ON_FAIL(context, ret, "ct_cmd_alloc() failed.");

	/*
	--  * Set @@textsize to 60000 so it'll hold our image: * 
	-- dbsetopt(q_dbproc, DBTEXTSIZE, IMAGESIZE, -1);
	*/
	result_type = IMAGESIZE;
	ret = ct_options(conn, CS_SET, CS_OPT_TEXTSIZE,
			 (CS_VOID *)&result_type, CS_UNUSED, (CS_INT *)NULL);
	EXIT_ON_FAIL(context, ret, "ct_options() failed.");

	/*
	-- strcpy(sqlcmd," select pic from au_pix where au_id = '");
	-- strcat(sqlcmd,au_id);
	-- strcat(sqlcmd,"'");
	-- dbcmd(q_dbproc, sqlcmd);
	-- dbsqlexec(q_dbproc);
	*/

	/*
	** Send a language command to select the column containing the image
	** of interest.
	*/
	strcpy(sqlcmd," select pic from au_pix where au_id = '");
	strcat(sqlcmd,au_id);
	strcat(sqlcmd,"'");

	ret = ct_command(cmd, CS_LANG_CMD, (CS_VOID *)sqlcmd, strlen(sqlcmd),
			 CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "ct_command() failed.");   
	ret = ct_send(cmd);
	EXIT_ON_FAIL(context, ret, "ct_send() failed.");   

	/*
	-- while ((result_code = dbresults(q_dbproc)) != NO_MORE_RESULTS)
	-- {
	--     if (result_code == SUCCEED) 
	--     {
	--          * Write the image to the file: * 
	--         n_written = 0;
	--         while (dbnextrow(q_dbproc) != NO_MORE_ROWS) 
	--         {
	--             length = dbdatlen(q_dbproc, 1);
	--             image = (DBBINARY *)dbdata(q_dbproc,1);
	--             while (length - n_written >= DBBUFSIZE)
	--             {
	--                 if (fwrite((int)(image + n_written), 
	--                            sizeof(DBBINARY), (int)DBBUFSIZE, fp) 
	--                     != DBBUFSIZE)
	--                 {
	--                   error("Error in writing image!\n", (char *)NULL);
	--                 }
	--
	--                 n_written += DBBUFSIZE;
	--             }
	--
	--             if (length - n_written >= 0)
	--             {
	--                 if (fwrite(image + n_written, 
	--                            sizeof(DBBINARY), length - n_written, fp) 
	--                     != length - n_written)
	--                 {
	--                     error("Error in writing final image!\n", 
	--                            (char *)NULL);
	--                 }
	--             }
	--         }    
	--     }
	-- }
	*/

	/*
	** Handle the results. When we fetch a CS_ROW_RESULT row, we'll read
	** the data from column 1 and write it to the output file.
	*/
	while ((results_ret = ct_results(cmd, &result_type)) == CS_SUCCEED)
	{
		switch((int)result_type)
		{
		  case CS_ROW_RESULT:
			/*
			** This is the image data. We need to fetch the row,
			** then we can retrieve the value. If the author id
			** entered by the user is not in the table, then no
			** rows are returned, and ct_fetch() 
			** returns CS_END_DATA on the first call.
			*/
			ret = CT10_FETCH(cmd, (CS_INT *)NULL);
			if (ret == CS_END_DATA)
			{ 
				printf("Row with author id '%s' not found.\n",
				       au_id);

				/*
				** Set up to skip the image retrieval.
				*/
				get_ret = CS_END_DATA;
			}
			else
			{
				/*
				** ret should be CS_SUCCEED if we got a row.
				*/
				EXIT_ON_FAIL(context, ret,
					     "ct_fetch() failed.");

				/*
				** Get the length of the image by calling
				** ct_get_data() with a zero buffer lenghth
				** specified, then calling ct_data_info().
				** ct_data_info() fills a CS_IODESC structure,
				** and the length is placed in the total_txtlen
				** field of the structure.
				**
				** We don't need the length for the retrieval
				** algorithm that used below. This code is
				** here only to illustrate how to get the
				** column length for text/image columns.
				*/        
				ret = ct_get_data(cmd, 1, (CS_VOID *)image_buf,
						  0, (CS_INT *)NULL);
				EXIT_ON_FAIL(context, ret,
					     "ct_get_data() failed.");

				ret = ct_data_info(cmd, CS_GET, 1, &iodesc);
				EXIT_ON_FAIL(context, ret,
					     "ct_data_info(GET) failed");

				length = iodesc.total_txtlen;

				/*
				** Set up the image retrieval loop.
				*/
				get_ret = CS_SUCCEED;
			}

			/*
			** Image retrieval (ct_get_data) loop.
			** 
			** Call ct_get_data in a loop to read the image,
			** and write each chunk to the output file as
			** it is read.
			*/
			n_written = 0;
			while(get_ret == CS_SUCCEED)
			{
				get_ret = ct_get_data(cmd, 1,
						      (CS_VOID *)image_buf,
						      IMAGE_BUF_SIZE, &n_read);
				if (get_ret != CS_SUCCEED 
				    && get_ret != CS_END_DATA 
				    && get_ret != CS_END_ITEM)
				{
					EXIT_ON_FAIL(context, CS_FAIL,
						     "ct_get_data() failed.");
				}

				if ((int)fwrite((CS_VOID *)image_buf,
						sizeof(CS_IMAGE), (int)n_read,
						fp)
				    != n_read)
				{
					EXIT_ON_FAIL(context, CS_FAIL,
						     "Error writing to output file.");
				}

				n_written += n_read;
			}

			/*
			** Flush the rest of the row results, if any remain.
			*/
			ret = ct_cancel((CS_CONNECTION *)NULL, cmd,
					CS_CANCEL_CURRENT);
			EXIT_ON_FAIL(context, ret,
				     "ct_cancel(CURRENT) failed.");
			break;

		  case CS_CMD_FAIL:
			/*
			** Command failed on the server. 
			*/
			EXIT_ON_FAIL(context, CS_FAIL, 
				     "ct_results() returned unexpected result type");

		  case CS_CMD_SUCCEED:
		  case CS_CMD_DONE:
			/*
			** These are expected.
			*/
			break;

		  default:
			EXIT_ON_FAIL(context, CS_FAIL, 
				     "ct_results() returned unexpected result type");
		}
	}

	/*
	** Check for ct_results() errors.
	*/
	switch((int)results_ret)
	{
	  case CS_END_RESULTS:
		/*
		** This is the expected value.
		*/
		break;
	  case CS_FAIL:
		EXIT_ON_FAIL(context, CS_FAIL, "ct_results() failed");
		break;
	  default:
		EXIT_ON_FAIL(context, CS_FAIL,
			     "ct_results() returned unexpected value.");
		break;
	}

	/*
	-- dbexit();
	*/
	
	/*
	** Clean up and exit. The ex_* cleanup routines are defined in exutils.c
	*/
	fclose(fp);
	ret = ct_cmd_drop(cmd);
	ret = ex_con_cleanup(conn, ret);
	ret = ex_ctx_cleanup(context, ret);

	exit(0);
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
	** Ignore 5701 (changed database) message.
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

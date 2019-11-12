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
**      This example uses the au_pix table in the pubs2 database
**      to show the basic outline of how to use the image
**      routines in DB-Library to retrieve data from a table
**      in the database and put the result into a file.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sybfront.h>
#include <sybdb.h>
#include <syberror.h>
#include "sybdbex.h"

#define  IMAGESIZE "60000"

/* Forward reference
*/
void error(char *, char *);		/* print error message and die */

int
main(int argc, char *argv[])
{
	DBPROCESS    *q_dbproc;         /* connection with server. */
	LOGINREC     *login;            /* login information.      */
	char         sqlcmd[255];
	DBBINARY     *image;
	char         au_id[15];
	char         filename[80];
	long         length, n_written;
	FILE         *fp;
	RETCODE      result_code;

	/* Start by creating the recipient file */
	fprintf(OUT_CH, "Please enter the author-id ");
	fprintf(OUT_CH, "to retrieve: \n");
	fscanf(stdin,"%s", au_id);
	if ( strlen(au_id) >= 15)
	{
		fprintf(OUT_CH,"Author-id must be less than 15 digits\n");
		exit(ERREXIT);
	}

	fprintf(OUT_CH, "Please enter the filename that will ");
	fprintf(OUT_CH, "hold the image: \n");
	fscanf(stdin,"%s",filename);

	if ((fp = fopen(filename, "wb")) == (FILE*)NULL)
	{
		fprintf(ERR_CH,"Can't create %s\n", filename);
		exit(ERREXIT);
	}

	printf("Demo of SQL queries in a command batch\n\n");
	fflush(stdout);

	/* Initialize DB-Library:  */
	if (dbinit() == FAIL)
	{
		fprintf(ERR_CH,"Can't initialize dblib...\n");
		exit(ERREXIT);
	}

	/*
	** Install the user-supplied error-handling and message-handling
	** routines. They are defined at the bottom of this source file.
	*/
	dberrhandle((EHANDLEFUNC)err_handler);
	dbmsghandle((MHANDLEFUNC)msg_handler);

	/* Get a LOGINREC:  */
	login = dblogin();

	DBSETLUSER(login,USER);
	DBSETLPWD(login, PASSWORD);

	/*
	**  Get a DBPROCESS structure to talk to ASE:
	*/
	q_dbproc = dbopen(login, NULL);

	/*
	**  The database column "pic" is an image column.    
	**  Retrieve the value of one of its rows:
	*/
	dbuse(q_dbproc,"pubs2");

	/* Set @@textsize to 60000 so it'll hold our image: */
	dbsetopt(q_dbproc, DBTEXTSIZE, IMAGESIZE, -1);

	/* Get a pointer to the image: */
	strcpy(sqlcmd," select pic from au_pix where au_id = '");

	/* lint complains:  strcpy:  value declared inconsistently */
	strcat(sqlcmd,au_id);
	strcat(sqlcmd,"'");

	/* lint complains:  strcat:  value declared inconsistently */
	dbcmd(q_dbproc, sqlcmd);
	dbsqlexec(q_dbproc);

	/*
	**  It is good programming practice to always call dbresults
	**  in a loop:
	*/
	while ((result_code = dbresults(q_dbproc)) != NO_MORE_RESULTS)
	{
		if (result_code == SUCCEED) 
		{
			/* Write the image to the file: */
			n_written = 0;
			while (dbnextrow(q_dbproc) != NO_MORE_ROWS) 
			{
				length = dbdatlen(q_dbproc, 1);
				image = (DBBINARY *)dbdata(q_dbproc,1);

				while (length - n_written >= DBBUFSIZE)
				{
					if (fwrite(image + n_written, sizeof(DBBINARY),
					(int)DBBUFSIZE, fp) != DBBUFSIZE)
					{
						error("Error in writing image!\n",
							(char *)NULL);
					}

					n_written += DBBUFSIZE;
				}

				if (length - n_written >= 0)
				{
					if ((CS_INT)fwrite(image + n_written, 
					    sizeof(DBBINARY), length - n_written, fp) 
					    != length - n_written)
					{
						error("Error in writing final image!\n", 
						       (char *)NULL);
					}	
				}
			}    
		}
	}
	dbexit();
	exit(0);
}

int CS_PUBLIC 
err_handler(DBPROCESS *dbproc, int severity, int dberr, int oserr, 
	    char *dberrstr, char *oserrstr)
{
	if ((dbproc == NULL) || (DBDEAD(dbproc)))
	{
		return (INT_EXIT);
	}
	else 
	{
		fprintf(ERR_CH, "DB-Library error:\n\t%s\n", dberrstr);

		if (oserr != DBNOERR)
		{
			fprintf(ERR_CH, "Operating-system error:\n\t%s\n", oserrstr);
		}

		return (INT_CANCEL);
	}
}

int CS_PUBLIC
msg_handler(DBPROCESS *dbproc, DBINT msgno, int msgstate, int severity, char *msgtext,
	    char *srvname, char *procname, int line)
{
	/*
	** Ignore the 'Changed database to' and 'Changed language to'
	** messages.
	*/
	if (msgno == 5701 || msgno == 5703)
	{
		return (0);
	}

	fprintf(ERR_CH, "Msg %d, Level %d, State %d\n", 
		msgno, severity, msgstate);

	if (strlen(srvname) > 0)
	{
		fprintf(ERR_CH, "Server '%s', ", srvname);
	}

	if (strlen(procname) > 0)
	{
		fprintf(ERR_CH, "Procedure '%s', ", procname);
	}

	if (line > 0)
	{
		fprintf(ERR_CH, "Line %d", line);
	}

	fprintf(ERR_CH, "\n\t%s\n", msgtext);
	return (0);
}

void
error(char *s1, char *s2)        /* print error message and die */
{
	fprintf(ERR_CH, s1, s2);
	fprintf(ERR_CH, "\n");
	dbexit();
	exit(ERREXIT);
}


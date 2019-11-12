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
**      routines in DB-Library to insert data from a file
**      into the database. 
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

/* Forward reference
*/
void error PROTOTYPE((char *, char *));			/* print error message and die */

int
main(int argc, char *argv[])
{
	DBPROCESS    *q_dbproc;          /* connection with server. */
	DBPROCESS    *u_dbproc;          /* ibid.                   */
	LOGINREC     *login;             /* login information.      */
	char         filename[80];
	char         sqlcmd[SQLBUFLEN];
	char         au_id[12];
	DBBINARY     buf[DBBUFSIZE];     /* DBBINARY so 1st bytes   */
				     /* come out right...       */
	long         length, num_done;
	FILE         *fp;
	RETCODE      result_code;

	/*
	**  First, we need to get two pieces of data
	**  from the user: the au_id and the filename:
	*/

	fprintf(OUT_CH, "Please enter an author-id: \n");
	fscanf(stdin,"%s", au_id);
	if ( strlen(au_id) >= 12)
	{
		fprintf(OUT_CH,"Author-id must be less than 12 digits\n");
		exit(ERREXIT);
	}

	fprintf(OUT_CH, "Please enter the name of a ");
	fprintf(OUT_CH, "file that holds an image: \n");
	fscanf(stdin,"%s",filename);

	/* Open the recipient file: */
	if ((fp = fopen(filename, "rb")) == (FILE *)NULL)
	{
		fprintf(ERR_CH, "Can't open %s!", filename);
		exit(ERREXIT);
	}

	/* Get length of file to read: */
	length = 0;            
	while(feof(fp) == 0)
	{
		length += fread(buf, sizeof(DBBINARY), DBBUFSIZE, fp);
	}
	rewind(fp);

	printf("Demo of SQL queries in a command batch\n\n");
	fflush(stdout);

	/* Initialize DB-Library:  */
	if (dbinit() == FAIL)
	{
		exit(ERREXIT);
	}

	/*
	** Install the user-supplied error-handling and message-handling
	** routines. They are defined at the bottom of this source file:
	*/
	dberrhandle((EHANDLEFUNC)err_handler);
	dbmsghandle((MHANDLEFUNC)msg_handler);

	/* Get a LOGINREC:    */
	login = dblogin();

	DBSETLUSER(login, USER);
	DBSETLPWD(login, PASSWORD);

	/*
	** Get two DBPROCESS structures to talk to the ASE.
	** One is for querying and one for updating.          
	*/
	q_dbproc = dbopen(login, NULL);
	u_dbproc = dbopen(login, NULL);

	dbuse(q_dbproc,"pubs2");
	dbuse(u_dbproc,"pubs2");


	/*
	**  The database column "pic" is an image column.  
	**  Example 10 must perform the following steps in
	**  order to insert data into this column:
	**
	**   1. INSERT all data into the row except the image.
	**   2. UPDATE the row to create a valid text pointer.
	**   3. SELECT the row to pull the text pointer and
	**      text timestamp into the DBPROCESS.
	**   4. Call dbtxtptr() to retrieve the text pointer 
	**      from the DBPROCESS.
	**   5. Call dbtxtimestamp() to retrieve the text timestamp 
	**      from the DBPROCESS.
	**   6. Write the image value to ASE, using dbwritetext().
	*/ 

	/*
	**  Step 1:  INSERT the data:
	*/
	sprintf(sqlcmd," insert au_pix (au_id) values ('"); 
	strcat(sqlcmd,au_id);
	strcat(sqlcmd, "')");

	dbcmd(q_dbproc, sqlcmd);
	dbsqlexec(q_dbproc);

	/*
	**  It is good programming practice to always call dbresults
	**  in a loop:
	*/
	while (dbresults(q_dbproc) != NO_MORE_RESULTS)
	{
		continue;
	}

	/*
	**  Step 2:  UPDATE the row:
	*/
	strcpy(sqlcmd," update au_pix set pic = null \
			   where au_id = '");
	strcat(sqlcmd,au_id);
	strcat(sqlcmd, "'");

	dbcmd(q_dbproc, sqlcmd);
	dbsqlexec(q_dbproc);

	/*
	**  It is good programming practice to always call dbresults
	**  in a loop:
	*/
	while (dbresults(q_dbproc) != NO_MORE_RESULTS)
	{
		continue;
	}

	/*
	**  Step 3:  SELECT the row:
	*/
	strcpy(sqlcmd," select pic from au_pix \
			   where au_id = '");
	strcat(sqlcmd,au_id);
	strcat(sqlcmd, "'");

	dbcmd(q_dbproc, sqlcmd);
	dbsqlexec(q_dbproc);

	/*
	** It is good programming practice to always call dbresults 
	** in a loop.
	*/
	while ((result_code = dbresults(q_dbproc)) != NO_MORE_RESULTS)
	{
		if (result_code == SUCCEED) 
		{
			/* One row is returned but we are not interested in it: */
			dbcanquery(q_dbproc);
		}
	}

	/*
	**  Steps 4, 5, and 6:  dbtxtptr(), dbtxtimestamp(),
	**  and dbwritetext():
	*/
	dbwritetext (u_dbproc, "au_pix.pic", dbtxptr(q_dbproc, 1), 
		DBTXPLEN, dbtxtimestamp(q_dbproc, 1), 
		TRUE, (DBINT)length, NULL);

	dbsqlok(u_dbproc);
	dbresults(u_dbproc);

	/*
	**  More Step 6:  dbmoretext() is being used in conjunction
	**  with dbwritetext() to write the image:
	*/
	num_done = 0;
	while((length - num_done) >= DBBUFSIZE) 
	{
		if (fread( (char *)buf, sizeof(DBBINARY), DBBUFSIZE, fp) != DBBUFSIZE) 
		{
			fprintf(ERR_CH,"Error in reading image after %ld characters!",
				length);
		}

		dbmoretext(u_dbproc, DBBUFSIZE, buf);
		num_done += DBBUFSIZE;
	}

	if ((length - num_done) > 0)   
	{
		if ((CS_INT)fread(buf, sizeof(DBBINARY), (int)(length - num_done), fp) 
			!= length - num_done)
		{
			error("Error in reading image...\n!", (char *)NULL);
		}

		dbmoretext(u_dbproc, (int) (length - num_done), buf);
	}

	dbsqlok(u_dbproc);
	dbresults(u_dbproc);

	dbexit();
	exit(STDEXIT);
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
msg_handler(DBPROCESS *dbproc, DBINT msgno, int msgstate, int severity,
	    char *msgtext, char *srvname, char *procname, int line)
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
error(char *s1, char *s2)		/* print error message and die */
{
	fprintf(ERR_CH,s1, s2);
	fprintf(ERR_CH,"\n");
	dbexit();
	exit(ERREXIT);
}


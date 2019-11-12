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
**      routines in DB-Library.
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

#include    <stdio.h>
#include    <stdlib.h>
#include    <sybfront.h>
#include    <sybdb.h>
#include    "sybdbex.h"

#define     IMAGE_LENGTH         ((long)301)
#define     AU_ID                ((char*)"000-00-0000")

/* Forward declarations of the error handler and message handler. */
void doexit(int value);
void getimage(char *ptr);
int compare(char *src1, char *src2, int length);

int
main(int argc, char *argv[])
{
	DBPROCESS       *q_dbproc;  /* query connection with ASE */
	DBPROCESS       *u_dbproc;  /* update connection with ASE */
	LOGINREC        *login;     /* login information. */
	char            data[IMAGE_LENGTH];
	RETCODE         result_code;
	int             compare();

	/*
	** Here are the variables which will be used to store the
	** data being examined:
	**
	*/
	void            getimage();
	BYTE            *ret_p;            /* to hold image pointer */
	DBBINARY        *txptr;            /* text pointer */
	DBBINARY        *ts_ptr;           /* time stamp */

	printf("Demo of SQL queries in a command batch\n\n");
	fflush(stdout);

	/* Initialize DB-Library. */
	if (dbinit() == FAIL)
	exit(ERREXIT);

	/*
	**  Install the user-supplied error-handling and message-handling
	**  routines. They are defined at the bottom of this source file.
	*/
	dberrhandle((EHANDLEFUNC)err_handler);
	dbmsghandle((MHANDLEFUNC)msg_handler);

	/*
	**  Get a LOGINREC structure and fill it with the necessary
	**  login information.
	*/
	login = dblogin();
	DBSETLUSER(login,USER);           /* defined in sybdbex.h */
	DBSETLPWD(login,PASSWORD);
	DBSETLAPP(login, "Example 9");

	q_dbproc = dbopen(login, NULL);
	u_dbproc = dbopen(login, NULL);

	/* Send  ``use database'' commands: */
	dbuse(q_dbproc,"pubs2");
	dbuse(u_dbproc,"pubs2");

	/* 
	**  Get the image for insertion
	*/
	getimage(data);

	/*
	**  The database column "pic" is an image column.  
	**  Example 9 must perform the following steps in
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
	dbfcmd(q_dbproc, "insert au_pix (au_id) values ('%s')",AU_ID);
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
	dbfcmd(q_dbproc, "update au_pix set pic = null \
		      where au_id = '%s'",AU_ID);
	dbsqlexec(q_dbproc);

	/*
	**  It is good programming practice to call always call dbresults 
	**  in a loop.
	*/
	while (dbresults(q_dbproc) != NO_MORE_RESULTS)
	{
		continue;
	}

	/*
	**  Step 3:  SELECT the row.
	*/
	dbfcmd(q_dbproc, "select pic from au_pix \
		      where au_id = '%s'",AU_ID);
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
	**  Step 4:  dbtxtptr():
	*/

	/* Get the text pointer: */
	txptr = dbtxptr(q_dbproc, 1);

	/*
	**  Step 5:  dbtxtimestamp():
	*/

	/* Get a pointer to the text timestamp */
	ts_ptr = dbtxtimestamp(q_dbproc, 1);

	/*
	**  Step 6:  dbwritetext():
	*/

	/* Write the image: */
	dbwritetext (u_dbproc, "au_pix.pic", txptr, DBTXPLEN, 
		 ts_ptr, TRUE, IMAGE_LENGTH, (BYTE *)data);

	/*
	**  Image data has been inserted in to the server
	**  and we are about to select it out for compare
	**  to see if it got there OK.
	*/
	dbfcmd(q_dbproc, "select pic from au_pix \
	   where au_id = '%s'",AU_ID);
	dbsqlexec(q_dbproc);

	/*
	** It is good programming practice to call dbresults in a loop
	** even if only a single set of results is expected:
	*/

	while ((result_code = dbresults(q_dbproc)) != NO_MORE_RESULTS)
	{
		if (result_code == SUCCEED) 
		{
			/* We are expecting only a single row: */
			while ( dbnextrow(q_dbproc) != NO_MORE_ROWS)
			{
				ret_p = dbdata(q_dbproc,1);
			}
		}
	}

	/* Compare the two images for symmetry: */
	if (compare ((char *)ret_p, data, (int)IMAGE_LENGTH) != 0)
	{
		fprintf(ERR_CH,"\n Image data not symmetric, exiting.");
		doexit(ERREXIT);
	}
	else
	{
		fprintf(OUT_CH,"\n Image data symmetric, everything OK.");
	}

	fprintf(OUT_CH,"\nBYE.\n");

	/* Now we clean up: */
	dbfcmd(u_dbproc, "delete au_pix where au_id = '%s'",AU_ID);
	dbsqlexec(u_dbproc);

	/*
	** It is good programming practice to call dbresults even if
	** no results are expected:
	*/
	while ((result_code = dbresults(u_dbproc)) != NO_MORE_RESULTS)
	{
		if (result_code == SUCCEED) 
		{
			/*
			** No return rows are expected, but it is good
			** programming practice to call dbcanquery, just
			** in case someone tinkers with the command batch.
			*/
			dbcanquery(u_dbproc);
		}
	}

	doexit(STDEXIT);
	return (0);
}

void
doexit(int value)
{
	dbexit();            /* always call dbexit before returning to OS */
	exit(value);
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

/*
** generate some random information 
** to put in the image row
*/
void
getimage(char *ptr)
{
	long        loop;

	/* fill the buffer with ordered data */
	for( loop=0;loop<IMAGE_LENGTH;loop++ )
	{
		ptr[loop] = (loop % 255);        /* keep in range of single byte */
	}

	return;
}

/*
** A function to compare two arrays of data, 
** equivalent to bcmp or memcmp,
** used in place of either of these to be system independent.
*/
int
compare(char *src1, char *src2, int length)
{
	for (; length > 0; length--, ++src1, ++src2)
        {
		if ( (src1 == NULL) || (src2 == NULL) )
		{
			return (-1);
		}

		if (*src1 != *src2)
		{
			return (*src1 < *src2 ? -1 : 1);
		}
        }

        return (0);
}

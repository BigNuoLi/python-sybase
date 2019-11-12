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
**    	This example retrieves data from the "pubs2"
**    	database and prints the resulting info
**    	in us_english format using 4.2 dblib calls.
**
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <sybfront.h>
#include <sybdb.h>
#include <syberror.h>
#include "sybdbex.h"

#define DATESIZE        30
#define AUTHORSIZE      40
#define TITLESIZE       80
#define DATELENGTH      24
#define AUTHORLENGTH    20
#define TITLELENGTH     40

/* Forward declarations of functions to be used in the program: */

int strcompare(char *, char *);
DBINT process_results(void);
void print_header(void);
void fixdate(char *, char *);
void padstr(char *, int);
void error(char *, char *);       /* print error message and die */

DBPROCESS    *dbproc;            /* connection with server */
DBSORTORDER  *sortorder;

typedef int (*qsortCmpFunc)(const void *, const void *);
typedef struct    row
{    
    char    author[AUTHORSIZE];
    char    date[DATESIZE];
    char    title[TITLESIZE];
} row;
row record[30];

/**************************************************************
**
**                  strcompare (s1, s2) 
**
**  strcompare() takes two strings and compares them, using the 
**  sort order that is currently in the server.  It returns 0 
**  if they are equal, >0 if s1 > s2, <0 if s1 < s2.  
**  It is used by qsort().
*/
int
strcompare(char *s1, char *s2)
{
	return (dbstrcmp(dbproc, s1, AUTHORSIZE, s2, AUTHORSIZE, sortorder));
}


/**************************************************************
**                        main()  
** 
**  main() gets a connection to the server, sets the language 
**  to LANGUAGE, and gets data from the pubs2 database.  
**  It formats the data using the LANGUAGE date format and sorts 
**  the data by author according to the sort order currently 
**  active.  It uses a header returned by the query and adjusts 
**  the spacing to print a report onscreen.
*/

int
main(int argc, char *argv[])
{
	LOGINREC    *login;            /* login information.      */
	char         sqlcmd[SQLBUFLEN];
	char        *dateorder;
	DBINT        num_rows;         /* number of rows query returns */
	DBINT        j;

	printf("Demo of 4.2 calls: dbdateorder, dbcmd and dbsqlexec\n\n");
	fflush(stdout);

	/* Initialize DB-Library.  */
	if (dbinit() == FAIL)
	{
		fprintf(ERR_CH,"Couldn't initialize DB-Library...");
		exit(ERREXIT);
	}

	/*
	** Install the user-supplied error-handling and message-handling
	** routines. They are defined at the bottom of this source file.
	*/
	dberrhandle((EHANDLEFUNC)err_handler);
	dbmsghandle((MHANDLEFUNC)msg_handler);

	/* Get a LOGINREC:    */
	login = dblogin();

	DBSETLUSER(login, USER);
	DBSETLPWD(login, PASSWORD);
	DBSETLNATLANG(login,LANGUAGE);

	/* Get a DBPROCESS structure to talk to the ASE:  */
	dbproc = dbopen(login, NULL);
	if (dbuse(dbproc,"pubs2") == FAIL)
	{
		error("Interpubs database needs to be installed for this example \
			to run correctly.\n",(char *)NULL);
		exit(ERREXIT);
	}

	/* Get dateorder - is month or day first? */
	/* For 4.2, dbdateorder isn't declared in sybdb.h, so */
	/* cast its return to character:                      */
	dateorder = (char *)dbdateorder(dbproc, NULL);
	if (dateorder == (char *)NULL)
	{
		error("Couldn't get date order for %s...\n \
			Perhaps it's not in your server.", LANGUAGE);
	}

	if ((sortorder = dbloadsort(dbproc)) == (DBSORTORDER *)NULL) 
	{
		error( "Couldn't get sort order for %s...\n \
			Perhaps it's not in your server.\n", LANGUAGE);
	}

	strcpy(sqlcmd," select distinct \"Publication Date\"=t.pubdate, ");
	strcat(sqlcmd," \"Author\" = a.au_lname, \"Title\" = t.title");
	dbcmd(dbproc, sqlcmd);

	strcpy(sqlcmd," from titles t, authors a, titleauthor ta");
	strcat(sqlcmd," where t.title_id = ta.title_id");
	strcat(sqlcmd," and a.au_id = ta.au_id");
	dbcmd(dbproc, sqlcmd);

	dbsqlexec(dbproc);
	num_rows = process_results();
	print_header();
	qsort((char *)record, (int)num_rows, sizeof(record[0]), (qsortCmpFunc)strcompare);

	for(j = 0; j < num_rows; j++)
	{
		fixdate(record[j].date, dateorder);
		padstr(record[j].date,DATELENGTH);
		padstr(record[j].author,AUTHORLENGTH);
		fprintf(OUT_CH,"%s%s%s\n",record[j].date,
		       record[j].author,record[j].title);
	}

	dbexit();
	exit(0);
}

/**************************************************************
**
**                      process_results()  
** 
**  process_results() takes the dbproc that selected the data 
**  from pubs2 and binds the results to the array, "record".
*/

DBINT
process_results(void)
{
	DBINT	result_code, rows;
	char	tempdate[DATESIZE],
		tempauthor[AUTHORSIZE],
		temptitle[TITLESIZE];

	rows = 0;
	while ((result_code = dbresults(dbproc)) != NO_MORE_RESULTS)
	{
		if (result_code == SUCCEED)
		{
			/* Bind program variables. */
			dbbind(dbproc, 1, NTBSTRINGBIND, 0, (BYTE *)tempdate);
			dbbind(dbproc, 2, NTBSTRINGBIND, 0, (BYTE *)tempauthor);
			dbbind(dbproc, 3, NTBSTRINGBIND, 0, (BYTE *)temptitle);

			while (dbnextrow(dbproc) != NO_MORE_ROWS) 
			{
				strcpy(record[rows].date,tempdate);
				strcpy(record[rows].author,tempauthor);
				strcpy(record[rows].title,temptitle);

				/* When we're done, rows has number of rows: */
				rows++;                
			}
		}
	}

	return (rows);
}

/**************************************************************
**
**                        print_header()  
**
**  print_header() takes the header returned by the query in 
**  dbproc, puts it in a buffer, and copies the results into 
**  the header to be printed.  It removes blanks from the 
**  buffer and adjusts the spacing so the header lines up 
**  correctly.  Then it prints a series of hyphens underneath 
**  the header row.
*/

void
print_header(void)
{
	char        *buffer;
	char        *header;
	int          i;
	char        *temp, *temp1;

	fprintf(OUT_CH,"\n\n");
	if ((buffer = malloc(DBBUFSIZE)) == NULL)
	{
		error("unable to allocate space for buffer\n", (char *)NULL);
	}

	if ((header = malloc(DBBUFSIZE)) == NULL)
	{
		error("unable to allocate space for header\n", (char *)NULL);
	}

	/* Store raw header in buffer: */
	if(dbsprhead(dbproc, buffer, DBBUFSIZE) < 0)
	{
		error("dbsprhead failed...\n", (char *)NULL);
	}

	/* Copy date part of buffer to header: */
	strncpy(header, buffer, DATELENGTH);   

	/* Save address of header for printing and freeing: */
	temp = header;  

	/* Save address of buffer for free(): */
	temp1 = buffer; 

	/* Align author part of header correctly: */
	header += DATELENGTH;                         
	buffer += DATELENGTH;

	/* Skip extra spaces in buffer: */
	while (isspace(*buffer) != 0) 
	buffer++;

	/* Copy author part of buffer to header:  */
	for (i = 0; i < AUTHORLENGTH; i++)  
	*header++ = *buffer++;

	/* Skip extra spaces in buffer:  */
	while (isspace(*buffer) != 0)          
	buffer++;

	/* Copy title part of buffer to header:  */
	while (isspace(*buffer) == FALSE )            
	*header++ = *buffer++;
	*header = '\0';

	/* Print modified header:  */
	fprintf(OUT_CH,"%s\n",temp);           

	/* Print underline for header:  */
	for (i = 0; i < DATELENGTH -1; i++)           
	{
		fprintf(OUT_CH,"-");
	}

	fprintf(OUT_CH," ");
	for (i = 0; i < AUTHORLENGTH -1; i++) 
	{
		fprintf(OUT_CH,"-");
	}

	fprintf(OUT_CH," ");
	for (i = 0; i < TITLELENGTH -1; i++) 
	{
		fprintf(OUT_CH,"-");
	}

	fprintf(OUT_CH,"\n");

	free(temp);
	free(temp1);
}

/**************************************************************
**
**                  fixdate(date, dateorder)  
**
**  fixdate(date,dateorder) sets up an array of short versions 
**  of the months that us_english uses.  It then checks the 
**  month portion of the date it got back from the select query 
**  and retrieves the correct monthname from the server.  Then,
**  depending on the dateorder, it either puts the day first 
**  or the month first.
**  (A more complete version would check all possible 
**  combinations of mdy in dateorder; this just checks to see 
**  if d is the first character.)
*/

void
fixdate(char *date, char *dateorder)
{
	int     i, month_num;
	char    day[3],
		month[10],
		year[5];

/*
**  For different languages, replace these short month names 
**  with those found in the new language's common.loc file.
*/

	static char *month_name[] =                   
	{
		"Illegal month",		/* can't have 0th month  */
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	for(i = 1; i < 13; i++)			/* go through months     */
	{
		/* Is this the month? */
		if(strncmp(date,month_name[i],3) == 0)  
		{
			break;			/* if so, stop looping   */
		}
	}

	if((month_num = i) > 12)		/* not find month?       */
	{
        	error("month not found: number too large...\n", (char *) NULL); 
	}

	/* Day part = 2 chars + space: */
	for(i = 0; i < 2; i++)              
	{
		/* Copy date string into day: */ 
		day[i] = date[i+4];             
	}

	day[2] = '\0';

	/* Year part = 4 chars + space: */
	for(i = 0; i < 4; i++)              
	{
		year[i] = date[i+7];             
	}

	/* Copy date string into year: */
	year[4] = '\0';

	/* Get month name: */                 
	strcpy(month,(char *)dbmonthname(dbproc, NULL, month_num, FALSE)); 
					 
	/* Does day part come first? */
	if(*dateorder == 'd')           
	{
		/* Build new date string: */
		sprintf(date,"%s %s, %s",day, month, year); 
	} else
	{
		/* Build new date string: */
		sprintf(date,"%s %s, %s",month, day, year); 
	}
}


/**************************************************************
**
**                      padstr(s,l)
**
**  padstr(s, l) takes a string, s, and pads it with spaces, 
**  transforming it into a null-terminated string of length l.
*/

void
padstr(char *s, int l)
{
	int i,k;
	k = strlen(s);        /* save strlen so we don't overwrite it */
	s += l;               /* go to end of new string, length l    */
	*s = '\0';            /* delimit the new string               */
	s--;                  /* back up one char to not overwrite \0 */
	for (i = 0; i < l-k; i++)  /* back up until end of data       */
	*s-- = ' ';           /* put a space in each char             */
}


/**************************************************************
**
**                      error(s1, s2)
**
**  error(s1, s2) prints an error message, does a dbexit(), 
**  and exits the program.
*/

void
error(char *s1, char *s2)
{
	    fprintf(ERR_CH,s1, s2);
	    fprintf(ERR_CH,"\n");
	    dbexit();
	    exit(ERREXIT);
}


/**************************************************************
**
**                      err_handler
**
*/


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
		fprintf(ERR_CH,"DB-Library error:\n\t%s\n", dberrstr);

		if (oserr != DBNOERR)
		{
			fprintf(ERR_CH,"Operating-system error:\n\t%s\n", oserrstr);
		}

		return (INT_CANCEL);
	}
}


/**************************************************************
**
**                      msg_handler
**
*/

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

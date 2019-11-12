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
**	This is an example how ESQL/C code can be
**	used with HA-Failover. This sample does what example1
**	does, but with failover processing. This approach uses error handlers
**	to detect and handle failover. Another way is to test the
**	sqlca.sqlcode return after each EXEC.
*/

#include <stdio.h>
#include "sybsqlex.h"

/* Declare the SQLCA. */
EXEC SQL INCLUDE SQLCA;

/* 
** These tokens must be declared in a declare section
** because they are used in declare sections below.
*/
EXEC SQL BEGIN DECLARE SECTION;
#define TYPESIZE 	13
#define	TIDSIZE 	6
/* storage for login name and password. */
char	username[30];
char	password[30];
EXEC SQL END DECLARE SECTION;

#define  EOLN	'\0'

/* 
** Forward declarations of the error and message handlers and
** other subroutines called from main().
*/
void	error_handler();
void	warning_handler();
int	get_type();
int	get_titleid();
void	show_book();
void	print_failover_msg();
void	usedb();
void	open_typelist(void);
void	close_typelist(void);
void	open_titlelist(char *);
void	open_authorlist(char *);

#define ISWORDSPACE(c) (c == ' ' || c == '\t')

int
main(int argc, char *argv[])
{
	EXEC SQL BEGIN DECLARE SECTION;
	EXEC SQL END DECLARE SECTION;

	char	u_type[TYPESIZE+1];
	char	u_titleid[TIDSIZE+1];

	EXEC SQL WHENEVER SQLERROR CALL error_handler();
	EXEC SQL WHENEVER SQLWARNING CALL warning_handler();
	EXEC SQL WHENEVER NOT FOUND CONTINUE;

	/* 
	** Copy the user name and password defined in sybsqlex.h to
	** the variables declared for them in the declare section. 
	*/
	strcpy(username, USER);
	strcpy(password, PASSWORD);

	EXEC SQL CONNECT :username IDENTIFIED BY :password;

	usedb();


	/*
	** This loop controls the query.
	*/
	while (get_type(u_type))
	{
		while (get_titleid(u_titleid, u_type))
		{
			show_book(u_titleid);
		}
	}

	if (sqlca.sqlcode == CS_RET_HAFAILOVER)
	{
		close_typelist();
	}

	EXEC SQL DISCONNECT DEFAULT;

	return(STDEXIT);
}

void
open_typelist(void)
{
	/* Declare a cursor to select a list of book types. */
	EXEC SQL DECLARE typelist CURSOR FOR
		SELECT DISTINCT type FROM titles;

	/* Open the cursor. */
	EXEC SQL OPEN typelist;
}

void
close_typelist(void)
{
	/* CLOSE the cursor. */
	EXEC SQL CLOSE typelist;
}

/*
** int get_type(type) 
**
**	Displays a list of book types from the titles
**	table and lets the user enter one. 
*/
int
get_type(char *type)
{
	/* 
	** Declare a character string to hold types fetched
	** from the titles table.
	*/ 
	EXEC SQL BEGIN DECLARE SECTION;
	char	a_type[TYPESIZE+1];
	EXEC SQL END DECLARE SECTION;

	/* A buffer to hold user's fgets() entry. */
	char	buf[128];
	/* and count the rows */
	CS_INT	rowcount;
	CS_INT	currentcount;

	/*
	** List the book types on the screen. Break out of the
	** loop when a FETCH sets sqlca.sqlcode to 100.
	*/
	printf("\n\nSelect a Book Type:\n\n");
	open_typelist();
	rowcount = 0;
	currentcount = 0;
	if (sqlca.sqlcode == CS_RET_HAFAILOVER)
	{
		open_typelist();
	}
	for (;;)
	{
		EXEC SQL FETCH typelist INTO :a_type;

		/*
		**  If we've failed over at this point, we need
		**  to tell the secondary server that we're 
		**  using the pubs2 database, redeclare our
		** cursors and reissue the command that returned
		** failed over. All of this is done in error_handler().
		*/
		if (sqlca.sqlcode == CS_RET_HAFAILOVER)
		{
			/*
			** Do a continue to re-execute the fetch.
			*/
			rowcount = 0;
			open_typelist();
			continue;
		}
		/*
		** Reposition the cursor, if necessary
		*/
		if ( rowcount < currentcount)
		{
			rowcount++;
			continue;
		}

		if (sqlca.sqlcode == 100)
		{
			break;
		}
		rowcount++;
		currentcount++;
		printf("\t%s\n", a_type);
	}
	close_typelist();

	/*
	** Prompt the user for the book type. We're using fgets()
	** for the input function. We'll have to read the complete
	** input from stdin, therefore we'll fgets() into a large
	** temporary buffer, remove possible "\n" at the end of the
	** string and copy the correct number of bytes into the
	** variable passed from main().
	*/
	printf("\nbook type? ");
	fgets(buf, sizeof(buf), stdin);
	if (buf[strlen(buf) - 1] == '\n')
	{
		buf[strlen(buf) - 1] = '\0';
	}

	strncpy(type, buf, TYPESIZE);
	type[TYPESIZE] = EOLN;

	return (strlen(type));	
}

/*
** void titlelist_error()
** 
**	Displays error codes and numbers from the SQLCA and exits with
**	an ERREXIT status. 
*/
void
titlelist_error(char *type)
{
	/*
	**	at the very least, we should know to
	**	use our db. We also re-open the typelist.
	*/
	if (sqlca.sqlcode == CS_RET_HAFAILOVER)
	{
		usedb();
		open_titlelist(type);

		/*
		** We could also put the logic here to re-open the
		** author list and title list if we used a global
		** to track their current state.
		*/

		print_failover_msg(CS_TRUE);
		/*
		** put the CS_RET_HAFAILOVER error code back
		** as it was overwritten by the usedb() and
		** re opening the cursors.
		*/
		sqlca.sqlcode = CS_RET_HAFAILOVER;
		return;
	}

	/*
	** Not an HA problem, call the default error handler.
	*/
	error_handler();
}

void
open_titlelist(char *type)
{
	EXEC SQL BEGIN DECLARE SECTION;
	/* 
	** We need to make a copy of the type argument because
	** we can't put the formal argument in a declare section.
	*/
	char	b_type[TYPESIZE+1];
	EXEC SQL END DECLARE SECTION;

	/*
	** Declare a different error handler so that if HA event
	** occurs while we are trying to open the cursor, we will
	** retry the open.
	*/
	EXEC SQL WHENEVER SQLERROR CALL titlelist_error(type);

	/*
	** Copy the type into b_type so we can use it to declare
	** the cursor.
	*/
	strcpy(b_type, type);

	/*
	** This cursor selects title_id and title to display
	** a list for the user. The substring() function truncates
	** titles, if necessary, so they'll fit on an 80 column
	** display.
	*/
	EXEC SQL DECLARE titlelist CURSOR FOR
	SELECT	title_id, substring(title,1,64) 
	FROM	titles
	WHERE	type LIKE :b_type;

	/*
	** Open the cursor.
	*/
	EXEC SQL OPEN titlelist;

	return;
}

/*
**	Displays a list of title ids and titles that are
**	from the selected type. Lets the user enter a title id. 
**	Copies the entry into tid, and returns the number of 
**	characters entered.
*/
int
get_titleid(char *tid, char *type)
{
	/*
	** Declare destination variables for the fetch. 
	*/
	EXEC SQL BEGIN DECLARE SECTION;
	char	b_titleid[TIDSIZE+1];
	char	b_title[65];
	CS_INT	rowcount;
	CS_INT	currentcount;

	EXEC SQL END DECLARE SECTION;

	EXEC SQL WHENEVER SQLERROR CALL error_handler();

	/* A buffer to hold keyboard input. */
	char	buf[128];

	/*
	** Set b_titleid to null so we'll know if no titles
	** were selected for the specified type.
	*/
	b_titleid[0] = EOLN;

	/*
	** Open the title list cursor.
	*/
	open_titlelist(type);

	/* Display the list of titles */ 
	printf("\n\nSelect a title:\n\n");
	rowcount = 0;
	currentcount = 0;


	for (;;)
	{
		EXEC SQL FETCH titlelist INTO :b_titleid, :b_title;

		/*
		**  If we've failed over at this point, our
		**  error handler will have opened our type list
		**  cursor, but not the title list.
		*/
		if (sqlca.sqlcode == CS_RET_HAFAILOVER)
		{
			rowcount = 0;
			open_titlelist(type);
			continue;
		}
		/*
		** Reposition the cursor, if necessary
		*/
		if ( rowcount < currentcount)
		{
			rowcount++;
			continue;
		}

		if (sqlca.sqlcode == 100)
		{
			break;
		}

		rowcount++;
		currentcount++;
		printf("   %-8s %s\n", b_titleid, b_title);
	}

	/* If b_titleid is still null, no titles were found, so 
	** we won't ask the user to select one. 
	*/
	if (!strlen(b_titleid))
	{
		printf("\nThere are no '%s' titles.\n", type);
		tid[0] = EOLN;
	}
	else
	{
		/*
		** We use fgets() as the input function. A title_id is
		** only 6 bytes long but we'll have to read in the
		** complete input from stdin. Therefore, we'll fgets() into
		** a longer buffer, remove any possible "\n" at the end of
		** the string and copy 6 bytes into the tid variable.
                */
		printf("\ntitle ID? ");
		fgets(buf, sizeof(buf), stdin);
		if (buf[strlen(buf) - 1] == '\n')
		{
			buf[strlen(buf) - 1] = '\0';
		}
		strncpy(tid, buf, TIDSIZE);
		tid[TIDSIZE] = EOLN;
	}

	/*
	** Close the cursor and return the number of characters entered.
	*/
	EXEC SQL CLOSE titlelist;

	return (strlen(tid));	
}

void
author_error(char *titleid)
{
	/*
	**	at the very least, we should know to
	**	use our db. We also re-open the typelist.
	*/
	if (sqlca.sqlcode == CS_RET_HAFAILOVER)
	{
		usedb();
		open_authorlist(titleid);

		/*
		** We could also put the logic here to re-open the
		** author list and title list if we used a global
		** to track their current state.
		*/

		print_failover_msg(CS_TRUE);
		/*
		** put the CS_RET_HAFAILOVER error code back
		** as it was overwritten by the usedb() and
		** re opening the cursors.
		*/
		sqlca.sqlcode = CS_RET_HAFAILOVER;
		return;
	}

	/*
	** Not an HA problem, call the default error handler.
	*/
	error_handler();
}

void
open_authorlist(char *titleid)
{
	EXEC SQL BEGIN DECLARE SECTION;
	/*
	** Destination variables to declare cursors.
	*/
	char	m_titleid[7];
	EXEC SQL END DECLARE SECTION;

	/*
	** For defensive programming, one should validate the length
	** of titleid.
	*/
	strcpy(m_titleid, titleid);

	/*
	** Ensure that if HA event occurs while declaring author cursor,
	** it gets handled correctly.
	*/
	EXEC SQL WHENEVER SQLERROR CALL author_error(titleid);

	/*
	** There can be more than one author, so we need a cursor
	** to list them. We can let ASE combine the first
	** and last names so that only one column is returned.
	*/
	EXEC SQL DECLARE authors CURSOR FOR
	SELECT  au_fname + ' ' + au_lname 
	FROM	authors, titleauthor
	WHERE	authors.au_id = titleauthor.au_id
	AND	title_id = :m_titleid;

	/* Open the cursor. */
	EXEC SQL OPEN authors;

	if (sqlca.sqlcode == CS_RET_HAFAILOVER)
	{
		sqlca.sqlcode = 0;
	}

}

/*
** void show_book(typeid) 
**
**	Displays information about the book with the specified
**	title id. A book can have coauthors, so we need a cursor
**	to list the authors.
*/
void
show_book(char *titleid)
{
	EXEC SQL BEGIN DECLARE SECTION;
	/* Destination variables for fetches. */
	char	m_titleid[7];
	char	m_title[65];
	char	m_pubname[41];
	char	m_pubcity[21];
	char	m_pubstate[3];	
	char	m_notes[201];
	char	m_author[62];
	CS_INT	rowcount;
	CS_INT	currentcount;
	/* An indicator variable for notes. */ 
	short	i_notes;
	EXEC SQL END DECLARE SECTION;

	EXEC SQL WHENEVER SQLERROR CALL error_handler();

	/* These variables are used in the word-wrapping routine
	** that display the notes.
	*/
	char	*p, noteline[65];
	int	i, notelen;

	/* We need a local copy of the titleid to use in the
	** fetch.
	*/
	strcpy(m_titleid, titleid);	

	/*
	** Need a do-while loop here to handle failover during the select.
	** A possibility here would be a counter to avoid infinite failovers.
	*/
	do
	{
		/*
		** Fetch the information about the title and publisher. A
		** cursor is not needed because the title id is unique. 
		** No more than one row can be returned.
		*/
		EXEC SQL 
		SELECT	substring(title, 1, 64), notes, pub_name, city, state
		INTO	:m_title, :m_notes:i_notes, :m_pubname, :m_pubcity, :m_pubstate
		FROM	titles, publishers
		WHERE	titles.pub_id = publishers.pub_id 
		AND	title_id = :m_titleid;
	} while (sqlca.sqlcode == CS_RET_HAFAILOVER);

	/*
	** sqlca.sqlcode is set to 100 if the title_id isn't found.
	*/
	if (sqlca.sqlcode == 100) 
	{
		printf("\n\t** Can't find title '%s'.\n", m_titleid);
		return;
	}

	/* Display the title and publisher's name and address. */
	printf("\n\n%s", m_title);
	printf("\n\nPub:\t%s", m_pubname);
	printf("\n\t%s %s", m_pubcity, m_pubstate);

	/*
	** Open the cursor for authors
	*/
	open_authorlist(m_titleid);

	/* Each row fetched is an additional coauthor. */ 
	printf("\n\nBy:");
	rowcount = 0;
	currentcount = 0;
	if (sqlca.sqlcode == CS_RET_HAFAILOVER)
	{
		open_authorlist(m_titleid);
	}	
	for(;;)
	{
		EXEC SQL FETCH authors INTO :m_author;

		/*
		**  If we've failed over at this point, we need
		**  to tell the secondary server that we're 
		**  using the pubs2 database, redeclare and
		**  re-open our cursor and then
		** reissue the command that returned failed over.
		*/
		if (sqlca.sqlcode == CS_RET_HAFAILOVER)
		{
			rowcount = 0;
			open_authorlist(m_titleid);
			continue;
		}	
		/*
		** Reposition the cursor, if necessary
		*/
		if ( rowcount < currentcount)
		{
			rowcount++;
			continue;
		}

		if (sqlca.sqlcode == 100)
		{
			break;
		}

		rowcount++;
		currentcount++;
		printf("\t%s\n", m_author);
	}

	EXEC SQL CLOSE authors;

	/*
	** This routine prints the notes column on multiple lines
	** with word wrapping. Nothing is printed if the indicator
	** variable was set to -1.
	*/
	if (i_notes != -1)
	{
		register  char  *q;

		printf("\nNotes:");
		notelen = strlen(m_notes);
		p = m_notes;
		while (notelen > 64)
		{
			noteline[64] = EOLN;
			strncpy(noteline, p, 64);
			for (i = strlen(noteline), q = noteline + i; 
				i  &&  ! (*q == ' ' || *q == '\t'); 
				i--, q--);
			noteline[i] = EOLN;
			notelen -= strlen(noteline);
			printf("\t%s\n", noteline);
			p += (strlen(noteline)+1);
		}

		if (notelen)
		{
			printf("\t%s\n", p);
		}
	}

	/* Pause before returning so the user can read the display. */
	printf("\npress return...");
	getchar();
	return;
}

/*
** void error_handler()
** 
**	Displays error codes and numbers from the SQLCA and exits with
**	an ERREXIT status. 
*/
void
error_handler(void)
{
	/*
	**	at the very least, we should know to
	**	use our db.
	*/
	if (sqlca.sqlcode == CS_RET_HAFAILOVER)
	{
		usedb();

		/*
		** We could also put the logic here to re-open the
		** author list and title list if we used a global
		** to track their current state.
		*/

		print_failover_msg(CS_TRUE);
		/*
		** put the CS_RET_HAFAILOVER error code back
		** as it was overwritten by the usedb() and
		** re opening the cursors.
		*/
		sqlca.sqlcode = CS_RET_HAFAILOVER;
		return;
	}

	fprintf(stderr, "\n** SQLCODE=(%ld)", sqlca.sqlcode);

	if (sqlca.sqlerrm.sqlerrml)
	{
		fprintf(stderr, "\n** ASE Error ");
		fprintf(stderr, "\n** %s", sqlca.sqlerrm.sqlerrmc);
	}

	fprintf(stderr, "\n\n");

	exit(ERREXIT);
}

/*
** void warning_handler()
** 
**	Displays warning messages.
*/
void
warning_handler(void)
{

	if (sqlca.sqlwarn[1] == 'W')
	{
		fprintf(stderr, 
			"\n** Data truncated.\n");
	}

	if (sqlca.sqlwarn[3] == 'W')
	{
		fprintf(stderr, 
			"\n** Insufficient host variables to store results.\n");
	}	
	return;
}

/*
** void print_failover_msg()
** 
**	Displays informative message that failover has occured.
**	returns a normal return, as should any error routine catching this.
**	any other desired processing can be done in a similar function.
*/
void
print_failover_msg(CS_BOOL ha_success)
{
	if (ha_success)
	{
		fprintf(stderr, "\nHA Failover occurred\n");
		return;
	}
/*      
**      else - error handler will catch and print the failover msg.
*/
}

void
usedb(void)
{
	EXEC SQL USE pubs2;
}

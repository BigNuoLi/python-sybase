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
**	This example is a non-interactive query program that 
**	shows the user some actions executed by a scrollable,
**	semi-sensitive cursor. This serves as a demo for usage
**	of scrollable cursors in ESQL/C.
*/

#include <stdio.h>
#include "sybsqlex.h"

EXEC SQL INCLUDE SQLCA;

EXEC SQL BEGIN DECLARE SECTION;
#define TITLE_STRING	65
EXEC SQL END DECLARE SECTION;

void	error_handler();
void	warning_handler();
void	notfound_handler();

int
main(int argc, char *argv[])
{
	EXEC SQL BEGIN DECLARE SECTION;
	char	username[30];
	char	password[30];
	char	a_type[TITLE_STRING+1];
	EXEC SQL END DECLARE SECTION;

	EXEC SQL WHENEVER SQLERROR CALL error_handler();
	EXEC SQL WHENEVER SQLWARNING CALL warning_handler();
	EXEC SQL WHENEVER NOT FOUND CALL notfound_handler();

	strcpy(username, USER);
	strcpy(password, PASSWORD);

	EXEC SQL CONNECT :username IDENTIFIED BY :password;

	EXEC SQL USE pubs2;

	/*
	** Declare an insensitive scrollable cursor against the
	** titles table. 
	*/

	EXEC SQL DECLARE typelist SEMI_SENSITIVE SCROLL CURSOR FOR
	SELECT DISTINCT title FROM titles;

	EXEC SQL OPEN typelist;

	printf("\n==> Selecting the FIRST book Title:\n");

	/*
	** Fetch the first row in cursor resultset
	*/
	EXEC SQL FETCH FIRST FROM typelist INTO :a_type;

	printf("\n%s\n", a_type);

	/*
	** Fetch the last row in cursor resultset
	*/
	printf("\n==> Selecting the LAST book Title:\n");

	EXEC SQL FETCH LAST FROM typelist INTO :a_type;

	printf("\n%s\n", a_type);

	/*
	** Fetch the previous (PRIOR) row based on current
	** cursor position
	*/
	printf("\n==> Selecting the PREVIOUS book Title:\n");

	EXEC SQL FETCH PRIOR FROM typelist INTO :a_type;

	printf("\n%s\n", a_type);

	/*
	** Jump 5 rows back from current cursor position
	*/
	printf("\n==> Rewinding 5 STEPS through the Book selection...:\n");

	EXEC SQL FETCH RELATIVE -5 FROM typelist INTO :a_type;

	printf("\n%s\n", a_type);

	/*
	** Fetch the next row based on current cursor position
	*/
	printf("\n==> Selecting the NEXT book Title:\n");

	EXEC SQL FETCH NEXT FROM typelist INTO :a_type;

	printf("\n%s\n", a_type);

	/*
	** Jump out of the cursor result set. Note that this will
	** lead to a "no rows found" condition. There are only 18
	** rows in 'titles'.
	*/

	a_type[0] = '\0';

	printf("\n==> Jumping out of the resultset.\n");

	EXEC SQL FETCH ABSOLUTE 100 FROM typelist INTO :a_type;

	printf("\n%s\n", a_type);

	/* Close shop */
	EXEC SQL CLOSE typelist;

	printf("\n==> That's it for now.\n");

	EXEC SQL DISCONNECT DEFAULT;

	return(STDEXIT);
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
** void notfound_handler()
** 
**	Displays notfound messages.
*/
void
notfound_handler(void)
{

	if (sqlca.sqlcode == 100)
	{
		fprintf(stderr, "No rows found\n");
	}	
	return;
}

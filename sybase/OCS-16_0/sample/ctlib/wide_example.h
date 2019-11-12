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
** 	This is the header file that goes with the SAP
** 	Client-Library example programs. It is used in wide_*.c programs.
*/

/*
** Define symbolic names, constants, and macros
*/

#define RETURN_IF(a,b) if (a != CS_SUCCEED) \
	{fprintf(stdout,"error in %s\n",b); return a;}

#define EXIT_IF(a) if (a != CS_SUCCEED)\
	{fprintf(stdout,"FATAL error on line %d \n",__LINE__); exit(-1) ;}

/* Global variables */
#define MAX_COLUMN	10
#define MAX_COLSIZE     1024
#define MAX_COMMAND     1024

extern char row_buffer[MAX_COLUMN][MAX_COLSIZE] ;

extern CS_RETCODE init_db(CS_CONTEXT **);
extern CS_RETCODE connect_db(CS_CONTEXT *, CS_CONNECTION **, CS_CHAR *, CS_CHAR *);
extern CS_RETCODE cleanup_db(CS_CONTEXT *, CS_RETCODE);
extern CS_RETCODE handle_returns(CS_COMMAND *);
extern CS_RETCODE fetch_n_print(CS_COMMAND *);
extern CS_RETCODE bind_columns(CS_COMMAND *);
extern CS_RETCODE open_cursor(CS_COMMAND *);
extern CS_RETCODE single_row(CS_COMMAND *);


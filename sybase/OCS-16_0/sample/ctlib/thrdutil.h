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
** 	Header file which contains the defines and prototypes for the utility
** 	functions in thrdutil.c
*/

/*
** global variables
*/
extern CS_BOOL display_server_address;

/*****************************************************************************
** 
** defines and typedefs used 
** 
*****************************************************************************/

#ifndef MAX
#define MAX(X,Y)	(((X) > (Y)) ? (X) : (Y))
#endif

#ifndef MIN
#define MIN(X,Y)	(((X) < (Y)) ? (X) : (Y))
#endif

/*
** Maximum character buffer for displaying a column
*/
#define MAX_CHAR_BUF	1024

/*
** MEM_ALIGN_BOUNDARY - memory alignment boundary
*/
#define MEM_ALIGN_BOUNDARY	8

/*
** MEM_ALIGN_SIZE - Align size to nearest higher multiple 
** of MEM_ALIGN_BOUNDARY
*/
#define MEM_ALIGN_SIZE(blocksize)	((((blocksize)+MEM_ALIGN_BOUNDARY-1)/MEM_ALIGN_BOUNDARY)*MEM_ALIGN_BOUNDARY) 
/*
** Maximum size of memory block to be used during array binding
*/
#define MAX_MEM_BLOCK_SIZE	50000

/*
** EX_GET_COLUMNVALUE - calculate column value pointer given the row number
*/
#define EX_GET_COLUMN_VALUE(coldata, row, col, colfmt) \
	(coldata[col].value + ((row) * (colfmt[col].maxlength)))

/*
** EX_GLOBAL_PROTECT
**	Protect access to shared global by taking global lock
*/
#define EX_GLOBAL_PROTECT(Operation) 			\
	{ 					\
		(void)ex_take_global_mutex();	\
		Operation;			\
		(void)ex_release_global_mutex();\
	}

/*
** Query type
*/
#define EX_CURSOR_QUERY		1
#define EX_REGULAR_QUERY 	2

/*
** Define structure where row data is bound.
** Allow for column arrays - for array binding.
*/
typedef struct _ex_column_data
{
	CS_SMALLINT	*indicator;
	CS_CHAR		*value;
	CS_INT		*valuelen;
} EX_COLUMN_DATA;

/*
** Define thread task structure
**
**	threadnum - logical thread number ( 0 - n ) maintained by application
**	thread - thread pointer 
**	context - cs context structure used by the thread
**	query	- Query to be processed
**	dbname - database name for the query
**	operation - Query type
**	
*/
typedef struct _ex_task
{
	CS_INT		threadnum;
	CS_VOID		*thread;
	CS_CONTEXT	*context;
	CS_CHAR		*query;
	CS_CHAR		*dbname;
	CS_INT		operation;
} EX_TASK;

/*****************************************************************************
** 
** protoypes for all public functions 
** 
*****************************************************************************/
/* thrdutil.c */
extern CS_INT CS_PUBLIC ex_display_dlen PROTOTYPE((
	CS_DATAFMT *column
	));
extern CS_RETCODE CS_PUBLIC ex_display_header PROTOTYPE((
	CS_INT threadnum,
	CS_INT numcols,
	CS_DATAFMT columns[]
	));
extern CS_RETCODE CS_PUBLIC ex_display_column PROTOTYPE((
	CS_INT threadnum,
	CS_CONTEXT *context,
	CS_DATAFMT *colfmt,
	CS_VOID *data,
	CS_INT datalength,
	CS_INT indicator
	));
extern CS_VOID CS_PUBLIC ex_panic PROTOTYPE((
	CS_INT threadnum,
	char *msg
	));
extern CS_VOID CS_PUBLIC ex_error PROTOTYPE((
	CS_INT threadnum,
	char *msg
	));
extern CS_RETCODE CS_PUBLIC ex_clientmsg_cb PROTOTYPE((
	CS_CONTEXT *context,
	CS_CONNECTION *connection,
	CS_CLIENTMSG *errmsg
	));
extern CS_RETCODE CS_PUBLIC ex_servermsg_cb PROTOTYPE((
	CS_CONTEXT *context,
	CS_CONNECTION *connection,
	CS_SERVERMSG *srvmsg
	));
extern CS_RETCODE CS_PUBLIC ex_init PROTOTYPE((
	CS_INT	threadnum,
	CS_CONTEXT **context
	));
extern CS_RETCODE CS_PUBLIC ex_connect PROTOTYPE((
	CS_INT threadnum,
	CS_CONTEXT *context,
	CS_CONNECTION **connection,
	CS_CHAR *appname,
	CS_CHAR *username,
	CS_CHAR *password,
	CS_CHAR *server
	));
extern CS_RETCODE CS_PUBLIC ex_con_cleanup PROTOTYPE((
	CS_CONNECTION *connection,
	CS_RETCODE status
	));
extern CS_RETCODE CS_PUBLIC ex_ctx_cleanup PROTOTYPE((
	CS_CONTEXT *context,
	CS_RETCODE status
	));
extern CS_RETCODE CS_PUBLIC ex_execute_cmd PROTOTYPE((
	CS_CONNECTION *connection,
	CS_CHAR *cmdbuf
	));
extern CS_RETCODE CS_PUBLIC ex_fetch_data PROTOTYPE((
	CS_COMMAND *cmd
	));
extern CS_RETCODE CS_PUBLIC ex_create_db PROTOTYPE((
	CS_CONNECTION *connection,
	char *dbname
	));
extern CS_RETCODE ex_remove_db PROTOTYPE((
	CS_CONNECTION *connection,
	char *dbname
	));
extern CS_RETCODE ex_use_db PROTOTYPE((
	CS_CONNECTION *connection,
	char *dbname
	));
extern CS_RETCODE ex_get_threadnum PROTOTYPE((
	CS_CONTEXT *context,
	CS_CONNECTION *connection,
	CS_INT	*threadnum
	));
extern CS_RETCODE CS_PUBLIC ex_display_rows PROTOTYPE((
	CS_INT          threadnum,
	CS_INT		numrows,
	CS_INT		numcols,
	CS_DATAFMT      *colfmt,
	EX_COLUMN_DATA	*coldata
	));

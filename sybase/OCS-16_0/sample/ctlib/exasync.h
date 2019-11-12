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
** 	This is the header file which defines constants
** 	and data structures which are used in the async example program.
**
*/

#ifndef __EXASYNC_H__

#define __EXASYNC_H__

/*****************************************************************************
** 
** port specific defines used 
** 
*****************************************************************************/

#if SUNOS

extern void usleep PROTOTYPE((unsigned useconds));
extern int fputc PROTOTYPE((char c, FILE *stream));
extern int malloc_debug PROTOTYPE((int level));

#endif /* SUNOS */

/*****************************************************************************
** 
** generic defines used 
** 
*****************************************************************************/

#define ERROR_EXIT	(-1)

/*
** function callback id
*/
#define EX_ASYNC_QUERY	(CT_USER_FUNC + 1)

/*
** async handle
*/
#define EX_ASYNC	CS_VOID

/*****************************************************************************
** 
** data structures used 
** 
*****************************************************************************/

/*
** result output area created by library routine
*/
typedef struct _ex_results
{
	CS_INT		numrows;	/* number of rows is result set */
	CS_INT		numcols;	/* number of columns is result set */
	CS_DATAFMT	*colfmts;	/* column formats used */
	CS_BYTE		**data;		/* column data is stored here */
	CS_INT		**datalen;	/* data lengths are stored here */
	CS_INT		**indicator;	/* indicators are stored here */
} EX_RESULTS;

/*****************************************************************************
** 
** functions used
** 
*****************************************************************************/

/* ex_alib.c */
extern CS_RETCODE CS_PUBLIC ex_async_query PROTOTYPE((
	EX_ASYNC *ex_handle,
	CS_CHAR *query,
	CS_INT	querylen
	));
extern CS_RETCODE CS_PUBLIC ex_async_alloc PROTOTYPE((
	CS_CONNECTION *connection,
	EX_RESULTS *results,
	CS_VOID *mempool,
	CS_INT maxpool,
	EX_ASYNC **ex_handle
	));
extern CS_RETCODE CS_PUBLIC ex_async_free PROTOTYPE((
	EX_ASYNC *ex_handle
	));

#endif /* __EXASYNC_H__ */

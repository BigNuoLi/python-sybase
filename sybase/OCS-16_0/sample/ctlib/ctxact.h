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
**	This file contains header files for the two phase commit
**	functions in ctxact.c.
**
*/

/*
** SAP header files required by Open Client
*/

#include	<ctpublic.h>

/*
** Function Prototypes
*/
CS_RETCODE 	open_commit 	PROTOTYPE((
		CS_CONNECTION	*conn,
		CS_CHAR		*service,
		CS_INT		service_len
		));
CS_RETCODE 	close_commit	PROTOTYPE((
		CS_CONNECTION	*conn
		));
CS_RETCODE 	build_xact_string PROTOTYPE((
		CS_CHAR		*xact_name,
		CS_INT		xact_len,
		CS_CHAR		*service_name,
		CS_INT		service_len,
		CS_INT		commid,
		CS_CHAR		*result,
		CS_INT		len,
		CS_INT		*outlen
		));
CS_INT 		start_xact 	PROTOTYPE((
		CS_CONNECTION	*conn,
		CS_CHAR		*app_name,
		CS_INT		app_len,
		CS_CHAR		*xact_name,
		CS_INT		xact_len,
		CS_INT		count
		));
CS_RETCODE 	commit_xact	PROTOTYPE((
		CS_CONNECTION	*conn,
		CS_INT		commid
		));
CS_RETCODE 	abort_xact	PROTOTYPE((
		CS_CONNECTION	*conn,
		CS_INT		commid
		));
CS_RETCODE 	remove_xact	PROTOTYPE((
		CS_CONNECTION	*conn,
		CS_INT		commid,
		CS_INT		n
		));
CS_RETCODE 	scan_xact	PROTOTYPE((
		CS_CONNECTION	*conn,
		CS_INT		commid
		));
CS_INT		stat_xact	PROTOTYPE((
		CS_CONNECTION	*conn,
		CS_INT		commid
		));


CS_RETCODE	pr_head		PROTOTYPE((
		CS_COMMAND	*cmd
		));
CS_RETCODE	pr_line		PROTOTYPE((
		CS_COMMAND	*cmd
		));
CS_RETCODE	pr_row		PROTOTYPE((
		CS_COMMAND	*cmd
		));

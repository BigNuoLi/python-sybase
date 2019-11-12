/*
** SAP Open Client 
** ---------------
**
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
*/

/*
** thrdfunc.h
**
** Header file which contains the defines and prototypes for the utility
** functions in thrdfunc.c
**
*/

#ifndef	__THRDFUNC_H__
#define __THRDFUNC_H__

/* 
** prototypes for thread and synchronization routines 
*/
extern CS_RETCODE  ex_create_mutex  PROTOTYPE((
	CS_VOID **mutex
	));
extern CS_RETCODE  ex_take_mutex  PROTOTYPE((
	CS_VOID *mutex
	));
extern CS_RETCODE  ex_release_mutex  PROTOTYPE((
	CS_VOID *mutex
	));
extern CS_RETCODE  ex_delete_mutex  PROTOTYPE((
	CS_VOID *mutex
	));

extern CS_RETCODE  ex_create_sem PROTOTYPE((
	CS_VOID **semaphore,
	CS_UINT count
	));
extern CS_RETCODE  ex_release_sem PROTOTYPE((
	CS_VOID *semaphore
	));
extern CS_RETCODE  ex_waitfor_sem PROTOTYPE((
	CS_VOID *semaphore
	));
extern CS_RETCODE  ex_delete_sem PROTOTYPE((
	CS_VOID *semaphore
	));

extern CS_RETCODE  ex_create_thread PROTOTYPE((
        CS_VOID **thread,
        CS_VOID *startRoutine,
        CS_VOID *arg
        ));
extern CS_RETCODE  ex_detach_thread PROTOTYPE((
        CS_VOID *thread
	));
extern CS_VOID  ex_yield PROTOTYPE((
	));

extern CS_RETCODE  ex_create_global_mutex PROTOTYPE((
	CS_VOID
	));
extern CS_RETCODE  ex_delete_global_mutex PROTOTYPE((
	CS_VOID
	));
extern CS_RETCODE  ex_take_global_mutex PROTOTYPE((
	CS_VOID
	));
extern CS_RETCODE  ex_release_global_mutex PROTOTYPE((
	CS_VOID
	));

#endif /* __THRDFUNC_H__ */

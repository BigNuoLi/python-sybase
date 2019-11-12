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
**  	Thread and synchronization routines for NT threads. 
**
**
*/
 

#include 	<windows.h>
#include        <stdio.h>
#include 	<stdlib.h>
#include        <string.h>
#include        <errno.h>
#include        <process.h>
#include        <ctpublic.h>
#include        <thrdfunc.h>
 
/* 
** global mutex 
*/
CS_STATIC CS_VOID *Ex_global_mutex;

/*
** ex_create_mutex()
**
** Type of function:
**	USER API.
**
** Purpose:
**	Creates a mutex . 
**
** Returns:
**	CS_SUCCEED	If the mutex was created
**	CS_FAIL	The mutex failed.
**
** Side effects:
**
** History:
*/

CS_RETCODE 
ex_create_mutex(CS_VOID **mutex)
{
	HANDLE *handle;

	if((handle = (HANDLE *)malloc(CS_SIZEOF(HANDLE))) == (HANDLE)NULL)
		return CS_FAIL;

	if((*handle = CreateMutex(NULL, FALSE, NULL)) == (HANDLE)NULL)
	{
		free(handle);
		return CS_FAIL;
	}

	*(HANDLE **)mutex = handle;

	return CS_SUCCEED;
}

/*
** ex_take_mutex()
**
** Type of function:
**	USER API.
**
** Purpose:
**	Waits for mutex control.  This function blocks until the thread
**	currently holding the mutex gives it up.  
**
** Returns:
**	CS_SUCCEED	If the thread has control of the mutex
**	CS_FAIL	If the control cannot be obtained, or an error.
**
** Side effects:
**
** History:
*/

CS_RETCODE 
ex_take_mutex(CS_VOID *mutex)
{
	DWORD retcode;
	
	if((retcode =
		WaitForSingleObject(*(HANDLE *)mutex, INFINITE))
			!= WAIT_OBJECT_0)
	{
		return CS_FAIL ;
	}

	return CS_SUCCEED;
}

/*
** ex_release_mutex()
**
** Type of function:
**	USER API.
**
** Purpose:
**	Releases control of the mutex, so other threads can grab it.
**
** Returns:
**	CS_SUCCEED	If the thread gave up control
**	CS_FAIL	If the control cannot be released, or an error.
**
** Side effects:
**
** History:
*/

CS_RETCODE 
ex_release_mutex(CS_VOID *mutex)
{
	if(ReleaseMutex(*(HANDLE *)mutex) != TRUE)
		return CS_FAIL;

	return CS_SUCCEED;
}

/*
** ex_delete_mutex
**
** Type of function:
**	USER API.
**
** Purpose:
**	This function deletes the mutex from the system.
**	Any threads waiting on this object will return failure after this.
**
** Returns:
**	CS_SUCCEED	If the mutex was successfully deleted
**	CS_FAIL		If the semaphore was successfully deleted.
**
** Side effects:
**
** History:
*/

CS_RETCODE 
ex_delete_mutex(CS_VOID *mutex)
{
	if(CloseHandle(*(HANDLE *)mutex) != TRUE)
		return CS_FAIL;

	free(mutex);

	return CS_SUCCEED;
}

/*
** ex_create_sem()
**
** Type of function:
**      Internal USER API.
**
** Purpose:
**	Create a semaphore.
**
** Returns:
**      CS_SUCCEED     	If semaphore created successfully 
**      CS_FAIL         If there were errors. 
**
** Side effects:
**
** History:
*/
CS_RETCODE 	
ex_create_sem(CS_VOID **sempp, CS_UINT count)
{
	HANDLE		*handle;

	/* Initialize */
	*sempp = (CS_VOID *)NULL;

	if ((handle =  (HANDLE *)malloc(CS_SIZEOF(HANDLE))) == (HANDLE *)NULL)
	{
		return(CS_FAIL);
	}

	if ((*handle = CreateSemaphore(NULL, 0, count, NULL)) == (HANDLE)NULL)
	{
		free(handle);
		return(CS_FAIL);
	}

	*sempp = handle;
	return CS_SUCCEED;
}

/*
** ex_delete_sem()
**
** Type of function:
**      Internal USER API.
**
** Purpose:
**      Delete a semaphore.
**
** Returns:
**      CS_SUCCEED      Always.  
**
** Side effects:
**
** History:
*/
CS_RETCODE  
ex_delete_sem(CS_VOID *semp)
{
	HANDLE handle;

	/* Initialize */
	handle = *((HANDLE *)semp);

	/* Get the real free function  and free the handle */
	free(semp);
	CloseHandle(handle);
	return CS_SUCCEED;
}

/*
** ex_release_sem()
**
** Type of function:
**      Internal USER API.
**
** Purpose:
**      Release the semaphore. Increments semaphore count internally and
**	wakes up if a process is waiting on this semaphore.
**
** Returns:
**      CS_SUCCEED      If semaphore released successfully
**      CS_FAIL         If there were errors.
**
** Side effects:
**
** History:
*/
CS_RETCODE 
ex_release_sem(CS_VOID *semp)
{
	HANDLE handle;

	/* Initialize */
	handle = *((HANDLE *)semp);
	if (ReleaseSemaphore(handle, 1, NULL) != TRUE)
	{
		return CS_FAIL;
	}
	return CS_SUCCEED;
}

/*
** ex_waitfor_sem()
**
** Type of function:
**      Internal USER API.
**
** Purpose:
**     	Wait for the semaphore. 
**
** Returns:
**      CS_SUCCEED      If semaphore was obtained successfully. 
**      CS_FAIL         If there were errors.
**
** Side effects:
**
** History:
*/
CS_RETCODE 
ex_waitfor_sem(CS_VOID	*semp)
{
	HANDLE handle;
	DWORD retcode;

	/* Initialize */
	handle = *((HANDLE *)semp);

	if ((retcode = WaitForSingleObject(handle, INFINITE)) == WAIT_TIMEOUT)
	{
		return CS_TIMED_OUT;
	}
	else if (retcode == WAIT_OBJECT_0)
	{
		return CS_SUCCEED;
	}
	
	return CS_FAIL;
}



/*
** ex_detach_thread()
**
** Type of function:
**      Internal USER API.
**
** Purpose:
**	Detach thread internal resources. Thread object is not accessible
**	any more after this call is completed.
**
** Returns:
**      CS_SUCCEED      If thread resources are released successfully 
**      CS_FAIL         If there were errors.
**
** Side effects:
**
** History:
*/
CS_RETCODE  
ex_detach_thread(CS_VOID *threadp)
{
	HANDLE handle;

	/* Initialize */
	handle = *((HANDLE *)threadp);

	/* Get the real free function  and free the handle */
	free(threadp);

	return CS_SUCCEED;
}

/*
** ex_create_thread()
**
** Type of function:
**      Internal USER API.
**
** Purpose:
**	Create a thread with default stack size.
**
** Returns:
**      CS_SUCCEED      If thread was created successfully
**      CS_FAIL         If there were errors.
**
** Side effects:
**
** History:
*/
CS_RETCODE 
ex_create_thread(CS_VOID **threadpp, CS_VOID *startroutine, CS_VOID *args)
{
	HANDLE		*handle;
 
        /* Initialize */
        *threadpp = (CS_VOID *)NULL;
 
        if ((handle =  (HANDLE *)malloc(CS_SIZEOF(HANDLE))) == (HANDLE *)NULL)
        {
                return(CS_FAIL);
        }
 
	/*
	** start thread with default stack - On NT, this is same size as that 
	** of primary thread and grows as necessary
	*/ 
        if ((*handle = (HANDLE)_beginthread(startroutine, 0, (LPVOID)args)) 
				== (HANDLE)NULL)
        {
                free(handle);
                return(CS_FAIL);
        }
 
        *threadpp = handle;
        return CS_SUCCEED;
}

/*
** ex_yield()
**
** Type of function:
**      Internal USER API.
**
** Purpose:
**	Relinquish rest of cpu time slice to other threads.
**
** Returns:
**	None.
**
** Side effects:
**
** History:
*/
CS_VOID  
ex_yield(void)
{
	Sleep(0);
	return ;
}

/*
** ex_take_global_mutex()
**
**      Take global mutex
**
** Returns:
**      CS_SUCCEED 
**	CS_FAIL
**
** Side Effects:
**      None
*/
CS_RETCODE
ex_take_global_mutex(void)
{
	return(ex_take_mutex(Ex_global_mutex));
}
 
/*
** ex_release_global_mutex()
**
**      Release global mutex
**
** Returns:
**      CS_SUCCEED 
**      CS_FAIL
**
** Side Effects:
**      None
*/
CS_RETCODE
ex_release_global_mutex(void)
{
	return(ex_release_mutex(Ex_global_mutex));
}
 
/*
** ex_create_global_mutex()
**
**      Setup global mutex mechanism
**
** Returns:
**      CS_SUCCEED 
**      CS_FAIL
**
** Side Effects:
**      None
*/
CS_RETCODE
ex_create_global_mutex(void)
{
	if (Ex_global_mutex == (CS_VOID *)NULL)
	{
		return(ex_create_mutex(&Ex_global_mutex));
	}
	else
	{
		return CS_SUCCEED;
	}
}

/*
** ex_delete_global_mutex()
**
**      Delete global mutex
**
** Returns:
**      CS_SUCCEED 
**      CS_FAIL
**
** Side Effects:
**      None
*/
CS_RETCODE
ex_delete_global_mutex(void)
{
	CS_RETCODE ret;
	ret = ex_delete_mutex(Ex_global_mutex);
	Ex_global_mutex = (CS_VOID *)NULL;
	return ret;
}


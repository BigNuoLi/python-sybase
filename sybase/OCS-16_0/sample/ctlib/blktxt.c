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
** 	This example program uses the bulk copy routines to copy
** 	static data to a server table. There are three rows of data
** 	that are bound to program variables and then sent to the
** 	server as a batch. The rows are again sent using textxfer
** 	to send the text data.
**
** Routines Used
** -------------
** 	blk_alloc	
** 	blk_init
** 	blk_bind
** 	blk_rowxfer
** 	blk_textxfer
** 	blk_done	with	CS_BATCH_ALL option
** 	blk_done	with	CS_BLK_ALL option
** 	blk_drop
**
** Input
** -----
**
**
** Output
** ------
** 	The number of rows transferred by the bulk copy is reported.
** 	The server table named is updated.
**
** Server Dependencies
** -------------------
**
**
** Server Version
** --------------
**
**
** Server Tables
** -------------
**
**
** Algorithm
** ----------
** 	Initialize the CT context and connection handles.
** 	Allocate a bulk copy descriptor
** 	Initialize the bulk copy operation, IN direction
** 	Bind the program variables to the server table columns
** 	For each line read from the static data, build and send a
** 	data row to the server. Send the data to the server as a
** 	batch.
** 	Clear all the binds to program variables.
** 	Send the rows again using binding and textxfer.
** 	Close the bulk copy operation
** 	Drop the bulk copy descriptor
** 	Clear out all the  context and connection handles
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctpublic.h>
#include <bkpublic.h>
#include "example.h"
#include "exutils.h"


/*****************************************************************************
** 
** defines and globals used.
** 
*****************************************************************************/
/*
** Global names used in this module
*/
CS_CHAR *Ex_appname  = "blk_sample";
CS_CHAR *Ex_dbname   = "sampledb";
CS_CHAR	*Ex_tabname  = "exblkin";	/* table on the server */
CS_CHAR *Ex_server   = EX_SERVER;
CS_CHAR *Ex_username = EX_USERNAME;
CS_CHAR *Ex_password = EX_PASSWORD;


/*
** Defines for the table used in the sample program
*/
#define	MAX_PUBID	5
#define	MAX_PUBNAME	41
#define MAX_PUBCITY	21
#define	MAX_PUBST	3
#define MAX_BIO         255
#define	MAXLEN		255
#define	DATA_END	(0)

typedef	struct	_blk_data
{
	CS_INT		pub_id;
	CS_CHAR		pub_name[MAX_PUBNAME];
	CS_CHAR		pub_city[MAX_PUBCITY];
	CS_CHAR		pub_st[MAX_PUBST];
	CS_TEXT         pub_bio[MAX_BIO];
} Blk_Data;


/*
** static definition of table use in the example
*/

static	Blk_Data BLKDATA[] =
{
	{22,	"Taylor & Ng",	"San Francisco",	"CA",
	 "So I borrowed a name!!"},
	{44,	"Scarey Books",	"Sleepy Hollow",	"MA",
	 "Founder was the Headless horseman."},
	{66,	"Witch Craft & Spells",	"Salem",	"MA",
	 "Descendent of Joan of Arc."},
	{DATA_END,	"",	"",	""},
};


/*
** Prototypes for routines in sample code
*/ 
CS_STATIC CS_RETCODE EstablishConnection PROTOTYPE((
	CS_CONTEXT *context,		
	CS_CONNECTION **connection
	));
CS_STATIC CS_RETCODE CreateDatabase PROTOTYPE((
        CS_CONNECTION *connection
	));
CS_STATIC CS_RETCODE CreateTable PROTOTYPE((
        CS_CONNECTION *connection
	));
CS_STATIC CS_INT BulkCopyIn PROTOTYPE((
        CS_CONNECTION *connection
        ));

/*
** main()
** 
** Purpose:
**	Entry point for example program.
** 
** Parameters:
**	None, argc and argv will not be used.
**
** Return:
** 	EX_EXIT_ERROR  or EX_EXIT_SUCCEED
**
*/

int
main(int argc, char *argv[])
{
	CS_CONTEXT	*context = NULL;
	CS_CONNECTION	*connection = NULL;
	CS_RETCODE	retcode;
	
	EX_SCREEN_INIT();

	fprintf(stdout, "Bulk Copy Example\n");
	fflush(stdout);

	/* 
	** Allocate a context structure and initialize Client-Library 
	*/
	retcode = ex_init(&context);
	if (retcode != CS_SUCCEED)
	{
		ex_panic("ex_init failed");
	}

	/* 
	** Allocate a connection structure, set its properties, and  
	** establish a connection. Since the blk example needs to define
	** additional properties before login, it uses its own connection
	** routine.
	*/
	retcode = EstablishConnection(context, &connection);

	/*
	** Create a database for the sample program and change to it.
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = CreateDatabase(connection);
	}

	/*
	** Create a table for the bulk copy in
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = CreateTable(connection);
	}

	/*
	** Execute the routines for the bulk copy in sample
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = BulkCopyIn(connection);

		/*
		** Remove the database that was created.  The error
		** code (if not succeed), will be used in the cleanup
		** routines.
		*/
		if (retcode == CS_SUCCEED)
		{
			retcode = ex_remove_db(connection, Ex_dbname);
		}
		else
		{
			(void)ex_remove_db(connection, Ex_dbname);
		}
	}

	/*
	** Deallocate the allocated structures, close the connection,
	** and exit Client-Library.
	*/
	if (connection != NULL)
	{
		retcode = ex_con_cleanup(connection, retcode);
	}
	
	if (context != NULL)
	{
		retcode = ex_ctx_cleanup(context, retcode);
	}

	return (retcode == CS_SUCCEED) ? EX_EXIT_SUCCEED : EX_EXIT_FAIL;
}

/*
** EstablishConnection()
**
** Type of function:
** 	blktxt program internal api
**
** Purpose:
** 	This routine establishes a connection to the server identified
** 	in example.h and sets the CS_USER, CS_PASSWORD, and 
** 	CS_APPNAME properties for the connection. 
**
** 	NOTE: The username, password, and server are defined
** 	in the example header file.
**
** 	If a connection property is NULL, it is not set.
**
** 	If this function should fail, it will deallocated all memory 
** 	allocations it has done.
**
** Parameters:
** 	context		- Pointer to CS_CONTEXT structure.
** 	connection 	- Pointer to CS_CONNECTION pointer.
**
** Return:
** 	Result of function calls from CT-Lib.
*/

CS_STATIC CS_RETCODE
EstablishConnection(CS_CONTEXT *context, CS_CONNECTION **connection)
{
        CS_INT		len;
	CS_RETCODE	retcode;
        CS_BOOL 	bool;

	/* 
	** Allocate a connection structure. 
	*/
	retcode = ct_con_alloc(context, connection);
	if (retcode != CS_SUCCEED)
	{
		ex_error("ct_con_alloc failed");
		return retcode;
	}

	/*	
	** If a username is defined in example.h, set the CS_USERNAME property.
	*/
	if (retcode == CS_SUCCEED && Ex_username != NULL)
	{
		if ((retcode = ct_con_props(*connection, CS_SET, CS_USERNAME, 
				Ex_username, CS_NULLTERM, NULL)) != CS_SUCCEED)
		{
			ex_error("ct_con_props(username) failed");
		}
	}

	/*	
	** If a password is defined in example.h, set the CS_PASSWORD property.
	*/
	if (retcode == CS_SUCCEED && Ex_password != NULL)
	{
		if ((retcode = ct_con_props(*connection, CS_SET, CS_PASSWORD, 
				Ex_password, CS_NULLTERM, NULL)) != CS_SUCCEED)
		{
			ex_error("ct_con_props(passwd) failed");
		}
	}

	/*	
	** Set the CS_APPNAME property.
	*/
	if (retcode == CS_SUCCEED && Ex_appname != NULL)
	{
		if ((retcode = ct_con_props(*connection, CS_SET, CS_APPNAME, 
				Ex_appname, CS_NULLTERM, NULL)) != CS_SUCCEED)
		{
			ex_error("ct_con_props(appname) failed");
		}
	}

	/*
	** Enable the bulk login property
	*/
	if (retcode == CS_SUCCEED)
	{
		bool = CS_TRUE;
		retcode = ct_con_props(*connection, CS_SET, CS_BULK_LOGIN,
					  (CS_VOID *)&bool, CS_UNUSED, NULL);
		if (retcode != CS_SUCCEED)
		{
			 ex_error("ct_con_props(bulk_login) failed");
		}
	}

	/*	
	** Open a Server connection.
	*/
	if (retcode == CS_SUCCEED)
	{
		len = (Ex_server == NULL) ? 0 : CS_NULLTERM;
		retcode = ct_connect(*connection, Ex_server, len);
		if (retcode != CS_SUCCEED)
		{
			ex_error("ct_connect failed");
		}
	}

	if (retcode != CS_SUCCEED)
	{
		ct_con_drop(*connection);
		*connection = NULL;
	}
	return retcode;
}

/*
** CreateDatabase()
**
** Type of function:
** 	blktxt program internal api
**
** Purpose:
** 	This routine uses ex_create_db() to create a database. It also enables
** 	the ability to bulkcopy into the newly created database.
**
** Parameters:
** 	connection	- Pointer to CS_CONNECTION structure.
**
** Return:
**	CS_SUCCEED if database was created.
**	Otherwise a Client-Library failure code.
*/

CS_STATIC CS_RETCODE 
CreateDatabase(CS_CONNECTION *connection)
{
	CS_CHAR         *cmdbuf;
	CS_RETCODE	retcode;

	/*
	** create database
	*/
	retcode = ex_create_db(connection, Ex_dbname);
	if (retcode != CS_SUCCEED)
	{
		ex_error("CreateDatabase: ex_create_db() failed");
		return retcode;
	}

	/*
	** Allocate the buffer for the command string.
	*/
	cmdbuf = (CS_CHAR *) malloc(EX_BUFSIZE);
	if (cmdbuf == (CS_CHAR *)NULL)
	{
		ex_error("CreateDatabase: malloc() failed");
		return CS_FAIL;
	}


	/*
	** Enable bulkcopy into the new database
	*/
	sprintf(cmdbuf, "exec sp_dboption %s, \"select into/bulkcopy\", true",
		                  Ex_dbname);
        if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
        {
                ex_error("CreateDatabase: ex_execute_cmd(sp_dboption) failed");
		free (cmdbuf);
		return retcode;
	}

	/*
	** connect to the new database.
	*/
        if ((retcode = ex_use_db(connection, Ex_dbname)) != CS_SUCCEED)
        {
                ex_error("CreateDatabase: ex_use_db(Ex_dbname) failed");
		free (cmdbuf);
                return retcode;
        }

	/*
	** Checkpoint the server
	*/
	sprintf(cmdbuf, "checkpoint");
        if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
        {
                ex_error("CreateDatabase: ex_execute_cmd(checkpoint) failed");
	}
	free(cmdbuf);	
	return retcode;
}

/*
** CreateTable()
**
** Type of function:
** 	blktxt program internal api
**
** Purpose:
** 	This routine sets up the table for testing - if the table
**	already exists, it first deletes it.
**
** Parameters:
** 	connection	- Pointer to CS_CONNECTION structure.
**
** Return:
**	CS_SUCCEED if table was created.
**	Otherwise a Client-Library failure code.
*/

CS_STATIC CS_RETCODE 
CreateTable(CS_CONNECTION *connection)
{
	CS_RETCODE	retcode;
	CS_CHAR		*cmdbuf;

	cmdbuf = (CS_CHAR *) malloc(EX_BUFSIZE);
	if (cmdbuf == (CS_CHAR *)NULL)
	{
		ex_error("CreateTable: malloc() failed");
		return CS_MEM_ERROR;
	}

	/*
	** set up the command to send to the server checking for
	** the the table.
	*/
	sprintf(cmdbuf, "if exists (select name from sysobjects \
			where name = \"%s\")\
				 drop table %s",
					Ex_tabname, Ex_tabname);
        if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
        {
                ex_error("CreateTable: ex_execute_cmd(drop table) failed");
		free (cmdbuf);
		return retcode;
	}
		
	/*
	** create the table with a column of each kind of data -
	*/
	sprintf(cmdbuf, "create table %s\
                       (pub_id char(4) not null,\
                        pub_name varchar(40) null,\
                        city varchar(20) null,\
                        state char(2) null,\
                        bio text not null)", Ex_tabname);
	
        if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
        {
                ex_error("CreateTable: ex_execute_cmd(create table) failed");
	}
	free(cmdbuf);
	return retcode;
}

/*
** BulkCopyIn()
**
** Type of function:
** 	blktxt program internal api
**
** Purpose:
** 	This routine is the main driver for doing the bulk copy operation.
** 	It assumes that the database and tables have been set up.
**
** Parameters:
** 	connection	- Pointer to CS_CONNECTION structure.
**
** Return:
*/

CS_STATIC CS_RETCODE 
BulkCopyIn(CS_CONNECTION *connection)
{
	CS_BLKDESC	*blkdesc;
    	CS_DATAFMT	datafmt;	/* hold the variable descriptions  */
	Blk_Data 	*dptr;		/* point to the data for transfer */
	CS_INT		datalen[5];	/* the length of the variable data */
	CS_INT          len;
	CS_INT		numrows;
        CS_INT          buffer=CS_TRUE ;


	/*
	** Ready to start the bulk copy in now that all the connections
	** have been made and have a table name.
	**
	** Start by getting the bulk descriptor and starting the initialization
	*/
	if (blk_alloc(connection, EX_BLK_VERSION, &blkdesc) != CS_SUCCEED)
	{
		ex_error("BulkCopyIn: blk_alloc() failed");
		return CS_FAIL;
	}

        if (blk_props(blkdesc,CS_SET,ARRAY_INSERT,&buffer,CS_UNUSED,NULL) == CS_FAIL)
        {
                ex_error("BulkCopyIn: blk_props() failed");
                return (CS_FAIL);
        }
	if (blk_init(blkdesc, CS_BLK_IN,
				 Ex_tabname, strlen(Ex_tabname)) == CS_FAIL)
	{
		ex_error("BulkCopyIn: blk_init() failed");
		return CS_FAIL;
	}


	/*
	** Now to bind the variables to the columns and transfer the data
	*/
        datafmt.locale = 0;
	datafmt.count = 1;
	dptr = BLKDATA;
	while (dptr->pub_id != DATA_END)
	{
		datafmt.datatype = CS_INT_TYPE;
		datafmt.maxlength = sizeof(CS_INT);
		datalen[0] = CS_UNUSED;

		if (blk_bind(blkdesc, 1, &datafmt, &dptr->pub_id, 
					&datalen[0], NULL) != CS_SUCCEED)
		{
			ex_error("BulkCopyIn: blk_bind(1) failed");
			return CS_FAIL;
		}
		datafmt.datatype = CS_CHAR_TYPE;
		datafmt.maxlength = MAX_PUBNAME - 1;
		datalen[1] = strlen(dptr->pub_name);
		if (blk_bind(blkdesc, 2, &datafmt, (CS_VOID *)dptr->pub_name,
					 &datalen[1], NULL) != CS_SUCCEED)
		{
			ex_error("BulkCopyIn: blk_bind(2) failed");
			return CS_FAIL;
		}
		datafmt.maxlength = MAX_PUBCITY - 1;
		datalen[2] = strlen(dptr->pub_city);
		if (blk_bind(blkdesc, 3, &datafmt, (CS_VOID *)dptr->pub_city,
					 &datalen[2], NULL) != CS_SUCCEED)
		{
			ex_error("BulkCopyIn: blk_bind(3) failed");
			return CS_FAIL;
		}
		datafmt.maxlength = MAX_PUBST - 1;
		datalen[3] = strlen(dptr->pub_st);
		if (blk_bind(blkdesc, 4, &datafmt, (CS_VOID *)dptr->pub_st,
					 &datalen[3], NULL) != CS_SUCCEED)
		{
			ex_error("BulkCopyIn: blk_bind(4) failed");
			return CS_FAIL;
		}
		datafmt.maxlength = MAX_BIO - 1;
		datalen[4] = strlen((char *)dptr->pub_bio);
		if (blk_bind(blkdesc, 5, &datafmt, (CS_VOID *)dptr->pub_bio,
					 &datalen[4], NULL) != CS_SUCCEED)
		{
			ex_error("BulkCopyIn: blk_bind(5) failed");
			return CS_FAIL;
		}
		if (blk_rowxfer (blkdesc) == CS_FAIL)
		{
			ex_error("BulkCopyIn: blk_rowxfer() failed");
			return CS_FAIL;
		}
		dptr++;
	}

	/*
        ** Send these rows over as a batch
	*/	
	if (blk_done(blkdesc, CS_BLK_BATCH, &numrows) == CS_FAIL)
	{
		ex_error("BulkCopyIn: blk_done() failed");
		return CS_FAIL;
	}
	fprintf(stderr, "Number of data rows transferred = %d.\n", numrows);
	fflush(stderr);

	/*
	** Now let's try the whole thing again with textxfer -
	** first get rid of all the previous binds.
        */
	if (blk_bind(blkdesc, CS_UNUSED, NULL, NULL,
		             NULL, NULL) == CS_FAIL)
        {
	        ex_error("BulkCopyIn: blk_bind(clear) failed");
		return CS_FAIL;
        }
	dptr = BLKDATA;
	while (dptr->pub_id != DATA_END)
	{
		datafmt.datatype = CS_INT_TYPE;
		datafmt.count = 1;
		datafmt.maxlength = sizeof(CS_INT);
		datalen[0] = CS_UNUSED;

		if (blk_bind(blkdesc, 1, &datafmt, (CS_VOID *)&dptr->pub_id, 
					&datalen[0], NULL) != CS_SUCCEED)
		{
			ex_error("BulkCopyIn: blk_bind(1) failed");
			return CS_FAIL;
		}
		datafmt.datatype = CS_CHAR_TYPE;
		datafmt.maxlength = MAX_PUBNAME - 1;
		datalen[1] = strlen(dptr->pub_name);
		if (blk_bind(blkdesc, 2, &datafmt, (CS_VOID *)dptr->pub_name,
					 &datalen[1], NULL) != CS_SUCCEED)
		{
			ex_error("BulkCopyIn: blk_bind(2) failed");
			return CS_FAIL;
		}
		datafmt.maxlength = MAX_PUBCITY - 1;
		datalen[2] = strlen(dptr->pub_city);
		if (blk_bind(blkdesc, 3, &datafmt, (CS_VOID *)dptr->pub_city,
					 &datalen[2], NULL) != CS_SUCCEED)
		{
			ex_error("BulkCopyIn: blk_bind(3) failed");
			return CS_FAIL;
		}
		datafmt.maxlength = MAX_PUBST - 1;
		datalen[3] = strlen(dptr->pub_st);
		if (blk_bind(blkdesc, 4, &datafmt, (CS_VOID *)dptr->pub_st,
					 &datalen[3], NULL) != CS_SUCCEED)
		{
			ex_error("BulkCopyIn: blk_bind(4) failed");
			return CS_FAIL;
		}
		datafmt.datatype = CS_TEXT_TYPE;
		datafmt.maxlength = MAX_BIO - 1;
		datalen[4] = strlen((char *)dptr->pub_bio);
		if (blk_bind(blkdesc, 5, &datafmt, NULL,
					 &datalen[4], NULL) != CS_SUCCEED)
		{
			ex_error("BulkCopyIn: blk_bind(5) failed");
			return CS_FAIL;
		}
		if (blk_rowxfer (blkdesc) == CS_FAIL)
		{
			ex_error("BulkCopyIn: EX_BLK - Failed on blk_rowxfer.");
			return CS_FAIL;
		}
		if (blk_textxfer(blkdesc, dptr->pub_bio, datalen[4], &len)
		                   == CS_FAIL)
		{
			ex_error("BulkCopyIn: blk_textxfer() failed");
			return CS_FAIL;
		}
		dptr++;
	}
	/*
	** ALL the rows sent so clear up
	*/
	if (blk_done(blkdesc, CS_BLK_ALL, &numrows) == CS_FAIL)
	{
		ex_error("BulkCopyIn: blk_done() failed");
		return CS_FAIL;
	}
	if (blk_drop(blkdesc) == CS_FAIL)
	{
		ex_error("BulkCopyIn: blk_drop() failed");
		return CS_FAIL;
	}

	/*
	** Give number of rows read/written
	*/
	fprintf(stderr, "Number of data rows transferred = %d.\n", numrows);
	fflush(stderr);

        return CS_SUCCEED;
}

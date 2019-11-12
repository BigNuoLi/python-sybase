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
**  	Demonstrates how to use Client-Library to query the
**   	directory service for available servers.
**
**   	This program searches for SAP server entries in the directory,
**   	prints a description of each entry found, and lets the user choose
**   	a server to connect to.
**
** Inputs
** ------
**   	None. The program relies on the directory driver configuration
**   	to specify the directory source and the subtree (DIT-Base) to
**   	search.
**
**   	If no directory driver is configured, the SYBASE interfaces file
**   	(or SQL.INI file for Windows and OS/2 machines) is searched.
**   	Directory drivers are configured in the driver configuration
**   	file (libtcl.cfg in the main SAP ASE SDK installation directory).
**
**   	The username and password defined in example.h are used to
**   	connect to the chosen server.
**
** Outputs
** -------
**   	Displays each server's information until the user picks a server to
**   	connect to.
**
** Routines Used
** -------------
**   	ct_ds_dropobj
**   	ct_ds_lookup
**   	ct_ds_objinfo
**   	ct_callback
**   	ct_con_alloc, ct_con_drop
**   	ct_con_props
*/

/*
** C Standard libraries.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
** Include the Client-Library header file.
*/
#include <ctpublic.h>

/*
** Example header files.
**  - example.h defines the user name, the password, and some global
**     macros.
**  - exutils.h prototypes the exutils.c utility routines. Any routine
**    called here that begins with ex_ is defined in exutils.c
*/
#include "example.h"
#include "exutils.h"

/*
** Global data and structures.
*/

/*
** Login property values for connecting to the server.
*/
CS_STATIC CS_CHAR  *Ex_appname = "Directory search example";
CS_STATIC CS_CHAR  *Ex_user = EX_USERNAME;
CS_STATIC CS_CHAR  *Ex_password = EX_PASSWORD;

/*
** AttributesToDisplay is a read-only static array used by
** the show_server_info() function. It contains the Object
** Identifier (OID) strings for the server attributes to
** display, in the order that they are to be displayed.
*/
typedef struct
{
	CS_CHAR             type_string[CS_MAX_DS_STRING];
	CS_CHAR             english_name[CS_MAX_DS_STRING];
} AttrForDisplay;

#define N_ATTRIBUTES 7
CS_STATIC AttrForDisplay AttributesToDisplay[N_ATTRIBUTES + 1] =
{
	{CS_OID_ATTRSERVNAME, "Server name"},
	{CS_OID_ATTRSERVICE, "Service type"},
	{CS_OID_ATTRVERSION, "Server entry version"},
	{CS_OID_ATTRSTATUS, "Server status"},
	{CS_OID_ATTRADDRESS, "Network addresses"},
	{CS_OID_ATTRRETRYCOUNT, "Connection retry count"},
	{CS_OID_ATTRLOOPDELAY, "Connection retry loop delay"},
	{"", ""}
};

/*
** (SERVER_INFO_LIST *) is the abstract data type that we use
** to collect a list of servers from the directory. The sil_
** routines operate on the list type.
*/
#define MAX_SERVER_LIST 256
typedef struct _server_info
{
	CS_DS_OBJECT       *object;
} SERVER_INFO;
typedef struct _server_info_list
{
	SERVER_INFO        *servers;	/* Array of size MAX_SERVER_LIST */
	CS_INT              len;/* Current length of list        */
} SERVER_INFO_LIST;

/*
** Private defines.
*/
#define STRLEN(s)  ((s) == NULL ? 0 : strlen(s))
#define CHOOSE_SERVER_MAX_INPUT 128

/*
** Private prototypes.
*/
CS_RETCODE provider_setup PROTOTYPE((
    CS_CONNECTION *conn
    ));
CS_RETCODE get_servers PROTOTYPE((
    CS_CONNECTION *conn,
    SERVER_INFO_LIST **servers
    ));
CS_RETCODE CS_PUBLIC directory_cb PROTOTYPE((
    CS_CONNECTION *conn,
    CS_INT reqid,
    CS_RETCODE status,
    CS_INT numentries,
    CS_DS_OBJECT *ds_object,
    CS_VOID *userdata
    ));
CS_RETCODE choose_server PROTOTYPE((
    CS_CONNECTION *conn,
    SERVER_INFO_LIST *slp,
    CS_CHAR server_name [],
    CS_INT buflen,
    CS_INT *outlen
    ));
CS_RETCODE show_server_info PROTOTYPE((
    CS_DS_OBJECT *ds_object,
    FILE *outfile
    ));
CS_RETCODE attr_get_by_type PROTOTYPE((
    CS_DS_OBJECT *ds_object,
    CS_CHAR *attr_type_str,
    CS_ATTRIBUTE *attr_metadata,
    CS_ATTRVALUE **p_attrvals
    ));
CS_RETCODE attr_display_values PROTOTYPE((
    CS_ATTRIBUTE *attr_meta,
    CS_ATTRVALUE attr_vals[],
    FILE *outfile
    ));
CS_RETCODE attr_val_as_string PROTOTYPE((
    CS_ATTRIBUTE *attr_metadata,
    CS_ATTRVALUE *val,
    CS_CHAR *buf,
    CS_INT buflen,
    CS_INT *outlen
    ));
CS_RETCODE attr_enum_english_name PROTOTYPE((
    CS_INT enum_val,
    CS_OID *attr_type,
    CS_CHAR *buffer,
    CS_INT buflen,
    CS_INT *outlen
    ));
int match_OID       PROTOTYPE((
    CS_OID *oid,
    CS_CHAR *oid_string
    ));
CS_RETCODE sil_init_list PROTOTYPE((
    SERVER_INFO_LIST **slp
    ));
CS_RETCODE sil_drop_list PROTOTYPE((
    CS_CONNECTION *conn,
    SERVER_INFO_LIST *slp
    ));
CS_RETCODE sil_add_object PROTOTYPE((
    SERVER_INFO_LIST *slp,
    CS_DS_OBJECT *ds_object
    ));
CS_RETCODE sil_extract_object PROTOTYPE((
    SERVER_INFO_LIST *slp,
    CS_INT number,
    CS_DS_OBJECT **p_dsobject
    ));
CS_INT sil_list_len PROTOTYPE((
    SERVER_INFO_LIST *slp
    ));

/*
** main() -- Entry point for our program.
*/
int
main(int argc, char *argv[])
{
	CS_RETCODE          ret;
	CS_CONTEXT         *ctx;
	CS_CONNECTION      *conn1;
	CS_CONNECTION      *conn2;
	SERVER_INFO_LIST   *servers;
	CS_CHAR             server_name[512];

        EX_SCREEN_INIT();

	/*
	** Initialize CS-Library and Client-Library.
	*/
	ret = ex_init(&ctx);
	if (ret != CS_SUCCEED)
	{
		ex_panic("Initialization failed.");
	}

	/*
	** Allocate a connection structure. We need it to query the
	** directory.
	*/
	ret = ct_con_alloc(ctx, &conn1);
	if (ret != CS_SUCCEED)
	{
		ex_panic("ct_con_alloc() failed.");
	}

	/*
	** Get a list of available servers.
	*/
	ret = get_servers(conn1, &servers);
	if (ret != CS_SUCCEED)
	{
		ex_panic("get_servers() failed.");
	}

	/*
	** Let the user choose a server and attempt a connection to it.
	*/
	ret = choose_server(conn1, servers, server_name, 512, NULL);
	if (ret != 0)
	{

		/*
		** User chose to quit, or an error occurred. In either
		** case, clean up and exit.
		*/
		fprintf(stdout, "Exiting.\n");
		(CS_VOID)sil_drop_list(conn1, servers);
		ret = ct_con_drop(conn1);
		ret = ex_ctx_cleanup(ctx, ret);
		exit(ret == 1 ? EX_EXIT_SUCCEED : EX_EXIT_FAIL);
	}

	/*
	** Connect to the chosen server. We could do this with the same
	** connection, that we used to query the directory. We don't do
	** that here because ex_connect() won't use a pre-allocated
	** connection.
	*/
	fprintf(stdout, "Connecting to %s ...\n", server_name);
	ret = ex_connect(ctx, &conn2, Ex_appname, Ex_user, Ex_password, 
			 server_name);
	if (ret != CS_SUCCEED)
	{
		ex_panic("Connect attempt failed.");
	}

	/*
	** Drop the list of servers.
	*/
	ret = sil_drop_list(conn1, servers);
	if (ret != CS_SUCCEED)
	{
		ex_panic("sil_drop_list() failed.");
	}

	/*
	** Clean up Client-Library and drop the CS_CONTEXT. We can't use
	** ex_con_cleanup() on conn1 because it's not open.
	*/
	ret = ct_con_drop(conn1);
	if (ret != CS_SUCCEED)
	{
		ex_error("Cleanup of connection 1 failed.");
	}
	ret = ex_con_cleanup(conn2, ret);
	if (ret != CS_SUCCEED)
	{
		ex_error("Cleanup of connection 2 failed.");
	}
	ret = ex_ctx_cleanup(ctx, ret);
	if (ret != CS_SUCCEED)
	{
		ex_error("Context-level cleanup failed.");
	}

	return(EX_EXIT_SUCCEED);

} /* main() */

/*
** get_servers() -- Query the directory for servers and
**    get a list of directory objects that contain details
**    for each.
**
** Parameters
**   conn -- Pointer to allocated connection structure.
**   pserver_list -- Address of a pointer to a SERVER_INFO_LIST.
**       Upon successful return, the list will be initialized
**       and contain an object for each server found in the
**       search.
**
**       NOTE: The caller must clean up the list with sil_drop_list()
**       when done with it.
**
** Returns
**   CS_SUCCEED or CS_FAIL.
*/
CS_RETCODE
get_servers(CS_CONNECTION *conn, SERVER_INFO_LIST **pserver_list)
{
	CS_RETCODE          ret;
	CS_INT              reqid;
	CS_VOID            *oldcallback;
	CS_OID              oid;
	CS_DS_LOOKUP_INFO   lookup_info;

	/*
	** Steps for synchronous-mode directory searches:
	** 
	** 1. If necessary, initialize application specific data structures
	**    (Our application collects directory objects in *pserver_list).
	** 2. Save the old directory callback and install our own (the
	**    directory_cb function, found below).
        ** 3. (Optional -- Network based directories only) 
        **    Set the base node in the directory to search beneath 
	**    (CS_DS_DITBASE property). 
        ** 4. Call ct_ds_lookup to begin the search, passing
	**    any application specific data structures as the userdata
	**    argument. 
        ** 5. Client-Library invokes our callback once for each
	**    found object. directory_cb() inserts each directory 
        **    object to the list of servers. 
        ** 6. Check the return status of ct_ds_lookup. 
        ** 7. Restore callbacks and properties
	**    that we changed.
	*/

	/*
	** Step 1. Initialize the data structure (*pserver_list).
	*/
	ret = sil_init_list(pserver_list);
	if (ret != CS_SUCCEED || (*pserver_list) == NULL)
	{
		ex_error("get_servers: Could not initialize list.");
		return CS_FAIL;
	}

	/*
	** Step 2. Save the old directory callback and install our own
	** callback, directory_cb(), to receive the found objects.
	*/
	ret = ct_callback(NULL, conn, CS_GET,
			  CS_DS_LOOKUP_CB, &oldcallback);
	if (ret == CS_SUCCEED)
	{
		ret = ct_callback(NULL, conn, CS_SET,
			       CS_DS_LOOKUP_CB, (CS_VOID *)directory_cb);
	}
	if (ret != CS_SUCCEED)
	{
		ex_error("get_servers: Could not install directory callback.");
		return CS_FAIL;
	}

	/*
	** Step 3. Set the base node in the directory to search beneath
	** (the CS_DS_DITBASE connection property).
	*/

	ret = provider_setup(conn);
	if (ret != CS_SUCCEED)
	{
		ex_error("get_servers: Provider-specific setup failed.");
		return CS_FAIL;
	}

	/*
	** Step 4. Call ct_ds_lookup to begin the search, passing the
	** server list pointer as userdata. Step 5. Client-Library invokes
	** our callback once for each found object (or once to report that
	** no objects were found). Our callback, directory_cb, will
	** receives a pointer to each found server object and appends it
	** to the list. Step 6. Check the return status of ct_ds_lookup.
	*/

	/*
	** Set the CS_DS_LOOKUP_INFO structure fields.
	*/
	lookup_info.path = NULL;
	lookup_info.pathlen = 0;
	lookup_info.attrfilter = NULL;
	lookup_info.attrselect = NULL;

	strcpy(oid.oid_buffer, CS_OID_OBJSERVER);
	oid.oid_length = (CS_INT)STRLEN(oid.oid_buffer);
	lookup_info.objclass = &oid;

	/*
	** Begin the search.
	*/
	ret = ct_ds_lookup(conn, CS_SET, &reqid,
			   &lookup_info, (CS_VOID *)pserver_list);
	if (ret != CS_SUCCEED)
	{
		ex_error("get_servers: Could not run search.");
		return CS_FAIL;
	}

	/*
	** Step 7.  Restore callbacks and properties that we changed.
	*/
	ret = ct_callback(NULL, conn, CS_SET,
			  CS_DS_LOOKUP_CB, oldcallback);
	if (ret != CS_SUCCEED)
	{
		ex_error("get_servers: Could not restore directory callback.");
		return CS_FAIL;
	}

	return CS_SUCCEED;

} /* get_servers() */

/*
** directory_cb() -- Directory callback to install in Client-Library.
**   When we call ct_ds_lookup(), Client-Library calls this function
**   once for each object that is found in the search.
**
**   This particular callback collects the objects in
**   the SERVER_INFO_LIST that is received as userdata.
**
** Parameters
**   conn -- The connection handle passed to ct_ds_lookup() to
**      begin the search.
**   reqid -- The request id for the operation (assigned by Client-Library).
**   status -- CS_SUCCEED when search succeeded (ds_object is valid).
**      CS_FAIL if the search failed (ds_object is not valid).
**   numentries -- The count of objects to be returned for the
**      search. Includes the current object. Can be 0 if search
**      failed.
**   ds_object -- Pointer to a CS_DS_OBJECT structure. Will
**      be NULL if the search failed.
**   userdata -- The address of user-allocated data that was
**      passed to ct_ds_lookup().
**
**      This particular callback requires userdata to be the
**      address of a valid, initialized SERVER_INFO_LIST pointer.
**      (SERVER_INFO_LIST is an application data structure defined
**      by this sample).
**
** Returns
**   CS_CONTINUE unless the SERVER_INFO_LIST pointed at by userdata fills
**     up, then CS_SUCCEED to truncate the search results.
*/
CS_RETCODE          CS_PUBLIC
directory_cb(CS_CONNECTION *conn, CS_INT reqid, CS_RETCODE status, CS_INT numentries, 
	     CS_DS_OBJECT *ds_object, CS_VOID *userdata)
{
	CS_RETCODE          ret;
	SERVER_INFO_LIST   *server_list;

	if (status != CS_SUCCEED 
	    || numentries <= 0
	    || ds_object == (CS_DS_OBJECT *)NULL)
	{
		return CS_SUCCEED;
	}

	/*
	** Append the object to the list of servers.
	*/
	server_list = *((SERVER_INFO_LIST **)userdata);
	ret = sil_add_object(server_list, ds_object);
	if (ret != CS_SUCCEED)
	{

		/*
		** Return CS_SUCCEED to discard the rest of the objects
		** that were found in the search.
		*/
		ex_error(
	   "directory_cb: Too many servers! Truncating search results.");
		return CS_SUCCEED;
	}

	/*
	** Return CS_CONTINUE so Client-Library will call us again if more
	** entries are found.
	*/
	return CS_CONTINUE;

} /* directory_cb() */

/*
** provider_setup() -- Set connection properties based on the
**   directory provider.
*/
CS_RETCODE
provider_setup(CS_CONNECTION *conn)
{
	CS_RETCODE          ret;
	CS_CHAR             scratch_str[512];

	/*
	** We just print out the default directory provider name. If we
	** were tailoring this example for a specific provider name, we
	** could set properties here such as CS_DS_DITBASE.
	** 
	** Instead, we depend on the default DIT-base being set in the driver
	** configuration file (libtcl.cfg on most platforms).
	*/
	ret = ct_con_props(conn, CS_GET, CS_DS_PROVIDER, scratch_str,
			   512, NULL);
	if (ret != CS_SUCCEED)
	{
		ex_error(
		   "provider_setup: Could not get directory provider name.");
		return CS_FAIL;
	}

	fprintf(stdout, "\nProvider is %s\n\n", scratch_str);

	return CS_SUCCEED;

} /* provider_setup() */

/*
** choose_server() -- Show the contents of the server list to
**   the user. Let the user choose a server to connect to.
**
** Parameters
**   conn -- Connection that the server objects were obtained
**      with (the CS_CONNECTION pointer that was passed to
**      ct_ds_objinfo)
**   server_list -- Pointer to the server list.
**   namebuf -- Buffer to receive the distinguished name of the
**      server that the user picks.
**   buflen -- Length of namebuf, in bytes.
**   outlen -- If supplied, will be set to number of bytes written
**      to namebuf.
**
** Returns
**   0   -- namebuf contains the name of the server to connect to.
**   1   -- user elected to quit the program.
**   < 0 -- error occurred.
*/
CS_RETCODE
choose_server(CS_CONNECTION *conn, SERVER_INFO_LIST *server_list, CS_CHAR namebuf[], 
	      CS_INT buflen, CS_INT *outlen)
{
	CS_RETCODE          ret;
	CS_INT              pos;
	CS_CHAR             in_buf[CHOOSE_SERVER_MAX_INPUT];
	CS_DS_OBJECT       *ds_object;
	CS_INT              return_cond = -1;
	CS_INT              intval;

	/*
	** Check for input errors.
	*/
	if (namebuf == NULL || buflen < 1)
	{
		ex_error("choose_server: buffer must be supplied.");
		return -1;
	}

	/*
	** Make the output fail safe.
	*/
	namebuf[0] = '\0';
	if (outlen != NULL)
	{
		*outlen = 0;
	}

	/*
	** Display the attributes for each server in the list. After each,
	** pause and ask the user if it's the one to connect to.
	*/
	pos = 1;
	while (pos <= sil_list_len(server_list) && return_cond == -1)
	{
		ret = sil_extract_object(server_list, pos, &ds_object);
		if (ret != CS_SUCCEED)
		{
			ex_error(
			  "choose_server: sil_extract_object() failed.");
			return -1;
		}

		if (ds_object != NULL)
		{

			/*
			** Show the server's attributes that were
			** retrieved from the directory.
			*/
			ret = show_server_info(ds_object, stdout);
			if (ret != CS_SUCCEED)
			{
				ex_error(
				 "choose_server: show_server_info() failed.");
				return -1;
			}

			/*
			** Ask the user what to do next.
			*/
			fprintf(stdout, "\n[%ld of %ld] ",
			     (long)pos, (long)sil_list_len(server_list));
			fprintf(stdout, 
              "Connect (c), see next (n), skip to (#), or quit (q): ");
			(CS_VOID)fgets(in_buf, CHOOSE_SERVER_MAX_INPUT, stdin);

			switch ((int)in_buf[0])
			{
			case 'c':
			case 'C':

       			/*
       			** User chose this server to connect to.
       			** Copy the object's distinguished name
       			** into the caller's buffer.
       			*/
       			ret = ct_ds_objinfo(ds_object, CS_GET, 
       			            CS_DS_DIST_NAME, CS_UNUSED,
       			            (CS_VOID *)namebuf, buflen, &intval);
       			if (ret != CS_SUCCEED)
       			{
       				ex_error(
				 "choose_server: ct_ds_objinfo(NAME) failed.");
       				return -1;
       			}

       			/*
       			** Set the caller's outlen variable if it
       			** was supplied.
       			*/
       			if (outlen != NULL)
       			{
       				*outlen = intval;
       			}
       			return_cond = 0;
       			break;

			case 'q':
			case 'Q':

		       	/*
		       	** User is tired and wants to quit.
		       	*/
		       	return_cond = 1;
		       	break;

			case 'n':
			case 'N':
			default:

		       	/*
		       	** Try to convert the input to an integer
		       	** -- if successful, then skip to that
		       	** number. Otherwise, default action shows
		       	** the next server in the list.
		       	*/
		       	intval = atoi(in_buf);
		       	if (intval != 0)
		       	{

		       		/*
		       		** Skip to a different server
		       		** number.
		       		*/
		       		if (intval > 0 
				    && intval <= sil_list_len(server_list))
		       		{
		       			pos = intval;
		       		}
		       		else
		       		{
		       			ex_error("Not that many servers.");
		       		}
	       		}
	       		else
	       		{
	       			/*
	       			** Show next.
	       			*/
	       			++pos;
	       		}
	       		fprintf(stdout, "\n");
	       		break;

		        } /* switch */
		} /* if */
	} /* while */

	return return_cond;

} /* choose_server() */

/*
** show_server_info()
**   Selectively display the attributes of a server directory
**   object.
**
** Parameters
**   ds_object -- Pointer to the CS_DS_OBJECT that describes the
**     server's directory entry.
**   outfile -- Open FILE handle to write the output to.
**
** Dependencies
**   Reads the contents of the AttributesToDisplay global array.
**
** Returns
**   CS_SUCCEED or CS_FAIL.
*/
CS_RETCODE
show_server_info(CS_DS_OBJECT *ds_object, FILE *outfile)
{
	CS_RETCODE          ret;
	CS_CHAR             scratch_str[512];
	CS_INT              outlen;
	CS_INT              cur_attr;
	CS_ATTRIBUTE        attr_metadata;
	CS_ATTRVALUE       *p_attrvals;

	/*
	** Distinguished name of the object.
	*/
	ret = ct_ds_objinfo(ds_object, CS_GET, CS_DS_DIST_NAME, CS_UNUSED,
			    (CS_VOID *)scratch_str, CS_SIZEOF(scratch_str),
			    &outlen);
	if (ret != CS_SUCCEED)
	{
		ex_error("show_server_info: get distinguished name failed.");
		return CS_FAIL;
	}

	fprintf(outfile, "Name in directory: %s\n", scratch_str);

	for (cur_attr = 0; cur_attr < N_ATTRIBUTES; cur_attr++)
	{

		/*
		** Look for the attribute. attr_get_by_type() fails if the
		** object instance does not contain a value for the
		** attribute. If this happens, we just go on to the next
		** attribute.
		*/
		ret = attr_get_by_type(ds_object,
			       AttributesToDisplay[cur_attr].type_string,
				       &attr_metadata, &p_attrvals);
		if (ret == CS_SUCCEED)
		{

			fprintf(outfile, 
			"%s:\n", AttributesToDisplay[cur_attr].english_name);

			/*
			** Display the attribute values.
			*/
			ret = attr_display_values(&attr_metadata, 
						  p_attrvals, outfile);
			if (ret != CS_SUCCEED)
			{
				ex_error(
			 "show_server_info: display attribute values failed.");
				free(p_attrvals);
				return CS_FAIL;
			}

			free(p_attrvals);

		}

	} /* for */

	return CS_SUCCEED;

} /* show_server_info() */

/*
** get_attr_by_type()
**   Get metadata and attribute values for a given attribute type.
**
** Parameters
**   ds_object -- Pointer to a valid CS_DS_OBJECT hidden structure.
**   attr_type_str -- Null-terminated string containing the OID for the
**            desired attribute type.
**   attr_metadata -- Pointer to a CS_ATTRIBUTE structure to
**            fill in.
**   p_attrvals -- Address of a CS_ATTRVALUE union pointer.
**            If successful, this routine allocates an array
**            of size attr_metadata->numvalues, retrieves values into
**            it, and returns the array address in *p_attr_values.
**            NOTE: The caller must free this array when it is no longer
**            needed.
**
** Returns
**   CS_FAIL if no attribute of the specified type was found.
**   CS_SUCCEED for success.
*/
CS_RETCODE
attr_get_by_type(CS_DS_OBJECT *ds_object, CS_CHAR *attr_type_str,
		 CS_ATTRIBUTE *attr_metadata, CS_ATTRVALUE **p_attrvals)
{
	CS_RETCODE          ret;
	CS_INT              num_attrs;
	CS_INT              cur_attr;
	CS_INT              outlen;
	CS_INT              buflen;
	CS_BOOL             found = CS_FALSE;

	/*
	** Check input pointers. If not NULL, make them fail safe.
	*/
	if (attr_metadata == NULL || p_attrvals == NULL)
	{
		return CS_FAIL;
	}
	attr_metadata->attr_numvals = 0;
	*p_attrvals = NULL;

	/*
	** Get number of attributes.
	*/
	ret = ct_ds_objinfo(ds_object, CS_GET, CS_DS_NUMATTR, CS_UNUSED,
			    (CS_VOID *)&num_attrs, CS_SIZEOF(num_attrs),
			    NULL);
	if (ret != CS_SUCCEED)
	{
		ex_error(
		  "attr_get_by_type: get number of attributes failed.");
		return CS_FAIL;
	}

	/*
	** Look for the matching attribute, get the values if found.
	*/
	for (cur_attr = 1;
	     (cur_attr <= num_attrs) && (found != CS_TRUE);
	     cur_attr++)
	{

		/*
		** Get the attribute's metadata.
		*/
		ret = ct_ds_objinfo(ds_object, CS_GET, CS_DS_ATTRIBUTE, 
				    cur_attr, (CS_VOID *)attr_metadata, 
				    CS_SIZEOF(CS_ATTRIBUTE), NULL);
		if (ret != CS_SUCCEED)
		{
			ex_error("attr_get_by_type: get attribute failed.");
			return CS_FAIL;
		}

		/*
		** Check for a match.
		*/
		if (match_OID(&(attr_metadata->attr_type), attr_type_str))
		{
			found = CS_TRUE;

			/*
			** Get the values -- we first allocate an array of
			** CS_ATTRVALUE unions.
			*/
			if (attr_metadata->attr_numvals <= 0)
			{
				ex_error(
				  "attr_get_by_type: bad numvals field!");
				return CS_FAIL;
			}
			*p_attrvals = (CS_ATTRVALUE *) 
			     malloc(sizeof(CS_ATTRVALUE)
				    * (attr_metadata->attr_numvals));
			if (p_attrvals == NULL)
			{
				ex_error("attr_get_by_type: out of memory!");
				return CS_FAIL;
			}

			buflen = CS_SIZEOF(CS_ATTRVALUE) 
			  * (attr_metadata->attr_numvals);
			ret = ct_ds_objinfo(ds_object, CS_GET, 
					    CS_DS_ATTRVALS, cur_attr,
					    (CS_VOID *)(*p_attrvals), 
					    buflen, &outlen);
			if (ret != CS_SUCCEED)
			{
				ex_error(
			  "attr_get_by_type: get attribute values failed.");
				free(*p_attrvals);
				*p_attrvals = NULL;
				attr_metadata->attr_numvals = 0;
				return CS_FAIL;
			}
		}
	}

	/*
	** Got the attribute.
	*/
	if (found == CS_TRUE)
	{
		return CS_SUCCEED;
	}

	/*
	** Not found.
	*/
	attr_metadata->attr_numvals = 0;
	return CS_FAIL;

} /* attr_get_by_type() */

/*
** attr_display_values()
**   Writes an attribute's values to the specified text
**   file.
**
** Parameters
**   attr_metadata -- address of the CS_ATTRIBUTE structure that
**      contains metadata for the attribute.
**   attr_vals -- address of an array of CS_ATTRVALUE structures.
**       This function assumes length is attr_metadata->attr_numvals
**       and value syntax is attr_metadata->attr_syntax.
**   outfile -- Open FILE handle to write to.
**
** Returns
**   CS_SUCCEED or CS_FAIL.
*/
CS_RETCODE
attr_display_values(CS_ATTRIBUTE *attr_metadata, CS_ATTRVALUE *attr_vals, FILE *outfile)
{
	CS_INT              i;
	CS_CHAR             outbuf[CS_MAX_DS_STRING * 3];
	CS_RETCODE          ret;

	/*
	** Print each value.
	*/
	for (i = 0; i < attr_metadata->attr_numvals; i++)
	{
		ret = attr_val_as_string(attr_metadata, attr_vals + i,
				     outbuf, CS_MAX_DS_STRING * 3, NULL);
		if (ret != CS_SUCCEED)
		{
			ex_error(
		     "attr_display_values: attr_val_as_string() failed.");
			return CS_FAIL;
		}
		fprintf(outfile, "\t%s\n", outbuf);
	}

	return CS_SUCCEED;

} /* attr_display_values() */

/*
** attr_val_as_string() -- Convert the contents of a CS_ATTRVALUE union to
**   a printable string.
**
** Parameters
**   attr_metadata -- The CS_ATTRIBUTE structure containing metadata
**       for the attribute value.
**   val -- Pointer to the CS_ATTRVALUE union.
**   buffer -- Address of the buffer to receive the converted value.
**   buflen -- Length of *buffer in bytes.
**   outlen -- If supplied, will be set to the number of bytes written
**      to *buffer.
**
** Returns
**   CS_SUCCEED or CS_FAIL.
*/
CS_RETCODE
attr_val_as_string(CS_ATTRIBUTE *attr_metadata, CS_ATTRVALUE *val, CS_CHAR *buffer, 
		   CS_INT buflen, CS_INT *outlen)
{
	CS_CHAR             outbuf[CS_MAX_DS_STRING * 4];
	CS_CHAR             scratch[CS_MAX_DS_STRING];
	CS_RETCODE          ret;

	if (buflen == 0 || buffer == NULL)
	{
		return CS_FAIL;
	}
	if (outlen != NULL)
	{
		*outlen = 0;
	}

	switch ((int)attr_metadata->attr_syntax)
	{
	case CS_ATTR_SYNTAX_STRING:
		sprintf(outbuf, "%.*s",
			(int)(val->value_string.str_length),
			val->value_string.str_buffer);
		break;

	case CS_ATTR_SYNTAX_BOOLEAN:
		sprintf(outbuf, "%s",
			val->value_boolean == CS_TRUE ? "True" : "False");
		break;

	case CS_ATTR_SYNTAX_INTEGER:
	case CS_ATTR_SYNTAX_ENUMERATION:

		/*
		** Some enumerated or integer attribute values should be
		** converted into an english-language equivalent.
		** attr_enum_english_name() contains all the logic to
		** convert #define's into human language.
		*/
		ret = attr_enum_english_name((CS_INT)(val->value_enumeration),
					     &(attr_metadata->attr_type),
					scratch, CS_MAX_DS_STRING, NULL);
		if (ret != CS_SUCCEED)
		{
			ex_error(
		    "attr_val_as_string: attr_enum_english_name() failed.");
			return CS_FAIL;
		}
		sprintf(outbuf, "%s", scratch);
		break;

	case CS_ATTR_SYNTAX_TRANADDR:

		/*
		** The access type is an enumerated value. Get an english
		** language string for it.
		*/
		switch ((int)(val->value_tranaddr.addr_accesstype))
		{
		case CS_ACCESS_CLIENT:
			sprintf(scratch, "client");
			break;
		case CS_ACCESS_ADMIN:
			sprintf(scratch, "administrative");
			break;
		case CS_ACCESS_MGMTAGENT:
			sprintf(scratch, "management agent");
			break;
		default:
			sprintf(scratch, "%ld", 
				(long)(val->value_tranaddr.addr_accesstype));
			break;
		}

		sprintf(outbuf, 
			"Access type '%s'; Transport type '%s'; Address '%s'",
			scratch,
			val->value_tranaddr.addr_trantype.str_buffer,
			val->value_tranaddr.addr_tranaddress.str_buffer);

		break;

	case CS_ATTR_SYNTAX_OID:
		sprintf(outbuf, "%.*s",
			(int)(val->value_oid.oid_length),
			val->value_oid.oid_buffer);
		break;

	default:
		sprintf(outbuf, "Unknown attribute value syntax");
		break;

	} /* switch */

	if ((CS_INT)STRLEN(outbuf) + 1 > buflen || buffer == NULL)
	{
		return CS_FAIL;
	}
	else
	{
		sprintf(buffer, "%s", outbuf);
		if (outlen != NULL)
		{
			*outlen = (CS_INT)STRLEN(outbuf) + 1;
		}
	}

	return CS_SUCCEED;

} /* attr_val_as_string() */

/*
** attr_enum_english_name()
**   Based on the attribute type, associate an english phrase with
**   a CS_INT value. Use this function to get meaningful names for
**   CS_ATTR_SYNTAX_ENUMERATION or CS_ATTR_SYNTAX_INTEGER attribute
**   values.
**
**   If the attribute type represents a quantity and not a numeric code,
**   then the value is converted to the string representation of the
**   number. Unknown codes are handled the same way.
**
** Parameters
**   enum_val -- The integer value to convert to a string.
**   attr_type -- Pointer to an OID structure containing the OID string
**      that tells the attribute's type.
**   buffer -- Address of the buffer to receive the converted value.
**   buflen -- Length of *buffer in bytes.
**   outlen -- If supplied, will be set to the number of bytes written
**      to *buffer.
**
** Bugs
**   This function only knows about the attributes of the Server
**   Directory object as it was defined at the time the function
**   was created.
**
** Returns
**   CS_SUCCEED or CS_FAIL
*/
CS_RETCODE
attr_enum_english_name(CS_INT enum_val, CS_OID *attr_type, CS_CHAR *buffer, 
		       CS_INT buflen, CS_INT *outlen)
{
	CS_CHAR             outbuf[CS_MAX_DS_STRING];

	if (buffer == NULL || buflen <= 0)
	{
		return CS_FAIL;
	}
	if (outlen != NULL)
	{
		*outlen = 0;
	}

	/*
	** Server version number.
	*/
	if (match_OID(attr_type, CS_OID_ATTRVERSION))
	{
		sprintf(outbuf, "%ld", (long)enum_val);
	}

	/*
	** Server's status.
	*/
	else if (match_OID(attr_type, CS_OID_ATTRSTATUS))
	{
		switch ((int)enum_val)
		{
		case CS_STATUS_ACTIVE:
			sprintf(outbuf, "running");
			break;
		case CS_STATUS_STOPPED:
			sprintf(outbuf, "stopped");
			break;
		case CS_STATUS_FAILED:
			sprintf(outbuf, "failed");
			break;
		case CS_STATUS_UNKNOWN:
			sprintf(outbuf, "unknown");
			break;
		default:
			sprintf(outbuf, "%ld", (long)enum_val);
			break;
		}
	}

	/*
	** Anything else is either an enumerated type that we don't know
	** about, or it really is just a number. We print the numeric
	** value.
	*/
	else
	{
		sprintf(outbuf, "%ld", (long)enum_val);
	}

	/*
	** Transfer output to the caller's buffer.
	*/
	if ((CS_INT)STRLEN(outbuf) + 1 > buflen || buffer == NULL)
	{
		return CS_FAIL;
	}
	else
	{
		sprintf(buffer, "%s", outbuf);
		if (outlen != NULL)
		{
			*outlen = (CS_INT)STRLEN(outbuf) + 1;
		}
	}

	return CS_SUCCEED;

} /* attr_enum_english_name() */

/*
** match_OID()
**   Compare a pre-defined OID string to the contents of a
**   CS_OID structure.
**
** Parameters
**   oid -- Pointer to a CS_OID structure. OID->oid_length should be
**     the length of the string, not including any null-terminator.
**   oid_string -- Null-terminated OID string to compare.
**
** Returns
**  Non-zero if contents of oid->oid_buffer matches contents
**  of oid_string.
*/
int
match_OID(CS_OID *oid, CS_CHAR *oid_string)
{
        return ( (oid->oid_length == (CS_INT)STRLEN(oid_string))
	  && (strncmp(oid_string, oid->oid_buffer, oid->oid_length) == 0));
} /* match_OID() */

/*
** sil_init_list() -- Initialize an instance of the
**    SERVER_INFO_LIST abstract data type.
**
** Parameters
**   pplist -- address of a SERVER_INFO_LIST *.
**
** Returns
**   CS_SUCCEED or CS_FAIL.
*/
CS_RETCODE
sil_init_list(SERVER_INFO_LIST **pplist)
{
	CS_INT              i;

	/*
	** Allocate a SERVER_INFO_LIST structure.
	*/
	*pplist = (SERVER_INFO_LIST *)malloc(sizeof(SERVER_INFO_LIST));

	if (*pplist == (SERVER_INFO_LIST *)NULL)
	{
		ex_error("sil_init_list: Out of memory.");
		return CS_FAIL;
	}

	/*
	** Allocate the array.
	*/
	(*pplist)->servers = (SERVER_INFO *)malloc(sizeof(SERVER_INFO)
						   * MAX_SERVER_LIST);

	if ((*pplist)->servers == (SERVER_INFO *)NULL)
	{
		ex_error("sil_init_list: Out of memory.");
		return CS_FAIL;
	}

	for (i = 0; i < MAX_SERVER_LIST; i++)
	{
		(*pplist)->servers[i].object = NULL;
	}

	(*pplist)->len = 0;

	return CS_SUCCEED;

} /* sil_init_list */

/*
** sil_add_object() -- Insert a CS_DS_OBJECT pointer
**   into an instance of the SERVER_INFO_LIST abstract
**   data type.
**
** Parameters
**   slp -- Pointer to an initialized SERVER_INFO_LIST.
**   ds_object -- Pointer to be inserted.
**
** Returns
**   CS_SUCCEED or CS_FAIL.
*/
CS_RETCODE
sil_add_object(SERVER_INFO_LIST *slp, CS_DS_OBJECT *ds_object)
{
	int                 i;

	/*
	** Check that the list has been initialized.
	*/
	if (slp == (SERVER_INFO_LIST *)NULL)
	{
		ex_error(
		  "sil_add_object: Received NULL server list parameter.");
		return CS_FAIL;
	}

	/*
	** List full?
	*/
	if (slp->len >= MAX_SERVER_LIST)
	{
		ex_error("sil_add_object: List is full.");
		return CS_FAIL;
	}

	/*
	** Perform the insert operation.
	*/
	i = slp->len;
	slp->servers[i].object = ds_object;
	slp->len += 1;

	return CS_SUCCEED;

} /* sil_add_object() */

/*
** sil_drop_list() -- Free the memory associated with
**   an instance of the SERVER_INFO_LIST abstract data
**   type.
**
** Parameters
**   conn -- the connection associated with the CS_DS_OBJECTS
**     stored in the list.
**   slp -- pointer to an initialized SERVER_INFO_LIST.
**
** Returns
**   CS_SUCCEED or CS_FAIL.
*/
CS_RETCODE
sil_drop_list(CS_CONNECTION *conn, SERVER_INFO_LIST *slp)
{
	CS_RETCODE          ret;
	int                 i;

	/*
	** Check that the list has been initialized.
	*/
	if (slp == (SERVER_INFO_LIST *)NULL)
	{
		ex_error(
		  "sil_drop_list: Received NULL server list parameter.");
		return CS_FAIL;
	}

	for (i = 0; i < MAX_SERVER_LIST; i++)
	{

		/*
		** Drop the object.
		*/
		if (slp->servers[i].object != NULL)
		{
			ret = ct_ds_dropobj(conn, slp->servers[i].object);
			if (ret != CS_SUCCEED)
			{
				ex_error(
				  "sil_drop_list: ct_ds_dropobj() failed.");
				return CS_FAIL;
			}
		}

	} /* while */

	/*
	** Drop the array.
	*/
	free(slp->servers);

	/*
	** Drop the list structure.
	*/
	free(slp);

	return CS_SUCCEED;

} /* sil_drop_list() */

/*
** sil_extract_object()
**   Retrieve a CS_DS_OBJECT from a node in the server list.
**
** Parameters
**   slp -- Pointer to an initialized SERVER_LIST_INFO structure. This
**      routine fails if it is NULL.
**   number -- The position of the list item of interest. The
**      first is 1. This routine fails if number is out of range.
**   pobject -- Address of a CS_DS_OBJECT pointer. *pobject is
**      a pointer to the extracted object handle upon successful
**      return.
**
** Returns
**   CS_SUCCEED or CS_FAIL.
*/
CS_RETCODE
sil_extract_object(SERVER_INFO_LIST *slp, CS_INT number, CS_DS_OBJECT **pobject)
{
	if (slp == NULL || pobject == NULL)
	{
		return CS_FAIL;
	}

	if (number < 1 || number > slp->len)
	{
		return CS_FAIL;
	}

	*pobject = slp->servers[number - 1].object;

	return CS_SUCCEED;

} /* sil_extract_object() */

/*
** sil_list_len() -- Returns the length of an instance of the
**   SERVER_INFO_LIST abstract data type.
**
** Parameters
**   slp -- Pointer to an initialized SERVER_INFO_LIST.
**
** Returns
**   CS_SUCCEED or CS_FAIL.
*/
CS_INT
sil_list_len(SERVER_INFO_LIST *slp)
{
	if (slp == NULL)
	{
		return 0;
	}

	return slp->len;

} /* sil_list_len() */

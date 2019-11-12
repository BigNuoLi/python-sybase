/*
** SAP Registered Procedure Sample Application
** -------------------------------------------
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
**
** Description
** -----------
**      Example client application that uses network-based security.
**
** Server dependencies
** -------------------
**      This example must be used with a server that supports 
**      network-based security, such the Security Guardian
**      gateway server. If you have SAP Open Server on
**      your system, there is a secsrv.c example server that
**      is designed to demonstrate security services.
**
** System configuration dependencies
** ---------------------------------
**      Your system must be configured to use the appropriate SAP
**      security driver. See the Open Client/Server Configuration Guide 
**      for information about setting up security drivers. Also see the 
**      Open Client/Server Release Bulletin for (possible) updated
**      information.
**
** Usage
** -----
**	secct [server_name] 
**            [-U user_name]
**            [-R server_principal_name] 
**            [-K keytab_filename]
**            [-Z security_mechanism]
**	
**      -S server_name
**      specifies the server to connect to. If server_name is 
**      omitted, secct connects with the open server using the DSQUERY 
**	environment value. If server_name is supplied, it must be the
**      first argument. Remaining arguments can be in any order.
**
**      -R server_principal_name specifies the name of the security 
**      principal associated with the server. A principal name is 
**      required if you use DCE or CyberSAFE security and the server's 
**      network name does not match its security principal name.
**
**      -U user_name is an optional argument that specifies the user's 
**      network user name. If a user name is not supplied, the name
**      associated with the default credential of the process is used.
**      When using DCE, both of the -U and -K parameters must be 
**      specified or both must be omitted.
**
**      -K keytab_filename is the full path to a DCE keytab file that
**      contains a key table for the user specified with the -U option. 
**      Specify the -K argument only when using DCE security.
**
**      -Z security_mechanism specifies the name of a security 
**      mechanism to use on the connection; the mechanism name
**      determines which security driver is used for the connection. 
**      This option is necessary only if the libtcl.cfg file lists 
**      more than one security mechanism and if you want to use a mechanism
**      other than the default. (The default is the first mechanism listed
**      in the [SECURITY] section). 
**     
**      See the Open Client/Server Configuration Guide for information
**      about setting up security mechanisms and drivers. Also see the 
**      Open Client/Server Release Bulletin for (possible) updated
**      information.
**
** Returns
** -------
**      0 on successful execution
**      1 on error
*/

/*
** Operating system header files required by this application
*/
#include        <stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>

/*
** SYBASE Client-Library header file.
*/
#include	<ctpublic.h>

/*
** example.h has common defines for the Client-Library
** example programs. exutils.c contains the "ex_" utility 
** routines used by the Client-Library example programs.
*/
#include	"example.h"
#include        "exutils.h"

/*
** Define and declare a data structure that we use
** to set the connection properties that enable
** the available security services.
*/
typedef struct _sec_service {
        CS_INT  service;
	CS_CHAR *name;
} SEC_SERVICE;

static SEC_SERVICE Sec_services[] = {
  { CS_SEC_NETWORKAUTH,     "Network user authentication" },
  { CS_SEC_MUTUALAUTH,      "Mutual client/server authentication" }, 
  { CS_SEC_CONFIDENTIALITY, "Data confidentiality" },
  { CS_SEC_INTEGRITY,       "Data integrity" },
  { CS_SEC_DATAORIGIN,      "Data origin stamping" },
  { CS_SEC_DETECTREPLAY,    "Data replay detection" },
  { CS_SEC_DETECTSEQ,       "Data out-of-sequence detection" },
  { CS_SEC_CHANBIND,        "Channel binding" },
  { CS_UNUSED,              "" }
};

/*
** This structure holds the values for command-line arguments
** and their lengths.
*/
typedef struct _sec_args {
       CS_CHAR secmech[CS_MAX_CHAR];    /* Security mechanism      */
       CS_INT  secmechlen;
       CS_CHAR sname[CS_MAX_CHAR];      /* Server name             */
       CS_INT  snamelen;
       CS_CHAR principal[CS_MAX_CHAR];  /* Server principal name   */
       CS_INT  plen;
       CS_CHAR keytab[CS_MAX_CHAR];     /* Keytab file name        */
       CS_INT  klen;
       CS_CHAR uname[CS_MAX_CHAR];      /* User name               */
       CS_INT  unamelen;
} SEC_ARGS;

/*
** Global application name.
*/
CS_STATIC CS_CHAR Ex_appname[] = "secct";

/*
** Forward declarations for internal routines.
*/
CS_STATIC CS_RETCODE SecServices PROTOTYPE ((
        CS_CONNECTION *conn,
	CS_CHAR       *secmech,
	CS_INT        secmechlen ));

CS_STATIC CS_RETCODE proc_sec_args PROTOTYPE((
	SEC_ARGS *sec_args,
	CS_INT argc,
	CS_CHAR **argv	));

CS_STATIC CS_INT arg_cp PROTOTYPE ((
        CS_CHAR *arg,
	CS_CHAR *buf,
        CS_INT  buflen,
        CS_INT  *outlen ));

/*
** main() -- entry point for this example program.
**
** Parameters:
**    See "Usage" in the block comment at the top of this file.
**
** Returns:
**      EX_EXIT_ERROR  or EX_EXIT_SUCCEED
*/
int
main(int argc, char *argv[])
{
	CS_CONNECTION	*conn = (CS_CONNECTION *)NULL;
	CS_CONTEXT	*cp;
	SEC_ARGS        sec_args;

	EX_SCREEN_INIT();

	fprintf(stdout, "Security services client example program\n\n");

	/* 
	** Initialize CS-Library and Client-Library and 
	** install Client-Library error handlers.
	*/
	if (ex_init(&cp) != CS_SUCCEED)
	{
		ex_panic("secct: Can't initialize Client-Library. Exiting\n");
	}

	/* 
	** Allocate a connection handle for the connection attempt.
	*/
	if (ct_con_alloc(cp, &conn) != CS_SUCCEED)
	{
	        (CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
	        (CS_VOID)cs_ctx_drop(cp);
		ex_panic("secct: ct_con_alloc() failed. Exiting\n");
	}

	/*
	** Retrieve command-line options.
	*/
	if ( proc_sec_args(&sec_args, argc, argv) == CS_FAIL)
	{
	        /*
		** proc_sec_args() already printed a description
		** of the error.
		*/
	        (CS_VOID)ct_con_drop(conn);
	        (CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
	        (CS_VOID)cs_ctx_drop(cp);
                ex_panic("proc_sec_args() failed. Exiting.");
	}

	/* 
	** Set user name.
	*/
	if (sec_args.unamelen > 0
	    && ct_con_props(conn, CS_SET, CS_USERNAME, 
			    sec_args.uname, sec_args.unamelen,
			     (CS_INT *)NULL) != CS_SUCCEED)
	{
	        (CS_VOID)ct_con_drop(conn);
	        (CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
	        (CS_VOID)cs_ctx_drop(cp);
		ex_panic("secct: ct_con_props(USERNAME) failed.");
	}

	/*
	** Set server principal name.
	*/
	if (sec_args.plen > 0
	    && ct_con_props(conn, CS_SET, CS_SEC_SERVERPRINCIPAL, 
			    sec_args.principal, sec_args.plen,
			    (CS_INT *)NULL) != CS_SUCCEED)
	{
	        (CS_VOID)ct_con_drop(conn);
	        (CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
	        (CS_VOID)cs_ctx_drop(cp);
		ex_panic("secct: ct_con_props(SEC_SERVERPRINCIPAL) failed.");
	}

	/*
	** Set keytab filename.
	*/
	if (sec_args.klen > 0
	    && ct_con_props(conn, CS_SET, CS_SEC_KEYTAB, 
			    sec_args.keytab, sec_args.klen,
			    (CS_INT *)NULL) != CS_SUCCEED)
	{
	        (CS_VOID)ct_con_drop(conn);
	        (CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
	        (CS_VOID)cs_ctx_drop(cp);
		ex_panic("secct: ct_con_props(SEC_KEYTAB) failed.");
	}

	/*
	** Configure security services to be used on the connection.
	** SecServices() enables all services that are supported by
	** the security mechanism.
	*/
	if (SecServices(conn, sec_args.secmech, sec_args.secmechlen) 
	    != CS_SUCCEED)
	{
	        /*
		** SecServices() already printed a description
		** of the error.
		*/
	        (CS_VOID)ct_con_drop(conn);
	        (CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
	        (CS_VOID)cs_ctx_drop(cp);
                ex_panic("SecServices() failed. Exiting.");
	}

	/*
	** Set application name.
	*/
	if (ct_con_props(conn, CS_SET, CS_APPNAME, Ex_appname, 
		CS_NULLTERM, (CS_INT *)NULL) != CS_SUCCEED)
	{
	        (CS_VOID)ct_con_drop(conn);
	        (CS_VOID)ct_exit(cp, CS_FORCE_EXIT);
	        (CS_VOID)cs_ctx_drop(cp);
		ex_panic("secct: ct_con_props(CS_APPNAME) failed.");
	}

	/*
	** Attempt a connection to the server.
	*/
	if (ct_connect(conn, sec_args.sname, sec_args.snamelen) 
	    != CS_SUCCEED)
	{
	        ex_error("Connect attempt failed.\n");
	}
	else
	{
	        /*
		** Connection was successfully opened. Tell the 
		** user about it, then close the connection. 
		*/
	        if (ct_con_props(conn, CS_GET, CS_SERVERNAME,
				 &sec_args.sname, CS_MAX_CHAR,
				 NULL) != CS_SUCCEED)
		{
		        ex_error("ct_con_props(SERVERNAME) failed.");
		}
		else
		{
		        fprintf(stdout, "\nConnection to %s successful.\n", 
				sec_args.sname);
		}
		
	        (CS_VOID)ct_close(conn, CS_UNUSED);
	}

	/* 
	** Release allocated control structures.
	*/
	(CS_VOID)ct_con_drop(conn);
	(CS_VOID)ex_ctx_cleanup(cp, CS_UNUSED);

	return(EX_EXIT_SUCCEED);
}

/*
** SecServices() -- Enable all supported
**    security services for a connection.
**
** Parameters
**    conn -- Pointer to the connection structure.
**    mechname -- An optional security mechanism name (if not supplied,
**         the default security mechanism is used.)
**    mechnamelen -- The length of mechname, in bytes, or 0 if mechname
**         is NULL.
**
** Returns
**    CS_SUCCEED or CS_FAIL.
*/
CS_STATIC CS_RETCODE 
SecServices(CS_CONNECTION *conn, CS_CHAR *mechname, CS_INT mechnamelen)
{
	CS_BOOL    boolval;
	CS_INT     outlen;
	CS_CHAR    buf[CS_MAX_CHAR];
	int        i;

	/*
	** Set the security mechanism name if a mechanism name was supplied.
	*/
	if (mechnamelen > 0 
	    && mechname != NULL
	    && ct_con_props(conn, CS_SET, CS_SEC_MECHANISM,
			    mechname, mechnamelen, NULL) != CS_SUCCEED)
	{
	        fprintf(stdout, 
	       		"ct_con_props(CS_SEC_MECHANISM) failed.\n");
	        return CS_FAIL;
	}

	/*
	** Loop through the security service properties to see
	** which ones are supported by the security driver.
	** If a service is supported, set the property to
	** enable the service.
	*/
	fprintf(stdout, "Requesting security services:\n");
	for (i = 0;
	     Sec_services[i].service != CS_UNUSED;
	     i++)
	{
	        /*
                ** Determine whether the driver supports the service.
		*/
                if (ct_con_props(conn, CS_SUPPORTED, 
				 Sec_services[i].service, &boolval,
				 CS_UNUSED, NULL) != CS_SUCCEED)
		{
		        fprintf(stdout, 
				"ct_con_props(CS_SUPPORTED, %s) failed.\n", 
				Sec_services[i].name);
		        return CS_FAIL;
		}

		if (boolval == CS_TRUE)
		{
		        fprintf(stdout, "\t%s\n", 
				Sec_services[i].name);
		}
		
		if (boolval == CS_TRUE
		    && ct_con_props(conn, CS_SET, 
				    Sec_services[i].service, &boolval,
				    CS_UNUSED, NULL) != CS_SUCCEED)
		{
		        fprintf(stdout, 
				"ct_con_props(CS_SET, %s) failed.\n", 
				Sec_services[i].name);
		        return CS_FAIL;
		}
	}

	fprintf(stdout, "\n");

	if (ct_con_props(conn, CS_GET, CS_SEC_MECHANISM, buf,
			 CS_MAX_CHAR, &outlen) != CS_SUCCEED)
	{
	        fprintf(stdout, "Could not get security mechanism name.\n");
	        return CS_FAIL;
	}

	fprintf(stdout, "Security mechanism is '%.*s'\n\n", 
		(int)outlen, buf);

	return CS_SUCCEED;
}

/*
** ARG_CP -- Copy a command-line argument. 
**
** PARAMETERS
**   arg -- argument to copy. Must be null terminated.
**   buf -- destination space.
**   buflen -- length of buf
**   outlen -- length of arg, not including null-terminator,
**       or 0 if buffer would overflow. 
**  
** RETURNS
**   0 to indicate success;
**   > 0 to indicate buffer overflow.
*/
CS_STATIC CS_INT 
arg_cp(CS_CHAR *arg, CS_CHAR *buf, CS_INT buflen, CS_INT *outlen)
{
        (CS_VOID)strncpy(buf, arg, buflen);
	*outlen = strlen(arg);

	if (*outlen >= buflen)
	{
	        buflen = *outlen;
	        *outlen = 0;
		return buflen;
	}

	return 0;
}

/*
** PROC_SEC_ARGS
**
** 	This routine parses the command line arguments passed to the
** 	Open Server application.
**
** 	The syntax expected is as follows:
**
** 	sample_name [open server network name] 
**                  [-R server principal name] 
**                  [-K keytab file name]
**                  [-U user name]
*/
CS_STATIC CS_RETCODE
proc_sec_args(SEC_ARGS *sec_args, CS_INT argc, CS_CHAR **argv)
{
	/*
	** Initialization.
	*/
	sec_args->secmech[0] = '\0';
	sec_args->secmechlen = 0;
	sec_args->sname[0] = '\0';
	sec_args->snamelen = 0;
	sec_args->principal[0] = '\0';
	sec_args->plen = 0;
	sec_args->keytab[0] = '\0';
	sec_args->klen = 0;
	sec_args->uname[0] = '\0';
	sec_args->unamelen = 0;

	/*
	** If we have arguments, the first one should be the
	** Open Server name.
	*/
	if (argc > 1 )
	{
		if( argv[1][0] == '-')
		{
			/*
			** We don't have a server name
			** argument, so just jump to the first flag.
			*/
			argv++;
		}
		else
		{
			/*
			** The first argument is the server name.
			*/
		        if (arg_cp(argv[1], sec_args->sname, CS_MAX_CHAR,
				   &(sec_args->snamelen)) != 0)
			{
			        fprintf(stdout, 
					"ERROR: Server name too long.\n");
				return CS_FAIL;
			}
			
			/*
			** Skip to the second argument, if any.
			*/
			argv++;
			argv++;
		}
	}
	else
	{
		/*
		** We don't have any arguments, so we're all done.
		*/
		return CS_SUCCEED;
	}

	for (; *argv != (CS_CHAR *)NULL; argv++ )
	{
		/*
		**	Get the security mechanism name, if available.
		*/
		if ( strcmp(*argv, "-Z") == 0 )
		{
			argv++;
			if (arg_cp(*argv, sec_args->secmech,
				   CS_MAX_CHAR, &(sec_args->secmechlen))
			    != 0)
			{
			        fprintf(stdout, 
					"ERROR: Mechanism name too long.\n");
				return CS_FAIL;
			}			
			continue;
		}

		/*
		**	Get the server principal name, if available.
		*/
		else if ( strcmp(*argv, "-R") == 0 )
		{
			argv++;
			if (arg_cp(*argv, sec_args->principal,
				   CS_MAX_CHAR, &(sec_args->plen))
			    != 0)
			{
			        fprintf(stdout, 
					"ERROR: Principal name too long.\n");
				return CS_FAIL;
			}			
			continue;
		}

		/*
		**	Get the keytab file name, if available.
		*/
		else if ( strcmp(*argv, "-K") == 0 )
		{
			argv++;
			if (arg_cp(*argv, sec_args->keytab,
				   CS_MAX_CHAR, &(sec_args->klen))
			    != 0)
			{
			        fprintf(stdout, 
					"ERROR: Keytab filename too long.\n");
				return CS_FAIL;
			}
			continue;
		}

		/*
		**	Get the user name, if available.
		*/
		else if ( strcmp(*argv, "-U") == 0 )
		{
			argv++;
			if (arg_cp(*argv, sec_args->uname,
				   CS_MAX_CHAR, &(sec_args->unamelen))
			    != 0)
			{
			        fprintf(stdout, 
					"ERROR: user name too long.\n");
				return CS_FAIL;
			}
			continue;
		}

		/*
		** Unknown argument.
		*/
		else
		{
			fprintf(stdout, 
			        "ERROR: Unknown argument '%s'.\n", 
				*argv);
			return CS_FAIL;
		}
	}

	/*
	** All done.
	*/
	return CS_SUCCEED;
}


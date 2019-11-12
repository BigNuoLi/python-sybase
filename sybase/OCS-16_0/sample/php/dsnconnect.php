<?php
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
**	This example is meant to be run from a command line.
**	When called with the '-i' command line parameter:
**	The user is queried for a server name, user name and password.
**	A sample DSN string is generated, the user may change it.
**	A connection, using the DSN string is opened to the server.
**	The server name we're connected to, the login used and the current
**	database are retrieved and displayed.
**	The connection is closed again.
**	When called without the '-i' (or a different) command line parameter:
**	A valid DSN string is generated with the default server name,
**	user name and password as defined in sample_utils.php, a connection
**	to the server is opened, if successful a message confirming this is
**	displayed and the connection is closed again.
**
** Requirements
** ------------
**	An ASE with a valid user account on it.
*/
require_once("sample_utils.php");

/*
** Switch PHP error and warning messages information on.
*/
ini_set('display_errors',1);
error_reporting(E_ALL|E_STRICT);

/*
** Were we called with the '-i' option?
*/
if (($argc > 1) && (trim($argv[1]) === "-i"))
{
	/*
	** Verbose mode
	*/
	$qry_user = TRUE;
}
else
{
	/*
	** Silent mode
	*/
	$qry_user = FALSE;
}

/*
** Get the values for name of the 's'erver, 'u'ser and
** the 'p'assword to connect with.
*/
smpl_get_conn_input("sup", $qry_user);

/*
** Create a valid DSN string.
*/
$dsnstring = "servername=$Dsquery;user=$Username;password=$Passwd";

if ($qry_user === TRUE)
{
	$stdin = fopen ("php://stdin", "r");
	print "\nDSN string to connect with\n$dsnstring ?\n";
	$line = fgets($stdin);
	if(trim($line) != '')
	{
		$dsnstring = trim($line);
	}
	
	fclose($stdin);
}

/*
** Connect to the server.
*/
$conn = sybase_connect($dsnstring);
if ($conn == FALSE)
{
	die("$Appname: Failed to connect to $Dsquery.\n");
}

if ($qry_user === TRUE)
{
	/*
	** Which server are we connected to?
	*/
	print "We're connected to '". smpl_qry_1_1("select @@servername") . "'\n";
	
	/*
	** With which account are we logged in?
	*/
	print "We're logged in as '" . smpl_qry_1_1("select suser_name()") . "'\n";
	
	/*
	** In which database?
	*/
	print "The current database is '" . smpl_qry_1_1("select db_name()") . "'\n";
}
else
{
	print "Successfully established a connection, exiting.\n";
}

/*
** Close the connection now.
*/
if ($conn != NULL)
{
	$closed = sybase_close($conn);
	if ($closed != TRUE)
	{
		die("$Appname: Failed to close the connection.\n");
	}
}

?>

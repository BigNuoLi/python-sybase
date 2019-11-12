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
**	The user is queried for a server name, user name and password.
**	A connection is opened to the server, the pubs2 database is 'use'd
**	and a list of first and last names in the 'authors' table is displayed.
**	The connection is closed again.
**
** Requirements
** ------------
**	An ASE with pubs2 installed, and a valid user account on it.
*/

global $Dsquery;
global $Dbname;
global $Username;
global $Passwd;
global $Appname;

/*
** Helper function that queries the user for input.
*/
function
firstapp_init()
{
	global $Dsquery, $Dbname, $Username, $Passwd, $Appname;

	/*
	** Initialize the global variables.
	*/
	$Appname = basename($_SERVER['SCRIPT_FILENAME'], ".php");
	$Dsquery = getenv("DSQUERY");
	$Dbname = "pubs2";
	$Username = "sa";
	$Passwd = "";

	$stdin = fopen ("php://stdin","r");
	echo "Server to connect to ($Dsquery)? ";
	$line = fgets($stdin);
	if(trim($line) != '')
	{
	    $Dsquery = trim($line);
	}

	echo "User name to connect with ($Username)? ";
	$line = fgets($stdin);
	if(trim($line) != '')
	{
	    $Username = trim($line);
	}

	echo "Password to connect with ($Passwd)? ";
	$line = fgets($stdin);
	if(trim($line) != '')
	{
	    $Passwd = trim($line);
	}

	fclose($stdin);

	return TRUE;
}


/*
** main
** This is the entry point of the PHP script
**
** Switch PHP error and warning messages information on.
*/
ini_set('display_errors',1);
error_reporting(E_ALL|E_STRICT);

/*
** Get the coordinates to connect with.
*/
firstapp_init();

/*
** Connect to the server.
*/
$conn = sybase_connect($Dsquery, $Username, $Passwd);
if ($conn == FALSE)
{
	die("$Appname: Failed to connect to $Dsquery.\n");
}

/*
** First use the database specified.
** We'll create a query for this.
*/
$aquery = "use $Dbname";
$resultset = sybase_query($aquery, $conn);
if ($resultset != TRUE)
{
	die("$Appname: sybase_query(\$aquery, \$conn) for $aquery failed\n");
}

/*
** The query that will get us the names in the authors table.
*/
$aquery = "select au_fname, au_lname from authors";

/*
** Note that we're not supplying the optional $connection parameter to
** sybase_query(); the most recently opened connection will be used,
** the one where we changed database.
*/
$resultset = sybase_query($aquery);
if ($resultset != TRUE)
{
	die("$Appname: sybase_query(\$aquery) for $aquery failed\n");
}

print "\nThe names in the authors table in $Dbname:\n";

/*
** Now get the row data as an associative array
*/
while ($row = sybase_fetch_assoc($resultset))
{
	/*
	** Print each field.
	*/
	foreach ($row as $col=>$val)
	{
		print "\t\t$val";
	}

	print "\n";
}

/*
** Clean up the resultset, we're done with it.
*/
sybase_free_result($resultset);

/*
** Close our connection to the database
*/
if ($conn != NULL)
{
	$closed = sybase_close($conn);
	if ($closed != TRUE)
	{
		die("$Appname: Failed to close the connection to $Dsquery.\n");
	}
}

?>

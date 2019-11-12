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
**	This file contains helper functions and global variables for the
**	SAP ASE PHP driver samples.
**
** Requirements
** ------------
**	-
*/

global $Dsquery;
global $Dbname;
global $Username;
global $Passwd;
global $Appname;

/*
** smpl_get_conn_input
** 	Helper function that provides connection/login information.
**	Either the user is queried for it, or default values are returned.
**
** Parameter
**	$coordinates	a string that is a concatenation of the characters
**			s	for server name
**			u	for user name
**			p	for password
**	if $coordinates is not provided (note you'll get warnings then)
**	or an empty string:
**			for all of the above
**	$qry_user	a boolean indicating whether to query the user for
**			values other than the defaults
**
** Returns
**	-
*/
function
smpl_get_conn_input($coordinates, $qry_user)
{
	global $Dsquery, $Dbname, $Username, $Passwd, $Appname;

	/*
	** Initialize the application name, a global variable.
	*/
	$Appname = basename($_SERVER['SCRIPT_FILENAME'], ".php");
	
	if (strlen($coordinates) == 0)
	{
		/*
		** Query for everything now.
		*/
		$coordinates = "sup";
	}

	if ($qry_user === TRUE)
	{
		$stdin = fopen ("php://stdin","r");
	}

	/*
	** Only set the global variables that are requested in $coordinates
	*/
	if (stripos($coordinates, "s") !== FALSE)
	{
		/*
		** Set default value
		*/
		$Dsquery = getenv("DSQUERY");
		if ($qry_user === TRUE)
		{
			print "Server to connect to ($Dsquery)? ";
			$line = fgets($stdin);
			if(trim($line) != '')
			{
				$Dsquery = trim($line);
			}
		}
	}

	if (stripos($coordinates, "u") !== FALSE)
	{
		/*
		** Set default value
		*/
		$Username = "sa";
		if ($qry_user === TRUE)
		{
			print "User name to connect with ($Username)? ";
			$line = fgets($stdin);
			if(trim($line) != '')
			{
				$Username = trim($line);
			}
		}
	}

	if (stripos($coordinates, "p") !== FALSE)
	{
		/*
		** Set default value
		*/
		$Passwd = "";
		if ($qry_user === TRUE)
		{
			print "Password to connect with ($Passwd)? ";
			$line = fgets($stdin);
			if(trim($line) != '')
			{
				$Passwd = trim($line);
			}
		}
	}

	if ($qry_user === TRUE)
	{
		fclose($stdin);
	}

	return;
}

/*
** smpl_qry_1_1
**	Helper function that executes a simple query which returns
**	just 1 column in 1 row.
**
** Parameter
**	$qrystring	a string containing the query
**
** Returns
**	empty string when no results were returned otherwise
**	the result of the query
*/
function
smpl_qry_1_1($qrystring)
{
	$resultset = sybase_query($qrystring);
	if ($resultset != TRUE)
	{
		die("$Appname: sybase_query() for $qrystring failed\n");
	}
	
	/*
	** We only expect one row with one column
	*/
	$row = sybase_fetch_array($resultset);
	
	/*
	** Clean up the resultset.
	*/
	sybase_free_result($resultset);

	if ($row === FALSE)
	{
		return "";
	}
	else
	{
		return  $row[0];
	}
}

?>

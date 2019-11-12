#
# Copyright Notice and Disclaimer
# -------------------------------
#       (c) Copyright 2013.
#       SAP AG or an SAP affiliate company. All rights reserved.
#       Unpublished rights reserved under U.S. copyright laws.
#
#       SAP grants Licensee a non-exclusive license to use, reproduce,
#       modify, and distribute the sample source code below (the "Sample Code"),
#       subject to the following conditions:
#
#       (i) redistributions must retain the above copyright notice;
#
#       (ii) SAP shall have no obligation to correct errors or deliver
#       updates to the Sample Code or provide any other support for the
#       Sample Code;
#
#       (iii) Licensee may use the Sample Code to develop applications
#       (the "Licensee Applications") and may distribute the Sample Code in
#       whole or in part as part of such Licensee Applications, however in no
#       event shall Licensee distribute the Sample Code on a standalone basis;
#
#       (iv) and subject to the following disclaimer:
#       THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
#       INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
#       AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
#       SAP AG or an SAP affiliate company OR ITS LICENSORS BE LIABLE FOR ANY DIRECT, 
#	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#       BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
#       OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#       ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
#       TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
#       USE OF THE SAMPLE CODE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
#       DAMAGE.
#
# Note 
# ----
#	no path to perl, use perl ./exec.pl (Unix/Linux)
# 	or perl .\exec.pl (Windows) or alternatively, add a path
# 	on the first line to accomodate for your specific platform:
#
# 	Unix/linux example: #!/usr/local/bin/perl
# 	Windows example:    #!perl
#
# Description
# -----------
#    	This is a Perl example program that shows basic usage of stored
#	procedures in Perl. It connects to a server, creates two stored
#	procedures, calls prepare and bind/executes the procedures.
#	Results are written to STDOUT. The example drops the stored 
#	procedures, disconnects and exits.
#
#    	Make sure the correct version of the Perl interpreter is in your
#    	path.
#
# Input
# ------
#    	None.
#
# Database 
# --------
#    	This program requires the pubs2 database.
#
# Requirements
# ------------
#    	ASE 15.7 and OCS 15.7 client installation
#    	Linux/Unix: Perl 5.14.0 or 5.14.1 installation.
#    	Windows: ActiveState Perl version:
#		ActivePerl-5.14.1.1401-MSWin32-x64-294969 or later.
#
#    	DBI installed, version 1.616
#    	Set PERL5LIB to $SYBASE/$SYBASE_OCS/perl/<mode>/<r>
#    	where <mode> is sybaseperl_32 or sybaseperl_64
#    	and <r> is either "r" for re-entrant or omitted.
#
#    	Example for linux on Intel, 64bit, re-entrant using
#    	non-debug libraries: 
#
#    	$SYBASE/$SYBASE_OCS/perl/sybaseperl_64r/lib/lib
#    	$SYBASE/$SYBASE_OCS/perl/sybaseperl_64r/lib/arch
#
#    	Example for Windows on Intel, 64bit, non-debug libraries: 
#
#    	$SYBASE/$SYBASE_OCS/perl/sybaseperl_64/dll/lib
#    	$SYBASE/$SYBASE_OCS/perl/sybaseperl_64/dll/arch
#
#
use strict;

use DBI qw(:sql_types);
use DBD::SybaseASE;

require_version DBI 1.51;

my $uid = "sa";
my $pwd = "";
my $srv = $ENV{"DSQUERY"} || die 'DSQUERY appears not set';
my $dbase = "tempdb";

my $dbh;
my $sth;
my $rc;

my $col1;
my $col2;
my $col3;
my $col4;

# Connect to the target server.
#
$dbh = DBI->connect("dbi:SybaseASE:server=$srv;database=$dbase",
        $uid, $pwd, {PrintError => 1});

# One way to exit if things fail.
#
if(!$dbh) {
    warn "Connection failed, check if your credentials are set correctly?\n";
    exit(0);
}

# Ignore errors on scale for numeric. There is one marked call below
# that will trigger a scale error in ASE. Current settings suppress
# this.
#
$dbh->do("set arithabort off") 
	|| die "ASE response not as expected";

# Drop the stored procedures in case they linger in ASE.
#
$dbh->do("if object_id('my_test_proc') != NULL drop proc my_test_proc")
	|| die "Error processing dropping of an object";

$dbh->do("if object_id('my_test_proc_2') != NULL drop proc my_test_proc_2")
	|| die "Error processing dropping of an object";

# Create a stored procedure on the fly for this example. This one 
# takes input args and echo's them back.
#
$dbh->do(qq{
create proc my_test_proc \@col_one varchar(25), \@col_two int, 
	\@col_three numeric(5,2), \@col_four date
as
    select \@col_one, \@col_two, \@col_three, \@col_four
}) || die "Could not create proc";


# Create another stored procedure on the fly for this example. 
# This one takes dumps the pubs2..authors table. Note that the
# format used for printing is defined such that only four columns
# appear in the output list.
#
$dbh->do(qq{
create proc my_test_proc_2 
as
    select * from pubs2..authors
}) || die "Could not create proc_2";

# Call a prepare stmt on the first proc.
#
$sth = $dbh->prepare("exec my_test_proc \@col_one = ?, \@col_two = ?, 
	\@col_three = ?, \@col_four = ?") 
		|| die "Prepare exec my_test_proc failed";

# Bind values to the columns. If SQL type is not given the default
# is SQL_CHAR. Param 3 gives scale errors if arithabort is disabled.
#
$sth->bind_param(1, "a_string");
$sth->bind_param(2, 2, SQL_INTEGER);
$sth->bind_param(3, 1.5411111, SQL_DECIMAL);
$sth->bind_param(4, "Jan 12 2012", SQL_DATETIME);

# Execute the first proc.
#
$rc = $sth->execute || die "Could not execute my_test_proc";

# Print the bound args
#
dump_info($sth);

# Execute again, using different params.
#
$rc = $sth->execute("one_string", 25, 333.2, "Jan 1 2012")
	|| die "Could not execute my_test_proc";

dump_info($sth);

# Enable retrieving the proc status.
$sth->{syb_do_proc_status} = 1;

$rc = $sth->execute(undef, 0, 3.12345, "Jan 2 2012")
        || die "Could not execute my_test_proc";
dump_info($sth);

$rc = $sth->execute("raisin", 1, 1.78, "Jan 3 2012")
       || die "Could not execute my_test_proc";
dump_info($sth);

$rc = $sth->execute(undef, 0, 3.2233, "Jan 4 2012")
       || die "Could not execute my_test_proc";
dump_info($sth);


$rc = $sth->execute(undef, 0, 3.2234, "Jan 5 2012")
	|| die "Could not execute my_test_proc";
dump_info($sth);

$rc = $sth->execute("raisin_2", 1, 3.2235, "Jan 6 2012")
	|| die "Could not execute my_test_proc";
dump_info($sth);
$rc = $sth->execute(undef, 0, 3.2236, "Jan 7 2012")
	|| die "Could not execute my_test_proc";
dump_info($sth);

# End of part one, generate blank line.
#
print "\n";

# Undef the handles.
#
undef $sth;
undef $rc;

# Prepare the second stored proc.
#
$sth = $dbh->prepare("exec my_test_proc_2") 
		|| die "Prepare exec my_test_proc_2 failed";

# Execute and print
#
$rc = $sth->execute || die "Could not execute my_test_proc_2";
dump_info($sth);

#
# An example of a display/print function.
#
sub dump_info {
    my $sth = shift;
    my @display;

    do {
	while(@display = $sth->fetchrow) {
	  foreach (@display) {
	     $_ = '' unless defined $_;
	  }
          $col1 = $display[0];
          $col2 = $display[1];
          $col3 = $display[2];
          $col4 = $display[3];

	  # Proc status is suppressed here, assume proc
	  # execution was always successful. Show proc
	  # status by changing the write statement below.
	  #
	  #write;
	  write unless $col1 eq 0;
	}
    } while($sth->{syb_more_results});
}

#
# The FORMAT template for this example.
#
format STDOUT_TOP =

Column1         Column2       Column3       Column4
-----------     ------------  -----------   ------------
.

# Treat all data as left-justified strings
#
format STDOUT =
@<<<<<<<<<<<<   @<<<<<<<<<<<< @<<<<<<<<<<<< @<<<<<<<<<<<<
$col1, $col2, $col3, $col4
.

# The End.....
#
$dbh->do("drop proc my_test_proc");
$dbh->do("drop proc my_test_proc_2");

$dbh->disconnect;

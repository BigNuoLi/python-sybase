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
#	no path to perl, use perl ./retrieve.pl (Unix/Linux)
# 	or perl .\retrieve.pl (Windows) or alternatively, add a path
# 	on the first line to accomodate for your specific platform:
#
# 	Unix/linux example: #!/usr/local/bin/perl
# 	Windows example:    #!perl
#
# Description
# -----------
#    	This is a Perl example program that retrieves rows from the pubs2
#	authors table. It then goes on to insert into tempdb, appending
#	new rows doing a batch insert. Then the updated authors table is
#	printed to STDOUT. It then disconnects and exits.
#
#    	Make sure the correct version of the Perl interpreter and the DBI
#	is in your path.
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

use DBI ();
use DBD::SybaseASE ();

require_version DBI 1.51;

# trace(n) where n ranges from 0 - 15. 
# use 2 for sufficient detail.
#DBI->trace(2); # 0 - 15, use 2 for sufficient detail

# Login credentials, handles and other variables. 
#
my $uid = "sa";
my $pwd = "";
my $srv = $ENV{"DSQUERY"} || die 'DSQUERY appears not set';
my $dbase = "tempdb";
my $temp_table = "$dbase..authors";

my $rows;
my $col1;
my $col2;
my $dbh;
my $sth;
my $rc;

# Connect to the target server:
#
$dbh = DBI->connect("dbi:SybaseASE:server=$srv;database=$dbase",
        $uid, $pwd, {PrintError => 0, AutoCommit => 0}) 
	|| die "Connect failed, did you set correct credentials?";

# Switch to the pubs2 database.
#
$rc = $dbh->do("use pubs2") || die "Could not change to pubs2";

# Retrieve 2 columns from pubs2..authors table.
#
$sth = $dbh->prepare(
	"select au_lname, city from authors where state = 'CA'")
	|| die "Prepare select on authors table failed";

$rc = $sth->execute 
	|| die "Execution of first select statement failed";

# We may have rows now, present them.
#
$rows = dump_info($sth);
print "\nTotal # rows: $rows\n\n";

# Switch back to tempdb, we take a copy of pubs2..authors
# and insert some rows and present these.
#
$rc = $dbh->do("use $dbase") || die "Could not change to $dbase";

# Drop the authors table in tempdb if present
#
$rc = $dbh->do("if object_id('$temp_table') != NULL drop table $temp_table")
	|| die "Could not drop $temp_table";

# No need to create a tempdb..authors table as the select into will
# do that.

$rc = $dbh->do("select * into $temp_table from pubs2..authors") 
	|| die "Could not select into table $temp_table";


# Example of a batch insert...
#
$sth = $dbh->prepare("
	insert into $temp_table 
	(au_id, au_lname, au_fname, phone, address, city, state, 
		country, postalcode) values 
	('172-39-1177', 'Simpson', 'John', '408 496-7223', 
		'10936 Bigger Rd.', 'Menlo Park', 'CA', 'USA', '94025')

	insert into $temp_table 
	(au_id, au_lname, au_fname, phone, address, city, state, 
		country, postalcode) values
	('212-49-4921', 'Greener', 'Morgen', '510 986-7020', 
		'309 63rd St. #411', 'Oakland', 'CA', 'USA', '94618')

	insert into $temp_table 
	(au_id, au_lname, au_fname, phone, address, city, state, 
		country, postalcode ) values
	('238-95-4766', 'Karson', 'Chernobyl', '510 548-7723', 
		'589 Darwin Ln.', 'Berkeley', 'CA', 'USA', '94705')

	insert into $temp_table 
	(au_id, au_lname, au_fname, phone, address, city, state, 
		country, postalcode ) values
	('267-41-4394', 'OLeary', 'Mich', '408 286-2428', 
		'22 Cleveland Av. #14', 'San Jose', 'CA', 'USA', '95128')

	insert into $temp_table 
	(au_id, au_lname, au_fname, phone, address, city, state, 
		country, postalcode ) values
	('274-80-4396', 'Straight', 'Shooter', '510 834-2919', 
		'5420 College Av.', 'Oakland', 'CA', 'USA', '94609')

	insert into $temp_table 
	(au_id, au_lname, au_fname, phone, address, city, state, 
		country, postalcode ) values
	('345-22-1785', 'Smiths', 'Neanderthaler', '913 843-0462', 
		'15 Mississippi Dr.', 'Lawrence', 'KS', 'USA', '66044')

	insert into $temp_table 
	(au_id, au_lname, au_fname, phone, address, city, state, 
		country, postalcode ) values
	('405-56-7012', 'Bennetson', 'Abra', '510 658-9932', 
		'6223 Bateman St.', 'Berkeley', 'CA', 'USA', '94705')

	insert into $temp_table 
	(au_id, au_lname, au_fname, phone, address, city, state, 
		country, postalcode ) values
	('427-17-2567', 'Dullest', 'Annie', '620 836-7128', 
		'3410 Blonde St.', 'Palo Alto', 'CA', 'USA', '94301')

	insert into $temp_table 
	(au_id, au_lname, au_fname, phone, address, city, state, 
		country, postalcode ) values
	('527-72-3246', 'Greene', 'Mstar', '615 297-2723', 
		'22 Graybar House Rd.', 'Nashville', 'TN', 'USA', '37215')

	insert into $temp_table 
	(au_id, au_lname, au_fname, phone, address, city, state, 
		country, postalcode ) values
	('672-91-3249', 'Yapan', 'Okiko', '925 935-4228', 
		'3305 Silver Ct.', 'Walnut Creek', 'CA', 'USA', '94595')
");

$rc = $sth->execute || die "Could not insert row";

# Retrieve 2 columns from tempdb..authors table and present these
#
$sth = $dbh->prepare(
	"select au_lname, city from $temp_table where state = 'CA'")
	|| die "Prepare select on $temp_table table failed";

$rc = $sth->execute 
	|| die "Execution of second select statement failed";

# Output
#
$rows = dump_info($sth);
print "\nTotal # rows: $rows";
print "\n";

sub dump_info {
    my $sth = shift;
    my @display;
    my $rows = 0;

    while(@display = $sth->fetchrow) {
        $rows++;
	foreach (@display) {
	   $_ = '' unless defined $_;
	}
        $col1 = $display[0];
        $col2 = $display[1];
	write;
    }
    $rows;
}

# The FORMAT template for this example.
#
format STDOUT_TOP =

Lastname        City
-------------	------------ 
.

format STDOUT =
@<<<<<<<<<<<<	@<<<<<<<<<<<<
$col1, $col2
.

$dbh->disconnect;


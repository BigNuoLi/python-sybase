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
#	no path to perl, use perl ./firstapp.pl (Unix/Linux)
# 	or perl .\firstapp.pl (Windows) or alternatively, add a path
# 	on the first line to accomodate for your specific platform:
#
# 	Unix/linux example: #!/usr/local/bin/perl
# 	Windows example:    #!perl
#
# Description
# -----------
#    	This is the Perl equivalent example program that is printed in 
#    	Chapter 1 of the Client-Library Programmer's Guide. It connects 
#    	to a server, sends a select query, prints the rows, disconnects, 
#    	and exits.
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

use DBI ();
use DBD::SybaseASE ();

require_version DBI 1.51;

# trace(n) where n ranges from 0 - 15. 
# use 2 for sufficient detail.
#DBI->trace(2); # 0 - 15, use 2 for sufficient detail

# Login credentials. Default database chosen as 'tempdb'.
# Target database is 'pubs2', but this example shows usage
# of the dbh->do() method.
#
my $uid = "sa";
my $pwd = "";
my $srv = $ENV{"DSQUERY"} || die 'DSQUERY appears not set';
my $dbase = "tempdb";
my $rows;

# Column formatting helpers.
my $col1;
my $col2;
my $len;
my $filler_add;
my $filler_len;

# Connect to the target server:
my $dbh = DBI->connect("dbi:SybaseASE:server=$srv;database=$dbase",
        $uid, $pwd, {PrintError => 0, AutoCommit => 0});

if(!$dbh) {
    warn "Connection failed, check if your credentials are set correctly?\n";
    exit(0);
}

my $rc = $dbh->do("use pubs2");
if(!$rc){
    warn "Could not change to requested database\n";
    exit(0);
}

my $sth = $dbh->prepare(
	"select au_lname, city from authors where state = 'CA'");
if(!$sth){
    warn "Prepare select on authors table failed\n";
    exit(0);
}

$rc = $sth->execute || die 'Execution of statement failed';

$rows = 0;

# Formatted output
print "\n";
print "\tLastname\tCity\n\n";

while(my @data = $sth->fetchrow) {
    $rows++;
    foreach (@data) {
	$_ = '' unless defined $_;
    }
    $col1 = $data[0];
    $len = length($col1);

    # random len, 15 enough for this example
    $filler_len = 15 - $len;
    $filler_add = " " x $filler_len;

    $col2 = $data[1];
    print "\t$col1$filler_add\t$col2\n";
}

print "\n\tTotal # rows: $rows\n";

undef $sth;

$dbh->disconnect;


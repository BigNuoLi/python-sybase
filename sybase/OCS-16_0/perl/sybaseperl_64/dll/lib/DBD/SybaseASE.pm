#
# $Id: SybaseASE.pm version 0.80 2011/12/04 08:59:17 $
#
#  Copyright (c) 2012 Sybase Inc, All rights reserved
#
#   The Open source DBD downloaded file from which the perl language
#   modules are used is licensed under the Artistic License described at
#        http://dev.perl.org/licenses/artistic.html
#
#         Copyright (c) 1994 -1997 Tim Bunce
#         Copyright (c) 1996 -2011 Michael Peppler
#
# 
#
# NOTE from Sybase: 
#  1) The Sybase ASE database driver for Perl
#     i) Uses the Package name SybaseASE 
#     ii) Requires Sybase SDK 15.7 or later
#     Refer to the Sybase user documentations for more information
#  2) Sybase has no obligation to support SybaseASE.pm file if you make 
#     any modifications to it

{
	package DBD::SybaseASE;

	use DBI        ();
	use DynaLoader ();
	use Exporter   ();

	use Sys::Hostname ();

	@ISA = qw(DynaLoader Exporter);

	@EXPORT = qw(CS_ROW_RESULT CS_CURSOR_RESULT CS_PARAM_RESULT
	  CS_STATUS_RESULT CS_MSG_RESULT CS_COMPUTE_RESULT);

	$hostname  = Sys::Hostname::hostname();
	$init_done = 0;

	$VERSION   = '0.80';
	my $Revision = substr( q$Revision: 0.080 $, 10 );
	require_version DBI 1.51;

	bootstrap DBD::SybaseASE $VERSION;

	$drh = undef;    # holds driver handle once initialised

	sub driver {
		return $drh if $drh;
		my ( $class, $attr ) = @_;
		$class .= "::dr";
		($drh) = DBI::_new_drh(
			$class,
			{
				'Name'        => 'SybaseASE',
				'Version'     => $VERSION,
				'Attribution' => 'Sybase OCS',
			}
		);
		if ( $DBI::VERSION >= 1.51 && !$DBD::SybaseASE::init_done ) {
			DBD::SybaseASE::db->install_method('syb_date_fmt');
			DBD::SybaseASE::db->install_method('syb_isdead');
			DBD::SybaseASE::st->install_method('syb_ct_get_data');
			DBD::SybaseASE::st->install_method('syb_ct_data_info');
			DBD::SybaseASE::st->install_method('syb_ct_send_data');
			DBD::SybaseASE::st->install_method('syb_ct_prepare_send');
			DBD::SybaseASE::st->install_method('syb_ct_finish_send');
			DBD::SybaseASE::st->install_method('syb_describe');
			DBD::SybaseASE::st->install_method('syb_output_params');
			++$DBD::SybaseASE::init_done;
		}

		$drh;
	}

	sub CLONE {
		undef $drh;
	}

	1;
}

{
	package DBD::SybaseASE::dr;    # ====== DRIVER ======
	use strict;

	sub connect {
		my ( $drh, $dbase, $user, $auth, $attr ) = @_;
		my $server = $dbase || $ENV{DSQUERY} || 'SYBASE';

		my ($this) = DBI::_new_dbh(
			$drh,
			{
				'Name'         => $server,
				'Username'     => $user,
				'CURRENT_USER' => $user,
			}
		);

		DBD::SybaseASE::db::_login( $this, $server, $user, $auth, $attr )
		  or return undef;

		return $this;
	}

	sub data_sources {
		my @s;
		if ( $^O eq 'MSWin32' ) {
			open( INTERFACES, "$ENV{SYBASE}/ini/sql.ini" ) or return;
			@s = map { /\[(\S+)\]/i; "dbi:SybaseASE:server=$1" } grep /\[/i,
                          <INTERFACES>;
			close(INTERFACES);
		}
		else {
			open( INTERFACES, "$ENV{SYBASE}/interfaces" ) or return;
			@s = map { /^(\S+)/i; "dbi:SybaseASE:server=$1" } grep /^[^\s\#]/i,
			  <INTERFACES>;
			close(INTERFACES);
		}
		return @s;
	}
}

{
	package DBD::SybaseASE::db;    # ====== DATABASE ======
	use strict;

	use DBI qw(:sql_types);
	use Carp;

	sub prepare {
		my ( $dbh, $statement, @attribs ) = @_;

		# create a 'blank' sth

		my $sth = DBI::_new_sth( $dbh, { 'Statement' => $statement, } );

		DBD::SybaseASE::st::_prepare( $sth, $statement, @attribs )
		  or return undef;

		$sth;
	}

	sub tables {
		my $dbh     = shift;
		my $catalog = shift;
		my $schema  = shift || '%';
		my $table   = shift || '%';
		my $type    = shift || '%';
		$type =~ s/[\'\"\s]//g;    # strip quotes and spaces
		if ( $type =~ /,/ ) {      # multiple types
			$type = '['
			  . join( '', map { substr( $_, 0, 1 ) } split /,/, $type ) . ']';
		}
		else {
			$type = substr( $type, 0, 1 );
		}
		$type =~ s/T/U/;

		my $sth;
		if ( $catalog and $catalog ne '%' ) {
			$sth =
			  $dbh->prepare(
"select o.name from $catalog..sysobjects o, $catalog..sysusers u where o.type like '$type' and o.name like '$table' and o.uid = u.uid and u.name like '$schema'"
			  );
		}
		else {
			$sth =
			  $dbh->prepare(
"select o.name from sysobjects o, sysusers u where o.type like '$type' and o.name like '$table' and o.uid = u.uid and u.name like '$schema'"
			  );
		}

		$sth->execute;
		my @names;
		my $dat;
		while ( $dat = $sth->fetch ) {
			push( @names, $dat->[0] );
		}
		@names;
	}

	sub do {
		my ( $dbh, $statement, $attr, @params ) = @_;

		my $sth = $dbh->prepare( $statement, $attr ) or return undef;
		$sth->execute(@params) or return undef;
		return undef if $sth->err;
		if ( defined( $sth->{syb_more_results} ) ) {
			{
				while ( my $dat = $sth->fetch ) {
					return undef if $sth->err;
				}
				redo if $sth->{syb_more_results};
			}
		}
		my $rows = $sth->rows;

		( $rows == 0 ) ? "0E0" : $rows;
	}

	sub last_insert_id {
		my ( $dbh, $catalog, $schema, $table, $field, $attr ) = @_;

		# parameters are ignored.

		my $sth = $dbh->prepare('select @@identity');
		if ( !$sth->execute ) {
			return undef;
		}
		my $value;
		($value) = $sth->fetchrow_array;
		$sth->finish;

		return $value;
	}

	sub table_info {
		my $dbh     = shift;
		my $catalog = $dbh->quote(shift);
		my $schema  = $dbh->quote(shift);
		my $table   = $dbh->quote(shift);
		my $type    = $dbh->quote(shift);

		my $sth = $dbh->prepare("sp_tables $table, $schema, $catalog, $type");

		$sth->execute;
		$sth;
	}

	{
		my $names = [
			qw(TABLE_CAT TABLE_SCHEM TABLE_NAME COLUMN_NAME DATA_TYPE
			  TYPE_NAME COLUMN_SIZE BUFFER_LENGTH DECIMAL_DIGITS
			  NUM_PREC_RADIX NULLABLE REMARKS COLUMN_DEF SQL_DATA_TYPE
			  SQL_DATETIME_SUB CHAR_OCTET_LENGTH ORDINAL_POSITION
			  IS_NULLABLE
			  )
		];

		# Technique of using DBD::Sponge borrowed from DBD::mysql...
		sub column_info {
			my $dbh     = shift;
			my $catalog = $dbh->quote(shift);
			my $schema  = $dbh->quote(shift);
			my $table   = $dbh->quote(shift);
			my $column  = $dbh->quote(shift);

			my $sth =
			  $dbh->prepare("sp_columns $table, $schema, $catalog, $column");
			return undef unless $sth;

			if ( !$sth->execute() ) {
			   return DBI::set_err( $dbh, $sth->err(), $sth->errstr() );
			}
			my @cols;
			while ( my $d = $sth->fetchrow_arrayref() ) {
				push( @cols, [ @$d[ 0 .. 11 ], @$d[ 14 .. 19 ] ] );
			}
			my $dbh2;
			if ( !( $dbh2 = $dbh->{'~dbd_driver~_sponge_dbh'} ) ) {
				$dbh2 = $dbh->{'~dbd_driver~_sponge_dbh'} =
				  DBI->connect("DBI:Sponge:");
				if ( !$dbh2 ) {
					DBI::set_err( $dbh, 1, $DBI::errstr );
					return undef;
				}
			}
			my $sth2 = $dbh2->prepare(
				"SHOW COLUMNS",
				{
					'rows'          => \@cols,
					'NAME'          => $names,
					'NUM_OF_FIELDS' => scalar(@$names)
				}
			);
			if ( !$sth2 ) {
				DBI::set_err( $sth2, $dbh2->err(), $dbh2->errstr() );
			}
			$sth2->execute;
			$sth2;
		}
	}

	sub primary_key_info {
		my $dbh     = shift;
		my $catalog = $dbh->quote(shift);    # == database in Sybase terms
		my $schema  = $dbh->quote(shift);    # == owner in Sybase terms
		my $table   = $dbh->quote(shift);

		my $sth = $dbh->prepare("sp_pkeys $table, $schema, $catalog");

		$sth->execute;
		$sth;
	}

	sub foreign_key_info {
		my $dbh        = shift;
		my $pk_catalog = $dbh->quote(shift);    # == database in Sybase terms
		my $pk_schema  = $dbh->quote(shift);    # == owner in Sybase terms
		my $pk_table   = $dbh->quote(shift);
		my $fk_catalog = $dbh->quote(shift);    # == database in Sybase terms
		my $fk_schema  = $dbh->quote(shift);    # == owner in Sybase terms
		my $fk_table   = $dbh->quote(shift);

		my $sth =
		  $dbh->prepare(
	"sp_fkeys $pk_table, $pk_catalog, $pk_schema, $fk_table, $fk_catalog, $fk_schema"
		  );

		$sth->execute;
		$sth;
	}

	sub statistics_info {
		my $dbh       = shift;
		my $catalog   = $dbh->quote(shift);    # == database in Sybase terms
		my $schema    = $dbh->quote(shift);    # == owner in Sybase terms
		my $table     = $dbh->quote(shift);
		my $is_unique = shift;
		my $quick     = shift;

		my $sth =
		  $dbh->prepare(
	"sp_indexes \@\@servername, $table, $catalog, $schema, NULL, $is_unique"
		  );

		$sth->execute;
		$sth;
	}

	sub type_info_all {
		my ($dbh) = @_;

		my $sth = $dbh->prepare("sp_datatype_info");
		my $data;
		if ( $sth->execute ) {
			$data = $sth->fetchall_arrayref;
		}
		my $ti = [
			{
				TYPE_NAME          => 0,
				DATA_TYPE          => 1,
				PRECISION          => 2,
				LITERAL_PREFIX     => 3,
				LITERAL_SUFFIX     => 4,
				CREATE_PARAMS      => 5,
				NULLABLE           => 6,
				CASE_SENSITIVE     => 7,
				SEARCHABLE         => 8,
				UNSIGNED_ATTRIBUTE => 9,
				MONEY              => 10,
				AUTO_INCREMENT     => 11,
				LOCAL_TYPE_NAME    => 12,
				MINIMUM_SCALE      => 13,
				MAXIMUM_SCALE      => 14,
				SQL_DATA_TYPE      => 15,
				SQL_DATETIME_SUB   => 16,
				NUM_PREC_RADIX     => 17,
				INTERVAL_PRECISION => 18,
			},
		];
		push( @$ti, @$data );

		return $ti;
	}
}

{
	package DBD::SybaseASE::st;    # ====== STATEMENT ======

	use strict;

	# Installable module to return the result type of the current resultset. 
	# Used when executing stored procs to find out what type of information
	# is fetchable. 
	sub syb_output_params {
		my ($sth) = @_;

		my @results;
		my $status;

		{
			while ( my $d = $sth->fetch ) {
				# handle param results, otherwise handle status
				# results
				if ( $sth->FETCH('syb_result_type') == 4042 ) {
					push( @results, @$d );
				}
				elsif ( $sth->FETCH('syb_result_type') == 4043 ) {
					$status = $d->[0];
				}
			}
			redo if $sth->FETCH('syb_more_results');
		}
		@results;
	}

	sub exec_proc {
		my ($sth) = @_;

		my @results;
		my $status;

		$sth->execute || return undef;

		{
			while ( my $d = $sth->fetch ) {
				if ( $sth->FETCH('syb_result_type') == 4043 ) {
					$status = $d->[0];
				}
			}
			redo if $sth->FETCH('syb_more_results');
		}

		$status;
	}
}

1;


#!/usr/bin/perl -w

# Read test configuration
$inifile = 'test.ini';
open ( INI, "< $inifile" ) or die "Cannot open '$inifile' file!\n";
$wmid = <INI>;
$wmid =~ s/\n//;
$pass = <INI>;
$pass =~ s/\n//;
$key  = <INI>;
$key =~ s/\n//;
$exe  = <INI>;
$exe =~ s/\n//;
close(INI);

use Test::More tests => 10;

$ver = qr/wmsigner, Version .* \(c\) WebMoney Transfer \(r\), \d{4}/;

# Test version output
$t = `$exe --version`;
like ( $t, $ver , 'version' );
$t = `$exe -v`;
like ( $t, $ver , 'version short' );

# Test help output
@a = `$exe --help`;
cmp_ok($#a, '>=', 11, 'help' );
@a = `$exe -h`;
cmp_ok($#a, '>=', 11, 'help short' );

# Write test wmsigner.ini
open (WMINI, "> wmsigner.ini");
print WMINI  "$wmid\n$pass\n$key";
close (WMINI);

# Test wmsigner old behaviour
$t = sign( 'Test123' );
like ( $t, qr/[0-9a-f]{132}/, "old behaviour" );

# Test fake password
open (WMINI, "> wmsigner.ini");
print WMINI  "$wmid\nfake\n$key";
close (WMINI);
$t = sign( 'Test123' );
is ( $t, 'Error -3', "fake password" );

# Test empty wmid
open (WMINI, "> wmsigner.ini");
print WMINI  "\n$pass\n$key";
close (WMINI);
$t = sign( 'Test123' );
is ( $t, 'Error -4', "empty wmid" );

# Test empty pass
open (WMINI, "> wmsigner.ini");
print WMINI  "$wmid\n\n$key";
close (WMINI);
$t = sign( 'Test123' );
is ( $t, 'Error -5', "empty pass" );

# Test empty key_line
open (WMINI, "> wmsigner.ini");
print WMINI  "$wmid\n$pass\n";
close (WMINI);
$t = sign( 'Test123' );
is ( $t, 'Error -6', "empty key_line" );

# Test key_file not found
open (WMINI, "> wmsigner.ini");
print WMINI  "$wmid\n$pass\nbla-bla";
close (WMINI);
$t = sign( 'Test123' );
is ( $t, "!LoadKeys\nError 2", "key_file not found" );

sub sign {
  my $r;
  if ( $exe =~ /\.exe$/) {
    use IPC::Open2;
    open2(*READ, *WRITE, $exe) or die "Cannot run '$exe'!\n";
    print WRITE $_[0]."\004\r\n";
    while ( $i = <READ> ){
        $r .= $i;
    };
    close(WRITE);
    close(READ); 
    $r =~ s/\r\n/\n/;
  } else {
    $r = `echo -ne '$_[0]\004\r\n' | $exe`;
  }
    return $r;
}
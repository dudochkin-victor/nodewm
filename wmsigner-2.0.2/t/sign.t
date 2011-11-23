#!/usr/bin/perl -w
use strict;

use Test::More tests => 18;

my ( $planstr, $signstr, $request, $response );

# Read test configuration
my $inifile = 'test.ini';
open ( INI, "< $inifile" ) or die "Cannot open '$inifile' file!\n";
my $wmid = <INI>;
$wmid =~ s/\n//;
my $pass = <INI>;
$pass =~ s/\n//;
my $key  = <INI>;
$key =~ s/\n//;
my $exe = <INI>;
$exe =~ s/\n//;
close(INI);

open (KEY_B64, "< test.b64");
my $key_b64 = <KEY_B64>;
close (KEY_B64);

# Write wmsigner.ini
write_ini ( $wmid, $pass, $key );

# Test sign
my $res = make_req ( '' );
like ( $res, qr/<retval>0<\/retval>/ , "sign" );

# Test plan_as_param
$res = make_req ( '', '--sign' );
like ( $res, qr/<retval>0<\/retval>/ , "plan_as_param" );

$res = make_req ( '', '-s' );
like ( $res, qr/<retval>0<\/retval>/ , "plan_as_param short" );

# Test without wmsigner.ini
unlink( 'wmsigner.ini' );
$res = make_req ( "--wmid $wmid --password $pass --key-base64 $key_b64" );
like ( $res, qr/<retval>0<\/retval>/ , "without wmsigner.ini" );

$res = make_req ( "-w $wmid -p $pass -k $key" );
like ( $res, qr/<retval>0<\/retval>/ , "without wmsigner.ini short" );

$res = make_req ( "-w $wmid -p $pass -k $key", "-s" );
like ( $res, qr/<retval>0<\/retval>/ , "without wmsigner.ini short and plan" );

# Test with wmid
write_ini ( '123456789012', $pass, $key );
$res = make_req ( "--wmid $wmid" );
like ( $res, qr/<retval>0<\/retval>/ , "with wmid" );

$res = make_req ( "-w $wmid" );
like ( $res, qr/<retval>0<\/retval>/ , "with wmid short" );

# Test with pass
write_ini ( $wmid, 'fake', $key );
$res = make_req ( "--password $pass" );
like ( $res, qr/<retval>0<\/retval>/ , "with pass" );

$res = make_req ( "-p $pass" );
like ( $res, qr/<retval>0<\/retval>/ , "with pass short" );

# Test with key_path
write_ini ( $wmid, $pass, './fake.kwm' );
$res = make_req ( "--key-path $key" );
like ( $res, qr/<retval>0<\/retval>/ , "with key_path" );

$res = make_req ( "-k $key" );
like ( $res, qr/<retval>0<\/retval>/ , "with key_path short" );

# Test with three fake
write_ini ( 'fake', 'fake', './fake.kwm' );
$res = make_req ( "--key-path $key --password $pass --wmid $wmid" );
like ( $res, qr/<retval>0<\/retval>/ , "with three fake" );

$res = make_req ( "-k $key -p $pass -w $wmid" );
like ( $res, qr/<retval>0<\/retval>/ , "with three fake short" );

# Test ini_path
$res = make_req ( "--ini-path ./test.ini" );
like ( $res, qr/<retval>0<\/retval>/ , "with ini_path" );

$res = make_req ( "-i ./test.ini" );
like ( $res, qr/<retval>0<\/retval>/ , "with ini_path short" );

# Test with base64

write_ini ( $wmid, $pass, './fake.kwm' );
$res = make_req ( "--key-base64 $key_b64");
like ( $res, qr/<retval>0<\/retval>/ , "with base64" );

$res = make_req ( "-K64 $key_b64");
like ( $res, qr/<retval>0<\/retval>/ , "with base64 short" );

sub write_ini {
    open (WMINI, "> wmsigner.ini");
    print WMINI  $_[0]."\n".$_[1]."\n".$_[2];
    close (WMINI);
};

sub sign {
    use IPC::Open2;
    my $cmd = $exe ." ". $_[1]; 
    if ( defined $_[2] ){ $cmd .= " ". $_[2] };
#    print "Command: ".$cmd."\n";
    open2(*READ, *WRITE, $cmd) or die "Cannot run '$exe'!\n";
    print WRITE $_[0]."\004\r\n";
    my $r;
	while (<READ>){
	    $r .= $_;
	};
    close(WRITE);
    close(READ);
    $r =~ s/\r\n/\n/;
    return $r;
}

sub gen_reqn {
    my ($s, $mi, $h, $d, $m, $y, $ms);
    use Time::HiRes qw( gettimeofday );
    ($s, $mi, $h, $d, $m, $y) = (localtime)[0..5];
    $ms = (gettimeofday)[1];
    return sprintf("%04d%02d%02d%02d%02d%02d%02d", $y+1900, $m+1, $d, $h, $mi, $s, substr($ms, 0, 2)) ;
};

sub make_req {
    my $params = $_[0];
    my $reqn = gen_reqn();
    my ($day, $month, $year);
    ($day, $month, $year) = (localtime) [3,4,5];
    my $date = sprintf ("%04d%02d%02d", $year+1900, $month+1, $day);
    my $datestart = "$date 00:00:00";
    my $datefinish = "$date 23:59:59";
    my ($plan, $t, $req);
    $planstr = $plan = "$wmid"."0$datestart$datefinish$reqn";
    if ( defined $_[1] ){
	my $extra = $_[1].' "'.$plan.'"';
	$signstr = $t = sign( '', $params, $extra );
    } else {
	$signstr = $t = sign( $plan, $params );
    };
    if ( $t !~ /^[0-9a-f]{132}$/ ) {
	return "ERROR SIGNING: $t";
    }
    $request = $req =  "<?xml version=\"1.0\"?><w3s.request><reqn>$reqn</reqn><wmid>$wmid</wmid><sign>$t</sign>".
	    "<getininvoices><wmid>$wmid</wmid><datestart>$datestart</datestart><datefinish>$datefinish</datefinish></getininvoices>".
	    "</w3s.request>";
    return https_req ( $req );
};

sub https_req {	
    # Run request
    use LWP::UserAgent;
    my $ua  = LWP::UserAgent->new;
    my $r = HTTP::Request->new(POST => 'https://w3s.webmoney.ru/asp/XMLInInvoices.asp');
    $r->content_type('application/x-www-form-urlencoded');
    $r->content($_[0]);
    my $res = $ua->request($r);
    $response = $res->content;
    if ( $response !~ m/<retval>0<\/retval>/) { details(); }
    return $response;
};

sub details {
  if (-t STDIN && -t STDOUT) {
#    print "\033[32mPlan:\033[0m $planstr\n";
     print "Plan: $planstr\n";
#    print "\033[31mSign:\033[0m $signstr\n";
     print "Sign: $signstr\n";
#    print "\033[34mRequest:\033[0m $request\n";
     print "Request: $request\n";
  }
};


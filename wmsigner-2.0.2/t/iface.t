#!/usr/bin/perl -w
use strict;

use Test::More tests => 12;

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

# Write wmsigner.ini
write_ini ( $wmid, $pass, $key );

# Test x6
my $res = x6 ('text string');
like ( $res, qr/<retval>0<\/retval>/, 'x6');
like ( $res, qr/<datecrt>\d{8} \d{2}:\d{2}:\d{2}<\/datecrt>/);

$res = x6 ("text\r\nline");
like ( $res, qr/<retval>0<\/retval>/, 'x6 CR');
like ( $res, qr/<datecrt>\d{8} \d{2}:\d{2}:\d{2}<\/datecrt>/);

$res = x6 ("text\nline");
like ( $res, qr/<retval>0<\/retval>/ );

$res = x6 ("text line\004\r\n");
like ( $res, qr/<retval>0<\/retval>/ );

$res = x6 ("text\nline\r\n");
like ( $res, qr/<retval>0<\/retval>/ );

# Test x8
$res = x8 ();
like ( $res, qr/<retval>1<\/retval>/, 'x8' );
like ( $res, qr/<testwmpurse><wmid>$wmid<\/wmid><purse><\/purse><\/testwmpurse>/ );

# Test x9
$res = x9 ();
like ( $res, qr/<retval>0<\/retval>/, 'x9' );
like ( $res, qr/<purse id="\d+"><pursename>[A-Z]\d{12}<\/pursename><amount>\d+(\.\d+)?<\/amount><desc>.*<\/desc><outsideopen>\d<\/outsideopen><lastintr>.*<\/lastintr><lastouttr>.*<\/lastouttr><\/purse>/ );

# Test x10
$res = x10 ();
like ( $res, qr/<retval>0<\/retval>/, 'x10' );

sub x6 {
    my $reqn = gen_reqn();
    my ($plan, $t, $req, $text, $string);
    $string = $text = $_[0];
    $planstr = $plan = "$wmid$reqn$text";
    $string =~ s/\n$//;
    $string =~ s/\r$//;
    $string =~ s/\004$//;
    $signstr = $t = sign( $plan );
    if ( $t !~ /^[0-9a-f]{132}$/ ) {
	return "ERROR SIGNING: $t";
    }
    $request = $req =  "<?xml version=\"1.0\"?><w3s.request><reqn>$reqn</reqn><wmid>$wmid</wmid><sign>$t</sign>".
	    "<message><receiverwmid>$wmid</receiverwmid><msgsubj/><msgtext>$string</msgtext></message>".
	    "</w3s.request>";
    return https_req ( $req, 'https://w3s.webmoney.ru/asp/XMLSendMsg.asp');
}


sub x8 {
    my $reqn = gen_reqn();
    my ($plan, $t, $req);
    $planstr = $plan = "$wmid";
    $signstr = $t = sign( $plan );
    if ( $t !~ /^[0-9a-f]{132}$/ ) {
	return "ERROR SIGNING: $t";
    }
    $request = $req =  "<?xml version=\"1.0\"?><w3s.request><reqn>$reqn</reqn><wmid>$wmid</wmid><sign>$t</sign>".
	    "<testwmpurse><wmid>$wmid</wmid><purse/></testwmpurse>".
	    "</w3s.request>";
    return https_req ( $req, 'https://w3s.webmoney.ru/asp/XMLFindWMPurse.asp');
}

sub x9 {
    my $reqn = gen_reqn();
    my ($day, $month, $year) = (localtime) [3,4,5];
    my ($plan, $t, $req);
    $planstr = $plan = "$wmid$reqn";
    $signstr = $t = sign( $plan );
    if ( $t !~ /^[0-9a-f]{132}$/ ) {
	return "ERROR SIGNING: $t";
    }
    $request = $req =  "<?xml version=\"1.0\"?><w3s.request><reqn>$reqn</reqn><wmid>$wmid</wmid><sign>$t</sign>".
	    "<getpurses><wmid>$wmid</wmid></getpurses>".
	    "</w3s.request>";
    return https_req ( $req, 'https://w3s.webmoney.ru/asp/XMLPurses.asp');
}

sub x10 {
    my $reqn = gen_reqn();
    my ($day, $month, $year) = (localtime) [3,4,5];
    my $date = sprintf ("%04d%02d%02d", $year+1900, $month+1, $day);
    my $datestart = "$date 00:00:00";
    my $datefinish = "$date 23:59:59";
    my ($plan, $t, $req);
    $planstr = $plan = "$wmid"."0$datestart$datefinish$reqn";
    $signstr = $t = sign( $plan );
    if ( $t !~ /^[0-9a-f]{132}$/ ) {
	return "ERROR SIGNING: $t";
    }
    $request = $req =  "<?xml version=\"1.0\"?><w3s.request><reqn>$reqn</reqn><wmid>$wmid</wmid><sign>$t</sign>".
	    "<getininvoices><wmid>$wmid</wmid><datestart>$datestart</datestart><datefinish>$datefinish</datefinish></getininvoices>".
	    "</w3s.request>";
    return https_req ( $req, 'https://w3s.webmoney.ru/asp/XMLInInvoices.asp');
};

sub write_ini {
    open (WMINI, "> wmsigner.ini");
    print WMINI  $_[0]."\n".$_[1]."\n".$_[2];
    close (WMINI);
};

sub sign {
    use IPC::Open2;
    my $cmd = $exe; 
    if ( defined $_[2] ){ $cmd .= " ". $_[2] };
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
    my $ret = sprintf("%04d%02d%02d%02d%02d%02d%02d", $y+1900, $m+1, $d, $h, $mi, $s, substr($ms, 0, 2));
    return $ret;
};


sub https_req {	
    # Run request
    use LWP::UserAgent;
    my $ua  = LWP::UserAgent->new;
    my $r = HTTP::Request->new(POST => $_[1]);
    $r->content_type('application/x-www-form-urlencoded');
    $r->content($_[0]);
    my $res = $ua->request($r);
    $response = $res->content;
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


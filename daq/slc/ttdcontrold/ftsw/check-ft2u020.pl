#!/usr/bin/perl

## ---------------------------------------------------------------------
# check-ft2u019.pl
#
# check all the register dump of FT2U firmware
#
# 20130905 0.01  first version
#
## ---------------------------------------------------------------------

my $verbose_dump = ($ARGV[1] eq "-v") ? 1 : 0;

die "usage: check-ft3u.pl <board-id>\n" if ($ARGV[0] < 8 or $ARGV[0] >= 100);
$ID=$ARGV[0];

sub getreg {
  my ($a) = @_;
  my ($v) = `regft -$ID $a`;
  chop $v;
  $v =~ s/regft.*: $a //;
  return $v;
}

sub setreg {
  my ($a, $v) = @_;
  my ($x) = `regft -$ID $a $v`;
}

sub timstr {
  my ($a) = @_;
  if ($a >= 3600*100) {
    $a = sprintf("%3.1fd", $a/(3600*24.0));
  } elsif ($a > 60*300) {
    $a = sprintf("%3.1fh", $a/(3600.0));
  } elsif ($a > 60*5) {
    $a = sprintf("%3.1fm", $a/(60.0));
  } else {
    $a = sprintf("%ds", $a);
  }
}

## ---------------------------------------------------------------------

# ---------------------------------------------------- ftswid (0x000) --
my $reg_ftswid = getreg("0");
if ($reg_ftswid ne "46545357") {
  print "invalid ftswid (a=000): $reg_ftswid\n";
  exit 1;
} else {
  print "a=000  ftswid=$reg_ftswid...ok\n";
}

# ---------------------------------------------------- cpldver (0x010) --
my $reg_cpldver = getreg("10");
$reg_cpldver =~ m/(..)(..)(....)/;
my $cpldver_a = hex($1);
my $cpldver_b = hex($2);
my $cpldver_c = hex($3);
if ($cpldver_c < 44) {
  print "invalid cpldver (a=010): $reg_cpldver\n";
  exit 1;
} else {
  printf "a=010 cpldver=$reg_cpldver...ok version 0.%02d\n", $cpldver_c;
}

# ------------------------------------------------------ conf (0x020) --
my $reg_conf = getreg("20");
my $conf = hex($reg_conf);
if ($conf & 0x80) {
  print "a=020    conf=$reg_conf...ok\n";
} else {
  print "a=020    conf=$reg_conf...ng FPGA is not programmed\n";
  exit 1;
}

# ------------------------------------------------------ cclk (0x030) --
my $reg_cclk = getreg("30");
print "a=030    cclk=$reg_cclk...ok\n";

# ---------------------------------------------------- fpgaid (0x100) --
my $reg_fpgaid = getreg("100");
my $fpgaids;
for (0..3) {
  my $a = sprintf("%c", hex(substr($reg_fpgaid, $_*2, 2)));
  $fpgaids .= ($a =~ m/\w/) ? $a : ".";
}

if ($reg_fpgaid ne "46543255") {
  print "invalid fpgaid (a=100): $reg_fpgaid ($fpgaids)\n";
  exit 1;
} else {
  print "a=100  fpgaid=$reg_fpgaid...ok $fpgaids\n";
}

# --------------------------------------------------- fpgaver (0x110) --
my $reg_fpgaver = getreg("110");
$reg_fpgaver =~ m/(..)(..)(....)/;
my $fpgaver_a = hex($1);
my $fpgaver_b = hex($2);
my $fpgaver_c = hex($3);
$fpgaver_c = sprintf("0.%02d", $fpgaver_c);
if ($fpgaver_a != 3 or $fpgaver_b != $ID or $fpgaver_c ne "0.20") {
  die "invalid fpgaver (a=110): $reg_fpgaver version $fpgaver_c\n";
} else {
  printf "a=110 fpgaver=$reg_fpgaver...ok version $fpgaver_c\n";
}

# --------------------------------------------------- settime (0x120) --
my $settime = getreg("120");
my $dateset = hex($settime);
printf "a=120 settime=$settime...ok ";
print (($dateset == 0) ? "never set" : "set at " . localtime($dateset));
print "\n";

# --------------------------------------------------- clkfreq (0x130) --
my $reg_clkfreq = getreg("130");
my $clkfreq = (hex($reg_clkfreq) & 0x00ffffff) + 0x07000000;
printf "a=130 clkfreq=$reg_clkfreq...ok %d Hz\n", $clkfreq;

# ----------------------------------------------------- utime (0x140) --
my $reg_utime = getreg("140");
my $dateget = hex($reg_utime);
my $tdiff = $dateget - $dateset;
$tdiff = timstr($tdiff);

printf "a=140   utime=$reg_utime...ok ";
if ($dateset == 0) {
  printf "up for $tdiff\n";
} else {
  printf "now at %s, up for $tdiff\n", localtime($dateget)."";
}

# ----------------------------------------------------- ctime (0x150) --
my $reg_ctime = getreg("150");
my $ctime = hex($reg_ctime);

printf "a=150   ctime=$reg_ctime...ok +%8.6fs\n", $ctime * 1.0 / $clkfreq;

# ----------------------------------------------------- udead (0x160) --
my $reg_udead = getreg("160");
my $udead = hex($reg_udead);

printf "a=160   udead=$reg_udead...ok %s\n", timstr($udead);

# ----------------------------------------------------- cdead (0x170) --
my $reg_cdead = getreg("170");
my $cdead = hex($reg_cdead);

printf "a=170   cdead=$reg_cdead...ok +%8.6fs\n", $cdead * 1.0 / $clkfreq;

# ----------------------------------------------------- omask (0x180) --
my $reg_omask = getreg("180");
my $omask = hex($reg_omask);

my $s3query = ($omask >> 31) & 1;
my $xsel    = ($omask >> 28) & 7;
my $osel    = ($omask >>  8) & 15;
my $omask   = ($omask >>  0) & 255;
my $xmask   = ($omask >> 12) & 15; 
my $xorclk  = ($omask >> 16) & 255; 
printf("a=180   omask=$reg_omask...ok s3q=%d xmask=%x xsel=%d osel=%d ",
       $s3query, $xmask, $xsel, $osel);
if ($omask == 0) {
  print "masked=none";
} else {
  print "masked=";
  for (0..9) {
    my $a=9-$_;
    print "$a" if ($omask & (1<<$a));
  }
}
if ($xorclk == 0) {
  print " clk-xored=none";
} else {
  print " clk-xored=";
  for (0..9) {
    my $a=9-$_;
    print "$a" if ($xorclk & (1<<$a));
  }
}
print "\n";

# --------------------------------------------------- stamisc (0x190) --
my $reg_stamisc = getreg("190");
printf "a=190 stamisc=$reg_stamisc...ok ";
$stalink =~ m/.(.)(.)(.)(....)/;
my $ckmux = hex($1) & 3;
my $suser = $2;
my $s3ver = hex($3);
$stalink = hex($4);
$ckmux = ("IN", "XTAL", "FMC", "PIN")[$ckmux];
printf "CLK $ckmux SW $suser S3 0.%02d ", $s3ver;
if ($enjtag == 0) {
  print "active LINK=none";
} else {
  print "active LINK=";
  for (0..9) {
    my $a=9-$_;
    print "$a" if ($enotrg & (1<<$a));
  }
}
print "\n";

# ------------------------------------------------------ jtag (0x1a0) --
my $reg_jtag = getreg("1a0");
printf "a=1a0    jtag=$reg_jtag...ok ";
my $enjtag = hex($enjtag);
if ($enjtag == 0) {
  print "enabled JTAG=none";
} else {
  print "enabled JTAG=";
  for (0..9) {
    my $a=9-$_;
    print "$a" if ($enotrg & (1<<$a));
  }
}
print "\n";

# ----------------------------------------------- jreg (0x1c0, 0x1d0) --
setreg("1c0", "e");
my $jreg0 = getreg("1d0");
setreg("1c0", "2e");
my $jreg1 = getreg("1d0");
setreg("1c0", "4e");
my $jreg2 = getreg("1d0");
if ($jreg0 ne "4a200d50") {
  die "wrong jitter cleaner setup (reg0 = $jreg0)\n";
} else {
  printf "a=1d0   jreg0=$jreg0...ok\n";
}
if ($jreg1 !~ m/.3874061/) {
  die "wrong jitter cleaner setup (reg1 = $jreg1)\n";
} else {
  printf "a=1d0   jreg1=$jreg1...ok\n";
}
printf "a=1d0   jreg2=$jreg2...ok\n";

# ------------------------------------------------------ jrst (0x1e0) --
my $reg_jrst = getreg("1e0");
if ($reg_jrst ne "00000000") {
  die "wrong jitter cleaner setup (jrst = $reg_jrst)\n";
} else {
  printf "a=1e0    jrst=$reg_jrst...ok\n";
}

# ------------------------------------------------------ jsta (0x1f0) --
my $reg_jsta = getreg("1f0");
if ($reg_jsta !~ m/cc(..)8000/) {
  die "wrong jitter cleaner status (jsta = $reg_jsta)\n";
} else {
  printf "a=1f0    jsta=$reg_jsta...ok";
  printf(" retry %d", $1) if ($1 ne "00");
  print "\n";
}

# ----------------------------------------------------- reset (0x200) --
my $reg_reset = getreg("200");
if ($reg_reset eq "00000000") {
  printf "a=200   reset=$reg_reset...ok\n";
} elsif ($reg_reset eq "80000000") {
  printf "a=200   reset=$reg_reset...ok  no FIFO\n";
} else {
  die "wrong reset register status (reset = $reg_reset)\n";
}

# ---------------------------------------------------- utimrst (0x210) --
my $reg_utimrst = getreg("210");

my $utimrst = hex($reg_utimrst);
$tdiff = $dateget - $utimrst;

printf "a=210 utimrst=$reg_utimrst...ok ";
if ($utimrst == 0) {
  printf "never reset\n";
} else {
  printf "reset at %s, %s ago\n", localtime($utimrst)."", timstr($tdiff);
}

# ----------------------------------------------------- ctimrst (0x220) --
my $reg_ctimrst = getreg("220");
my $ctimrst = hex($reg_ctimrst) & 0x07ffffff;
my $rstsrc  = (hex($reg_ctimrst) >> 27) & 3;
my @rstsrc = ( "never reset", "by counter reset",
	       "by run reset", "by run/counter reset" );

printf "a=220 ctimrst=$reg_ctimrst...ok +%8.6fs %s\n",
     $ctimrst * 1.0 / $clkfreq, $rstsrc[$rstsrc];

# ---------------------------------------------------- utimerr (0x230) --
my $reg_utimerr = getreg("230");

my $utimerr = hex($reg_utimerr);
$tdiff = $dateget - $utimerr;

printf "a=230 utimerr=$reg_utimerr...ok ";
if ($utimerr == 0) {
  printf "no error\n";
} else {
  printf "error at %s, %s ago\n", localtime($utimerr)."", timstr($tdiff);
}

# ----------------------------------------------------- ctimerr (0x240) --
my $reg_ctimerr = getreg("240");
my $ctimerr = hex($reg_ctimerr) & 0x07ffffff;
my $errsrc  = (hex($reg_ctimerr) >> 27) & 3;

printf "a=240 ctimerr=$reg_ctimerr...ok +%8.6fs %s\n",
     $ctimerr * 1.0 / $clkfreq;

# ------------------------------------------------------ errsrc (0x250) --
my $reg_errsrc = getreg("250");
my $errsrc = hex($reg_errsrc) & 0x0ff;

printf "a=250  errsrc=$reg_errsrc...ok\n";

# --------------------------------------------------------- tlu (0x270) --
my $reg_tlu = getreg("270");
my $tlu = hex($reg_tlu) & 0x0ff;

printf "a=270     tlu=$reg_tlu...ok\n";

# ------------------------------------------------------ trgset (0x280) --
my $reg_trgset = getreg("280");
my $trgset   = hex($reg_trgset);
my $trgopt   = ($trgset>>20) & 0xfff;
my $rateval  = ($trgset>>8) & 0x3ff;
my $rateexp  = ($trgset>>4) & 0xf;
my $notrgclr = ($trgset>>3) & 1;
my $seltrg   = ($trgset>>0) & 7;

printf "a=280  trgset=$reg_trgset...ok sel=%d rate=%d exp=%d opt=%03x\n",
    $seltrg, $rateval, $rateexp, $trgopt;

# ------------------------------------------------------ tlimit (0x290) --
my $reg_tlimit = getreg("290");
my $tlimit = hex($reg_tlimit);

printf "a=290  tlimit=$reg_tlimit...ok %d\n", $tlimit;

# ------------------------------------------------------ tincnt (0x2a0) --
my $reg_tincnt = getreg("2a0");
my $tincnt = hex($reg_tincnt);

printf "a=2a0  tincnt=$reg_tincnt...ok %d\n", $tincnt;

# ----------------------------------------------------- toutcnt (0x2b0) --
my $reg_toutcnt = getreg("2b0");
my $toutcnt = hex($reg_toutcnt);

printf "a=2b0 toutcnt=$reg_toutcnt...ok %d\n", $toutcnt;

# ------------------------------------------------------ tlast (0x2c0) --
my $reg_tlast = getreg("2c0");
my $tlast = hex($reg_tlast);

printf "a=2c0   tlast=$reg_tlast...ok %d\n", $tlast;

# ---------------------------------------------------- stafifo (0x2d0) --
my $reg_stafifo = getreg("2d0");
my $stafifo  = hex($reg_stafifo);
my $fifoemp  = ($stafifo>>28)&1 ? " empty" : "";
my $fifoahi  = ($stafifo>>29)&1 ? " ahi"  : "";
my $fifoorun = ($stafifo>>30)&1 ? " orun" : "";
my $fifoful  = ($stafifo>>31)&1 ? " full" : "";
my $fifosome = ($stafifo>>28)&15 ? "" : " some data";

printf "a=2d0 stafifo=$reg_stafifo...ok%s%s%s%s%s\n",
    $fifosome, $fifoemp, $fifoahi, $fifoorun, $fifoful;

# ----------------------------------------------------- enstat (0x300) --
my $reg_enstat = getreg("300");
$reg_enstat =~ m/(..)(.)(.)(..)(.)(.)/;
my $enstat_cntoc = hex($1);
my $enstat_2b    = $2;
my $enstat_isk   = $3;
my $enstat_octet = $4;
my $enstat_cnten = hex($5);
my $enstat_cntpl = hex($6);
printf "a=300  enstat=$reg_enstat...ok";
printf " cnt (oc %d en %d pl %d) octet %s isk %s 2b %s\n",
  $enstat_cntoc, $enstat_cnten, $enstat_cntpl,
  $enstat_octet, $enstat_isk, $enstat_2b;

# --------------------------------------------------- revopos (0x310) --
my $reg_revopos = getreg("310");
my $revopos = hex($reg_revopos);
printf "a=310 revopos=$reg_revopos...ok %d\n", $revopos;

# ---------------------------------------------------- revoin (0x320) --
my $reg_revoin = getreg("320");
my $revoin = hex($reg_revoin);
my $cntrevocand = ($revoin >> 16) & 0xffff;
my $revocand = $revoin & 0x7ff;
printf "a=320  revoin=$reg_revoin...ok cand %d count %x\n",
    $revocand, $cntrevocand;

# ---------------------------------------------------- norevo (0x330) --
my $reg_revocnt = getreg("330");
my $revocnt = hex($reg_revocnt);
my $badrevo = ($revocnt >> 16) & 0xffff;
my $norevo  = ($revocnt >>  0) & 0xffff;
printf "a=330 revocnt=$reg_revocnt...ok badrevo %d norevo %d\n",
    $badrevo, $norevo;

# ------------------------------------------------------ stat (0x380) --
my $reg_stat = getreg("380");
my $stat = hex($reg_stat);
my $fifoerr = ($stat >> 31) & 1;
my $stabusy = ($stat >> 29) & 1;
my $sigbusy = ($stat >> 28) & 1;
my $xnwff   = ($stat >> 24) & 15;
my $xbusy   = ($stat >> 20) & 15;
my $obusy   = ($stat >> 12) & 255;
my $xbsysig = ($stat >>  8) & 15;
my $obsysig = ($stat >>  0) & 255;

printf "a=380    stat=$reg_stat...ok%s xnff %x busy %d %d X(%x/%x) O(%02x/%02x)\n",
    $fifoerr ? " fifoerr" : "",
    $xnwff,
    $stabusy, $sigbusy, $xbusy, $xbsysig, $obusy, $obsysig;

# ---------------------------------------------------- linkup (0x390) --
my $reg_linkup = getreg("390");
my $linkup = hex($reg_linkup);
my $b2lup = ($linkup >> 24) & 255;
my $xalive = ($linkup >> 20) & 15;
my $oalive = ($linkup >> 12) & 255;
my $xlinkup = ($linkup >> 8) & 15;
my $olinkup = ($linkup >> 0) & 255;

printf "a=390  linkup=$reg_linkup...ok b2lup %02x alive X(%x) O(%02x) linkup X(%x) O(%02x)\n",
    $b2lup, $xalive, $oalive, $xlinkup, $olinkup;

# ----------------------------------------------------- error (0x3a0) --
my $reg_error = getreg("3a0");
my $error = hex($reg_error);
my $xerr    = ($error >>  8) & 15;
my $oerr    = ($error >>  0) & 255;

printf "a=3a0   error=$reg_error...ok err X(%x) O(%02x)\n",
    $xerr, $oerr;

# ---------------------------------------------------- acksig (0x3b0) --
my $reg_acksig = getreg("3a0");
printf "a=3a0  acksig=$reg_acksig...ok\n";

# ------------------------------------------- odecode (0x3c0...0x4b0) --
foreach $i (0..7) {
  my $a = sprintf("%3x", 0x3c0 + 32*$i);
  my $b = sprintf("%3x", 0x3d0 + 32*$i);
  my $reg_a = getreg($a);
  my $reg_b = getreg($b);
  printf "a=$a..$b O$i=$reg_a..$reg_b\n";
}

foreach $i (0..3) {
  my $a = sprintf("%3x", 0x4c0 + 16*$i);
  my $reg_a = getreg($a);
  printf "a=$a      X$i=$reg_a\n";
}

# ----------------------------------------- dump0..7,k (0x580..0x600) --

# to make dump0..7,k working, it has to be fetched once by writing 1 to 650.
setreg("650", "1");
select(undef,undef,undef,0.1); # 100ms sleep

my $dumpk = getreg("600");
my @dump4;
my @dump;
foreach $i (0..7) {
  my $a = sprintf("%3x", 0x580 + 16*$i);
  $dump4[$i] = getreg($a);
  if ($verbose_dump) {
    printf "a=$a   dump$i=$dump4[$i]...ok\n";
  }
  foreach $j (0..3) {
    $dump[$i*4 + $j] = substr($dump4[$i], (3-$j)*2, 2);
  }
}

my $found = 0;
my $k = hex($dumpk);

for ($found = 31; $found >= 0; $found--) {
  last if ($k & (1<<$found)) and $dump[$found] eq "3c";
  if (($k & (1<<$found)) and $dump[$found] eq "bc" and $found) {
    $found--;
    last;
  }
}

printf "a=600   dumpk=$dumpk...";
print( ($found > 15) ? "ok\n" : "frame boundary not found ($found)\n" );

# ----------------------------------------------------- dumpi (0x650) --
my $dumpi = getreg("650");
printf "a=650   dumpi=$dumpi...";
$dumpi =~ m/(....)(....)/;
my $dumpi_cnt = hex($1);
my $dumpi_hex = hex($2);
print( ($dumpi_cnt == 0x8000) ? "ok " : "count=dumpi_cnt " );
printf "misc=%x cnt=%d\n", $dumpi_hex>>3, $dumpi_hex & 7;

# ------------------------------------------- dump0..7 (0x580..0x600) --
print "a=580..5f0 dump0..7\n" unless ($verbose_dump);
print " " x ((($found-15) % 16) * 4);

for ($i = 31; $i >= 0; $i--) {
  print "\n" if ($i == $found and $i != 31);
  print "\n" if ($i == $found - 16);
  printf(" %s%s",
	 $k & (1<<$i) ? "K" : "D",
	 $dump[$i]);
}
print "\n";

# ---------------------------------------------- dump2 (0x660..0x6f0) --
my @dump2;
my $dumpa;
foreach $i (0..9) {
  my $a = sprintf("%3x", 0x660 + 16*$i);
  $dump2[$i] = getreg($a);
  if ($verbose_dump) {
    printf "a=$a  dumpa$i=$dump2[$i]...ok\n";
  }
  $dumpa = $dump2[$i] . $dumpa;
}

my @bit;
my %hex;
for (0..15) {
  $bit[$_] = sprintf("%d%d%d%d", ($_>>3)&1, ($_>>2)&1, ($_>>1)&1, $_&1);
  $hex{$bit[$_]} = sprintf("%x", $_);
}
my $dumpb;
for (0..79) {
  $dumpb .= $bit[hex(substr $dumpa, $_, 1)];
}

if ($dumpb =~
     m/(.*)(0011111001|1100000110)(.{150})(0011111001|1100000110)(.*)/) {
  $dumpb = "x" x (160 - length($1)) . $dumpb . "x" x (160 - length($4.$5));
}

if ($verbose_dump) {
  print $dumpa,"\n";
  print $dumpb,"\n";
}

print "a=660..6f0 dumpa0..9\n" unless ($verbose_dump);
for ($i=0; $i<length($dumpb); $i+=10) {
  $a[0] = "00" . substr($dumpb, $i, 2);
  $a[1] = substr($dumpb, $i+2, 4);
  $a[2] = substr($dumpb, $i+6, 4);
  print " ";
  for (0..2) {
    print defined($hex{$a[$_]}) ? $hex{$a[$_]} : "x";
  }
  print "\n" if ($i % 160) == 150;
}

# ----------------------------------------------------------------------
print "done.\n";

#----(emacs outline mode setup)
# Local Variables: ***
# mode:outline ***
# outline-regexp:"# --" ***
# End: ***

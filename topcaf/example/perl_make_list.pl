#!/usr/bin/perl
use strict;
use warnings;

#loop from n1 to n2 creating file00n.root
my ($n1, $n2, $filename, $suffixappend) = @ARGV;
open(my $subf, '>', $filename);

#if suffixappend has "slot" then the numbering scheme is slightly different
if( (index($suffixappend,"slot") != -1) || (index($suffixappend,"cpr2061") != -1) ){
    for (; $n1<($n2+1); $n1++){
	my $n3 = sprintf("%06d",$n1);
	print $subf "run$n3$suffixappend\n";
    }
}
else{
    for (; $n1<($n2+1); $n1++){
	print $subf "run00$n1$suffixappend\n";
    }
}

#!/usr/bin/perl
use strict;
use warnings;

#loop from n1 to n2 creating file00n.root
my ($n1, $n2, $filename, $suffixappend) = @ARGV;
open(my $subf, '>', $filename);
for (; $n1<($n2+1); $n1++){
    print $subf "run00$n1$suffixappend\n";
}

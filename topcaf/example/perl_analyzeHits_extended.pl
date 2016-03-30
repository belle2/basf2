#!/usr/bin/perl
use strict;
use warnings;
use File::Copy qw(copy);

my ($runlist, $subname, $cutnum) = @ARGV;

if (not defined $runlist){
    die "Need inputs: runlist, subname, cutnum \n";
}
my $sub_filename = 'sub_' . $subname . 'analyzeHits_extended.sh';
open(my $subf, '>', $sub_filename);
print $subf "#!/bin/sh\n";

open(my $fh, '<', $runlist)
    or die "Could not open file";
while (my $line = <$fh>){
    chomp $line;
    #drop .suffix
    $line =~ s{\.[^.]+$}{}; 
    #create shell script
    my $sh_filename = $subname . '_sh_scripts_batch/' . $line . 'analyzeHits_extended.sh';
    open(my $shf, '>', $sh_filename);
    print $shf "#!/bin/sh\n";
    print $shf "analyzeHits_extended /group/belle2/testbeam/TOP/CRT_analysis/hits/${line}_writehits.root /group/belle2/testbeam/TOP/CRT_analysis/hits/${line}_plothits_$cutnum.root $cutnum\n";
    close $shf;
    chmod 0777, $sh_filename;
    
    print $subf "bsub -K -q s -o /group/belle2/testbeam/TOP/CRT_analysis/logs/${line}_analyzeHits_extended_$cutnum.txt $sh_filename &\n";
    print $subf "sleep 0.2\n";
}
print $subf "wait\n";
chmod 0777, $sub_filename;


#make a script that will hadd everything later
my $sub_filename2 = 'hadd_' . $subname . '_plothits_' . $cutnum . '.sh';
open(my $subf2, '>', $sub_filename2);
print $subf2 "#!/bin/sh\n";
print $subf2 "hadd -f /group/belle2/testbeam/TOP/CRT_analysis/combined/${subname}_plothits_$cutnum.root ";

open(my $fh2, '<', $runlist)
    or die "Could not open file";
while (my $line2 = <$fh2>){
    chomp $line2;
    $line2 =~ s{\.[^.]+$}{};
    print $subf2 "/group/belle2/testbeam/TOP/CRT_analysis/hits/${line2}_plothits_$cutnum.root ";

}
chmod 0777, $sub_filename2;

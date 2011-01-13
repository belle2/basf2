#!/bin/bash
# Tom Fifield 2010-12
# Used to replicate a software tarball onto every belle VO Se
# usage ./replicate_file_everywhere.sh tarball.tar.gz
ses=`lcg-infosites -v 1 --vo belle se| sort | uniq | grep '\.'`
file=$1
# first, store the file in a close SE and register it with the LFC
lcg-cr -vv -l /grid/belle/software/$file $file
# loop through every SE trying to create a replica
for i in $ses
do
 lcg-rep -vv -n 20 -d $i lfn:/grid/belle/software/$file
done

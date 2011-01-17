#!/bin/bash
#
# uses ldapsearch to find existing ce/site with belle software installs
#
# $1 = build
# $2/${LCG_GFAL_INFOSYS} = topbdii
#
# Tom Fifield 2011-01

BUILD=${1:-`echo "build_2010-12-13"`}

LCG_GFAL_INFOSYS=${LCG_GFAL_INFOSYS:-$2}

LCG_GFAL_INFOSYS=`echo $LCG_GFAL_INFOSYS | awk -F: {'print $1}'`

ldapsearch -x -l 60 -b mds-vo-name=local,o=grid  -h $LCG_GFAL_INFOSYS -p 2170 -b o=grid "(GlueHostApplicationSoftwareRunTimeEnvironment=vo-belle-$BUILD)" GlueSiteName | perl -00pe 's/\r*\n //g'  | grep dn | awk -F= '{print $3 $4}' | awk -F,Mds-Vo-name '{ print $2 "\t" $1 }'


#!/bin/bash

#################################################################
#                                                               #
#    script to setup the workdire folder                        #
#                                                               #
#    written by Giulia Casarosa, INFN Pisa                      #
#    giulia.casarosa@pi.infn.it                                 #
#                                                               #
# USAGE:
#
# release> source tracking/examples/TrackingPerformanceEvaluation_scripts/setup_workdir.sh relase
#
#################################################################

echo "setting the workdir for release" $1

mkdir workdir
cd workdir
mkdir /group/belle2/users/casarosa/trackingValidation/$1
ln -s /group/belle2/users/casarosa/trackingValidation/$1 ./
cd -

echo "ls workdir"
ls workdir

echo "done"

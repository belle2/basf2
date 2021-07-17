#!/bin/bash

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#################################################################
#                                                               #
#    script to setup the workdire folder                        #
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

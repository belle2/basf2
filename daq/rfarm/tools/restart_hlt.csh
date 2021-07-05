#! /bin/tcsh
#set hltunit = "hlt03"

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
set hltunit = $1
#set configname = "PHASE3"
set configname = $2

# Stop HLT
stop_hlt_globalif $hltunit $configname
stop_rfarm_local $configname

# Start HLT
start_rfarm_local $configname
start_hlt_globalif $hltunit $configname

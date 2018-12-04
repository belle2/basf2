#! /bin/tcsh
set hltunit = "hlt03"
set configname = "PHASE2"

# Stop HLT
stop_hlt_globalif $hltunit $configname
stop_rfarm_local $configname

# Start HLT
start_rfarm_local $configname
start_hlt_globalif $hltunit $configname

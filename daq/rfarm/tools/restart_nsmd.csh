#! /bin/tcsh

if ( $#argv<2 ) then
  echo "Usage: restart_nsmd.csh unitname configname"
  exit -1
endif
 
# Kill basic servers
echo "Killing nsm2socket, logcollectord, and daqdbprovider"
killall nsm2socket
killall logcollectord
killall daqdbprovider

# Force stop all NSMDs
echo "Kill all nsmds and remove IPCs"
stop_nsmd_clean
sleep 3

# start local NSMD for HLT
echo "Start NSMD for HLT local"
start_nsmd $2

# start NSMDs for global interface.
echo "Start NSMD for global interface"
bootnsmd2 -g -c rc_$1
bootnsmd2 -g -c b$1

# Start nsm2socket interface
echo "Start nsm2socket on port 9090"
nsm2socket 9090 -d

# Start servers
echo "Start daqdbprovider and logcollectord"
daqdbprovider $1 -d
logcollectord $1 -d



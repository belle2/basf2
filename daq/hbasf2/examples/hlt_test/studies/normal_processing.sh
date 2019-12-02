set -e

# Start the monitoring
b2hlt_monitor.py "hltin:7000" "hltout:7000" --dat output.dat &
b2hlt_monitor.py "hltctl:7000" "hltin:7000" "hltout:7000" --dat output_full.dat &

sleep 1

# First, load
rcrequest rc_zhlttest LOAD -c rc_zhlttest  || true

# Load a second time if the first was in a timeout
rcrequest rc_zhlttest LOAD -c rc_zhlttest || true
rcrequest rc_zhlttest LOAD -c rc_zhlttest || true
rcrequest rc_zhlttest LOAD -c rc_zhlttest 

# Then start
rcrequest rc_zhlttest START 1 1  -c rc_zhlttest

# Wait for some time
sleep 50

# Then stop
rcrequest rc_zhlttest STOP  -c rc_zhlttest || true
rcrequest rc_zhlttest STOP  -c rc_zhlttest || true
rcrequest rc_zhlttest STOP  -c rc_zhlttest 

# Then start again
rcrequest rc_zhlttest START 1 1  -c rc_zhlttest

# Wait for some time
sleep 60

# Then stop
rcrequest rc_zhlttest STOP  -c rc_zhlttest || true
rcrequest rc_zhlttest STOP  -c rc_zhlttest || true
rcrequest rc_zhlttest STOP  -c rc_zhlttest 

# And abort
sleep 10
rcrequest rc_zhlttest ABORT  -c rc_zhlttest

# Now finish the monitoring
kill %1
kill %2

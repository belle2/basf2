set -e

# Start the monitoring
b2hlt_monitor.py "hltin:7000" "hltout:7000" --dat output_kill_workers.dat &
b2hlt_monitor.py "hltctl:7000" "hltin:7000" "hltout:7000" --dat output_kill_workers_full.dat &

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
sleep 20

# Then kill the first workers
echo "killing"
ssh hltwk01 "pgrep python3 | tail -n1 | xargs kill -SEGV"

# Wait another seconds
sleep 20

# Then kill the another workers
echo "killing"
ssh hltwk01 "pgrep python3 | tail -n1 | xargs kill -SEGV"

# Wait
sleep 20

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

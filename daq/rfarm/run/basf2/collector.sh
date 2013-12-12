#! /bin/bash

# Set trap
trap "echo signal; killall basf2; exit" INT TERM HUP

# Run basf2
basf2 --no-stats collector.py $1 $2 &
#basf2 --no-stats roitest.py $1 $2 &

# For Debugging with valgrind
#valgrind --leak-check=full basf2 --no-stats collector.py $1 $2 &

# For Debugging using gdb, cannot stop
#gdb basf2 <<EOF
#run --no-stats collector.py $1 $2
#bt
#frame 0
#p (*(EvtHeader*)m_data)
#quit
#EOF

# Waiting for the completion forever
while :;do
  sleep 2
done


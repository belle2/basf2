#!/bin/bash
for host in erctl erin erwk01 erwk02 display; do
    docker exec -it -d ${host} /bin/bash /work/scripts/restart.sh
done
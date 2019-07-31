#!/bin/bash
for host in hltin hltout hltwk01 hltwk02 storage hltctl; do
    docker exec -it -d ${host} /bin/bash /work/scripts/restart.sh
done
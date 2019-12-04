#!/bin/bash
source /root/setup_basf2 "" > /dev/null 2> /dev/null;

set -e

pkill -9 b2hlt || true
pkill -9 python3 || true

# Restart apps
if [[ "$(hostname)" == "hltctl" ]]; then
    pkill -9 runcontrold || true
    pkill -9 histoserverd || true
    pkill -9 nsm2socket || true
    pkill -9 nsmbridged || true
    sleep 1
    # only needed for testing setup
    runcontrold runcontrol_hlt_docker -d
    nsmbridged bhlt_docker -d
    nsm2socket 9090 -d

    runcontrold rc_hlt_docker -d
    histoserverd dqmserver_hlt_docker -d

    #hrelay dqmhistos dqmsrv1.dqmnet 9991 30 &
elif [[ "$(hostname)" == "hltin" ]]; then
    pkill -9 distributord || true
    pkill -9 test_ebd || true
    sleep 1
    distributord distributor_hlt_docker -d
    test_ebd eb1_hlt_docker -d
elif [[ "$(hostname)" == "hltout" ]]; then
    pkill -9 finalcollectord || true
    sleep 1
    finalcollectord collector_hlt_docker -d
elif [[ "$(hostname)" == "storage" ]]; then
    pkill -9 test_storaged || true
    sleep 1
    test_storaged storage_hlt_docker -d
else
    pkill -9 hltworkerd || true
    sleep 1
    hltworkerd evp_hltwk${HOSTNAME##hltwk}_hlt_docker -d
fi

#!/bin/bash
source /root/setup_basf2 "" > /dev/null 2> /dev/null;

set -e

pkill -9 b2hlt || true
pkill -9 python3 || true

# Restart apps
if [[ "$(hostname)" == "erctl" ]]; then
    pkill -9 runcontrold || true
    pkill -9 histoserverd || true
    pkill -9 collectord || true
    pkill -9 outputadapterd || true
    pkill -9 nsm2socket || true
    pkill -9 nsmbridged || true
    sleep 1
    # only needed for testing setup
    runcontrold runcontrol_ereco_docker -d
    nsmbridged bereco_docker -d
    nsm2socket 9090 -d

    runcontrold rc_ereco_docker -d
    histoserverd dqmserver_ereco_docker -d
    collectord outputcollector_ereco_docker -d
    outputadapterd outputadapter_ereco_docker -d

    #hrelay dqmhistos dqmsrv1.dqmnet 9991 30 &
elif [[ "$(hostname)" == "erin" ]]; then
    pkill -9 collectord || true
    pkill -9 inputadapterd || true
    pkill -9 test_ebd || true
    sleep 1
    collectord inputcollector_ereco_docker -d
    inputadapterd inputadapter_ereco_docker -d
    test_ebd eb2_ereco_docker -d
else
    pkill -9 hltworkerd || true
    sleep 1
    hltworkerd evp_erwk${HOSTNAME##erwk}_ereco_docker -d
fi

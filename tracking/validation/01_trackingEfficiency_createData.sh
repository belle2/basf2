#!/bin/bash

echo "Create events with 10 muon tracks with fixed pt values"

for i in $(seq 0 9)
do
	nohup basf2 02_trackingEfficiency_createData.py 13 $i >/dev/null 2>&1  &
done

echo "Create events with 10 pion tracks with fixed pt values"

for i in $(seq 0 9)
do
	nohup basf2 02_trackingEfficiency_createData.py 211 $i >/dev/null 2>&1  &
done

echo "Create events with 10 kaon tracks with fixed pt values"

for i in $(seq 0 9)
do
	nohup basf2 02_trackingEfficiency_createData.py 321 $i >/dev/null 2>&1  &
done

echo "Create events with 10 proton tracks with fixed pt values"

for i in $(seq 0 9)
do
	nohup basf2 02_trackingEfficiency_createData.py 2212 $i >/dev/null 2>&1  &
done

echo "All jobs are submitted!"
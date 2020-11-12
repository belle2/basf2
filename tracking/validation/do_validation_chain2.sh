#!/bin/bash

i=0
imax=2
while [ "$i" -le "$imax" ]; do
    python3 raster_validation_helpertask2.py $i
    python3 fullTrackingValidation2.py
    ((i++))
done

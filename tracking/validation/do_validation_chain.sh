#!/bin/bash

i=0
imax=2
while [ "$i" -le "$imax" ]; do
    python3 raster_validation_helpertask.py $i
    python3 fullTrackingValidation.py
    ((i++))
done

#!/bin/bash

i=22
imax=43
while [ "$i" -le "$imax" ]; do
    python3 raster_validation_helpertask2.py $i
    python3 fullTrackingValidation2.py
    ((i++))
done

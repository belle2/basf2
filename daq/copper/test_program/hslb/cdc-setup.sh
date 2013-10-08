#!/bin/sh
#
# FINNESSE A
#
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -a link
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -a checkfee
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -a verbose
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -a trigger
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -a -p delay 4
#/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -a -p window 20
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -a -p window 4
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -a -p tdcth 3700
#
# FINNESSE C
#
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -c link
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -c checkfee
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -c verbose
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -c trigger
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -c -p delay 4
#/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -c -p window 20
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -c -p window 4
/home/usr/yamadas/daq/hslb/hslb/B2LDover1 -c -p tdcth 3700


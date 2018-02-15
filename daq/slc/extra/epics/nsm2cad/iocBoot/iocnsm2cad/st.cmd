#!../../bin/linux-x86_64/rcbrigded

## You may have to change rcbrigded to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/nsm2cad.dbd"
nsm2cad_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/arich_feb_temp.db","nodename=CPR4001,hslb=0")
dbLoadRecords("db/arich_feb_temp.db","nodename=CPR4001,hslb=1")
dbLoadRecords("db/arich_feb_temp.db","nodename=CPR4001,hslb=2")
dbLoadRecords("db/arich_feb_temp.db","nodename=CPR4001,hslb=3")
nsmConfig("NSM2CA","arich10", 9122)

cd ${TOP}/iocBoot/${IOC}
iocInit


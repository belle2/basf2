#!../../bin/linux-x86_64/rcbrigded

## You may have to change rcbrigded to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/nsm2cad.dbd"
nsm2cad_registerRecordDeviceDriver pdbbase

## Load record instances
#dbLoadRecords("db/hvnode.db","nodename=PXDPS")
#dbLoadRecords("db/rcnode.db","nodename=PXDRC")
#dbLoadRecords("db/hvnode.db","nodename=SVDPS")
#dbLoadRecords("db/rcnode.db","nodename=SVDRC")
#dbLoadRecords("db/hvnode.db","nodename=HVMASTER")
dbLoadRecords("db/rcnode.db","nodename=RUNCONTROL")
#dbLoadRecords("db/konno_hv.db","nodename=KONNO_HV")
#dbLoadRecords("db/store_ecl_status.db","nodename=STORE_ECL")
#nsmConfig("NSM2CA","ropc02.b2nsm.kek.jp.", 9122)
nsmConfig("NSM2CA","belle-rpc1.desy.de", 9122)

cd ${TOP}/iocBoot/${IOC}
iocInit


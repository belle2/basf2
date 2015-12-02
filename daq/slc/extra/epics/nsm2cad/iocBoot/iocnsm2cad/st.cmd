#!../../bin/linux-x86_64/rcbrigded

## You may have to change rcbrigded to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/nsm2cad.dbd"
nsm2cad_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/hv.db","nodename=KONNO_HV")
dbLoadRecords("db/konno_hv.db","nodename=KONNO_HV")
#dbLoadRecords("db/store_ecl_status.db","nodename=STORE_ECL")
#nsmConfig("NSM2CA","ropc02.b2nsm.kek.jp.", 9122)
nsmConfig("NSM2CA","b2slow2.kek.jp.", 8122)

cd ${TOP}/iocBoot/${IOC}
iocInit


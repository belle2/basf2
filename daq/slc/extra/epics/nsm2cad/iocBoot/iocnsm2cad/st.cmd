#!../../bin/linux-x86_64/rcbrigded

## You may have to change rcbrigded to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/nsm2cad.dbd"
nsm2cad_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/nsmnode.db","nodename=TOMO")
dbLoadRecords("db/nsmnode.db","nodename=STORAGE")
dbLoadRecords("db/nsmdata_storage.db","dataname=STORAGE_STATUS")

rcConfig("NSM2CA","b2slow2.kek.jp", 9122)

cd ${TOP}/iocBoot/${IOC}
iocInit


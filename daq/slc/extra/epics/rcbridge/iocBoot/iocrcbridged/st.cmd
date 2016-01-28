#!../../bin/linux-x86_64/rcbrigded

## You may have to change rcbrigded to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/rcbridged.dbd"
rcbridged_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/rc.db","user=PXD")

rcConfig("PXD","b2slow2.kek.jp", 8122)

cd ${TOP}/iocBoot/${IOC}
iocInit


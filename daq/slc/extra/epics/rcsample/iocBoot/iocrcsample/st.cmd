#!../../bin/linux-x86_64/rcsample

## You may have to change rcsample to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/rcsample.dbd"
rcsample_registerRecordDeviceDriver pdbbase

## Load record instances
#dbLoadRecords("db/xxx.db","user=tkonnoHost")

cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
seq rcsample,"USER=PXD"

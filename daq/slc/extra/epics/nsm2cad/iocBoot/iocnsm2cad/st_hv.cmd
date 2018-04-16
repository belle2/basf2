#!../../bin/linux-x86_64/rcbrigded

## You may have to change rcbrigded to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/nsm2cad.dbd"
nsm2cad_registerRecordDeviceDriver pdbbase

## Load record instances
#dbLoadRecords("db/arich_hv.db","nodename=ARICH_HV,val=cmon")
#dbLoadRecords("db/arich_hv.db","nodename=ARICH_HV,val=vmon")

dbLoadRecords("db/nsmvget_text.db","name=ARICH_HV,val=config")
dbLoadRecords("db/nsmvget_text.db","name=ARICH_HV,val=config:standby")
dbLoadRecords("db/nsmvget_text.db","name=ARICH_HV,val=config:peak")
dbLoadRecords("db/nsmvget_text.db","name=ARICH_HV,val=hvstate")
dbLoadRecords("db/nsmvget_int.db","name=ARICH_HV,val=ncrates")

nsmConfig("CA_ARICHHV","arichsc.b2nsm.kek.jp", 8120)

cd ${TOP}/iocBoot/${IOC}
iocInit

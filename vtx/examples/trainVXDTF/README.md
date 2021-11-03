This folder contains modified copies of the scripts in the folder: tracking/examples/VXDTF\_redesign/  
They have been modified to produce sectorMaps for the new detector.

from https://confluence.desy.de/display/BI/Software+VXDTF2+Training+Sample+Definition

VXDTF2 training procedure
======

To be reworked with VTX containers.

The VXDTF2 training should be realized in three steps :  

* eventSimulation.py  
Production of the MC data used as Training Sample. Editable to configure Particle Gun or Y(4S) events. Some convenience functions are outsourced to the script setup_modules.py. Number of events and output filename can be defined with options -n and -o. E.g.: 'basf2 eventSimulation.py -n 1000 -o trainingSample.root'  
The scripts takes two optional command line arguments: the first will be interpreted as random seed, the second as directory for the output. E.g.: basf2 'eventSimulation.py 12354 ./datadir/'. For use on KEKCC, there is a bash script to create training data. E.g.: bash submitSomeJobs.sh  

* trainingPreparation.py  
Preparation of the Training sample before the actual training of the Sector Map. The input file can be provided with -i command line argument, and should be the output of eventSimulation.py  
The script takes two additional argument via argparse:  --enable selection (False by default) and --disable_checkFit (True by default). The first enable the NoKickCuts (see next section), the second disable the exclusion of the failing-fit tracks (by default only RecoTracks with valid fit are taken by training).  
The script produces several root output file, the relevant one is VTXDefault\_Belle2\_VTX.root.  

* trainSecMap.py  
Performs the Sector Map training based on the prepared Training Sample. The scripts takes to required arguments via argparse: --train\_sample, and --secmap. The first is the output of TrainingPreparation.py i.e.  VTXDefault\_Belle2\_VTX.root. The second is the name of the output Sector Map.

* testVXDTF2.py
Test of VXDTF2 which applies a trained Sector Map on the provided data. The root file containing the SecMap can be passed via the
none-basf2 commandline argument --secmap.

After testing, the output sectormap can be prepared for uploading to a GT. For this, create a folder 'localdb' containing a file 'database.txt'. The content of 
the file is a single line "dbstore/VTXSectorMaps.root 1 0,0,0,-1" giving the name and IoV of the to be uploaded payload. Next you can copy the sectormap file 
into the localdb and change its name to dbstore\_VTXSectorMaps.root\_rev\_1.root. 


The concrete steps to be executed are: 

1. copy vtx/example/trainVXDTF2 folder somewhere in /group/belle2/users/benjamin  where you have few hundred GB space

2. cd into that new directory

3. Set the geometry. For example: 

export  BELLE2_VTX_UPGRADE_GT=upgrade_2021-07-16_vtx_7layer  

4. Create folder for training events

mkdir datadir 

5. Submit jobs via bsub to simulate events (~2TB space needed)

bash submitSomeJobs.sh

6. prepare training: 

bsub -q l -o logTrainingPreparation.log  'basf2 -l WARNING trainingPreparation.py -i "./datadir/SimEvts*.root" '

7. Training requires >8GB RAM and takes long (>1h). Run it with nohup in background on KEKCC worker node 


nohup basf2 trainSecMap.py -- --train VTXDefault_Belle2_VTX.root  --secmap VTXSectorMaps.root &


8) Upload results. Do not forget to delete datadir and other big intermediate files


After testing, the output sectormap can be prepared for uploading to a GT. For this, create a folder 'localdb' containing a file 'database.txt'. The content of
the file is a single line "dbstore/VTXSectorMaps.root 1 0,0,0,-1" giving the name and IoV of the to be uploaded payload. Next you can copy the sectormap file
into the localdb and change its name to dbstore\_VTXSectorMaps.root\_rev\_1.root.
The localdb can be uploaded to the condDB server commond line tool b2conditionsdb.

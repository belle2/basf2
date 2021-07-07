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


This folder contains scripts for comparing the impact parameter resolution 
between Belle II with two PXD layers (so called PXD 2021) and Belle II outer 
detector with uprade vertex detector (VTX) instead of PXD+SVD. The second 
scenario is called Belle III for simplicity. 

Procedure for producing plots: 

* runSimReco_Belle3.py

Production of tuples for Belle III. Number of events can be defined by user. Editable to configure Particle 
Gun or Y(4s) or DiMuon events. Default is 'dimuon'. Other options are 'bbar' and 'gun'.

Usage: basf2 runSimReco_Belle3.py -n 10000 -- --gen "dimuon"


* runSimReco_Belle2.py

Production of tuples for Belle II. Number of events can be defined by user. Editable to configure Particle 
Gun or Y(4s) or DiMuon events. Default is dimuon. Other options are 'bbar' and 'gun'.

Usage: basf2 runSimReco_Belle2.py -n 10000 -- --gen "dimuon"

* compareResolution.py

Produces comparison plots for impact parameter resolution. Expects two files as input. The first file is Belle II and 
the second file is Belle II. 


Usage: basf2 compareResolution.py -- -f1 belle3_analysis_dimuon_all_charged.root -f2 belle2_analysis_dimuon_all_charged.root





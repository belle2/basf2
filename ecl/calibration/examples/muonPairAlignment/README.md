## Instructions to align the ECL using muon pairs

For a description of the alignment procedure, see BELLE2-NOTE-TE-2020-031. This README describes how to repeat the alignment process, broken down into these steps:
- produce ntuples of muons from data and simulation
- analyze the ntuples using root macros
- incorporate the new alignment into an ECL geometry payload
- perform some basic tests

The required scripts and macros are in the directory `ecl/calibration/examples/muonPairAlignment`

--- 

### Produce ntuples

Use the script `muPairAlignment.py` to generate ntuples from data and simulation. For data, it is convenient to run on a muon pair skim, such as `hlt_mumu_2trk`. Example usage:

`basf2 -i "inputDataFiles*.root" muPairAlignment.py dataOutputName.root`

This produces an ntuple of name `dataOutputName.root`. In practice, you will need to run on the batch system. It takes approximately 10,000 CPU seconds to process 10 fb-1 of data.

--- 

### Analyze the ntuples 

#### Produce profile plots of e1 versus location

Separately process the data and simulation ntuples to produce plots of energy in the most-energetic crystal versus location in the ECL using macro `e1ProfilePlots.C`. Example usage:

`root -b -q 'e1ProfilePlots.C("dataOutputName*.root", "exp14_data", "e1ProfilePlots_exp14_data.root" )'`

The second argument is a label included in the histogram titles. 

#### Determine the data-MC location differences

Macro `dataMinusMC.C` is used to find the difference in location between data and MC in each of the 1792 plots produced in the previous step. Edit the TStrings at the top of the file to specify the locations of the data and MC root files produced by `e1ProfilePlots.C`; a directory for pdf files; a label; and the location of the output root file. In addition to storing the location difference for each plot, the code produces 1792 pdf files of the data e1 vs location plot overlaid with MC before and after it is shifted to match data. Usage:

`root -q -b dataMinusMC.C`

The specified output root file contains 2D plots showing the data-MC difference in each region of the ECL. The pdf files are useful to verify that the results are sensible. 

#### Fit the data-MC location differences

The alignment parameters are found by fitting the output of  `dataMinusMC.C` using macro `fitDataMinusMC.C`. Edit the file to specify the name and location of the root file produced by `dataMinusMC.C`, and a label to be used on the histograms and output file name. 

`root fitDataMinusMC.C`

Run this interactively; it produces a useful 1D version of the data-MC differences versus region overlaid with the fit results. There are three separate fits. The output of each is written to screen. e.g. 

`Parameters for ecl/data/crystal_backward.dat`<br>
`Original: 1000  0.000000  0.000000  0.000000  1020.000000  3.141593  0.000000`<br>
`     New: 1000  0.001211  1.005916  0.055523  1019.509953  3.138587 -1.215087`

The fit uses six templates read from `readGeoFiles.root`. You will only need to modify this file if you change the definitions of the ECL regions in `e1ProfilePlots.C`.  `readGeoFiles.root` is the output of macro `readGeoFiles.C`, which reads files containing the locations of each crystal from `crystalGeometry\*.txt`. It also requires a root output of `e1ProfilePlots.C` to get the specification of each ECL region.  

--- 

### Creating a new geometry payload

The location and direction of each crystal is specified by the payload `ECLCrystalsShapeAndPosition`. It is created by script `ecl/scripts/create_ECLCrystalsShapeAndPosition_payload.py`, which in turn obtains the required information from files `ecl/data/crystal_placement_XXX.dat`. These files should be edited to reflect the output produced by `fitDataMinusMC.C` before running the script. You will need to check out a copy of the code to do this. 

`basf2 create_ECLCrystalsShapeAndPosition_payload.py` <br>
produces a directory `localdb` containing file `database.txt` and two payloads. `database.txt` should be edited to delete the line:<br>
`dbstore/GeoConfiguration 1 0,0,-1,-1`<br>
You do not want to upload this payload anywhere, since it contains only ECL geometry. 

For test purposes, you can upload the new `ECLCrystalsShapeAndPosition` payload to global tag `ecl_alignment`:<br>
`b2conditionsdb upload ecl_alignment localdb/database.txt`

Otherwise, you can request upload to central global tags using `b2conditionsdb-request`.

---

### Basic tests

The script `ecl/examples/EclDumpGeometry.py` can be used to print out the location and direction of every ECL crystal. For example, 

`basf2 -n 1 EclDumpGeometry.py 2 > newGeometry.txt`<br>
where the 2 causes the new payload to be read from the localdb directory. 

It may be useful to generate some single particle events using, for example, the script `singleMuAlignment.py` to ensure there are no GEANT errors. 

`basf2 singleMuAlignment.py 100 singleMu 2` 

The code describes the arguments. 








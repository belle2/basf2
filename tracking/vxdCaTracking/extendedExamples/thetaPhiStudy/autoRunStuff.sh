 #!/bin/bash
nEvents=5000
echo "runs for muons, beta gamma 1 2 and 5, for different theta ranges and overall.\n TFAnalizerResults.root does contain the data I used for plots"
date
date
date
date
date
echo "3 muons per event, 105.7 MeV/c = beta gamma 1, theta 85-95 degrees:"
pwd
basf2 TFRedesignDemo.py $nEvents 0.1057 0.1057 3 85. 95. 13 >> log.txt
roo -b -q standardRootPlotterV2.C
mkdir MuonBG1T85to95/ -p
mv VXDTFoutput.root MuonBG1T85to95/
mv TFAnalizerResults.root MuonBG1T85to95/
mv log.txt MuonBG1T85to95/
mv *.pdf MuonBG1T85to95/

date
date
date
echo "3 muons per event, 211.4 MeV/c = beta gamma 2, theta 85-95 degrees:"
pwd
basf2 TFRedesignDemo.py $nEvents 0.2114 0.2114 3 85. 95. 13 >> log.txt
roo -b -q standardRootPlotterV2.C
mkdir MuonBG2T85to95/ -p
mv VXDTFoutput.root MuonBG2T85to95/
mv TFAnalizerResults.root MuonBG2T85to95/
mv log.txt MuonBG2T85to95/
mv *.pdf MuonBG2T85to95/
date
date
date
echo "3 muons per event, 528.5 MeV/c = beta gamma 5, theta 85-95 degrees:"
pwd
basf2 TFRedesignDemo.py $nEvents 0.5285 0.5285 3 85. 95. 13 >> log.txt
roo -b -q standardRootPlotterV2.C
mkdir MuonBG5T85to95/ -p
mv VXDTFoutput.root MuonBG5T85to95/
mv TFAnalizerResults.root MuonBG5T85to95/
mv log.txt MuonBG5T85to95/
mv *.pdf MuonBG5T85to95/
date
date
date
date
date
echo "3 muons per event, 105.7 MeV/c = beta gamma 1, theta 20-30 degrees:"
pwd
basf2 TFRedesignDemo.py $nEvents 0.1057 0.1057 3 20. 30. 13 >> log.txt
roo -b -q standardRootPlotterV2.C
mkdir MuonBG1T20to30/ -p
mv VXDTFoutput.root MuonBG1T20to30/
mv TFAnalizerResults.root MuonBG1T20to30/
mv log.txt MuonBG1T20to30/
mv *.pdf MuonBG1T20to30/
date
date
date
echo "3 muons per event, 211.4 MeV/c = beta gamma 2, theta 85-30 degrees:"
pwd
basf2 TFRedesignDemo.py $nEvents 0.2114 0.2114 3 20. 30. 13 >> log.txt
roo -b -q standardRootPlotterV2.C
mkdir MuonBG2T20to30/ -p
mv VXDTFoutput.root MuonBG2T20to30/
mv TFAnalizerResults.root MuonBG2T20to30/
mv log.txt MuonBG2T20to30/
mv *.pdf MuonBG2T20to30/
date
date
date
echo "3 muons per event, 528.5 MeV/c = beta gamma 5, theta 20-30 degrees:"
pwd
basf2 TFRedesignDemo.py $nEvents 0.5285 0.5285 3 20. 30. 13 >> log.txt
roo -b -q standardRootPlotterV2.C
mkdir MuonBG5T20to30/ -p
mv VXDTFoutput.root MuonBG5T20to30/
mv TFAnalizerResults.root MuonBG5T20to30/
mv log.txt MuonBG5T20to30/
mv *.pdf MuonBG5T20to30/
date
date
date
date
date
echo "3 muons per event, 105.7 MeV/c = beta gamma 1, theta 135-145 degrees:"
pwd
basf2 TFRedesignDemo.py $nEvents 0.2114 0.2114 3 135. 145. 13 >> log.txt
roo -b -q standardRootPlotterV2.C
mkdir MuonBG1T135to145/ -p
mv VXDTFoutput.root MuonBG1T135to145/
mv TFAnalizerResults.root MuonBG1T135to145/
mv log.txt MuonBG1T135to145/
mv *.pdf MuonBG1T135to145/
date
date
date
echo "3 muons per event, 211.4 MeV/c = beta gamma 2, theta 135-145 degrees:"
pwd
basf2 TFRedesignDemo.py $nEvents 0.2114 0.2114 3 135. 145. 13 >> log.txt
roo -b -q standardRootPlotterV2.C
mkdir MuonBG2T135to145/ -p
mv VXDTFoutput.root MuonBG2T135to145/
mv TFAnalizerResults.root MuonBG2T135to145/
mv log.txt MuonBG2T135to145/
mv *.pdf MuonBG2T135to145/
date
date
date
echo "3 muons per event, 528.5 MeV/c = beta gamma 5, theta 135-145 degrees:"
pwd
basf2 TFRedesignDemo.py $nEvents 0.5285 0.5285 3 135. 145. 13 >> log.txt
roo -b -q standardRootPlotterV2.C
mkdir MuonBG5T135to145/ -p
mv VXDTFoutput.root MuonBG5T135to145/
mv TFAnalizerResults.root MuonBG5T135to145/
mv log.txt MuonBG5T135to145/
mv *.pdf MuonBG5T135to145/
date
date
date
date
date
echo "5 muons per event, 105.7 MeV/c = beta gamma 1, theta 12-152 degrees:"
pwd
basf2 TFRedesignDemo.py $nEvents 0.1057 0.1057 5 12. 152. 13 >> log.txt
roo -b -q standardRootPlotterV2.C
mkdir MuonBG1T12to152/ -p
mv VXDTFoutput.root MuonBG1T12to152/
mv TFAnalizerResults.root MuonBG1T12to152/
mv log.txt MuonBG1T12to152/
mv *.pdf MuonBG1T12to152/
date
date
date
echo "5 muons per event, 211.4 MeV/c = beta gamma 2, theta 12-152 degrees:"
pwd
basf2 TFRedesignDemo.py $nEvents 0.2114 0.2114 5 12. 152. 13 >> log.txt
roo -b -q standardRootPlotterV2.C
mkdir MuonBG2T12to152/ -p
mv VXDTFoutput.root MuonBG2T12to152/
mv TFAnalizerResults.root MuonBG2T12to152/
mv log.txt MuonBG2T12to152/
mv *.pdf MuonBG2T12to152/
date
date
date
echo "5 muons per event, 528.5 MeV/c = beta gamma 5, theta 12-152 degrees:"
pwd
basf2 TFRedesignDemo.py $nEvents 0.5285 0.5285 5 12. 152. 13 >> log.txt
roo -b -q standardRootPlotterV2.C
mkdir MuonBG5T12to152/ -p
mv VXDTFoutput.root MuonBG5T12to152/
mv TFAnalizerResults.root MuonBG5T12to152/
mv log.txt MuonBG5T12to152/
mv *.pdf MuonBG5T12to152/
date
date
date
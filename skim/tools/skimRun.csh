#!/bin/tcsh -m 

# This script runs all availble skims on the available MC9 testing samples at KEKcc.



foreach DSkim (Dark BtoCharmless BtoCharm ALP3Gamma BottomoniumEtabExclusive BottomoniumUpsilon TauGeneric SystematicsRadMuMu SystematicsRadEE LFVZpInvisible LFVZpVisible SinglePhotonDark SystematicsTracking BottomoniumUpsilon BottomoniumEtabExclusive  SystematicsLambda  Systematics Resonance ISRpipicc BtoDh_Kspipipi0 BtoPi0Pi0  CharmSemileptonic  BottomoniumEtabExclusive BottomoniumUpsilon feiSLB0WithOneLep feiBplusWithOneLep feiHadronicB0 feiHadronicBplus  BtoPi0Pi0  Charm3BodyHadronic2 Charm3BodyHadronic Charm3BodyHadronicD0 Charm2BodyHadronic  Charm2BodyNeutrals Charm2BodyNeutralsD0  BtoDh_Kspi0  BtoDh_hh BtoDh_Kshh Tau  PRsemileptonicUntagged SLUntagged LeptonicUntagged TCPV  CharmRare BtoXll BtoXgamma  TauLFV)

# To run on low multiplicity samples use the line below: 

#foreach DType (MC9_bsbs6SBGx1 MC9nonbsbs_6SBGx1 MC9_ccbar6SBGx1 MC9_uubar6SBGx1 MC9_ddbar6SBGx1 MC9_ssbar6SBGx1 MC9_taupair6SBGx1 MC9_bsbs5SBGx1 MC9_nonbsbs5SBGx1 MC9_ccbar5SBGx1 MC9_uubar5SBGx1 MC9_ddbar5SBGx1 MC9_ssbar5SBGx1 MC9_taupair5SBGx1 MC9_bsbs5SBGx0 MC9_nonbsbs5SBGx0 MC9_ccbar5SBGx0 MC9_uubar5SBGx0 MC9_ddbar5SBGx0 MC9_ssbar5SBGx0 MC9_taupair5SBGx0 generic3SBGx1 MC9_ccbar3SBGx1 MC9_uubar3SBGx1 MC9_ddbar3SBGx1 MC9_ssbar3SBGx1 MC9_taupair3SBGx1) 
foreach DType(MC9_mixedBGx0 MC9_chargedBGx0 MC9_uubarBGx0 MC9_ddbarBGx0 MC9_ssbarBGx0  MC9_ccbarBGx0 MC9_taupairBGx0 MC9_mixedBGx1 MC9_chargedBGx1 MC9_ccbarBGx1 MC9_uubarBGx1 MC9_ddbarBGx1 MC9_ssbarBGx1 MC9_taupairBGx1 )
echo $DSkim
echo $DType
set inputFile="dc"


if ($DType == "MC9_mixedBGx1") then
  set inputFile = "/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/mdst_000001_prod00002288_task00000001.root"
else if ($DType == "MC9_chargedBGx1") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002289/e0000/4S/r00000/charged/sub00/mdst_000001_prod00002289_task00000001.root'
else if ($DType == "MC9_ccbarBGx1") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002321/e0000/4S/r00000/ccbar/sub00/mdst_000001_prod00002321_task00000001.root'
else if ($DType == "MC9_ssbarBGx1") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002312/e0000/4S/r00000/ssbar/sub00/mdst_000001_prod00002312_task00000001.root'
else if ($DType == "MC9_uubarBGx1") then 
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002290/e0000/4S/r00000/uubar/sub00/mdst_000001_prod00002290_task00000001.root'
else if ($DType == "MC9_ddbarBGx1") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002311/e0000/4S/r00000/ddbar/sub00/mdst_000268_prod00002311_task00000268.root'
else if ($DType == "MC9_taupairBGx1") then 
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002322/e0000/4S/r00000/taupair/sub00/mdst_000001_prod00002322_task00000001.root'
else if ($DType == "MC9_mixedBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002166/e0000/4S/r00000/mixed/sub00/mdst_000001_prod00002166_task00000001.root'
else if ($DType == "MC9_chargedBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002167/e0000/4S/r00000/charged/sub00/mdst_000001_prod00002167_task00000001.root'
else if ($DType == "MC9_ccbarBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002171/e0000/4S/r00000/ccbar/sub00/mdst_000001_prod00002171_task00000001.root'
else if ($DType == "MC9_ssbarBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002170/e0000/4S/r00000/ssbar/sub00/mdst_000001_prod00002170_task00000001.root'
else if ($DType == "MC9_uubarBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002168/e0000/4S/r00000/uubar/sub00/mdst_000001_prod00002168_task00000001.root'
else if ($DType == "MC9_ddbarBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002169/e0000/4S/r00000/ddbar/sub00/mdst_000001_prod00002169_task00000001.root'
else if ($DType == "MC9_taupairBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002172/e0000/4S/r00000/taupair/sub00/mdst_000001_prod00002172_task00000001.root'





else if ($DType == "ccbar3SBGx1") then
  set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002709/e0000/3S/r00000/ccbar/sub00/mdst_000001_prod00002709_task00000001.root'
else if ($DType == "generic3SBGx1") then
  set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002705/e0000/3S/r00000/generic/sub00/mdst_000001_prod00002705_task00000001.root'
else if ($DType == "ddbar3SBGx1") then
  set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002707/e0000/3S/r00000/ddbar/sub00/mdst_000001_prod00002707_task00000001.root'
else if ($DType == "MC9_ssbar3SBGx1") then
  set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002708/e0000/3S/r00000/ssbar/sub00/mdst_000001_prod00002708_task00000001.root'
else if ($DType == "MC9_taupair3SBGx1") then
  set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002714/e0000/3S/r00000/taupair/sub00/mdst_000001_prod00002714_task00000001.root'
else if ($DType == "MC9_uubar3SBGx1") then
  set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002706/e0000/3S/r00000/uubar/sub00/mdst_000001_prod00002706_task00000001.root'
# 5S samples
else if ($DType == "MC9_bsbs5SBGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002281/e0000/5S/r00000/bsbs/sub00/mdst_000002_prod00002281_task00000002.root'
else if ($DType == "MC9_nonbsbs5SBGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002282/e0000/5S/r00000/nonbsbs/sub00/mdst_000002_prod00002282_task00000002.root'
else if ($DType == "MC9_uubar5SBGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002283/e0000/5S/r00000/uubar/sub00/mdst_000001_prod00002283_task00000001.root'
else if ($DType == "ddbar5SBGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002284/e0000/5S/r00000/ddbar/sub00/mdst_000001_prod00002284_task00000001.root'
else if ($DType == "MC9_ssbar5SBGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002285/e0000/5S/r00000/ssbar/sub00/mdst_000001_prod00002285_task00000001.root'
else if ($DType == "ccbar5SBGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002286/e0000/5S/r00000/ccbar/sub00/mdst_000001_prod00002286_task00000001.root'
else if ($DType == "MC9_taupair5SBGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002287/e0000/5S/r00000/taupair/sub00/mdst_000001_prod00002287_task00000001.root'
else if ($DType == "MC9_bsbs5SBGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002664/e0000/5S/r00000/bsbs/sub00/mdst_000001_prod00002664_task00000001.root'
else if ($DType == "MC9_nonbsbs5SBGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002665/e0000/5S/r00000/nonbsbs/sub00/mdst_000001_prod00002665_task00000001.root'
else if ($DType == "MC9_uubar5SBGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002667/e0000/5S/r00000/uubar/sub00/mdst_000001_prod00002667_task00000001.root'
else if ($DType == "ddbar5SBGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002669/e0000/5S/r00000/ddbar/sub00/mdst_000001_prod00002669_task00000001.root'
else if ($DType == "MC9_ssbar5SBGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002670/e0000/5S/r00000/ssbar/sub00/mdst_000001_prod00002670_task00000001.root'
else if ($DType == "ccbar5SBGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002671/e0000/5S/r00000/ccbar/sub00/mdst_000001_prod00002671_task00000001.root'
else if ($DType == "MC9_taupair5SBGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002698/e0000/5S/r00000/taupair/sub00/mdst_000001_prod00002698_task00000001.root'
#6S samples
else if ($DType == "MC9_bsbs6SBGx1") then
 set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002715/e0000/6S/r00000/bsbs/sub00/mdst_000001_prod00002715_task00000001.root'
else if ($DType == "MC9_nonbsbs6SBGx1") then
 set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002716/e0000/6S/r00000/nonbsbs/sub00/mdst_000001_prod00002716_task00000001.root'
else if ($DType == "ccbar6SBGx1") then
 set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002702/e0000/6S/r00000/ccbar/sub00/mdst_000001_prod00002702_task00000001.root'
else if ($DType == "MC9_uubar6SBGx1") then
 set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002699/e0000/6S/r00000/uubar/sub00/mdst_000001_prod00002699_task00000001.root'
else if ($DType == "ddbar6SBGx1") then
 set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002700/e0000/6S/r00000/ddbar/sub00/mdst_000001_prod00002700_task00000001.root'
else if ($DType == "MC9_ssbar6SBGx1") then
 set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002701/e0000/6S/r00000/ssbar/sub00/mdst_000001_prod00002701_task00000001.root'
else if ($DType == "MC9_taupair6SBGx1") then
 set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002703/e0000/6S/r00000/taupair/sub00/mdst_000001_prod00002703_task00000001.root'
endif



set script1='../standalone/'$DSkim'_Skim_Standalone.py'

set logF=$DSkim'_'$DType'.out'
set errF=$DSkim'_'$DType'.err'
set outputFile=$DSkim'_'$DType'.udst.root'

echo 'Running '$script1' on ' $inputFile ' to '$outputFile
bsub -q l -oo $logF -e $errF 'basf2 '$script1'  -o '$outputFile' -n 10000 -i '$inputFile
end
end

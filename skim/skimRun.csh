#!/bin/tcsh -m 


foreach DSkim (BottomoniumUpsilon BottomoniumEtabExclusive)#  SystematicsLambda  Systematics Resonance ISRpipicc BtoDh_Kspipipi0 BtoPi0Pi0  CharmSemileptonic  BottomoniumEtabExclusive BottomoniumUpsilon feiSLB0WithOneLep feiBplusWithOneLep feiHadronicB0 feiHadronicBplus  BtoPi0Pi0  Charm3BodyHadronic2 Charm3BodyHadronic Charm3BodyHadronicD0 Charm2BodyHadronic  Charm2BodyNeutrals Charm2BodyNeutralsD0  BtoDh_Kspi0  BtoDh_hh BtoDh_Kshh Tau  PRsemileptonicUntagged SLUntagged LeptonicUntagged TCPV  CharmRare BtoXll BtoXgamma)
foreach DType(mixedBGx1 bsbs6SBGx1 nonbsbs_6S_BGx1 ccbar_6S_BGx1 uubar_6S_BGx1 ddbar_6S_BGx1 ssbar_6S_BGx1 taupair_6S_BGx1 bsbs_5S_BGx1 nonbsbs_5S_BGx1 ccbar_5S_BGx1 uubar_5S_BGx1 ddbar_5S_BGx1 ssbar_5S_BGx1 taupair_5S_BGx1 bsbs_5S_BGx0 nonbsbs_5S_BGx0 ccbar_5S_BGx0 uubar_5S_BGx0 ddbar_5S_BGx0 ssbar_5S_BGx0 taupair_5S_BGx0 generic_Y3S_BGx1 ccbar_3S_BGx1 uubar_3S_BGx1 ddbar_3S_BGx1 ssbar_3S_BGx1 taupair_3S_BGx1) 
#foreach DType(mixedBGx0 chargedBGx0 uubarBGx0 ddbarBGx0 ssbarBGx0  ccbarBGx0 taupairBGx0 mixedBGx1 chargedBGx1 ccbarBGx1 uubarBGx1 ddbarBGx1 ssbarBGx1 taupairBGx1 )
echo $DSkim
echo $DType
set inputFile=""


if ($DType == "mixedBGx1") then
  set inputFile = "/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/mdst_000001_prod00002288_task00000001.root"
else if ($DType == "chargedBGx1") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002289/e0000/4S/r00000/charged/sub00/mdst_000001_prod00002289_task00000001.root'
else if ($DType == "ccbarBGx1") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002321/e0000/4S/r00000/ccbar/sub00/mdst_000001_prod00002321_task00000001.root'
else if ($DType == "ssbarBGx1") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002312/e0000/4S/r00000/ssbar/sub00/mdst_000001_prod00002312_task00000001.root'
else if ($DType == "uubarBGx1") then 
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002290/e0000/4S/r00000/uubar/sub00/mdst_000001_prod00002290_task00000001.root'
else if ($DType == "ddbarBGx1") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002311/e0000/4S/r00000/ddbar/sub00/mdst_000268_prod00002311_task00000268.root'
else if ($DType == "taupairBGx1") then 
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002322/e0000/4S/r00000/taupair/sub00/mdst_000001_prod00002322_task00000001.root'
else if ($DType == "mixedBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002166/e0000/4S/r00000/mixed/sub00/mdst_000001_prod00002166_task00000001.root'
else if ($DType == "chargedBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002167/e0000/4S/r00000/charged/sub00/mdst_000001_prod00002167_task00000001.root'
else if ($DType == "ccbarBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002171/e0000/4S/r00000/ccbar/sub00/mdst_000001_prod00002171_task00000001.root'
else if ($DType == "ssbarBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002170/e0000/4S/r00000/ssbar/sub00/mdst_000001_prod00002170_task00000001.root'
else if ($DType == "uubarBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002168/e0000/4S/r00000/uubar/sub00/mdst_000001_prod00002168_task00000001.root'
else if ($DType == "ddbarBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002169/e0000/4S/r00000/ddbar/sub00/mdst_000001_prod00002169_task00000001.root'
else if ($DType == "taupairBGx0") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002172/e0000/4S/r00000/taupair/sub00/mdst_000001_prod00002172_task00000001.root'
else if ($DType == "ccbar_Y3S_BGx1") then
  set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002709/e0000/3S/r00000/ccbar/sub00/mdst_000001_prod00002709_task00000001.root'
else if ($DType == "generic_Y3S_BGx1") then
  set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002705/e0000/3S/r00000/generic/sub00/mdst_000001_prod00002705_task00000001.root'
else if ($DType == "ddbar_Y3S_BGx1") then
  set inputFile ='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002707/e0000/3S/r00000/ddbar/sub00/mdst_000001_prod00002707_task00000001.root'
else if ($DType == "ssbar_Y3S_BGx1") then
  set inputFile ='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002708/e0000/3S/r00000/ssbar/sub00/mdst_000001_prod00002708_task00000001.root'
else if ($DType == "taupair_Y3S_BGx1") then
  set inputFile ='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002714/e0000/3S/r00000/taupair/sub00/mdst_000001_prod00002714_task00000001.root'
else if ($DType == "uubar_Y3S_BGx1") then
  set inputFile ='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002706/e0000/3S/r00000/uubar/sub00/mdst_000001_prod00002706_task00000001.root'
else if ($DType == "bsbs_5S_BGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002281/e0000/5S/r00000/bsbs/sub00/mdst_000001_prod00002281_task00000001.root'
else if ($DType == "nonbsbs_5S_BGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002282/e0000/5S/r00000/nonbsbs/sub00/mdst_000001_prod00002282_task00000001.root'
else if ($DType == "uubar_5S_BGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002283/e0000/5S/r00000/uubar/sub00/mdst_000001_prod00002283_task00000001.root'
else if ($DType == "ddbar_5S_BGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002284/e0000/5S/r00000/ddbar/sub00/mdst_000001_prod00002284_task00000001.root'
else if ($DType == "ssbar_5S_BGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002285/e0000/5S/r00000/ssbar/sub00/mdst_000001_prod00002285_task00000001.root'
else if ($DType == "ccbar_5S_BGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002286/e0000/5S/r00000/ccbar/sub00/mdst_000001_prod00002286_task00000001.root'
else if ($DType == "taupair_5S_BGx0") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002287/e0000/5S/r00000/taupair/sub00/mdst_000001_prod00002287_task00000001.root'
else if ($DType == "bsbs_5S_BGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002664/e0000/5S/r00000/bsbs/sub00/mdst_000001_prod00002664_task00000001.root'
else if ($DType == "nonbsbs_5S_BGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002665/e0000/5S/r00000/nonbsbs/sub00/mdst_000001_prod00002665_task00000001.root'
else if ($DType == "uubar_5S_BGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002667/e0000/5S/r00000/uubar/sub00/mdst_000001_prod00002667_task00000001.root'
else if ($DType == "ddbar_5S_BGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002669/e0000/5S/r00000/ddbar/sub00/mdst_000001_prod00002669_task00000001.root'
else if ($DType == "ssbar_5S_BGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002670/e0000/5S/r00000/ssbar/sub00/mdst_000001_prod00002670_task00000001.root'
else if ($DType == "ccbar_5S_BGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002671/e0000/5S/r00000/ccbar/sub00/mdst_000001_prod00002671_task00000001.root'
else if ($DType == "taupair_5S_BGx1") then
   set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002698/e0000/5S/r00000/taupair/sub00/mdst_000001_prod00002698_task00000001.root'
else if ($DType == "bsbs6SBGx1") then
 set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002715/e0000/6S/r00000/bsbs/sub00/mdst_000001_prod00002715_task00000001.root'
else if ($DType == "nonbsbs_6S_BGx1") then
 set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002716/e0000/6S/r00000/nonbsbs/sub00/mdst_000001_prod00002716_task00000001.root'
else if ($DType == "ccbar_6S_BGx1") then
 set inputFile='/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002702/e0000/6S/r00000/ccbar/sub00/mdst_000001_prod00002702_task00000001.root'
else if ($DType == "uubar_6S_BGx1") then
 set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002699/e0000/6S/r00000/uubar/sub00/mdst_000001_prod00002699_task00000001.root'
else if ($DType == "ddbar_6S_BGx1") then
 set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002700/e0000/6S/r00000/ddbar/sub00/mdst_000001_prod00002700_task00000001.root'
else if ($DType == "ssbar_6S_BGx1") then
 set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002701/e0000/6S/r00000/ssbar/sub00/mdst_000001_prod00002701_task00000001.root'
else if ($DType == "taupair_6S_BGx1") then
 set inputFile = '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002703/e0000/6S/r00000/taupair/sub00/mdst_000001_prod00002703_task00000001.root'
endif



set outputFile='outputFiles/'$DSkim'_'$DType'.udst.root'
set script1=$DSkim'_Skim_Standalone.py'
set input1='inputFiles/'$DType'.txt'
set output1='outputFiles/'$DSkim'_'$DType'.txt'
echo $script1
echo $input1
echo $output1
echo $inputFile
echo $outputFile
set logF='outputFiles/'$DSkim'_'$DType'.out'
set errF='outputFiles/'$DSkim'_'$DType'.err'


echo 'basf2 '$script1' -n 10000 -o '$outputFile ' -i ' $inputFile 
bsub -q l -oo $logF -e $errF 'basf2 '$script1' -n 10000 -o '$outputFile' -i  '$inputFile
end
end

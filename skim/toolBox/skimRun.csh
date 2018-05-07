#!/bin/tcsh -m
set MCcampaign='MC9'
foreach DSkim (TCPV feiHadronicB0 feiHadronicBplus feiSLBplusWithOneLep feiSLB0WithOneLep PRsemileptonicUntagged BtoPi0Pi0 BottomoniumEtabExclusive  BottomoniumUpsilon Charm3BodyHadronicD0 Charm2BodyHadronicD0 Charm3BodyHadronic2 Charm3BodyHadronic Charm2BodyHadronic CharmSemileptonic  Charm2BodyNeutrals Charm2BodyNeutralsD0 BtoDh_hh BtoDh_Kspi0 BtoDh_Kspipipi0 BtoDh_Kshh Tau  SystematicsJpsiee SystematicsJpsimumu SystematicsDstar SystematicsLambda DoubleCharm SLUntagged LeptonicUntagged TCPV  CharmRare CharmlessHad DoubleCharm BtoXll BtoXgamma)
foreach DType (mixedBGx0 chargedBGx0 uubarBGx0 ddbarBGx0 ssbarBGx0  ccbarBGx0 taupairBGx0 mixedBGx1 chargedBGx1 ccbarBGx1 uubarBGx1 ddbarBGx1 ssbarBGx1 taupairBGx1 eebarrel_BGx1 ee_ecldigitsBGx1 gg_ecldigitsBGx1 mumu_ecldigitsBGx1 singlephotonBGx1 eeeeBGx1 eeisrBGx1 )
 echo $DSkim
echo $DType

set script1=$DSkim'_Skim_Standalone.py'
set input1='inputFiles/'$MCcampaign'/'$DType'.txt'
set output1='outputFiles/'$MCcampaign'/'$DSkim'_'$DType'.txt'
echo $script1
echo $input1
echo $output1
set logF='outputFiles/'$DSkim'_'$DType'.out'
set errF='outputFiles/'$DSkim'_'$DType'.err'


bsub -q l -oo $logF -e $errF 'basf2 '$script1' -n 10000 '$DType' ' 
end
end

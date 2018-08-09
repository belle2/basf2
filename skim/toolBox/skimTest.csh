#!/bin/tcsh -m

foreach DSkim(Bottomonium BtoCharm Resonance EWP Combined Semileptonic SystematicsLambda SystematicsTracking  Systematics ISRpipicc BtoDh_Kspipipi0  CharmSemileptonic  BottomoniumEtabExclusive BottomoniumUpsilon TCPV feiHadronicB0 feiHadronicBplus  BtoPi0Pi0  Charm3BodyHadronic2 Charm3BodyHadronic Charm2BodyHadronic  Charm2BodyNeutrals Charm2BodyNeutralsD0  BtoDh_Kspi0  BtoDh_hh BtoDh_Kshh Tau  PRsemileptonicUntagged SLUntagged LeptonicUntagged TCPV  CharmRare BtoXll BtoXgamma)
echo $DSkim

set script1=$DSkim'_Skim_Standalone.py'
echo $script1

basf2 $script1 -n 10 
echo 'END OF  SKIM' 
end

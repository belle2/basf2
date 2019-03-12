#!/bin/tcsh -m 

# This script runs all availble skims on the GRID. 



foreach DSkim (Dark BtoCharmless BtoCharm Quarkonium Semileptonic feiHadronicCombined MiscCombined CombinedSystematics Resonance ALP3Gamma BottomoniumEtabExclusive BottomoniumUpsilon TauGeneric SystematicsRadMuMu SystematicsRadEE LFVZpInvisible LFVZpVisible SinglePhotonDark SystematicsTracking BottomoniumUpsilon BottomoniumEtabExclusive  SystematicsLambda  Systematics Resonance ISRpipicc BtoDh_Kspipipi0 BtoPi0Pi0  CharmSemileptonic  BottomoniumEtabExclusive BottomoniumUpsilon feiSLB0WithOneLep feiBplusWithOneLep feiHadronicB0 feiHadronicBplus  BtoPi0Pi0  Charm3BodyHadronic2 Charm3BodyHadronic Charm3BodyHadronicD0 Charm2BodyHadronic  Charm2BodyNeutrals Charm2BodyNeutralsD0  BtoDh_Kspi0  BtoDh_hh BtoDh_Kshh Tau  PRsemileptonicUntagged SLUntagged LeptonicUntagged TCPV  CharmRare BtoXll BtoXgamma  TauLFV)

set input_file = '/belle/MC/release-01-00-03/DB00000294/MC10/prod00004770/s00/e0000/4S/r00000/mixed/mdst/sub00/mdst_000996_prod00004770_task00000996.root'
set output_file = '/belle/user/rachac/skimTests3'
set script = $DSkim'_Skim_Standalone.py'
set projectName = 'test3_'$DSkim
gbasf2 ./$script -p $projectName -i $input_file -o $output_file  --force
end
end

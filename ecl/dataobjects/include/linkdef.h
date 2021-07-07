#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::ECLCellIdMapping+; // checksum=0x9b8992c6, version=1
#pragma link C++ class Belle2::ECLWaveforms+; // checksum=0x8e396ab6, version=1
#pragma link C++ class Belle2::ECLLocalMaximum+; // checksum=0x508c657d, version=3
#pragma link C++ class Belle2::ECLConnectedRegion+; // checksum=0xa7247f52, version=2
#pragma link C++ class Belle2::ECLPureCsIInfo+; // checksum=0x2710b222, version=2
#pragma link C++ class Belle2::ECLDigit+; // checksum=0x93164e6c, version=2
#pragma link C++ class Belle2::ECLCalDigit+; // checksum=0x85c12d90, version=6
#pragma link C++ class Belle2::ECLDsp+; // checksum=0x71bcc594, version=6
#pragma link C++ class Belle2::ECLDspWithExtraMCInfo+; // checksum=0x13725e34, version=1
#pragma link C++ class Belle2::ECLTrig+; // checksum=0x3de50c09, version=3
#pragma link C++ class Belle2::ECLSimHit+; // checksum=0x67f0e0ff, version=6
#pragma link C++ class Belle2::ECLHit+; // checksum=0xc5a4962f, version=4
#pragma link C++ class Belle2::ECLDebugHit+; // checksum=0xbe219607, version=1
#pragma link C++ class Belle2::ECLHitAssignment+; // checksum=0x2bf88ae8, version=1
#pragma link C++ class Belle2::ECLShower+; // checksum=0x3631f10f, version=15
#pragma link C++ class Belle2::ECLPidLikelihood+; // checksum=0x25e90278, version=3

//----------------------------------------------------------
//Support ECLShowers classDefversion < 9

#pragma read sourceClass="Belle2::ECLShower" version="[-8]" \
  source="int m_Status" \
  targetClass="Belle2::ECLShower" target="m_status" \
  code="{m_status = onfile.m_Status;}"

#pragma read sourceClass="Belle2::ECLShower" version="[-8]" \
  source="int m_ShowerId" \
  targetClass="Belle2::ECLShower" target="m_showerId" \
  code="{m_showerId = onfile.m_ShowerId;}"

#pragma read sourceClass="Belle2::ECLShower" version="[-8]" \
  source="double m_Energy" \
  targetClass="Belle2::ECLShower" target="m_energy" \
  code="{m_energy = onfile.m_Energy;}"

#pragma read sourceClass="Belle2::ECLShower" version="[-8]" \
  source="double m_EnedepSum" \
  targetClass="Belle2::ECLShower" target="m_energyRaw" \
  code="{m_energyRaw = onfile.m_EnedepSum;}"

#pragma read sourceClass="Belle2::ECLShower" version="[-8]" \
  source="double m_Theta" \
  targetClass="Belle2::ECLShower" target="m_theta" \
  code="{m_theta = onfile.m_Theta;}"

#pragma read sourceClass="Belle2::ECLShower" version="[-8]" \
  source="double m_Phi" \
  targetClass="Belle2::ECLShower" target="m_phi" \
  code="{m_phi = onfile.m_Phi;}"

#pragma read sourceClass="Belle2::ECLShower" version="[-8]" \
  source="double m_R" \
  targetClass="Belle2::ECLShower" target="m_r" \
  code="{m_r = onfile.m_R;}"

#pragma read sourceClass="Belle2::ECLShower" version="[-8]" \
  source="double m_Time" \
  targetClass="Belle2::ECLShower" target="m_time" \
  code="{m_time = onfile.m_Time;}"

#pragma read sourceClass="Belle2::ECLShower" version="[-8]" \
  source="double m_timeResolution" \
  targetClass="Belle2::ECLShower" target="m_deltaTime99" \
  code="{m_deltaTime99 = onfile.m_timeResolution;}"

#pragma read sourceClass="Belle2::ECLShower" version="[-8]" \
  source="double m_HighestEnergy" \
  targetClass="Belle2::ECLShower" target="m_energyHighestCrystal" \
  code="{m_energyHighestCrystal = onfile.m_HighestEnergy;}"

#pragma read sourceClass="Belle2::ECLShower" version="[-8]" \
  source="double m_NofCrystals" \
  targetClass="Belle2::ECLShower" target="m_numberOfCrystals" \
  code="{m_numberOfCrystals = onfile.m_NofCrystals;}"

#pragma read sourceClass="Belle2::ECLShower" version="[-8]" \
  source="double m_E9oE25" \
  targetClass="Belle2::ECLShower" target="m_E9oE21" \
  code="{m_E9oE21 = onfile.m_E9oE25;}"
//----------------------------------------------------------


#endif

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::ECLWaveforms+;
#pragma link C++ class Belle2::ECLLocalMaximum+;
#pragma link C++ class Belle2::ECLConnectedRegion+;
#pragma link C++ class Belle2::ECLEventInformation+;
#pragma link C++ class Belle2::ECLPureCsIInfo+;
#pragma link C++ class Belle2::ECLDigit+;
#pragma link C++ class Belle2::ECLCalDigit+;
#pragma link C++ class Belle2::ECLDsp+;
#pragma link C++ class Belle2::ECLTrig+;
#pragma link C++ class Belle2::ECLSimHit+;
#pragma link C++ class Belle2::ECLHit+;
#pragma link C++ class Belle2::ECLDebugHit+;
#pragma link C++ class Belle2::ECLHitAssignment+;
#pragma link C++ class Belle2::ECLShower+;
#pragma link C++ class Belle2::ECLPidLikelihood+;
#pragma link C++ class Belle2::ECLWaveformData+;
#pragma link C++ class Belle2::ECLWFAlgoParams+; // implicit
#pragma link C++ class Belle2::ECLNoiseData+; // implicit
#pragma link C++ class Belle2::ECLLookupTable+; // implicit

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

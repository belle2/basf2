#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::SimHitBase+; // checksum=0xf68f360, version=3
#pragma link C++ class Belle2::BeamBackHit+; // checksum=0xe6e14558, version=2
#pragma link C++ class Belle2::MCParticleTrajectory+; // checksum=0x7957688e, version=1
#pragma link C++ class Belle2::MCTrajectoryPoint+; // checksum=0xc11b5e2b, version=-1
#pragma link C++ class Belle2::SimClockState+; // checksum=0x6a8a681e, version=2

// Schema evolution for Belle2::BeamBackHit.
#pragma read sourceClass="Belle2::BeamBackHit" version="[-1]" \
  source="TVector3 m_position" \
  targetClass="Belle2::BeamBackHit" target="m_positionX" \
  include="TVector3.h" \
  code="{m_positionX = onfile.m_position.X();}"

#pragma read sourceClass="Belle2::BeamBackHit" version="[-1]" \
  source="TVector3 m_position" \
  targetClass="Belle2::BeamBackHit" target="m_positionY" \
  include="TVector3.h" \
  code="{m_positionY = onfile.m_position.Y();}"

#pragma read sourceClass="Belle2::BeamBackHit" version="[-1]" \
  source="TVector3 m_position" \
  targetClass="Belle2::BeamBackHit" target="m_positionZ" \
  include="TVector3.h" \
  code="{m_positionZ = onfile.m_position.Z();}"

#pragma read sourceClass="Belle2::BeamBackHit" version="[-1]" \
  source="TVector3 m_momentum" \
  targetClass="Belle2::BeamBackHit" target="m_momentumX" \
  include="TVector3.h" \
  code="{m_momentumX = onfile.m_momentum.X();}"

#pragma read sourceClass="Belle2::BeamBackHit" version="[-1]" \
  source="TVector3 m_momentum" \
  targetClass="Belle2::BeamBackHit" target="m_momentumY" \
  include="TVector3.h" \
  code="{m_momentumY = onfile.m_momentum.Y();}"

#pragma read sourceClass="Belle2::BeamBackHit" version="[-1]" \
  source="TVector3 m_momentum" \
  targetClass="Belle2::BeamBackHit" target="m_momentumZ" \
  include="TVector3.h" \
  code="{m_momentumZ = onfile.m_momentum.Z();}"

#pragma read sourceClass="Belle2::BeamBackHit" version="[-1]" \
  source="double m_t" \
  targetClass="Belle2::BeamBackHit" target="m_t" \
  code="{m_t = onfile.m_t;}"

#pragma read sourceClass="Belle2::BeamBackHit" version="[-1]" \
  source="double m_E_start" \
  targetClass="Belle2::BeamBackHit" target="m_E_start" \
  code="{m_E_start = onfile.m_E_start;}"

#pragma read sourceClass="Belle2::BeamBackHit" version="[-1]" \
  source="double m_E_end" \
  targetClass="Belle2::BeamBackHit" target="m_E_end" \
  code="{m_E_end = onfile.m_E_end;}"

#pragma read sourceClass="Belle2::BeamBackHit" version="[-1]" \
  source="double m_energyDeposit" \
  targetClass="Belle2::BeamBackHit" target="m_energyDeposit" \
  code="{m_energyDeposit = onfile.m_energyDeposit;}"

#pragma read sourceClass="Belle2::BeamBackHit" version="[-1]" \
  source="double m_trackLength" \
  targetClass="Belle2::BeamBackHit" target="m_trackLength" \
  code="{m_trackLength = onfile.m_trackLength;}"

#pragma read sourceClass="Belle2::BeamBackHit" version="[-1]" \
  source="double m_neutronWeight" \
  targetClass="Belle2::BeamBackHit" target="m_neutronWeight" \
  code="{m_neutronWeight = onfile.m_neutronWeight;}"

#endif

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::SimHitBase+; // checksum=0xf68f360, version=3
#pragma link C++ class Belle2::BeamBackHit+; // checksum=0x4ba78b6a, version=1
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

#endif

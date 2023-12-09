#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::CsiSimHit+; // checksum=0xdc211959, version=2
#pragma link C++ class Belle2::CsiDigiHit+; // checksum=0x5418ac9e, version=1
#pragma link C++ class Belle2::CsiHit+; // checksum=0xb4d424f0, version=3
#pragma link C++ class Belle2::CsiHit_v2+; // checksum=0x3dface73, version=2

#pragma read                                                                                          \
  sourceClass="Belle2::CsiHit_v2"                                                                     \
  source="TVector3 m_Momentum"                                                                        \
  version="[1]"                                                                                       \
  targetClass="Belle2::CsiHit_v2"                                                                     \
  target="m_Momentum"                                                                                 \
  include="TVector3.h"                                                                                \
  code="{ m_Momentum.SetXYZ(onfile.m_Momentum.X(), onfile.m_Momentum.Y(), onfile.m_Momentum.Z()); }"  \

#pragma read                                                                                          \
  sourceClass="Belle2::CsiHit_v2"                                                                     \
  source="TVector3 m_Position"                                                                        \
  version="[1]"                                                                                       \
  targetClass="Belle2::CsiHit_v2"                                                                     \
  target="m_Position"                                                                                 \
  include="TVector3.h"                                                                                \
  code="{ m_Position.SetXYZ(onfile.m_Position.X(), onfile.m_Position.Y(), onfile.m_Position.Z()); }"  \

#pragma read                                                                                          \
  sourceClass="Belle2::CsiSimHit"                                                                     \
  source="TVector3 m_Momentum"                                                                        \
  version="[1]"                                                                                       \
  targetClass="Belle2::CsiSimHit"                                                                     \
  target="m_Momentum"                                                                                 \
  include="TVector3.h"                                                                                \
  code="{ m_Momentum.SetXYZ(onfile.m_Momentum.X(), onfile.m_Momentum.Y(), onfile.m_Momentum.Z()); }"  \

#pragma read                                                                                          \
  sourceClass="Belle2::CsiSimHit"                                                                     \
  source="TVector3 m_Position"                                                                        \
  version="[1]"                                                                                       \
  targetClass="Belle2::CsiSimHit"                                                                     \
  target="m_Position"                                                                                 \
  include="TVector3.h"                                                                                \
  code="{ m_Position.SetXYZ(onfile.m_Position.X(), onfile.m_Position.Y(), onfile.m_Position.Z()); }"  \

#endif

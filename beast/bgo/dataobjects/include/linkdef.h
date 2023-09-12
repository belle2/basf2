#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::BgoSimHit+; // checksum=0x7d02ed92, version=2
#pragma link C++ class Belle2::BgoHit+; // checksum=0x3eed7dc7, version=2

#pragma read                                                                                          \
  sourceClass="Belle2::BgoHit"                                                                        \
  source="TVector3 m_Momentum"                                                                        \
  version="[1]"                                                                                       \
  targetClass="Belle2::BgoHit"                                                                        \
  target="m_Momentum"                                                                                 \
  include="TVector3.h"                                                                                \
  code="{ m_Momentum.SetXYZ(onfile.m_Momentum.X(), onfile.m_Momentum.Y(), onfile.m_Momentum.Z()); }"  \

#pragma read                                                                                          \
  sourceClass="Belle2::BgoHit"                                                                        \
  source="TVector3 m_Position"                                                                        \
  version="[1]"                                                                                       \
  targetClass="Belle2::BgoHit"                                                                        \
  target="m_Position"                                                                                 \
  include="TVector3.h"                                                                                \
  code="{ m_Position.SetXYZ(onfile.m_Position.X(), onfile.m_Position.Y(), onfile.m_Position.Z()); }"  \

#pragma read                                                                                          \
  sourceClass="Belle2::BgoSimHit"                                                                     \
  source="TVector3 m_Momentum"                                                                        \
  version="[1]"                                                                                       \
  targetClass="Belle2::BgoSimHit"                                                                     \
  target="m_Momentum"                                                                                 \
  include="TVector3.h"                                                                                \
  code="{ m_Momentum.SetXYZ(onfile.m_Momentum.X(), onfile.m_Momentum.Y(), onfile.m_Momentum.Z()); }"  \

#pragma read                                                                                          \
  sourceClass="Belle2::BgoSimHit"                                                                     \
  source="TVector3 m_Position"                                                                        \
  version="[1]"                                                                                       \
  targetClass="Belle2::BgoSimHit"                                                                     \
  target="m_Position"                                                                                 \
  include="TVector3.h"                                                                                \
  code="{ m_Position.SetXYZ(onfile.m_Position.X(), onfile.m_Position.Y(), onfile.m_Position.Z()); }"  \

#endif

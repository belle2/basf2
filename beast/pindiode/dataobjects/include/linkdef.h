#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::PindiodeSimHit+; // checksum=0xc3c30fde, version=2
#pragma link C++ class Belle2::PindiodeHit+; // checksum=0x10c7f6cf, version=1

#pragma read                                                                                          \
  sourceClass="Belle2::PindiodeSimHit"                                                                \
  source="TVector3 m_Momentum"                                                                        \
  version="[1]"                                                                                       \
  targetClass="Belle2::PindiodeSimHit"                                                                \
  target="m_Momentum"                                                                                 \
  include="TVector3.h"                                                                                \
  code="{ m_Momentum.SetXYZ(onfile.m_Momentum.X(), onfile.m_Momentum.Y(), onfile.m_Momentum.Z()); }"  \

#pragma read                                                                                          \
  sourceClass="Belle2::PindiodeSimHit"                                                                \
  source="TVector3 m_Position"                                                                        \
  version="[1]"                                                                                       \
  targetClass="Belle2::PindiodeSimHit"                                                                \
  target="m_Position"                                                                                 \
  include="TVector3.h"                                                                                \
  code="{ m_Position.SetXYZ(onfile.m_Position.X(), onfile.m_Position.Y(), onfile.m_Position.Z()); }"  \

#endif

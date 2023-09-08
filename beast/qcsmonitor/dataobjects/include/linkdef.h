#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::QcsmonitorSimHit+; // checksum=0x8cc50f91, version=2
#pragma link C++ class Belle2::QcsmonitorHit+; // checksum=0x92a6cedc, version=1

#pragma read                                                                                          \
  sourceClass="Belle2::QcsmonitorSimHit"                                                              \
  source="TVector3 m_Momentum"                                                                        \
  version="[1]"                                                                                       \
  targetClass="Belle2::QcsmonitorSimHit"                                                              \
  target="m_Momentum"                                                                                 \
  include="TVector3.h"                                                                                \
  code="{ m_Momentum.SetXYZ(onfile.m_Momentum.X(), onfile.m_Momentum.Y(), onfile.m_Momentum.Z()); }"  \

#pragma read                                                                                          \
  sourceClass="Belle2::QcsmonitorSimHit"                                                              \
  source="TVector3 m_Position"                                                                        \
  version="[1]"                                                                                       \
  targetClass="Belle2::QcsmonitorSimHit"                                                              \
  target="m_Position"                                                                                 \
  include="TVector3.h"                                                                                \
  code="{ m_Position.SetXYZ(onfile.m_Position.X(), onfile.m_Position.Y(), onfile.m_Position.Z()); }"  \

#endif

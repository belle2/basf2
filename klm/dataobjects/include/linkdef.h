#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

/* Data objects. */
#pragma link C++ class Belle2::KLMChannelMapValue<unsigned int>+; // checksum=0xd9c081e5, version=1
#pragma link C++ class Belle2::KLMClusterShape+; // checksum=0xad147c40, version=1
#pragma link C++ class Belle2::KLMDigit+; // checksum=0xe6c810d6, version=1
#pragma link C++ class Belle2::KLMDigitEventInfo+; // checksum=0xda5c212, version=5
#pragma link C++ class Belle2::KLMDigitRaw+; // checksum=0x8fdcd43e, version=2
#pragma link C++ class Belle2::KLMHit2d+; // checksum=0x34ea493f, version=1
#pragma link C++ class Belle2::KLMMuidHit+; // checksum=0x3eccc36c, version=2
#pragma link C++ class Belle2::KLMMuidLikelihood+; // checksum=0x22d1ff03, version=4
#pragma link C++ class Belle2::KLMScintillatorFirmwareFitResult+; // checksum=0xb25e8edc, version=1
#pragma link C++ class Belle2::KLMSimHit+; // checksum=0xbffbfea5, version=1
#pragma link C++ class Belle2::KLMTrack+;

/* Element numbers and indices. */
#pragma link C++ class Belle2::KLMChannelArrayIndex-;
#pragma link C++ class Belle2::KLMChannelIndex-;
#pragma link C++ class Belle2::KLMElementArrayIndex-;
#pragma link C++ class Belle2::KLMElementNumbers-;
#pragma link C++ class Belle2::KLMModuleArrayIndex-;
#pragma link C++ class Belle2::KLMPlaneArrayIndex-;
#pragma link C++ class Belle2::KLMSectorArrayIndex-;

// KLMMuidHit evolution
// In version 2- (1),
//   o   m_ExtPosition is stored in a float[3] (TVector3)
//   o   m_HitPosition is stored in a float[3] (TVector3)
#pragma read \
  sourceClass="Belle2::KLMMuidHit" source="TVector3 m_ExtPosition" version="[-1]"\
  targetClass="Belle2::KLMMuidHit" target="m_ExtPosition" \
  include="TVector3.h" \
  code = "{ \
            m_ExtPosition[0] = onfile.m_ExtPosition.X(); \
            m_ExtPosition[1] = onfile.m_ExtPosition.Y(); \
            m_ExtPosition[2] = onfile.m_ExtPosition.Z(); \
          }"
#pragma read \
  sourceClass="Belle2::KLMMuidHit" source="TVector3 m_HitPosition" version="[-1]"\
  targetClass="Belle2::KLMMuidHit" target="m_HitPosition" \
  include="TVector3.h" \
  code = "{ \
            m_HitPosition[0] = onfile.m_HitPosition.X(); \
            m_HitPosition[1] = onfile.m_HitPosition.Y(); \
            m_HitPosition[2] = onfile.m_HitPosition.Z(); \
          }"

#endif

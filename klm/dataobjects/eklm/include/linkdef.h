#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

/* Base classes. */
#pragma link C++ class Belle2::EKLMHitBase+; // checksum=0xa22e1ba5, version=3
#pragma link C++ class Belle2::EKLMHitCoord+; // checksum=0xa9fe8765, version=1
#pragma link C++ class Belle2::EKLMHitGlobalCoord+; // checksum=0xbcfbd91a, version=1
#pragma link C++ class Belle2::EKLMHitMCTime+; // checksum=0x1f31e80, version=1
#pragma link C++ class Belle2::EKLMHitMomentum+; // checksum=0x8dd557d0, version=1

/* Geant stage. */
#pragma link C++ class Belle2::EKLMSimHit+; // checksum=0x2c81cf8d, version=2

/* Reconstuction stage. */
#pragma link C++ class Belle2::EKLMHit2d+; // checksum=0x7d6dc915, version=5

/* Alignment. */
#pragma link C++ class Belle2::EKLMAlignmentHit+; // checksum=0x418bf1dc, version=1

/* Element identifiers. */
#pragma link C++ class Belle2::EKLMElementNumbers+; // checksum=0xd194955b, version=3

#endif

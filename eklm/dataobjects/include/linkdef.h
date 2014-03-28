#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

/* Base classes. */
#pragma link C++ class Belle2::EKLMHitBase+;
#pragma link C++ class Belle2::EKLMHitCoord+;
#pragma link C++ class Belle2::EKLMHitGlobalCoord+;
#pragma link C++ class Belle2::EKLMHitMCTime+;
#pragma link C++ class Belle2::EKLMHitMomentum+;

/* Geant stage. */
#pragma link C++ class Belle2::EKLMSimHit+;

/* Digitization stage. */
#pragma link C++ class Belle2::EKLMSim2Hit+;
#pragma link C++ class Belle2::EKLMDigit+;

/* Reconstuction stage. */
#pragma link C++ class Belle2::EKLMSectorHit+;
#pragma link C++ class Belle2::EKLMHit2d+;
#pragma link C++ class Belle2::EKLMK0L+;

#endif

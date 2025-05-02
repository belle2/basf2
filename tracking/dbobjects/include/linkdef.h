#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::CKFParameters+; // checksum=0x8dcd40df, version=1
#pragma link C++ class Belle2::TrackFitMomentumRange+; // checksum=0x8f6de621, version=1
#pragma link C++ class Belle2::TrackFlippingCuts+; // checksum=0x8e010941, version=4
#pragma link C++ class Belle2::DAFparameters+; // checksum=0xe9a67894, version=2
#pragma link C++ class Belle2::DAFConfiguration+;
#pragma link C++ class Belle2::ROICalculationParameters+;
#pragma link C++ class Belle2::SVDEventT0Configuration+; // checksum=0x3782a1d9, version=1
#pragma link C++ class Belle2::CkfToPXDFiltersSetting+; // checksum=0x4f402468, version=1
#pragma link C++ class Belle2::KinkFinderParameters+; // checksum=0x6ba64774, version=1

// Tracking MVAFilter payloads
#pragma link C++ class Belle2::TrackingMVAFilterParameters+; // checksum=0xd0221a33, version=1

// ----------------------------------------------------------------------------
// DAFparameters evolution
#pragma read sourceClass="Belle2::DAFparameters" version="[1]" \
  source="" \
  targetClass="Belle2::DAFparameters" target="m_MinimumPValue" \
  code="{m_MinimumPValue = 0xFFFF;}"

#endif

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;

#pragma link C++ class Belle2::SVDCoolingPipesPar+; // checksum=0x38b12745, version=5
#pragma link C++ class Belle2::SVDEndringsPar+; // checksum=0xafc773ec, version=5

#pragma link C++ class Belle2::SVDSupportRibsPar+; // checksum=0xac03bec5, version=5
#pragma link C++ class Belle2::SVDSupportBoxPar+; // checksum=0x54f42d6e, implicit, version=5
#pragma link C++ class Belle2::SVDSupportTabPar+; // checksum=0x5b59e6f, implicit, version=5
#pragma link C++ class Belle2::SVDEndmountPar+; // checksum=0xa22920d8, implicit, version=5

#pragma link C++ class Belle2::SVDEndringsPar+; // checksum=0xafc773ec, version=5
#pragma link C++ class Belle2::SVDEndringsTypePar+; // checksum=0x44c02c58, implicit, version=5

#pragma link C++ class Belle2::SVDSensorInfoPar+; // checksum=0x751fabfb, version=7
#pragma link C++ class Belle2::SVDGeometryPar+; // checksum=0x2241adfe, version=5

#pragma link C++ class Belle2::SVDLocalRunBadStrips+; // checksum=0x1914065, version=1

#pragma link C++ class Belle2::SVDCalibrationsBase<Belle2::SVDCalibrationsVector<float>>+; // checksum=0x80db1433, version=2
#pragma link C++ class Belle2::SVDCalibrationsVector<float> +; // checksum=0x5cfaf009, version=-1

#pragma link C++ class Belle2::SVDCalibrationsBase<Belle2::SVDCalibrationsVector<Belle2::SVDStripCalAmp>>+; // checksum=0x8a2533e1, version=2
#pragma link C++ class Belle2::SVDCalibrationsVector<Belle2::SVDStripCalAmp> +; // checksum=0xfff37002, version=-1
#pragma link C++ struct Belle2::SVDStripCalAmp +;

#pragma link C++ class Belle2::SVDCalibrationsBase<Belle2::SVDCalibrationsScalar<Belle2::SVDChargeSimCal>>+; // checksum=0x98e032e5, version=2
#pragma link C++ class Belle2::SVDCalibrationsScalar<Belle2::SVDChargeSimCal> +; // checksum=0x8b93e80, version=-1
#pragma link C++ struct Belle2::SVDChargeSimCal +;

#pragma link C++ class Belle2::SVDCalibrationsBase<Belle2::SVDCalibrationsBitmap> +; // checksum=0x34de8474, version=2
#pragma link C++ class Belle2::SVDCalibrationsBitmap +; // checksum=0xf2ee9bed, version=-1

#pragma link C++ class Belle2::SVDCalibrationsBase<Belle2::SVDCalibrationsScalar< Belle2::SVDCoGCalibrationFunction >> +; // checksum=0xaa3ea535, version=2
#pragma link C++ class Belle2::SVDCalibrationsScalar< Belle2::SVDCoGCalibrationFunction> +; // checksum=0x4c4e6ba4, version=-1
#pragma link C++ class Belle2::SVDCoGCalibrationFunction +; // checksum=0xb36585eb, version=6

#pragma link C++ struct Belle2::SVDStripCalAmp +;

#pragma link C++ class Belle2::SVDCalibrationsBase< Belle2::SVDCalibrationsScalar< Belle2::SVDPosErrScaleFactors>> +; // checksum=0x15aff4f5, version=2
#pragma link C++ class Belle2::SVDCalibrationsScalar< Belle2::SVDPosErrScaleFactors> +; // checksum=0x4599fb4c, version=-1
#pragma link C++ struct Belle2::SVDPosErrScaleFactors +;

#pragma link C++ class Belle2::SVDCalibrationsBase<Belle2::SVDCalibrationsScalar< Belle2::SVDPositionErrorFunction >> +; // checksum=0xa9702b4f, version=2
#pragma link C++ class Belle2::SVDCalibrationsScalar< Belle2::SVDPositionErrorFunction> +; // checksum=0xa7c28262, version=-1
#pragma link C++ class Belle2::SVDPositionErrorFunction +; // checksum=0x8ef416ec, version=2

#pragma link C++ class Belle2::SVDCalibrationsBase< Belle2::SVDCalibrationsScalar< Belle2::SVDClusterCuts>> +; // checksum=0x49d91267, version=2
#pragma link C++ class Belle2::SVDCalibrationsScalar< Belle2::SVDClusterCuts> +; // checksum=0x2f2a326, version=-1
#pragma link C++ struct Belle2::SVDClusterCuts +;

#pragma link C++ class Belle2::SVDCalibrationsBase< Belle2::SVDCalibrationsScalar< Belle2::SVDHitTimeSelectionFunction>> +; // checksum=0xd473b9ef, version=2
#pragma link C++ class Belle2::SVDCalibrationsScalar< Belle2::SVDHitTimeSelectionFunction> +; // checksum=0xfe6bb093, version=-1
#pragma link C++ class Belle2::SVDHitTimeSelectionFunction +; // checksum=0x996882b0, version=3

#pragma link C++ class Belle2::SVDLocalConfigParameters +; // checksum=0x56a63232, version=1
#pragma link C++ class Belle2::SVDGlobalConfigParameters +; // , checksum=0x1694a3c, version=3

#pragma link C++ class Belle2::SVDRecoConfiguration +; // checksum=0x5da73de6, version=1

#endif


// ----------------------------------------------------------------------------
// SVDGlobalConfigParameters
// As of version 2, a new data member is added
// classDef = 1 -> classDef = 2
// x ->  float m_hv
// when classDef = 1 m_hv = 50
#pragma read \
  sourceClass="Belle2::SVDGlobalConfigParameters"\
  source=""\
  version="[-1]" \
  targetClass="Belle2::SVDGlobalConfigParameters"\
  target="m_hv"          \
  code="{ m_hv = 50;}" \

// SVDGlobalConfigParameters
// As of version 3, a new data member is added
// classDef = 2 -> classDef = 3
// x ->  float m_relativeTimeShift
// when classDef = 1,2 m_relativeTimeShift = 0
#pragma read \
  sourceClass="Belle2::SVDGlobalConfigParameters"\
  source=""\
  version="[-2]" \
  targetClass="Belle2::SVDGlobalConfigParameters"\
  target="m_relativeTimeShift"          \
  code="{ m_relativeTimeShift = 0;}" \

// SVDGlobalConfigParameters
// As of version 3, a new data member is added
// classDef = 2 -> classDef = 3
// x ->  int m_nrFrames
// when classDef = 1,2 m_nrFrames = 6
#pragma read \
  sourceClass="Belle2::SVDGlobalConfigParameters"\
  source=""\
  version="[-2]" \
  targetClass="Belle2::SVDGlobalConfigParameters"\
  target="m_nrFrames"          \
  code="{ m_nrFrames = 6;}" \

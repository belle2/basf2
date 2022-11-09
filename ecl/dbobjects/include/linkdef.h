#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::ECLBeamBackgroundStudy+; // checksum=0xd15f8a0a, version=1
#pragma link C++ class Belle2::ECLCrystalsShapeAndPosition+; // checksum=0xb2c3c34, version=1
#pragma link C++ class Belle2::ECLDigitTimeConstants+; // checksum=0x80f96494, version=2
#pragma link C++ class Belle2::ECLDigitEnergyConstants+; // checksum=0x8e27b399, version=2
#pragma link C++ class Belle2::ECLDatabaseImporter-;
#pragma link C++ class Belle2::ECLShowerShapeSecondMomentCorrection; // checksum=0xbee9bee0, version=1
#pragma link C++ class Belle2::ECLShowerCorrectorLeakageCorrection; // checksum=0x3397034a, version=1
#pragma link C++ class Belle2::ECLShowerEnergyCorrectionTemporary; // checksum=0xcaa054cf, version=1
#pragma link C++ class Belle2::ECLCrystalCalib; // checksum=0x44484113, version=1
#pragma link C++ class Belle2::ECLDigitWaveformParameters; // checksum=0x1b4bedbe, version=2
#pragma link C++ class Belle2::ECLDigitWaveformParametersForMC; // checksum=0xc16a3107, version=1
#pragma link C++ class Belle2::ECLChargedPidPDFs+; // checksum=0x1bd4d566, version=2
#pragma link C++ class Belle2::ECLReferenceCrystalPerCrateCalib+; // checksum=0x5c126a47, version=3
#pragma link C++ enum Belle2::ECLChargedPidPDFs::InputVar+; //implicit
#pragma link C++ class Belle2::ECLChargedPidPDFs::VarTransfoSettings+; // checksum=0x1a8bc0c9, implicit, version=-1
#pragma link C++ class Belle2::ECLChargedPIDPhasespaceBinning+; // checksum=0x65ede86a, implicit, version=1
#pragma link C++ class Belle2::ECLChargedPIDPhasespaceCategory+; // checksum=0xc3de5acc, implicit, version=1
#pragma link C++ class Belle2::ECLChargedPIDMVAWeights+; // , checksum=0xcb04e543, version=1
#pragma link C++ enum Belle2::ECLChargedPIDPhasespaceCategory::MVAResponseTransformMode+; //implicit
#pragma link C++ class Belle2::PackedAutoCovariance; // checksum=0xb1a29561, implicit, version=1
#pragma link C++ class Belle2::ECLAutoCovariance; // checksum=0x1f75052d, version=1
#pragma link C++ class Belle2::ECLTrackClusterMatchingThresholds; // checksum=0xdc6f9453, version=1
#pragma link C++ class Belle2::ECLTrackClusterMatchingParameterizations+; // checksum=0xcb5d30c4, version=1
#pragma link C++ class Belle2::ECLHadronComponentEmissionFunction; // checksum=0x94f5457e, version=1
#pragma link C++ class Belle2::ECLDspData; // checksum=0x83a051b9, version=1
#pragma link C++ class Belle2::ECLChannelMap+; // checksum=0x31b65fac, version=1
#pragma link C++ class Belle2::ECLCrystalLocalRunCalib; // checksum=0xf7c81e44, version=1
#pragma link C++ class Belle2::ECLLocalRunCalibRef; // checksum=0x37dc8c50, version=1
#pragma link C++ class Belle2::ECLWaveformData+; // checksum=0x82c9e7d2, version=1
#pragma link C++ class Belle2::ECLWFAlgoParams+; // checksum=0x731094cf, implicit, version=1
#pragma link C++ class Belle2::ECLNoiseData+; // checksum=0xf538d169, implicit, version=1
#pragma link C++ class Belle2::ECLLookupTable+; // checksum=0xe3749e00, implicit, version=1
#pragma link C++ class Belle2::ECLLeakageCorrections; // checksum=0x262d45e4, version=2
#pragma link C++ class Belle2::ECLnOptimal+; // checksum=0xe49fab6f, version=1

// workaround for https://github.com/root-project/root/issues/7960
#pragma link C++ class std::pair<int,std::unordered_map<int,vector<Belle2::ECLChargedPidPDFs::InputVar>>>+; // checksum=0xacd09862, version=-1
#pragma link C++ class std::pair<int,std::vector<Belle2::ECLChargedPidPDFs::InputVar>>+; // checksum=0x537c79aa, version=-1
#pragma link C++ class std::pair<int,std::unordered_map<int,unordered_map<Belle2::ECLChargedPidPDFs::InputVar,TF1*>>>+; // checksum=0x516ef286, version=-1
#pragma link C++ class std::pair<int,std::unordered_map<Belle2::ECLChargedPidPDFs::InputVar,TF1*>>+; // checksum=0xb526ce3e, version=-1
#pragma link C++ class std::pair<Belle2::ECLChargedPidPDFs::InputVar,TF1*>+; // checksum=0x610aceb, version=-1
#pragma link C++ class std::__pair_base<int,std::unordered_map<int,vector<Belle2::ECLChargedPidPDFs::InputVar>>>+; // checksum=0x771547f5, version=-1
#pragma link C++ class std::__pair_base<int,std::vector<Belle2::ECLChargedPidPDFs::InputVar>>+; // checksum=0x5f8f36f7, version=-1
#pragma link C++ class std::__pair_base<int,std::unordered_map<int,unordered_map<Belle2::ECLChargedPidPDFs::InputVar,TF1*>>>+; // checksum=0xe95f49d8, version=-1
#pragma link C++ class std::__pair_base<int,std::unordered_map<Belle2::ECLChargedPidPDFs::InputVar,TF1*>>+; // checksum=0x7c760302, version=-1
#pragma link C++ class std::__pair_base<Belle2::ECLChargedPidPDFs::InputVar,TF1*>+; // checksum=0x1b51fbdc, version=-1

#endif

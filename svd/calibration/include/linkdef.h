#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

// The following two are needed for the python interface
//calibrations wrappers
#pragma link C++ class Belle2::SVDCrossTalkStripsCalibrations-; // checksum=0x6bd85b66, version=-1
#pragma link C++ class Belle2::SVDOccupancyCalibrations-; // checksum=0xf86a6c36, version=-1
#pragma link C++ class Belle2::SVDCoGTimeCalibrations-; // checksum=0x3c67791c, version=-1
#pragma link C++ class Belle2::SVD3SampleCoGTimeCalibrations-; // checksum=0x8950f4e1, version=-1
#pragma link C++ class Belle2::SVD3SampleELSTimeCalibrations-; // checksum=0x371ee746, version=-1
#pragma link C++ class Belle2::SVDHitTimeSelection-;
#pragma link C++ class Belle2::SVDClustering-;
#pragma link C++ class Belle2::SVDCoGOnlyErrorScaleFactors-;
#pragma link C++ class Belle2::SVDOldDefaultErrorScaleFactors-;
#pragma link C++ class Belle2::SVDNoiseCalibrations-; // checksum=0xe1bbb6f3, version=-1
#pragma link C++ class Belle2::SVDPulseShapeCalibrations-; // checksum=0x6bdaa4b, version=-1
#pragma link C++ class Belle2::SVDPedestalCalibrations-; // checksum=0xbb219a89, version=-1
#pragma link C++ class Belle2::SVDFADCMaskedStrips-; // checksum=0x5c3369a8, version=-1
#pragma link C++ class Belle2::SVDHotStripsCalibrations-; // checksum=0x94dfda0f, version=-1
#pragma link C++ class Belle2::SVDDetectorConfiguration-; // checksum=0x8ad578cb, version=-1
#pragma link C++ class Belle2::SVDChargeSimulationCalibrations-;

//calibration importers
#pragma link C++ class Belle2::SVDDatabaseImporter-;
#pragma link C++ class Belle2::SVDLocalCalibrationsImporter-;
#pragma link C++ class Belle2::SVDDetectorConfigurationImporter-;

//calibration algorithms
#pragma link C++ class Belle2::SVDCoGTimeCalibrationAlgorithm-; // checksum=0x249f61df, version=-1
#pragma link C++ class Belle2::SVD3SampleCoGTimeCalibrationAlgorithm-; // checksum=0x74ca57c3, version=-1
#pragma link C++ class Belle2::SVD3SampleELSTimeCalibrationAlgorithm-; // checksum=0x60f7430, version=-1
#pragma link C++ class Belle2::SVDOccupancyCalibrationsAlgorithm-; // checksum=0xd1c55b35, version=-1
#pragma link C++ class Belle2::SVDHotStripsCalibrationsAlgorithm-; // checksum=0x3e0c24c4, version=-1
#pragma link C++ class Belle2::SVDCrossTalkCalibrationsAlgorithm-; // checksum=0x71ffb51b, version=-1
#pragma link C++ class Belle2::SVDTimeValidationAlgorithm-;

#endif

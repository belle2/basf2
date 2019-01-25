#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

// The following two are needed for the python interface
#pragma link C++ class Belle2::SVDDatabaseImporter+;
#pragma link C++ class Belle2::SVDCoGTimeCalibrations+;
#pragma link C++ class Belle2::SVDClusterCalibrations+;
#pragma link C++ class Belle2::SVDNoiseCalibrations;
#pragma link C++ class Belle2::SVDPulseShapeCalibrations;
#pragma link C++ class Belle2::SVDPedestalCalibrations;
#pragma link C++ class Belle2::SVDFADCMaskedStrips;
#pragma link C++ class Belle2::SVDHotStripsCalibrations;
#pragma link C++ class Belle2::SVDLocalCalibrationsImporter+;

#endif

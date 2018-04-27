#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;

#pragma link C++ class Belle2::SVDCoolingPipesPar+;
#pragma link C++ class Belle2::SVDEndringsPar+;
#pragma link C++ class Belle2::SVDEndringsLayerPar+; // implicit

#pragma link C++ class Belle2::SVDSupportRibsPar+;
#pragma link C++ class Belle2::SVDSupportBoxPar+; // implicit
#pragma link C++ class Belle2::SVDSupportTabPar+; // implicit
#pragma link C++ class Belle2::SVDEndmountPar+; // implicit

#pragma link C++ class Belle2::SVDEndringsPar+;
#pragma link C++ class Belle2::SVDEndringsTypePar+; // implicit

#pragma link C++ class Belle2::SVDSensorInfoPar+;
#pragma link C++ class Belle2::SVDGeometryPar+;

#pragma link C++ class Belle2::SVDLocalRunBadStrips+;

#pragma link C++ class Belle2::SVDCalibrationsBase<Belle2::SVDCalibrationsVector<float>>+;
#pragma link C++ class Belle2::SVDCalibrationsBase<Belle2::SVDCalibrationsVector<Belle2::SVDStripCalAmp>>+;
#pragma link C++ class Belle2::SVDCalibrationsVector<Belle2::SVDStripCalAmp> +;
#pragma link C++ class Belle2::SVDCalibrationsVector<float> +;

#pragma link C++ class Belle2::SVDCalibrationsBase<Belle2::SVDCalibrationsScalar< Belle2::SVDTriggerBinDependentConstants >>+;
#pragma link C++ class Belle2::SVDCalibrationsScalar< Belle2::SVDTriggerBinDependentConstants> +;

#pragma link C++ class Belle2::SVDCalibrationsBase<Belle2::SVDCalibrationsBitmap> +;
#pragma link C++ class Belle2::SVDCalibrationsBitmap +;


#pragma link C++ class Belle2::SVDCalibrationsBase<Belle2::SVDCalibrationsScalar< Belle2::SVDCoGCalibrationFunction >> +;
#pragma link C++ class Belle2::SVDCalibrationsScalar< Belle2::SVDCoGCalibrationFunction> +;
#pragma link C++ class Belle2::SVDCoGCalibrationFunction +;

#pragma link C++ struct Belle2::SVDStripCalAmp +;
#pragma link C++ struct Belle2::SVDTriggerBinDependentConstants +;
#endif

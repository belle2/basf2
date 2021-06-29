#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::KLMAlignmentData+; // checksum=0xe7de8ba2, version=1
#pragma link C++ class Belle2::KLMChannelStatus+; // checksum=0x2477d125, version=1
#pragma link C++ class Belle2::KLMElectronicsChannel+; // checksum=0xd01960ae, version=1
#pragma link C++ class Belle2::KLMElectronicsMap+; // checksum=0x80823ee7, version=1
#pragma link C++ class Belle2::KLMLikelihoodParameters+; // checksum=0xb0134418, version=1
#pragma link C++ class Belle2::KLMScintillatorDigitizationParameters+; // checksum=0x83151ce0, version=2
#pragma link C++ class Belle2::KLMScintillatorFEEData+; // checksum=0xa1527940, version=1
#pragma link C++ class Belle2::KLMScintillatorFEEParameters+; // checksum=0xae2d1dcf, version=1
#pragma link C++ class Belle2::KLMStripEfficiency+; // checksum=0xbe1e8e53, version=1
#pragma link C++ class Belle2::KLMTimeCableDelay+; // checksum=0xdaabeabe, version=1
#pragma link C++ class Belle2::KLMTimeConstants+; // checksum=0x771043a0, version=1
#pragma link C++ class Belle2::KLMTimeConversion+; // checksum=0x411acf7f, version=2
#pragma link C++ class Belle2::KLMTimeWindow+; // checksum=0x546af69b, version=1

// Evolution of KLMScintillatorDigitizationParameters.
#pragma read \
  sourceClass="Belle2::KLMScintillatorDigitizationParameters" source="double m_ADCSamplingTime" version="[-1]"\
  targetClass="Belle2::KLMScintillatorDigitizationParameters" target="m_ADCSamplingTDCPeriods" \
  code = "{ m_ADCSamplingTDCPeriods = int(onfile.m_ADCSamplingTime * 1.017728000); }"


#endif

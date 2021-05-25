#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::TOPSimHit+; // checksum=0x9bc05baa, version=3
#pragma link C++ class Belle2::TOPSimPhoton+; // checksum=0x96ff89f7, version=2
#pragma link C++ class Belle2::TOPSimCalPulse+; // checksum=0xa0d37252, version=1
#pragma link C++ class Belle2::TOPDigit+; // checksum=0x43b82094, version=15
#pragma link C++ class Belle2::TOPBarHit+; // checksum=0x88bd7661, version=3
#pragma link C++ class Belle2::TOPLikelihood+; // checksum=0xd7c618b7, version=2
#pragma link C++ class Belle2::TOPRecBunch+; // checksum=0xa16167cd, version=3
#pragma link C++ class Belle2::TOPRawWaveform+; // checksum=0x581d60d9, version=9
#pragma link C++ class Belle2::TOPProductionEventDebug+; // checksum=0xfbeb4b5d, version=1
#pragma link C++ class Belle2::TOPProductionHitDebug+; // checksum=0x32688835, version=1
#pragma link C++ class Belle2::TOPTimeZero+; // checksum=0xb02c2f5a, version=3
#pragma link C++ class Belle2::TOPRawDigit+; // checksum=0xd13d975f, version=6
#pragma link C++ class Belle2::TOPSlowData+; // checksum=0x879d972e, version=1
#pragma link C++ class Belle2::TOPInterimFEInfo+; // checksum=0x740da743, version=1
#pragma link C++ class Belle2::TOPTemplateFitResult+; // checksum=0xcca7a717, version=2
#pragma link C++ class Belle2::TOPTriggerDigit+; // checksum=0x1b8ad25b, version=1
#pragma link C++ class Belle2::TOPTriggerMCInfo+; // checksum=0x75ce1ae2, version=1
#pragma link C++ class Belle2::TOPPull+; // checksum=0x6aabd603, version=2
#pragma link C++ class Belle2::TOPPDFCollection+; // checksum=0xa97488a7, version=3
#pragma link C++ class Belle2::TOPPixelLikelihood+; // checksum=0x3ac6e98b, version=3
#pragma link C++ class Belle2::TOPAsicMask+; // checksum=0xacc40676, version=1
#pragma link C++ class Belle2::TOPAssociatedPDF+; // checksum=0xc79734f7, version=2

// schema evolution rules to allow reading of old class versions

#pragma read sourceClass="Belle2::TOPSimHit" version="[-2]" \
  source="int m_barID" \
  targetClass="Belle2::TOPSimHit" target="m_moduleID" \
  code="{m_moduleID = onfile.m_barID;}"

#pragma read sourceClass="Belle2::TOPBarHit" version="[1]" \
  source="int m_barID" \
  targetClass="Belle2::TOPBarHit" target="m_moduleID" \
  code="{m_moduleID = onfile.m_barID;}"

#pragma read sourceClass="Belle2::TOPDigit" version="[-12]" \
  source="int m_TDC" \
  targetClass="Belle2::TOPDigit" \
  target="m_rawTime" \
  code="{m_rawTime = onfile.m_TDC / 16.0;}"
#pragma read sourceClass="Belle2::TOPDigit" version="[-12]" \
  source="int m_ADC" \
  targetClass="Belle2::TOPDigit" \
  target="m_pulseHeight" \
  code="{m_pulseHeight = onfile.m_ADC;}"

#pragma read sourceClass="Belle2::TOPDigit" version="[-6]"  \
  source="int m_barID" \
  targetClass="Belle2::TOPDigit" \
  target="m_moduleID" \
  code="{m_moduleID = onfile.m_barID;}"
#pragma read sourceClass="Belle2::TOPDigit" version="[-6]" \
  source="int m_channelID" \
  targetClass="Belle2::TOPDigit" \
  target="m_pixelID" \
  code="{m_pixelID = onfile.m_channelID;}"
#pragma read sourceClass="Belle2::TOPDigit" version="[-6]" \
  source="unsigned m_hardChannelID" \
  targetClass="Belle2::TOPDigit" \
  target="m_channel" \
  code="{m_channel = onfile.m_hardChannelID;}"


#endif

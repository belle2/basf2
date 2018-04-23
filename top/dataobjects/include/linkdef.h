#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::TOPSimHit+;
#pragma link C++ class Belle2::TOPSimPhoton+;
#pragma link C++ class Belle2::TOPSimCalPulse+;
#pragma link C++ class Belle2::TOPDigit+;
#pragma link C++ class Belle2::TOPBarHit+;
#pragma link C++ class Belle2::TOPLikelihood+;
#pragma link C++ class Belle2::TOPRecBunch+;
#pragma link C++ class Belle2::TOPRawWaveform+;
#pragma link C++ class Belle2::TOPProductionEventDebug+;
#pragma link C++ class Belle2::TOPProductionHitDebug+;
#pragma link C++ class Belle2::TOPPull+;
#pragma link C++ class Belle2::TOPTimeZero+;
#pragma link C++ class Belle2::TOPRawDigit+;
#pragma link C++ class Belle2::TOPSlowData+;
#pragma link C++ class Belle2::TOPInterimFEInfo+;
#pragma link C++ class Belle2::TOPTemplateFitResult+;
#pragma link C++ class Belle2::TOPTriggerDigit+;
#pragma link C++ class Belle2::TOPTriggerMCInfo+;
#pragma link C++ class Belle2::TOPPDFCollection+;

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

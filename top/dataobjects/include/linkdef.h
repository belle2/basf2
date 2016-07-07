#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::TOPSimHit+;
#pragma link C++ class Belle2::TOPSimPhoton+;
#pragma link C++ class Belle2::TOPDigit+;
#pragma link C++ class Belle2::TOPBarHit+;
#pragma link C++ class Belle2::TOPLikelihood+;
#pragma link C++ class Belle2::TOPRecBunch+;
#pragma link C++ class Belle2::TOPRawWaveform+;
#pragma link C++ class Belle2::TOPWaveform+;
#pragma link C++ class Belle2::TOPPull+;
#pragma link C++ class Belle2::TOPTimeZero+;
#pragma link C++ class Belle2::TOPRawDigit+;
#pragma link C++ class Belle2::TOPSlowData+;

// schema evolution rules to allow reading of old class versions

#pragma read sourceClass="Belle2::TOPSimHit" version="[-2]" \
  source="int m_barID" \
  targetClass="Belle2::TOPSimHit" target="m_moduleID" \
  code="{m_moduleID = onfile.m_barID;}"

#pragma read sourceClass="Belle2::TOPBarHit" version="[1]" \
  source="int m_barID" \
  targetClass="Belle2::TOPBarHit" target="m_moduleID" \
  code="{m_moduleID = onfile.m_barID;}"

#pragma read sourceClass="Belle2::TOPDigit" version="[-6]" \
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

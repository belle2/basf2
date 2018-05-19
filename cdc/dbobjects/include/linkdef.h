#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::CDCChannelMap+;
#pragma link C++ class Belle2::CDCTimeZeros+;
#pragma link C++ class Belle2::CDCBadWires+;
#pragma link C++ class Belle2::CDCPropSpeeds+;
#pragma link C++ class Belle2::CDCTimeWalks+;
#pragma link C++ class Belle2::CDCXtRelations+;
#pragma link C++ class Belle2::CDCSpaceResols+;
#pragma link C++ class Belle2::CDCDisplacement+;
#pragma link C++ class Belle2::CDCAlignment+;
#pragma link C++ class Belle2::CDCLayerAlignment+;
#pragma link C++ class Belle2::CDCMisalignment+;
#pragma link C++ class Belle2::CDCGeometry+;
#pragma link C++ class Belle2::CDCGeometry::Rib+;
#pragma link C++ class Belle2::CDCGeometry::Rib2+;
#pragma link C++ class Belle2::CDCGeometry::Rib3+;
#pragma link C++ class Belle2::CDCGeometry::Rib4+;
#pragma link C++ class Belle2::CDCGeometry::Cover+;
#pragma link C++ class Belle2::CDCGeometry::Cover2+;
#pragma link C++ class Belle2::CDCGeometry::NeutronShield+;
#pragma link C++ class Belle2::CDCGeometry::Frontend+;
#pragma link C++ class Belle2::CDCGeometry::MotherVolume+;
#pragma link C++ class Belle2::CDCGeometry::EndPlate+;
#pragma link C++ class Belle2::CDCGeometry::EndPlateLayer+;
#pragma link C++ class Belle2::CDCGeometry::InnerWall+;
#pragma link C++ class Belle2::CDCGeometry::OuterWall+;
#pragma link C++ class Belle2::CDCGeometry::SenseLayer+;
#pragma link C++ class Belle2::CDCGeometry::FieldLayer+;
#pragma link C++ class Belle2::CDCTriggerPlane+;
#pragma link C++ class Belle2::CDCADCDeltaPedestals+;
#pragma link C++ class Belle2::CDCFEElectronics+;
#pragma link C++ class Belle2::CDCEDepToADCConversions+;

#pragma link C++ class std::map <unsigned short, float>+;
#pragma link C++ class std::vector<float>+;
#pragma link C++ class std::map <unsigned short, std::vector<float>>+;
#pragma link C++ class std::pair<unsigned short, std::vector<float>>+;

#pragma read sourceClass="Belle2::CDCTimeWalks" version="[-1]" \
  source="std::map<unsigned short, float> m_tws" \
  targetClass="Belle2::CDCTimeWalks" \
  target="m_tws" \
  code="{ \
    std::vector<float> buf(1);			  \
    std::map<unsigned short, float>::iterator it;		      \
    for (it = onfile.m_tws.begin(); it != onfile.m_tws.end(); ++it) { \
      buf[0] = it->second; \
      m_tws.insert(std::pair<unsigned short, std::vector<float>>(it->first, buf)); \
    }\
  }"
#endif

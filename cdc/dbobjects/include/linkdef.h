#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::CDCChannelMap+; // checksum=0xadcbec5a, version=2
#pragma link C++ class Belle2::CDCTimeZeros+; // checksum=0x1d1c902a, version=2
#pragma link C++ class Belle2::CDCBadWires+; // checksum=0xa5fc544e, version=2
#pragma link C++ class Belle2::CDCPropSpeeds+; // checksum=0x24873426, version=1
#pragma link C++ class Belle2::CDCTimeWalks+; // checksum=0xa0bbe541, version=2
#pragma link C++ class Belle2::CDCXtRelations+; // checksum=0x4c48166, version=2
#pragma link C++ class Belle2::CDCSpaceResols+; // checksum=0xbbc7719, version=2
#pragma link C++ class Belle2::CDCDisplacement+; // checksum=0xf22e0113, version=4
#pragma link C++ class Belle2::CDCAlignment+; // checksum=0x93495a07, version=2
#pragma link C++ class Belle2::CDCLayerAlignment+; // checksum=0x84c87874, version=1
#pragma link C++ class Belle2::CDCMisalignment+; // checksum=0x67b5fdae, version=1
#pragma link C++ class Belle2::CDCGeometry+; // checksum=0xb2234303, version=5
#pragma link C++ class Belle2::CDCGeometry::Rib+; // checksum=0x4ba6e828, version=2
#pragma link C++ class Belle2::CDCGeometry::Rib2+; // checksum=0xbfd84675, version=1
#pragma link C++ class Belle2::CDCGeometry::Rib3+; // checksum=0xf00885b5, version=1
#pragma link C++ class Belle2::CDCGeometry::Rib4+; // checksum=0x37fac479, version=1
#pragma link C++ class Belle2::CDCGeometry::Rib5+; // checksum=0xaa54c218, version=1
#pragma link C++ class Belle2::CDCGeometry::Cover+; // checksum=0xa036546c, version=1
#pragma link C++ class Belle2::CDCGeometry::Cover2+; // checksum=0xfacdecfb, version=1
#pragma link C++ class Belle2::CDCGeometry::NeutronShield+; // checksum=0x6ebcef33, version=1
#pragma link C++ class Belle2::CDCGeometry::Frontend+; // checksum=0xf32d61e2, version=1
#pragma link C++ class Belle2::CDCGeometry::MotherVolume+; // checksum=0xdee26a1b, version=1
#pragma link C++ class Belle2::CDCGeometry::EndPlate+; // checksum=0x9f99f74f, version=1
#pragma link C++ class Belle2::CDCGeometry::EndPlateLayer+; // checksum=0x9994436e, version=1
#pragma link C++ class Belle2::CDCGeometry::InnerWall+; // checksum=0x29edf796, version=1
#pragma link C++ class Belle2::CDCGeometry::OuterWall+; // checksum=0xb1927149, version=1
#pragma link C++ class Belle2::CDCGeometry::SenseLayer+; // checksum=0xc500e4df, version=1
#pragma link C++ class Belle2::CDCGeometry::FieldLayer+; // checksum=0x4f3b3d3c, version=1
#pragma link C++ class Belle2::CDCTriggerPlane+; // checksum=0x7cd1e0e8, version=2
#pragma link C++ class Belle2::CDCADCDeltaPedestals+; // checksum=0x4f3c632d, version=1
#pragma link C++ class Belle2::CDCFEElectronics+; // checksum=0x14613816, version=2
#pragma link C++ class Belle2::CDCEDepToADCConversions+; // checksum=0x3d6b427c, version=3
#pragma link C++ class Belle2::CDCWireHitRequirements+; // checksum=0x4c504d88, version=2
#pragma link C++ class Belle2::CDCCrossTalkLibrary+; // checksum=0x4ace6024, version=2
#pragma link C++ class Belle2::CDCCorrToThresholds+; // checksum=0xbea4131b, version=2
#pragma link C++ class Belle2::CDCFudgeFactorsForSigma+; // checksum=0x37bcdd67, version=1
#pragma link C++ class Belle2::CDCdEdxPDFs+; // checksum=0xc074ee36, version=1

#pragma link C++ class Belle2::asicChannel+; // checksum=0x7a22a583, implicit, version=-1
#pragma link C++ class Belle2::adcAsicTuple+; // checksum=0x953c3230, implicit, version=-1
#pragma link C++ class Belle2::CDClayerTimeCut+; // checksum=0x6821631B, version=1

#pragma link C++ class std::map <unsigned short, float>+; // checksum=0x868d8139, version=6
#pragma link C++ class std::vector<float>+; // checksum=0x55a169b, version=6
#pragma link C++ class std::map <unsigned short, std::vector<float>>+; // checksum=0x8866143b, version=6
#pragma link C++ class std::pair<unsigned short, std::vector<float>>+; // checksum=0xfca0f10e, version=-1

#pragma link C++ class std::vector<Belle2::adcAsicTuple>+; // checksum=0x809077b9, version=6

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

#pragma read sourceClass="Belle2::CDCBadWires" version="[-1]" \
  source="std::vector<unsigned short> m_wires" \
  targetClass="Belle2::CDCBadWires" \
  target="m_wires" \
  code="{ \
    float effi = 0.; \
    for (int i=0; i < onfile.m_wires.size(); ++i) { \
      m_wires.insert(std::pair<unsigned short,float>(onfile.m_wires[i], effi)); \
    }\
  }"

#pragma read sourceClass="Belle2::CDCSpaceResols" version="[-1]" \
  source="unsigned short m_sigmaParamMode" \
  targetClass="Belle2::CDCSpaceResols" \
  target="m_maxSpaceResol" \
  code="{ \
    /* set 325um for an old object */ \
    m_maxSpaceResol = 0.0325; \
  }"


// CDCDisplacement evolution
// In version 4- (3)
//   o   m_fwd is of type ROOT::Math::XYZVector (was TVector3)
#pragma read \
  sourceClass="Belle2::CDCDisplacement" source="TVector3 m_fwd" version="[-3]" \
  targetClass="Belle2::CDCDisplacement" target="m_fwd" \
  include="TVector3.h" \
  code = "{ \
            m_fwd.SetXYZ(onfile.m_fwd.X(), \
                         onfile.m_fwd.Y(), \
                         onfile.m_fwd.Z()); \
          }"

// CDCDisplacement evolution
// In version 4- (3)
//   o   m_bwd is of type ROOT::Math::XYZVector (was TVector3)
#pragma read \
  sourceClass="Belle2::CDCDisplacement" source="TVector3 m_bwd" version="[-3]" \
  targetClass="Belle2::CDCDisplacement" target="m_bwd" \
  include="TVector3.h" \
  code = "{ \
            m_bwd.SetXYZ(onfile.m_bwd.X(), \
                         onfile.m_bwd.Y(), \
                         onfile.m_bwd.Z()); \
          }"

#endif

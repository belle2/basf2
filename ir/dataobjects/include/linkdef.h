#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::IRSimHit+; // checksum=0x676f71b9, version=2

// IRSimHit evolution
// In version 2- (1),
//   o   m_posIn is stored in a ROOT::Math::XYZVector (TVector3)
//   o   m_momIn is stored in a ROOT::Math::XYZVector (TVector3)
//   o   m_posOut is stored in a ROOT::Math::XYZVector (TVector3)
//   o   m_momOut is stored in a ROOT::Math::XYZVector (TVector3)
#pragma read \
  sourceClass="Belle2::IRSimHit" source="TVector3 m_posIn" version="[-1]"\
  targetClass="Belle2::IRSimHit" target="m_posIn" \
  include="TVector3.h" \
  code = "{ \
            m_posIn.SetXYZ(onfile.m_posIn.X(), onfile.m_posIn.Y(), onfile.m_posIn.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::IRSimHit" source="TVector3 m_momIn" version="[-1]"\
  targetClass="Belle2::IRSimHit" target="m_momIn" \
  include="TVector3.h" \
  code = "{ \
            m_momIn.SetXYZ(onfile.m_momIn.X(), onfile.m_momIn.Y(), onfile.m_momIn.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::IRSimHit" source="TVector3 m_posOut" version="[-1]"\
  targetClass="Belle2::IRSimHit" target="m_posOut" \
  include="TVector3.h" \
  code = "{ \
            m_posOut.SetXYZ(onfile.m_posOut.X(), onfile.m_posOut.Y(), onfile.m_posOut.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::IRSimHit" source="TVector3 m_momOut" version="[-1]"\
  targetClass="Belle2::IRSimHit" target="m_momOut" \
  include="TVector3.h" \
  code = "{ \
            m_momOut.SetXYZ(onfile.m_momOut.X(), onfile.m_momOut.Y(), onfile.m_momOut.Z()); \
          }"

#endif

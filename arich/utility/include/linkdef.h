#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::ARICHChannelHist+; // checksum=0xd05d461d, version=2
#pragma link C++ class Belle2::ARICHAerogelHist+; // checksum=0x27185530, version=2

#pragma read sourceClass="Belle2::ARICHAerogelHist" version="[-1]"  \
  source="std::map<Int_t, std::vector<TVector2>> m_verticesMap"     \
  targetClass="Belle2::ARICHAerogelHist"                            \
  target="m_verticesMap"                                            \
  include="TVector2.h"                                               \
  code="{for (const auto& [key, value] : onfile.m_verticesMap) {    \
           std::vector<ROOT::Math::XYVector> tmp;                   \
           for (const auto& vec : value)                            \
             tmp.push_back(ROOT::Math::XYVector(vec.X(), vec.Y())); \
           m_verticesMap[key] = tmp;                                \
         }                                                          \
        }"


#endif
